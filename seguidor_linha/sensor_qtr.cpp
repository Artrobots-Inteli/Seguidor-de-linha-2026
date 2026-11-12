#include "sensor_qtr.h"

SensorQtr::SensorQtr() {
  resetarCalibracao();
}

void SensorQtr::ler() {
  // Carrega os capacitores das saídas
  for (uint8_t i = 0; i < QTR_N; i++) {
    pinMode(QTR_PINOS[i], OUTPUT);
    digitalWrite(QTR_PINOS[i], HIGH);
  }
  delayMicroseconds(10);

  // Solta as saídas e marca o tempo de início da descarga
  noInterrupts();
  uint32_t t0 = micros();
  for (uint8_t i = 0; i < QTR_N; i++) {
    pinMode(QTR_PINOS[i], INPUT);
  }
  interrupts();

  for (uint8_t i = 0; i < QTR_N; i++) {
    valores_[i] = QTR_TIMEOUT_US;
  }

  // Registra quando cada canal cai para LOW (ou estoura o timeout)
  while (true) {
    noInterrupts();
    uint32_t t = micros() - t0;
    if (t >= QTR_TIMEOUT_US) {
      interrupts();
      break;
    }
    for (uint8_t i = 0; i < QTR_N; i++) {
      if (valores_[i] == QTR_TIMEOUT_US && digitalRead(QTR_PINOS[i]) == LOW) {
        valores_[i] = (uint16_t)t;
      }
    }
    interrupts();
  }
}

void SensorQtr::atualizarCalibracao() {
  for (uint8_t i = 0; i < QTR_N; i++) {
    if (valores_[i] < minimos_[i]) minimos_[i] = valores_[i];
    if (valores_[i] > maximos_[i]) maximos_[i] = valores_[i];
  }
}

void SensorQtr::resetarCalibracao() {
  for (uint8_t i = 0; i < QTR_N; i++) {
    minimos_[i] = QTR_TIMEOUT_US;
    maximos_[i] = 0;
  }
}

bool SensorQtr::calibrado() const {
  for (uint8_t i = 0; i < QTR_N; i++) {
    if ((int32_t)maximos_[i] - minimos_[i] >= QTR_FAIXA_MIN) return true;
  }
  return false;
}

int SensorQtr::posicao() {
  uint32_t somaPesos = 0;
  uint32_t somaPosicoes = 0;

  for (uint8_t i = 0; i < QTR_N; i++) {
    int32_t faixa = (int32_t)maximos_[i] - minimos_[i];
    if (faixa < QTR_FAIXA_MIN) continue;

    // Normaliza 0 (branco) .. 1000 (preto)
    int32_t norm = ((int32_t)valores_[i] - minimos_[i]) * 1000 / faixa;
    if (norm < 0) norm = 0;
    if (norm > 1000) norm = 1000;

    if (norm > 50) {
      somaPesos += norm;
      somaPosicoes += (uint32_t)norm * i * 1000;
    }
  }

  // Nenhum sensor vê a linha: mantém a última posição conhecida
  if (somaPesos == 0) return ultimaPosicao_;

  ultimaPosicao_ = somaPosicoes / somaPesos;
  return ultimaPosicao_;
}
