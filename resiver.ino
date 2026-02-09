#include <Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <RF24.h>
#include <stdio.h>

#define laser_beam A3
#define btn A0
#define CE 5
#define CSN 6
#define servox 2
#define servoy 3

Servo myservo;
[cite_start]Servo myservo1; [cite: 2]

RF24 radio(CE, CSN);
const byte address[6] = "00152";

static int fputchar(const char ch, FILE *stream) {
  Serial.write(ch);
  [cite_start]return ch; [cite: 3]
}
static FILE *sstream = fdevopen(fputchar, NULL);

typedef struct {
  int xpos;
  int ypos;
  int uid;
  int state;
[cite_start]} payload; [cite: 4]
payload res;

void setup() {
  stdout = sstream;
  Serial.begin(115200);

  myservo.attach(servox);
  myservo1.attach(servoy);
  mv(0, 0);
  pinMode(laser_beam, OUTPUT);
  pinMode(btn, INPUT_PULLUP);
  [cite_start]radio.begin(); [cite: 5]
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
}
int text = 0;
unsigned long restime = 0;
unsigned long printtime = 0;

[cite_start]void loop() { [cite: 6]
  if (radio.available()) {
    radio.read(&res, sizeof(res));
    [cite_start]printf("xpos %d, ypos %d, state %d\n", res.xpos, res.ypos, res.state); [cite: 7]
    restime = millis();
  [cite_start]} [cite: 8]
  if (millis() - restime >= 1000) {
    if (millis() - printtime >= 1000) {
      printf("no data in last %dms\n", (millis() - restime));
      [cite_start]printtime = millis(); [cite: 9]
      res.xpos = 0;
      res.ypos = 0;
      res.state = 0;
    }
  }
  mv(res.xpos, res.ypos);
  led(res.state);
[cite_start]} [cite: 10]

void led(int ledstate) {
  if (ledstate == 0)digitalWrite(laser_beam, 0);
  else if (ledstate == 1)digitalWrite(laser_beam, 1);
  else analogWrite(laser_beam, ledstate);
[cite_start]} [cite: 11]

int curxpos = -1;
int curypos = -1;
void mv(int localxpos, int localypos) {
  int writexpos = map(constrain(localxpos, -90, 90), -90, 90, 0, 180);
  [cite_start]int writeypos = map(constrain(localypos, -90, 90), -90, 90, 0, 180); [cite: 12]
  if (writexpos == curxpos && localypos == curypos)return;
  [cite_start]if (curxpos != writexpos) myservo.write(writexpos); [cite: 13]
  if (curypos != writeypos) myservo1.write(writeypos);
  delay(max(abs(writexpos - curxpos), abs(writeypos - curypos)));
  curxpos = localxpos;
  [cite_start]curypos = localypos; [cite: 14]
}


