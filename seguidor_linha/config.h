#pragma once

#include <Arduino.h>

// ===== Sensor QTR-8RC (saída RC, alimentado em 3.3 V) =====
constexpr uint8_t QTR_N = 8;
constexpr uint8_t QTR_PINOS[QTR_N] = {32, 33, 25, 26, 27, 14, 16, 17};
constexpr uint16_t QTR_TIMEOUT_US = 2500;
constexpr int QTR_FAIXA_MIN = 150;  // faixa min/max (us) mínima para um canal contar

// Posição com o sensor centralizado na linha (sensores 0..7 -> 0..7000)
constexpr int POSICAO_CENTRO = (QTR_N - 1) * 500;

// ===== Motores: dois lados, cada um com um TB6612FNG (canais A e B em paralelo) =====
constexpr uint8_t PIN_STBY = 22;

constexpr uint8_t ESQ_PWM   = 18;
constexpr uint8_t ESQ_IN1   = 19;
constexpr uint8_t ESQ_IN2   = 23;
constexpr uint8_t ESQ_ENC_A = 13;
constexpr uint8_t ESQ_ENC_B = 4;   // pull-up interno

constexpr uint8_t DIR_PWM   = 21;
constexpr uint8_t DIR_IN1   = 5;
constexpr uint8_t DIR_IN2   = 15;
constexpr uint8_t DIR_ENC_A = 34;  // só entrada: precisa de pull-up externo
constexpr uint8_t DIR_ENC_B = 35;  // só entrada: precisa de pull-up externo

constexpr uint8_t  PWM_BITS = 8;
constexpr uint32_t PWM_FREQ = 20000;

constexpr uint16_t ENCODER_PPR = 7;
constexpr uint16_t GEAR_RATIO  = 50;

// ===== Controle de linha (PD) =====
constexpr int      VELOCIDADE_PADRAO = 150;
constexpr float    KP = 0.06f;        // ganho proporcional (erro em unidades de 0..7000)
constexpr float    KD = 0.05f;        // ganho derivativo
constexpr bool     INVERTER_CORRECAO = false;  // troque para true se o robô virar para o lado errado

// ===== Temporização =====
constexpr uint32_t PERIODO_CONTROLE_MS = 10;
constexpr uint32_t PERIODO_LOG_MS      = 200;
