#define MAX_TEMP 15
#define NORMAL_TEMP 12
#define COOLING_TIME 150000
#define TIME_AFTER_START 600000

#define THERM_WATER_PIN A0
#define COOLER_RPIN 2
#define FAN_RPIN 3
#define YLED_PIN 12
#define RLED_PIN 11
#define GLED_PIN 10

#include "GyverNTC.h"

GyverNTC therm_water(THERM_WATER_PIN, 10000, 3435, 25, 10000);
uint32_t coolerTimer, tmrUptime;
float tempT;

bool relCoolState = false;
bool relFanState = false;

bool CoolTimerState = false;
bool OnOff = false;

void setup() {
  Serial.begin(9600);
  pinMode(COOLER_RPIN, OUTPUT); 
  pinMode(FAN_RPIN, OUTPUT);

  pinMode(RLED_PIN, OUTPUT);
  pinMode(YLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);
  
  digitalWrite(COOLER_RPIN, 1);
  digitalWrite(FAN_RPIN, 1);

  rgy_show(1,1,1);
  delay(1000);
  rgy_show(0,0,0);
  
  coolerTimer = millis();
}

void loop() {
  temperatureCheck();
}

void showTemp(float t) {
  static float lastT;
  static bool waitForChangeT = false;

  if (waitForChangeT) {

  }

  float averT = MAX_TEMP + NORMAL_TEMP;
  averT /= 2;

  t = round(t * 10);
  t /= 10;

  if (t >= MAX_TEMP) {
    lastT = t;
    waitForChangeT = true;
    rgy_show(1,0,0);
  }
  else if (t < MAX_TEMP && t >= averT) {
    lastT = t;
    waitForChangeT = true;
    rgy_show(0,0,1); 
  }
  else if (t < averT) {
    lastT = t;
    waitForChangeT = true;
    rgy_show(0,1,0);
  }

  Serial.println(t);
}

void rgy_show(bool r, bool g, bool y) {
  digitalWrite(RLED_PIN, r);
  digitalWrite(GLED_PIN, g);
  digitalWrite(YLED_PIN, y);
}

void relay(bool cool, bool fan) {
  relCoolState = cool;
  relFanState = fan;

  digitalWrite(COOLER_RPIN, !cool); 
  digitalWrite(FAN_RPIN, !fan);
}

uint32_t tempCheckTimer;
void temperatureCheck() {
  if (millis() - tempCheckTimer < 1000) return;
  
  tempCheckTimer = millis();

  float tw = therm_water.getTempAverage() + 1;

  showTemp(tw);
  checkErrors(tw);

  if (!OnOff && tw > MAX_TEMP - 0.1) {
    relay(1, 1);
    OnOff = true;
    tmrUptime = millis();
    tempT = tw;
  }
  
  if (OnOff && tw <= NORMAL_TEMP) {
    relay(0, 1);
    CoolTimerState = true;
    OnOff = false;
    coolerTimer = millis(); 
  }
  
  if (CoolTimerState && (millis() - coolerTimer) > COOLING_TIME) {
    relay(0,0);
    CoolTimerState = false;
  }
}

void strobeLed(uint8_t strobe, uint8_t led) {
  bool led2 = true;

  rgy_show(0,0,0);

  while (true) {
    for (int i = 0; i < strobe * 2;) {
      i++;
      digitalWrite(led, led2);
      led2 = !led2;
      delay(300);
    }
    digitalWrite(led, 0);
    delay(1000);
  }
}

void checkErrors(float tw) {
  if (relCoolState && !relFanState) {
    relay(0,0);
    strobeLed(3, RLED_PIN); 
  }

  if (tw < 0) {
    relay(0,0);
    strobeLed(3, YLED_PIN);
  }

  if (OnOff && (millis() - tmrUptime) > TIME_AFTER_START) {
    if (tw >= tempT - 0.1) {
      relay(0,0);
      OnOff = false;
      strobeLed(3, GLED_PIN);
    }
  }
}
