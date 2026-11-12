#pragma once

#include "config.h"

// Leitor do QTR-8RC: mede o tempo de descarga de cada canal (modo RC)
// e calcula a posição da linha entre 0 (sensor 0) e 7000 (sensor 7).
class SensorQtr {
 public:
  SensorQtr();

  void ler();                  // faz uma leitura dos 8 canais
  void atualizarCalibracao();  // expande min/max com a leitura atual
  void resetarCalibracao();
  bool calibrado() const;      // ao menos um canal com faixa suficiente
  int posicao();               // 0..7000, POSICAO_CENTRO = linha no meio

  uint16_t valor(uint8_t i) const { return valores_[i]; }
  uint16_t minimo(uint8_t i) const { return minimos_[i]; }
  uint16_t maximo(uint8_t i) const { return maximos_[i]; }

 private:
  uint16_t valores_[QTR_N];
  uint16_t minimos_[QTR_N];
  uint16_t maximos_[QTR_N];
  int ultimaPosicao_ = POSICAO_CENTRO;
};
