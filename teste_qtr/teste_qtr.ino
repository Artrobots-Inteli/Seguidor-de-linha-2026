#include <Arduino.h>

static const uint8_t NUM_SENSORES = 8;
static const uint8_t PINS[NUM_SENSORES] = {32, 33, 25, 26, 27, 14, 16, 17};
static const uint16_t TIMEOUT_US = 2500;
static const uint32_t PERIODO_MS = 100;

static uint16_t valores[NUM_SENSORES];
static uint16_t minimos[NUM_SENSORES];
static uint16_t maximos[NUM_SENSORES];
static int ultimaPos = 3500;

void lerSensores() {
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    pinMode(PINS[i], OUTPUT);
    digitalWrite(PINS[i], HIGH);
  }

  delayMicroseconds(10);

  noInterrupts();
  uint32_t t0 = micros();
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    pinMode(PINS[i], INPUT);
  }
  interrupts();

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    valores[i] = TIMEOUT_US;
  }

  while (true) {
    noInterrupts();
    uint32_t t = micros() - t0;
    if (t >= TIMEOUT_US) {
      interrupts();
      break;
    }
    for (uint8_t i = 0; i < NUM_SENSORES; i++) {
      if (valores[i] == TIMEOUT_US && digitalRead(PINS[i]) == LOW) {
        valores[i] = (uint16_t)t;
      }
    }
    interrupts();
  }
}

void atualizarCalibracao() {
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    if (valores[i] < minimos[i]) minimos[i] = valores[i];
    if (valores[i] > maximos[i]) maximos[i] = valores[i];
  }
}

int calcularPosicao() {
  uint32_t somaPesos = 0;
  uint32_t somaIndices = 0;

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    int32_t faixa = (int32_t)maximos[i] - minimos[i];
    if (faixa < 150) continue;

    int32_t norm = ((int32_t)valores[i] - minimos[i]) * 1000 / faixa;
    if (norm < 0) norm = 0;
    if (norm > 1000) norm = 1000;

    if (norm > 50) {
      somaPesos += norm;
      somaIndices += (uint32_t)norm * i * 1000;
    }
  }

  if (somaPesos == 0) return ultimaPos;

  ultimaPos = somaIndices / somaPesos;
  return ultimaPos;
}

void imprimir() {
  Serial.print("raw:");
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    Serial.printf(" %5u", valores[i]);
  }
  Serial.println();

  Serial.print("min:");
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    Serial.printf(" %5u", minimos[i]);
  }
  Serial.println();

  Serial.print("max:");
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    Serial.printf(" %5u", maximos[i]);
  }
  Serial.println();

  Serial.print("map:");
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    int32_t faixa = (int32_t)maximos[i] - minimos[i];
    int32_t limiar = minimos[i] + faixa / 2;
    bool naLinha = (faixa >= 150) && ((int32_t)valores[i] > limiar);
    Serial.print(naLinha ? "  # " : "  . ");
  }
  Serial.println();

  Serial.printf("pos: %d\n\n", calcularPosicao());
}

void setup() {
  Serial.begin(115200);
  delay(500);

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    minimos[i] = TIMEOUT_US;
    maximos[i] = 0;
  }

  Serial.println();
  Serial.println("=== Teste QTR-8RC ===");
  Serial.printf("Pinos: ");
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    Serial.printf("%u ", PINS[i]);
  }
  Serial.println();
  Serial.println("Branco = valor baixo | Preto (linha) = valor alto");
  Serial.println("Passe o sensor sobre a linha para calibrar min/max.\n");
}

void loop() {
  static uint32_t ultimo = 0;

  if (millis() - ultimo < PERIODO_MS) return;
  ultimo = millis();

  lerSensores();
  atualizarCalibracao();
  imprimir();
}
