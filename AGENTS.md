# SEG_LINHA — Seguidor de Linha (ESP32)

## Ambiente
- Toolchain: `arduino-cli` 1.5.1 (`~/.local/bin/arduino-cli`)
- Core: `esp32:esp32@3.3.12`
- Placa / FQBN: `esp32:esp32:esp32` (ESP32 Dev Module / WROOM-32, CP2102, 4MB)
- Monitor serial: 115200 baud

## Uso (recomendado)
Passo a passo completo em `README.md`.
```bash
./dev.sh go                # doctor + build + grava + monitor (1 passo)
./dev.sh doctor            # checa toolchain, core, porta e permissões
./dev.sh build             # compila o sketch da raiz (SEG_LINHA.ino)
./dev.sh flash             # compila + grava (auto-detecta a porta)
./dev.sh monitor           # monitor serial
./dev.sh run               # build + flash + monitor
./dev.sh watch             # recompila a cada alteração (.ino .h .hpp .c .cpp)
./dev.sh ports             # lista portas
./dev.sh clean             # limpa cache
./dev.sh setup             # (re)instala o core esp32:esp32
```

Seleção de sketch (subpasta) — o último escolhido fica salvo em `.devstate`:
```bash
./dev.sh -s teste_qtr go        # compila+grava+monitor do teste_qtr
./dev.sh -s . go                # volta para o sketch da raiz
SKETCH=teste_qtr ./dev.sh flash # equivalente via variável
```

Atalhos legados: `./build.sh`, `./flash.sh`, `./monitor.sh` (delegam ao `dev.sh`).

Config via variáveis ou arquivo `.devrc` (veja `.devrc.example`):
`SKETCH`, `FQBN`, `BAUD`, `TIMESTAMP` (1/0), `PORT`.

## Sensor de linha: Pololu QTR-8RC
- 8 pares IR, passo 0.375" (9.525 mm); altura ótima ~3 mm, máx. ~9.5 mm.
- Saída **RC (não é ADC)**: carrega o pino (`OUTPUT`+`HIGH`, 10 µs), solta (`INPUT`) e
  mede em µs o tempo até cair para LOW. **Branco = valor baixo; preto/linha = valor alto.**
- Os 8 canais são lidos em paralelo. Timeout (valor de "preto máximo") = 2500 µs.
- Alimentar em **3.3 V** e **curto-circuitar os pinos `3.3V BYPASS`** (aumenta o brilho).
  Nunca fazer o curto em 5 V. Não usar 5 V nas saídas (GPIO do ESP32 não tolera 5 V).
- Pino `LEDON` desconectado → LEDs sempre ligados.
- Fiação usada: `OUT1..OUT8 → GPIO 32, 33, 25, 26, 27, 14, 16, 17`; `VCC→3.3V`; `GND→GND`.
- Sketch de teste: `teste_qtr/` (driver RC próprio, imprimível: raw/min/max/mapa/posição).

## Motor N20 com encoder (driver TB6612FNG)
- ESP32 **não** liga o motor direto — precisa de ponte H. Motor em fonte própria (~6 V),
  GND comum com a ESP32.
- Fiação usada: `PWMA→18`, `AIN1→19`, `AIN2→23`, `STBY→22`;
  encoder `A→13`, `B→4`; encoder `VCC→3.3V`; `GND→GND`.
- Encoder lido por interrupção (quadratura x4). Ajustar `ENCODER_PPR` e `GEAR_RATIO`
  no sketch conforme o N20. PWM via LEDC (20 kHz, 8 bits).
- Sketch de teste: `teste_motores/` (velocidade fixa; serial `f`/`r`/`s`/`+`/`-`).

## Monitor serial
`arduino-cli monitor` não solta o auto-reset do ESP32 nesta placa (DTR/RTS).
Por isso o `dev.sh monitor/run` usa `serial_monitor.py` (só stdlib do Python),
que força o reset e libera o chip para executar o app. `TIMESTAMP=0` desliga a hora.

## Comandos crus (equivalente)
```bash
arduino-cli compile --fqbn esp32:esp32:esp32 .
arduino-cli upload  -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 .
```

## Permissões seriais (feito)
- Usuário já está no grupo `dialout`.
- `/etc/udev/rules.d/99-espressif.rules` marca `ID_MM_DEVICE_IGNORE=1` para
  VID `10c4` (CP210x), `1a86` (CH34x), `303a` (USB nativo Espressif), `0403` (FTDI),
  evitando o ModemManager travar a porta.
- O `dev.sh` entra no grupo `dialout` automaticamente via `sg` enquanto a sessão
  não tiver relogado.

## Notas
- Drivers USB-série (CP210x/CH340/FTDI/CDC) já vêm no kernel Linux; nada a instalar.
- USB nativo (S2/S3/C3) não precisa de driver.
