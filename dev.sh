#!/usr/bin/env bash
set -euo pipefail

SKETCH_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FQBN="${FQBN:-esp32:esp32:esp32}"
BAUD="${BAUD:-115200}"
TIMESTAMP="${TIMESTAMP:-1}"
PORT="${PORT:-}"
SKETCH="${SKETCH:-.}"

STATE_FILE="$SKETCH_DIR/.devstate"
MONITOR_PY="$SKETCH_DIR/serial_monitor.py"

[[ -f "$STATE_FILE" ]] && source "$STATE_FILE"
[[ -f "$SKETCH_DIR/.devrc" ]] && source "$SKETCH_DIR/.devrc"

export SKETCH
TARGET_DIR="$SKETCH_DIR/$SKETCH"

usage() {
  cat <<EOF
Uso: ./dev.sh [-s sketch] <comando> [porta]

Comandos:
  go               Doctor + build + grava + monitor (tudo em 1 passo)
  build            Compila o sketch
  flash [porta]    Compila e grava (auto-detecta a porta)
  monitor [porta]  Abre o monitor serial (${BAUD} baud)
  run [porta]      build + flash + monitor
  watch            Recompila a cada alteração em (.ino .h .hpp .c .cpp)
  ports            Lista as portas seriais detectadas
  clean            Limpa o cache de compilação
  setup            (Re)instala o core ${FQBN%:*}
  doctor           Diagnostica ambiente, core, porta e permissões
  help             Mostra esta ajuda

  -s, --sketch NOME  Seleciona a pasta do sketch (default "." = raiz)
                     ex.: ./dev.sh -s teste_qtr go
                     O último sketch usado é lembrado em .devstate.

Variáveis (ou crie um .devrc):
  SKETCH=$SKETCH
  FQBN=$FQBN
  BAUD=$BAUD
  TIMESTAMP=$TIMESTAMP   (1 = mostra hora em cada linha)
  PORT=${PORT:-<auto>}
EOF
}

die() { echo "erro: $*" >&2; exit 1; }

detect_port() {
  if [[ -n "${PORT:-}" ]]; then echo "$PORT"; return; fi
  arduino-cli board list 2>/dev/null \
    | awk '/\/dev\/tty(USB|ACM)[0-9]+/{print $1; exit}'
}

ensure_serial_group() {
  [[ -n "${DEV_SG_REEXEC:-}" ]] && return 0
  id -nG | tr ' ' '\n' | grep -qx dialout && return 0
  if getent group dialout | cut -d: -f4 | tr ',' '\n' | grep -qx "$USER"; then
    export DEV_SG_REEXEC=1
    echo ">> entrando no grupo dialout via sg (sem precisar relogar)"
    exec sg dialout -c "$(printf '%q ' "$0" "${ORIG_ARGS[@]}")"
  fi
  echo ">> aviso: você não está no grupo dialout; acesso à serial pode falhar" >&2
}

require_port() {
  local p; p="$(detect_port)"
  [[ -n "$p" ]] || die "nenhuma porta serial detectada (plugue o ESP32 ou passe a porta)"
  echo "$p"
}

cmd_build() {
  [[ -d "$TARGET_DIR" ]] || die "sketch não encontrado: $TARGET_DIR"
  echo ">> compilando $SKETCH ($FQBN)"
  arduino-cli compile --fqbn "$FQBN" "$TARGET_DIR"
}

cmd_flash() {
  ensure_serial_group "$@"
  local p; p="$(require_port)"
  echo ">> gravando $SKETCH em $p ($FQBN)"
  arduino-cli compile --fqbn "$FQBN" "$TARGET_DIR"
  arduino-cli upload -p "$p" --fqbn "$FQBN" "$TARGET_DIR"
  echo ">> upload concluído"
}

cmd_monitor() {
  ensure_serial_group "$@"
  local p; p="$(require_port)"
  local extra=(); [[ "$TIMESTAMP" == "1" ]] && extra=(-t)
  exec python3 "$MONITOR_PY" "$p" -b "$BAUD" "${extra[@]}"
}

