#include "motores.h"

namespace {

// Tabela de quadratura x4: índice = (estado anterior << 2) | estado atual
const int8_t TABELA_QUADRATURA[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0,
};

struct Lado {
  uint8_t pwm;
  uint8_t in1;
  uint8_t in2;
  uint8_t encA;
  uint8_t encB;
  bool pullupInterno;
  volatile int32_t contagens;
  volatile uint8_t estadoAnterior;
};

Lado esquerda = {ESQ_PWM, ESQ_IN1, ESQ_IN2, ESQ_ENC_A, ESQ_ENC_B, true, 0, 0};
Lado direita  = {DIR_PWM, DIR_IN1, DIR_IN2, DIR_ENC_A, DIR_ENC_B, false, 0, 0};

void IRAM_ATTR atualizarEncoder(Lado &lado) {
  uint8_t a = digitalRead(lado.encA);
  uint8_t b = digitalRead(lado.encB);
  uint8_t atual = (a << 1) | b;
  uint8_t indice = (lado.estadoAnterior << 2) | atual;
  lado.contagens += TABELA_QUADRATURA[indice];
  lado.estadoAnterior = atual;
}

void IRAM_ATTR isrEsquerda() { atualizarEncoder(esquerda); }
void IRAM_ATTR isrDireita()  { atualizarEncoder(direita); }

void iniciarLado(Lado &lado) {
  pinMode(lado.in1, OUTPUT);
  pinMode(lado.in2, OUTPUT);
  ledcAttach(lado.pwm, PWM_FREQ, PWM_BITS);

  pinMode(lado.encA, lado.pullupInterno ? INPUT_PULLUP : INPUT);
  pinMode(lado.encB, lado.pullupInterno ? INPUT_PULLUP : INPUT);
  lado.estadoAnterior = (digitalRead(lado.encA) << 1) | digitalRead(lado.encB);
}

void aplicar(Lado &lado, int velocidade) {
  velocidade = constrain(velocidade, -255, 255);

  if (velocidade > 0) {
    digitalWrite(lado.in1, HIGH);
    digitalWrite(lado.in2, LOW);
  } else if (velocidade < 0) {
    digitalWrite(lado.in1, LOW);
    digitalWrite(lado.in2, HIGH);
  } else {
    digitalWrite(lado.in1, LOW);
    digitalWrite(lado.in2, LOW);
  }

  ledcWrite(lado.pwm, (uint32_t)abs(velocidade));
}

}  // namespace

void motoresIniciar() {
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, HIGH);

  iniciarLado(esquerda);
  iniciarLado(direita);

  attachInterrupt(digitalPinToInterrupt(esquerda.encA), isrEsquerda, CHANGE);
  attachInterrupt(digitalPinToInterrupt(esquerda.encB), isrEsquerda, CHANGE);
  attachInterrupt(digitalPinToInterrupt(direita.encA), isrDireita, CHANGE);
  attachInterrupt(digitalPinToInterrupt(direita.encB), isrDireita, CHANGE);
}

void motoresDefinir(int esquerdaVel, int direitaVel) {
  aplicar(esquerda, esquerdaVel);
  aplicar(direita, direitaVel);
}

void motoresParar() {
  motoresDefinir(0, 0);
}

int32_t motoresContagem(Roda roda) {
  Lado &lado = (roda == Roda::ESQUERDA) ? esquerda : direita;

  noInterrupts();
  int32_t valor = lado.contagens;
  interrupts();
  return valor;
}
