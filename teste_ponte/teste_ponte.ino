#include <Arduino.h>

static const uint8_t PIN_PWM  = 18;
static const uint8_t PIN_AIN1 = 19;
static const uint8_t PIN_AIN2 = 23;
static const uint8_t PIN_STBY = 22;
static const uint8_t PIN_LED  = 2;

static const uint8_t PIN_ENC_A = 13;
static const uint8_t PIN_ENC_B = 4;

static const uint8_t  PWM_BITS = 8;
static const uint32_t PWM_FREQ = 20000;
static const uint32_t TROCA_MS = 2000;

volatile int32_t encCount = 0;
volatile uint8_t encPrev = 0;

void IRAM_ATTR isrEncoder() {
  uint8_t a = digitalRead(PIN_ENC_A);
  uint8_t b = digitalRead(PIN_ENC_B);
  uint8_t curr = (a << 1) | b;
  uint8_t idx = (encPrev << 2) | curr;
  static const int8_t tabela[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0,
  };
  encCount += tabela[idx];
  encPrev = curr;
}

void motorSet(int velocidade) {
  if (velocidade > 255) velocidade = 255;
  if (velocidade < -255) velocidade = -255;

  if (velocidade > 0) {
    digitalWrite(PIN_AIN1, HIGH);
    digitalWrite(PIN_AIN2, LOW);
  } else if (velocidade < 0) {
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, HIGH);
  } else {
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, LOW);
  }

  ledcWrite(PIN_PWM, (uint32_t)abs(velocidade));
}

bool manual = false;
int mdir = 1;
int mduty = 255;

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, HIGH);

  ledcAttach(PIN_PWM, PWM_FREQ, PWM_BITS);

  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  encPrev = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), isrEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), isrEncoder, CHANGE);

  Serial.println();
  Serial.println("=== Teste da PONTE H (TB6612FNG) ===");
  Serial.printf("PWM=%u IN1=%u IN2=%u STBY=%u LED=%u\n",
                PIN_PWM, PIN_AIN1, PIN_AIN2, PIN_STBY, PIN_LED);
  Serial.println("AUTO: frente 100%% -> para -> re 100%% -> para (repete)");
  Serial.println("Teclas: f=frente r=re s=parar +/-=duty a=auto");
  Serial.println();
}

void loop() {
  int dir;
  int duty;

  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'f') { manual = true; mdir = 1; }
    else if (ch == 'r') { manual = true; mdir = -1; }
    else if (ch == 's') { manual = true; mdir = 0; }
    else if (ch == '+') { manual = true; mduty = min(255, mduty + 25); }
    else if (ch == '-') { manual = true; mduty = max(0, mduty - 25); }
    else if (ch == 'a') { manual = false; }
  }

  static uint32_t tAuto = 0;
  static uint8_t fase = 0;

  if (!manual) {
    uint32_t agora = millis();
    if (agora - tAuto >= TROCA_MS) { tAuto = agora; fase = (fase + 1) % 4; }
    switch (fase) {
      case 0:  dir = 1;  duty = 255; break;
      case 1:  dir = 0;  duty = 0;   break;
      case 2:  dir = -1; duty = 255; break;
      default: dir = 0;  duty = 0;   break;
    }
  } else {
    dir = mdir;
    duty = mduty;
  }

  motorSet(dir * duty);

  static uint32_t tPrint = 0;
  static uint32_t tLed = 0;
  uint32_t agora = millis();

  if (agora - tLed >= 250) {
    tLed = agora;
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
  }

  if (agora - tPrint >= 250) {
    tPrint = agora;
    noInterrupts();
    int32_t c = encCount;
    interrupts();
    Serial.printf("%-4s dir=%+d duty=%3d  cnt=%8ld\n",
                  manual ? "MAN" : "AUTO", dir, duty, (long)c);
  }
}
