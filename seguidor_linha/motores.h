#pragma once

#include "config.h"

enum class Roda : uint8_t { ESQUERDA, DIREITA };

// Inicializa as duas pontes H, os PWMs e os encoders.
void motoresIniciar();

// Velocidades de -255 (ré total) a 255 (frente total) para cada lado.
void motoresDefinir(int esquerda, int direita);
void motoresParar();

// Contagens acumuladas do encoder de cada lado.
int32_t motoresContagem(Roda roda);
