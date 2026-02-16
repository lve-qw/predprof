#include <SPI.h>
#include <Wire.h>
#include <stdio.h>
#include <RF24.h>
#include <EEPROM.h>

RF24 radio(9, 10);
const byte address[6] = "00152";

const int fputchar(const char ch, FILE *stream) {
  Serial.write(ch);
  return ch;
}
static FILE *sstream = fdevopen(fputchar, NULL);

typedef struct {
  int xpos;
  int ypos;
  int uid;
  int state;
} payload;
payload res;

void setup() {
  stdout = sstream;
  Serial.begin(115200);

  radio.begin();
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();

  pinMode(8, INPUT_PULLUP);

  loadCalibrationPoints();

  Serial.println("Ожидание");
}

bool ok = false;
int sendAttempt = 0;

int mode = 0;
const int stepDelay = 1000;
unsigned long prevMillis = 0;
int prevMode = 0;

int posesx[36] = {};
int posesy[36] = {};

void loop() {
  if (mode == 0) {
    led(1);
    while (digitalRead(8)) {
      delay(10);
    }
    delay(150);
    prevMillis = millis();
    mode = 1;
    return;
  }

  if (millis() - prevMillis >= stepDelay) {
    mode++;
    prevMillis = millis();
  }

  if (mode >= 1 && mode <= 36) {
    mv(posesx[mode - 1], posesy[mode - 1]);
  } else if (mode > 36) {
    mv(0, 0);
    mode = 0;
    Serial.println("возврат в начальное положение");
    return;
  }

  sendto();

  if (prevMode != mode) {
    prevMode = mode;

    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("UID: K7M9P2XQ\n");
    printf("xpos: %4d  ypos: %4d\n", res.xpos, res.ypos);

    if (mode == 0) {
      printf("Возврат к начальному состоянию\n");
    } else if (mode <= 9) {
      printf("Горизонтальный скан\n");
    } else if (mode <= 18) {
      printf("Вертикальный скан\n");
    } else if (mode <= 27) {
      printf("Диагональный скан 1\n");
    } else {
      printf("Диагональный скан 2\n");
    }
  }
}

void sendto() {
  radio.stopListening();
  ok = radio.write(&res, sizeof(res));
  
  radio.startListening();
  
  unsigned long startWait = millis();
  bool replyReceived = false;

  while (millis() - startWait < 500) {
    if (radio.available()) {
      payload reply;
      radio.read(&reply, sizeof(reply));
      replyReceived = true;
      break;
    }
    delay(5);
  }

  if (!replyReceived) {
    sendAttempt++;
    if (sendAttempt >= 10) {
      printf("Connection lost. No Reply. (attempt: %d)\n", sendAttempt);
    }
    if (sendAttempt >= 500) {
      printf("FAIL! Resetting...\n");
      sendAttempt = 0;
      mode = 0;
    }
    radio.stopListening(); 
  } else {
    sendAttempt = 0;
    radio.stopListening();
  }
  ok = false;
}

void loadCalibrationPoints() {
  const int START_ADDR_X = 0;   
  const int START_ADDR_Y = 72;  

  for (int i = 0; i < 36; i++) {
    int tempX; 
    int tempY;  

    EEPROM.get(START_ADDR_X + i * 2, tempX);  
    EEPROM.get(START_ADDR_Y + i * 2, tempY);  

    posesx[i] = tempX;
    posesy[i] = tempY;
  }

  Serial.println("Калибровочные точки загружены из EEPROM.");
}

void mv(int x, int y) {
  res.xpos = x;
  res.ypos = y;
}
void led(int value) {
  res.state = value;
}
