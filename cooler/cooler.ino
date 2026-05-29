float MAX_TEMP = 15.0;
float NORMAL_TEMP = 12.0;

//#include "EEPROM.h"
//#define RD false
//#define WR false

#define THERMISTOR_PIN A0
#define SERIES_RESISTOR 10000.0  // Номинал постоянного резистора в Омах
#define THERM_NOMINAL 10000.0     // Сопротивление при номинальной температуре
#define TEMPERATURE_NOMINAL 25.0  // Номинальная температура (обычно 25°C)
#define B_COEFFICIENT 3435.0 

#define RELAY_PIN 5
#define YLED_PIN 2
#define GLED_PIN 3
#define RLED_PIN 4

bool isCooling = false;
bool isError = false;

void rgy_show(bool r, bool g, bool y) {
  digitalWrite(RLED_PIN, r);
  digitalWrite(GLED_PIN, g);
  digitalWrite(YLED_PIN, y);
}

/*
void settings(bool flag) {
  if (EEPROM[0] != 174) flag = true;

  if (flag) {
    EEPROM.put(0, 174);
    EEPROM.put(4, MAX_TEMP);
    EEPROM.put(8, NORMAL_TEMP);

    Serial.print("updated. MX_TMP: ");
    Serial.print(MAX_TEMP);
    Serial.print(";\n\rNM_TMP: ");
    Serial.println(NORMAL_TEMP);
  }
  else {
    EEPROM.get(4, MAX_TEMP);
    EEPROM.get(8, NORMAL_TEMP);
  }
  
}
*/

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT); 
  pinMode(RLED_PIN, OUTPUT);
  pinMode(YLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, HIGH);
}


float getTemperature() {
  float steinhart;

  steinhart = SERIES_RESISTOR / (1023.0 / (float)analogRead(THERMISTOR_PIN) - 1.0) / THERM_NOMINAL;
  steinhart = log(steinhart);                      // ln(R/Ro)
  steinhart /= B_COEFFICIENT;                      // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURE_NOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                     // Переворачиваем
  steinhart -= 273.15;                             // Переводим Кельвины в Цельсии

  return steinhart;
}

void temperatureCheck() {
  static uint32_t tempCheckTimer = millis();
  if (millis() - tempCheckTimer < 1000) return;
  tempCheckTimer = millis();

  float tw = getTemperature();
  Serial.println(isError);
  Serial.println(isCooling);
  Serial.println(tw);
  Serial.println();

  if (tw <= -1 || isError) {
    isError = true;
    isCooling = false;
    digitalWrite(RELAY_PIN, HIGH);
    return;
  }

  if (tw > MAX_TEMP - 0.1) {
    isCooling = true;
    digitalWrite(RELAY_PIN, LOW);
  }
  
  if (tw <= NORMAL_TEMP) {
    isCooling = false;
    digitalWrite(RELAY_PIN, HIGH);
  }
}

/*
void serialHandler() {
  if (!Serial.available()) return;

  String inputString = Serial.readStringUntil('\n');
  inputString.trim();

  if (inputString.indexOf("UPD_MAX_TEMP:") != -1) 
    MAX_TEMP = inputString.substring(inputString.indexOf(":") + 1).toFloat();
  
  else if (inputString.indexOf("UPD_NORMAL_TEMP:") != -1) 
    NORMAL_TEMP = inputString.substring(inputString.indexOf(":") + 1).toFloat();

  else 
    return;

  settings(WR);
}
*/

void loop() {
  temperatureCheck();
  rgy_show(isError, (!isError && !isCooling), (!isError && isCooling));
  //serialHandler();
}
