#include <Arduino.h>

// Diagnóstico das saídas do QTR-8RC clone: lê cada pino como digital e analógico.
// Se o analógico variar com a superfície, a saída é analógica; se só o digital muda, é digital.
static const uint8_t NUM_SENSORES = 8;
static const uint8_t PINS[NUM_SENSORES] = {32, 33, 25, 26, 27, 14, 16, 17};

void setup() {
  Serial.begin(115200);
  delay(500);

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    pinMode(PINS[i], INPUT);
  }

  Serial.println();
  Serial.println("=== Diagnostico saidas QTR-8RC ===");
}

void loop() {
  Serial.print("dig:");
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    Serial.printf(" %d", digitalRead(PINS[i]));
  }

  Serial.print(" | adc:");
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    Serial.printf(" %4d", analogRead(PINS[i]));
  }
  Serial.println();

  delay(200);
}
