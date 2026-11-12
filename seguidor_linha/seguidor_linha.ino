// Seguidor de linha: QTR-8RC (8 IR) + dois lados com TB6612FNG.
// Controle PD sobre a posição da linha.
//
// Comandos pela serial (115200):
//   c  liga/desliga calibração (mova o sensor sobre branco e preto)
//   x  apaga a calibração
//   g  começa a seguir a linha
//   s  para os motores
//   +/-  ajusta a velocidade base

#include "config.h"
#include "motores.h"
#include "sensor_qtr.h"

enum class Modo : uint8_t { PARADO, CALIBRANDO, SEGUINDO };

SensorQtr sensor;
Modo modo = Modo::PARADO;
int velocidadeBase = VELOCIDADE_PADRAO;
int erroAnterior = 0;

const char *nomeModo(Modo m) {
  switch (m) {
    case Modo::CALIBRANDO: return "CALIBRANDO";
    case Modo::SEGUINDO:   return "SEGUINDO";
    default:               return "PARADO";
  }
}

void imprimirAjuda() {
  Serial.println();
  Serial.println("=== Seguidor de linha ===");
  Serial.println("c=calibrar  x=apagar calibracao  g=seguir  s=parar  +/-=velocidade");
  Serial.println();
}

void tratarSerial() {
  if (!Serial.available()) return;

  char c = Serial.read();
  switch (c) {
    case 'c':
      if (modo == Modo::CALIBRANDO) {
        modo = Modo::PARADO;
        Serial.println("-> calibracao encerrada");
      } else {
        sensor.resetarCalibracao();
        modo = Modo::CALIBRANDO;
        Serial.println("-> calibrando: passe o sensor sobre branco e preto");
      }
      break;

    case 'x':
      sensor.resetarCalibracao();
      Serial.println("-> calibracao apagada");
      break;

    case 'g':
      if (!sensor.calibrado()) {
        Serial.println("-> calibre antes (c)");
        break;
      }
      erroAnterior = 0;
      modo = Modo::SEGUINDO;
      Serial.println("-> seguindo");
      break;

    case 's':
      modo = Modo::PARADO;
      motoresParar();
      Serial.println("-> parado");
      break;

    case '+':
      velocidadeBase = constrain(velocidadeBase + 25, 0, 255);
      Serial.printf("velocidade base = %d\n", velocidadeBase);
      break;

    case '-':
      velocidadeBase = constrain(velocidadeBase - 25, 0, 255);
      Serial.printf("velocidade base = %d\n", velocidadeBase);
      break;
  }
}

void imprimirStatus(int erro) {
  Serial.printf("%-10s pos=%4d erro=%+5d | cont E=%8ld D=%8ld\n",
                nomeModo(modo), sensor.posicao(), erro,
                (long)motoresContagem(Roda::ESQUERDA),
                (long)motoresContagem(Roda::DIREITA));
}

void setup() {
  Serial.begin(115200);
  delay(500);

  motoresIniciar();
  motoresParar();
  imprimirAjuda();
}

void loop() {
  static uint32_t ultimoControle = 0;
  static uint32_t ultimoLog = 0;

  tratarSerial();

  uint32_t agora = millis();
  if (agora - ultimoControle < PERIODO_CONTROLE_MS) return;
  ultimoControle = agora;

  sensor.ler();
  if (modo == Modo::CALIBRANDO) sensor.atualizarCalibracao();

  int erro = sensor.posicao() - POSICAO_CENTRO;

  if (modo == Modo::SEGUINDO) {
    float ajuste = KP * erro + KD * (erro - erroAnterior);
    if (INVERTER_CORRECAO) ajuste = -ajuste;

    int correcao = (int)ajuste;
    motoresDefinir(velocidadeBase + correcao, velocidadeBase - correcao);
  } else {
    motoresParar();
  }
  erroAnterior = erro;

  if (agora - ultimoLog >= PERIODO_LOG_MS) {
    ultimoLog = agora;
    imprimirStatus(erro);
  }
}
