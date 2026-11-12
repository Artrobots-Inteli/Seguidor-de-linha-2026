#include <Arduino.h>

// Dois lados do robô: cada lado é um TB6612FNG com os canais A e B em paralelo
// (os dois motores do lado recebem o mesmo PWM e a mesma direção).
// Um encoder por lado mede a velocidade.

static const uint8_t PIN_STBY = 22;

static const uint8_t PIN_ESQ_PWM   = 18;
static const uint8_t PIN_ESQ_IN1   = 19;
static const uint8_t PIN_ESQ_IN2   = 23;
static const uint8_t PIN_ESQ_ENC_A = 13;
static const uint8_t PIN_ESQ_ENC_B = 4;

static const uint8_t PIN_DIR_PWM   = 21;
static const uint8_t PIN_DIR_IN1   = 5;
static const uint8_t PIN_DIR_IN2   = 15;
static const uint8_t PIN_DIR_ENC_A = 34;   // só entrada: precisa de pull-up externo
static const uint8_t PIN_DIR_ENC_B = 35;   // só entrada: precisa de pull-up externo

static const uint16_t ENCODER_PPR = 7;
static const uint16_t GEAR_RATIO  = 50;
static const float CONTAGENS_POR_VOLTA = (float)ENCODER_PPR * GEAR_RATIO * 4.0f;

static const uint8_t  PWM_BITS = 8;
static const uint32_t PWM_FREQ = 20000;
static const int      VELOCIDADE_INICIAL = 150;
static const uint32_t PERIODO_MS = 100;

struct Lado {
  uint8_t pwm;
  uint8_t in1;
  uint8_t in2;
  uint8_t encA;
  uint8_t encB;
  volatile int32_t cont;
  volatile uint8_t prev;
  int alvo;
  int32_t contAnterior;
};

Lado ESQ = {PIN_ESQ_PWM, PIN_ESQ_IN1, PIN_ESQ_IN2, PIN_ESQ_ENC_A, PIN_ESQ_ENC_B, 0, 0, 0, 0};
Lado DIR = {PIN_DIR_PWM, PIN_DIR_IN1, PIN_DIR_IN2, PIN_DIR_ENC_A, PIN_DIR_ENC_B, 0, 0, 0, 0};

static const int8_t TABELA[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0,
};

void IRAM_ATTR encoderUpdate(Lado &l) {
  uint8_t a = digitalRead(l.encA);
  uint8_t b = digitalRead(l.encB);
  uint8_t curr = (a << 1) | b;
  uint8_t idx = (l.prev << 2) | curr;
  l.cont += TABELA[idx];
  l.prev = curr;
}

void IRAM_ATTR isrEsq() { encoderUpdate(ESQ); }
void IRAM_ATTR isrDir() { encoderUpdate(DIR); }

void ladoSet(Lado &l, int velocidade) {
  if (velocidade > 255) velocidade = 255;
  if (velocidade < -255) velocidade = -255;

  if (velocidade > 0) {
    digitalWrite(l.in1, HIGH);
    digitalWrite(l.in2, LOW);
  } else if (velocidade < 0) {
    digitalWrite(l.in1, LOW);
    digitalWrite(l.in2, HIGH);
  } else {
    digitalWrite(l.in1, LOW);
    digitalWrite(l.in2, LOW);
  }

  ledcWrite(l.pwm, (uint32_t)abs(velocidade));
  l.alvo = velocidade;
}

void ladoSetup(Lado &l, bool pullup) {
  pinMode(l.in1, OUTPUT);
  pinMode(l.in2, OUTPUT);
  ledcAttach(l.pwm, PWM_FREQ, PWM_BITS);
  pinMode(l.encA, pullup ? INPUT_PULLUP : INPUT);
  pinMode(l.encB, pullup ? INPUT_PULLUP : INPUT);
  l.prev = (digitalRead(l.encA) << 1) | digitalRead(l.encB);
}

void frente(int v)  { ladoSet(ESQ, v);  ladoSet(DIR, v); }
void re(int v)      { ladoSet(ESQ, -v); ladoSet(DIR, -v); }
void parar()        { ladoSet(ESQ, 0);  ladoSet(DIR, 0); }
void girarEsq(int v){ ladoSet(ESQ, -v); ladoSet(DIR, v); }
void girarDir(int v){ ladoSet(ESQ, v);  ladoSet(DIR, -v); }

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, HIGH);

  ladoSetup(ESQ, true);
  ladoSetup(DIR, false);

  attachInterrupt(digitalPinToInterrupt(ESQ.encA), isrEsq, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ESQ.encB), isrEsq, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DIR.encA), isrDir, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DIR.encB), isrDir, CHANGE);

  Serial.println();
  Serial.println("=== Teste dois lados (TB6612FNG x2) ===");
  Serial.printf("ESQ PWM=%u IN1=%u IN2=%u ENC A=%u B=%u\n",
                ESQ.pwm, ESQ.in1, ESQ.in2, ESQ.encA, ESQ.encB);
  Serial.printf("DIR PWM=%u IN1=%u IN2=%u ENC A=%u B=%u | STBY=%u\n",
                DIR.pwm, DIR.in1, DIR.in2, DIR.encA, DIR.encB, PIN_STBY);
  Serial.printf("Encoder: %u PPR x reducao %u = %.0f contagens/volta\n",
                ENCODER_PPR, GEAR_RATIO, CONTAGENS_POR_VOLTA);
  Serial.println("Comandos: f=frente  r=re  e=gira esq  d=gira dir  s=parar  +/-=velocidade");
  Serial.println();

  frente(VELOCIDADE_INICIAL);
}

void loop() {
  static uint32_t ultimo = 0;
  static int velocidade = VELOCIDADE_INICIAL;

  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'f')      frente(abs(velocidade));
    else if (ch == 'r') re(abs(velocidade));
    else if (ch == 'e') girarEsq(abs(velocidade));
    else if (ch == 'd') girarDir(abs(velocidade));
    else if (ch == 's') parar();
    else if (ch == '+') { velocidade = constrain(velocidade + 25, 0, 255); }
    else if (ch == '-') { velocidade = constrain(velocidade - 25, 0, 255); }
    if (ch == '+' || ch == '-') {
      Serial.printf("velocidade = %d\n", velocidade);
    }
  }

  uint32_t agora = millis();
  if (agora - ultimo < PERIODO_MS) return;

  float dt = (agora - ultimo) / 1000.0f;
  ultimo = agora;

  noInterrupts();
  int32_t cEsq = ESQ.cont;
  int32_t cDir = DIR.cont;
  interrupts();

  int32_t dEsq = cEsq - ESQ.contAnterior;
  int32_t dDir = cDir - DIR.contAnterior;
  ESQ.contAnterior = cEsq;
  DIR.contAnterior = cDir;

  float rpmEsq = (dEsq / CONTAGENS_POR_VOLTA) * (60.0f / dt);
  float rpmDir = (dDir / CONTAGENS_POR_VOLTA) * (60.0f / dt);

  Serial.printf("ESQ alvo=%4d %8.2f RPM | DIR alvo=%4d %8.2f RPM\n",
                ESQ.alvo, rpmEsq, DIR.alvo, rpmDir);
}