cmd_run() {
  ensure_serial_group "$@"
  local p; p="$(require_port)"
  cmd_build
  echo ">> gravando em $p ($FQBN)"
  arduino-cli upload -p "$p" --fqbn "$FQBN" "$TARGET_DIR"
  echo ">> upload concluído, abrindo monitor"
  local extra=(); [[ "$TIMESTAMP" == "1" ]] && extra=(-t)
  exec python3 "$MONITOR_PY" "$p" -b "$BAUD" "${extra[@]}"
}

cmd_go() {
  cmd_doctor
  echo
  cmd_run
}

cmd_watch() {
  command -v inotifywait >/dev/null || die "inotifywait não encontrado (instale inotify-tools)"
  echo ">> watch ativo em $TARGET_DIR (Ctrl+C para sair)"
  cmd_build || true
  while inotifywait -qq -r -e close_write,move,create \
      --include '\.(ino|h|hpp|c|cpp)$' "$TARGET_DIR"; do
    echo ">> mudança detectada $(date +%T), recompilando"
    cmd_build || echo ">> build falhou, aguardando..."
  done
}

cmd_ports() {
  arduino-cli board list
}

cmd_clean() {
  echo ">> limpando artefatos de $SKETCH"
  rm -rf "$TARGET_DIR/build"
  arduino-cli cache clean 2>/dev/null || true
}

cmd_setup() {
  local core="${FQBN%:*}"
  echo ">> atualizando índice de placas"
  arduino-cli core update-index
  echo ">> instalando core $core"
  arduino-cli core install "$core"
}

cmd_doctor() {
  echo "arduino-cli : $(arduino-cli version 2>/dev/null || echo AUSENTE)"
  echo "sketch      : $SKETCH"
  echo "FQBN        : $FQBN"
  echo "BAUD        : $BAUD"
  local core
  core="$(arduino-cli core list 2>/dev/null | awk -v id="${FQBN%:*}" '$1==id {print $1"@"$2}')"
  echo "core        : ${core:-NÃO INSTALADO}"
  echo "dialout     : $(id -nG | grep -qw dialout && echo OK || echo 'pendente (sg/resolução automática ativa)')"
  echo "udev rule   : $([[ -f /etc/udev/rules.d/99-espressif.rules ]] && echo OK || echo AUSENTE)"
  local p; p="$(detect_port)"
  echo "porta       : ${p:-NENHUMA}"
  if [[ -n "$p" ]]; then
    if id -nG | grep -qw dialout || getent group dialout | cut -d: -f4 | tr ',' '\n' | grep -qx "$USER"; then
      echo "acesso      : OK"
    else
      echo "acesso      : SEM GRUPO dialout"
    fi
  fi
}

main() {
  ORIG_ARGS=("$@")

  local args=()
  local sketch_from_cli=""
  while [[ $# -gt 0 ]]; do
    case "$1" in
      -s|--sketch) SKETCH="$2"; sketch_from_cli=1; shift 2 ;;
      *) args+=("$1"); shift ;;
    esac
  done
  set -- "${args[@]}"
  export SKETCH
  TARGET_DIR="$SKETCH_DIR/$SKETCH"

  if [[ -n "$sketch_from_cli" ]]; then
    [[ -d "$TARGET_DIR" ]] || die "sketch não encontrado: $TARGET_DIR"
    printf 'SKETCH=%q\n' "$SKETCH" > "$STATE_FILE"
  fi

  local cmd="${1:-help}"; shift || true
  case "$cmd" in
    go)      [[ ${1:-} ]] && PORT="$1"; cmd_go ;;
    build)   cmd_build "$@" ;;
    flash)   [[ ${1:-} ]] && PORT="$1"; cmd_flash ;;
    monitor) [[ ${1:-} ]] && PORT="$1"; cmd_monitor ;;
    run)     [[ ${1:-} ]] && PORT="$1"; cmd_run ;;
    watch)   cmd_watch ;;
    ports)   cmd_ports ;;
    clean)   cmd_clean ;;
    setup)   cmd_setup ;;
    doctor)  cmd_doctor ;;
    help|-h|--help) usage ;;
    *) usage; exit 1 ;;
  esac
}

main "$@"
