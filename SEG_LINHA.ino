#include <Arduino.h>
#include <esp_mac.h>

static const uint8_t LED_PIN = 2;
static const uint32_t INTERVALO_MS = 2000;

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  uint8_t mac[6] = {0};
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  Serial.println();
  Serial.println("=== Teste ESP32 ===");
  Serial.printf("Chip : %s rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("Cores: %d @ %u MHz\n", ESP.getChipCores(), (unsigned)getCpuFrequencyMhz());
  Serial.printf("Flash: %u bytes\n", (unsigned)ESP.getFlashChipSize());
  Serial.printf("MAC  : %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println("Digite algo e tecle Enter (teste de RX)...");
}

void loop() {
  static uint32_t ultimo = 0;
  static uint32_t piscadas = 0;
  static uint8_t nivel = LOW;

  if (millis() - ultimo >= INTERVALO_MS) {
    ultimo = millis();
    nivel = (nivel == LOW) ? HIGH : LOW;
    digitalWrite(LED_PIN, nivel);

    if (nivel == HIGH) {
      piscadas++;
      Serial.printf("[%6lus] LED ON  piscadas=%lu\n",
                    millis() / 1000, (unsigned long)piscadas);
    }
  }

  if (Serial.available()) {
    String linha = Serial.readStringUntil('\n');
    linha.trim();
    if (linha.length() > 0) {
      Serial.printf("Recebido: \"%s\" (%u caracteres)\n", linha.c_str(), linha.length());
    }
  }
}
