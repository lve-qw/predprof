[cite_start]#include <SPI.h> [cite: 15]
#include <Wire.h>
#include <stdio.h>
#include <RF24.h>
#include <EEPROM.h>

RF24 radio(9, 10);
const byte address[6] = "00152";

[cite_start]const int fputchar(const char ch, FILE *stream) { [cite: 16]
  Serial.write(ch);
  return ch;
[cite_start]} [cite: 17]
static FILE *sstream = fdevopen(fputchar, NULL);

typedef struct {
  [cite_start]int xpos; [cite: 18]
  int ypos;
  [cite_start]int uid; [cite: 19]
  int state;
} payload;
payload res;

[cite_start]void setup() { [cite: 20]
  stdout = sstream;
  Serial.begin(115200);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();

  pinMode(8, INPUT_PULLUP);
  [cite_start]loadCalibrationPoints(); [cite: 21]

  Serial.println("Ожидание");
}

bool ok = false;
int sendAttempt = 0;

[cite_start]int mode = 0; [cite: 22]
const int stepDelay = 1000;
[cite_start]unsigned long prevMillis = 0; [cite: 23]
int prevMode = 0;

int posesx[36] = {};
[cite_start]int posesy[36] = {}; [cite: 24]

void loop() {
  if (mode == 0) {
    [cite_start]led(1); [cite: 25]
    while (digitalRead(8)) {
      delay(10);
    }
    delay(150);
    [cite_start]prevMillis = millis(); [cite: 26]
    mode = 1;
    return;
  }

  if (millis() - prevMillis >= stepDelay) {
    mode++;
    [cite_start]prevMillis = millis(); [cite: 27]
  }

  if (mode >= 1 && mode <= 36) {
    mv(posesx[mode - 1], posesy[mode - 1]);
  [cite_start]} else if (mode > 36) { [cite: 28]
    mv(0, 0);
    mode = 0;
    Serial.println("возврат в начальное положение");
    return;
  [cite_start]} [cite: 29]

  sendto();

  if (prevMode != mode) {
    prevMode = mode;

    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("UID: K7M9P2XQ\n");
    [cite_start]printf("xpos: %4d  ypos: %4d\n", res.xpos, res.ypos); [cite: 30]

    if (mode == 0) {
      printf("Возврат к начальному состоянию\n");
    [cite_start]} else if (mode <= 9) { [cite: 31]
      printf("Горизонтальный скан\n");
    [cite_start]} else if (mode <= 18) { [cite: 32]
      printf("Вертикальный скан\n");
    [cite_start]} else if (mode <= 27) { [cite: 33]
      printf("Диагональный скан 1\n");
    } else {
      printf("Диагональный скан 2\n");
    [cite_start]} [cite: 34, 35]
  }
}

void sendto() {
  ok = radio.write(&res, sizeof(res));
  [cite_start]while (!ok) { [cite: 36]
    sendAttempt++;
    ok = radio.write(&res, sizeof(res));
    [cite_start]if (sendAttempt >= 10) { [cite: 37]
      printf("Connection lost. Reconnecting... (attempt: %d)\n", sendAttempt);
    [cite_start]} [cite: 38]
    if (sendAttempt >= 500) {
      printf("FAIL! Resetting...\n");
      sendAttempt = 0;
      [cite_start]mode = 0; [cite: 39]
      break;
    }
    delay(10);
  }
  sendAttempt = 0;
  ok = false;
[cite_start]} [cite: 40]

void loadCalibrationPoints() {
  const int START_ADDR_X = 0;   
  const int START_ADDR_Y = 72;
  [cite_start]for (int i = 0; i < 36; i++) { [cite: 41]
    int tempX; 
    int tempY;
    [cite_start]EEPROM.get(START_ADDR_X + i * 2, tempX); [cite: 42]
    EEPROM.get(START_ADDR_Y + i * 2, tempY);  

    posesx[i] = tempX;
    posesy[i] = tempY;
  [cite_start]} [cite: 43]

  Serial.println("Калибровочные точки загружены из EEPROM.");
}

void mv(int x, int y) {
  res.xpos = x;
  res.ypos = y;
[cite_start]} [cite: 44]

void led(int value) {
  res.state = value;
}
