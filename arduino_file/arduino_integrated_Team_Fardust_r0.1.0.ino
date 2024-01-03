//팀 파더스트 아두이노 통합 소스 코드 ver 0.1.0

#include <DFRobot_OxygenSensor.h>  //산소센서 처리 라이브러리
#include <LiquidCrystal_I2C.h>     // LCD I2C 제어 라이브러리
#include <Wire.h>                  // LCD 제어 라이브러리
#include <Ticker.h>                // 반복 처리 관련 라이브러리
#include <Adafruit_NeoPixel.h>     //LED Sttrip 제어 라이브러리
#include <SoftwareSerial.h>        //블루투스 제어 라이브러리

//센서 입출력 PIN
#define dustSampleTRIG A0
#define dustValueInput A1
#define ledStripPIN 4
#define PiezoPin 10
#define Oxygen_IICAddress ADDRESS_3
#define BT_RXD 6
#define BT_TXD 7
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial bt(BT_RXD, BT_TXD);


//[REQ-8]미세먼지 센서 처리 관련 변수 및 선언
#define smoothSet 50  // 미세먼지 신호 평균 샘플개수
float dustValue = 0, dustRaw = 0, dustVoltage = 0, dustValFinal = 0;
void getDust();
Ticker getDustTicker(getDust, 0, 0, MICROS_MICROS);
float dustSmooths[smoothSet];
float dustSmoothTotal = 0, dustSmooth = 0;

//[REQ-9]산소 센서 처리 관련 변수 및 선언
#define COLLECT_NUMBER 10  //산소센서 데이터 평균 횟수
DFRobot_OxygenSensor oxygen;
//[REQ-13]LCD 지시 관련 변수 및 선언
unsigned long lcdPreviousTime = millis();
long lcdRefreshTime = 1000;  //(1000ms)

//[REQ-10]알람 처리 관련 변수 및 선언
const static float dustBad = 81.0, dustGood = 30.0;      //pm2.5 기준
const static float oxygenBad = 18.0, oxygenGood = 20.5;  //대기 중 산소농도 기준
int alarmCondition;                                      //0 == BAD, 1 == NORMAL, 2 == GOOD

//[REQ-15]LEDSTRIP 처리 관련 변수 및 선언
#define maximumBrightness 20
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, ledStripPIN, NEO_GRB + NEO_KHZ800);
void controlLEDDim();
Ticker controlLEDDimTicker(controlLEDDim, 100);
uint32_t ledStripColor = strip.Color(0, 0, 0);

//[REQ-11]피에조 동작 관련 변수 및 선언
int piezoAlarm_i = 0;
int piezoAlarmRepeat = 0;
const int piezoRepeatMillis = 5000;  //경보음 반복 주기: 5000ms
unsigned long piezoPreviousMillis = 0;
void piezoAlarm();
Ticker piezoAlarmTicker(piezoAlarm, 150);

// 블루투스 통신 관련 변수 선언부
float Read_Value_ox;
unsigned int Read_Value_pm = 0;

void setup() {
  Serial.begin(9600);
  bt.begin(9600);
  pinMode(dustSampleTRIG, OUTPUT);
  pinMode(dustValueInput, INPUT);
  oxygenCheck(); //산소센서 I2C 확인 
  strip.begin(); //LED Strip 제어 시작 
  strip.show();//LED Strip 제어 시작 
  controlLEDDimTicker.start();
  getDustTicker.start();
  piezoAlarmTicker.start();
  lcdINIT();
  tone(PiezoPin, 2093.00, 100);
  delay(110);
  tone(PiezoPin, 2093.00, 100);
  delay(110);
  Serial.println("SETUP DONE");
  // put your setup code here, to run once:
  Serial.println((String) "|DUST|" + (String) "|OXGN|");
}

void loop() {
  float oxygenValFinal;
  getDustTicker.update();
  controlLEDDimTicker.update();
  //getDust가 평균을 모두 계산하고, LED 제어가 모두 끝난 상태에서 조건문 실행
  if ((((getDustTicker.counter() % 196) == 0)) && ((controlLEDDimTicker.counter() % 20) == 0)) {
    oxygenValFinal = getOxygen();
    Read_Value_ox = oxygenValFinal;
    bt.print(Read_Value_ox);
    bt.print(",");
    Read_Value_pm = dustValFinal;
    bt.print(Read_Value_pm);
    alarmCondition = alarmFunction(dustValFinal, oxygenValFinal);
    if ((millis() - lcdPreviousTime) >= lcdRefreshTime) {
      lcdControl(dustValFinal, oxygenValFinal);
      lcdPreviousTime = millis();
    }
    if (alarmCondition == 0) {  //Piezo 동작 관련 조건문
      if (millis() - piezoPreviousMillis > piezoAlarmRepeat) {
        while (piezoAlarm_i <= 5) {
          //Serial.println(piezoAlarm_i);
          piezoAlarmTicker.update();
        }
        piezoPreviousMillis = millis();
        piezoAlarmRepeat = piezoRepeatMillis;
        //Serial.println(piezoPreviousMillis);
        piezoAlarm_i = 0;
      }
    }
  }
  
}

void getDustAVG(float getDustVal) {  //미세먼지 평균 처리
  static int dustSmoothIndex;
  if (dustSmoothIndex < smoothSet) {
    dustSmooths[dustSmoothIndex] = getDustVal;
    dustSmoothTotal = dustSmoothTotal + dustSmooths[dustSmoothIndex];
    dustSmoothIndex += 1;
  }
  if (dustSmoothIndex >= smoothSet - 1) {
    dustSmooth = dustSmoothTotal / smoothSet;
    dustSmoothTotal = 0;
    dustSmoothIndex = 0;
    dustValFinal = dustSmooth;
  }
}

void getDust() {  //미세먼지 데이터 처리
  static int getDustCase;
  const int SAMPLING_T = 280, DELTA_T = 40, SLEEP_T = 9680;  //센서 Pulse 특성을 고려한 데이터 취득용 불변상수 지정
  float dustCalib = 0.542; //0.542 : 뚫린, //0.65: 막힌 공간                                   //[V]를 기준으로 하는 ZERO 보정용 변수
  switch (getDustCase) {
    case 0:  // IRED 'on'
      digitalWrite(dustSampleTRIG, LOW);
      getDustTicker.interval(SAMPLING_T);
      getDustCase = 1;
      break;
    case 1:  // Value Input
      dustRaw = analogRead(dustValueInput);
      digitalWrite(dustSampleTRIG, LOW);
      getDustTicker.interval(DELTA_T);
      getDustCase = 2;
      break;
    case 2:  // IRED 'off'
      digitalWrite(dustSampleTRIG, HIGH);
      getDustTicker.interval(SLEEP_T);
      getDustCase = 3;
      break;
    case 3:  // Value Calculation
      digitalWrite(dustSampleTRIG, HIGH);
      dustVoltage = (dustRaw * (5.0 / 1024.0)) - dustCalib;
      dustValue = 0.17 * dustVoltage * 1000;
      if (dustValue < 0) {
        dustValue = 0;
      }
      getDustAVG(dustValue);
      getDustCase = 0;
      break;
  }
}

void oxygenCheck() {  //산소 센서 I2C 연결 상태 확인
  while (!oxygen.begin(Oxygen_IICAddress)) {
    Serial.println("oxygen I2c device number error !");
    delay(1000);
  }
  Serial.println("oxygen I2c connect success !");
}

float getOxygen() {  //산소 센서 데이터 처리
  float oxygenData = oxygen.getOxygenData(COLLECT_NUMBER);
  return oxygenData;
}

void lcdINIT() {  //LCD 초기 설정 함수
  lcd.begin();
  lcd.backlight();
}
void lcdControl(int dust, float oxygen) {  //LCD 센서값 표기 함수
  //oxygen = floor(oxygen * 10) / 10;
  lcd.setCursor(0, 0);
  lcd.print((String) "PM2.5 :" + dust + (String) " uG/M3      ");
  lcd.setCursor(0, 1);
  lcd.print((String) "OXYGEN:" + oxygen + (String) " %      ");
}

int alarmFunction(float dust, float oxygen) {  //경보 판별 및 처리 함수
  static String dustStatus, oxygenStatus;
  int alarmFunctionReturn;  //0 == BAD, 1 == NORMAL, 2 == GOOD
  if (dust >= dustBad) {    //미세먼지 농도 판정
    dustStatus = "BAD";
  } else if (dust < dustBad && dust > dustGood) {
    dustStatus = "NORMAL";
  } else if (dust <= dustGood) {
    dustStatus = "GOOD";
  }

  if (oxygen <= oxygenBad) {  //산소 농도 판정
    oxygenStatus = "BAD";
  } else if (oxygen > oxygenBad && oxygen < oxygenGood) {
    oxygenStatus = "NORMAL";
  } else if (oxygen >= oxygenGood) {
    oxygenStatus = "GOOD";
  }
  //Serial.println((String) "time: " + millis());
  //Serial.println((String) "currnt dust: " + dust + ",  dust stat: " + dustStatus);
  //Serial.println((String) "currnt oxygen: " + oxygen + ",  oxygen stat: " + oxygenStatus);
  Serial.print(dust);
  Serial.println((String) "," + oxygen);

  if (dustStatus == "GOOD" && oxygenStatus == "GOOD") {
    alarmFunctionReturn = 2;  //GOOD
    return alarmFunctionReturn;
  } else if ((dustStatus == "NORMAL" || oxygenStatus == "NORMAL") && (!(dustStatus == "BAD" || oxygenStatus == "BAD"))) {
    alarmFunctionReturn = 1;  //NORMAL
    return alarmFunctionReturn;
  } else {
    alarmFunctionReturn = 0;  //BAD
    return alarmFunctionReturn;
  }
}
void controlLEDColor() {  //LED Strip 색상 제어 함수

  switch (alarmCondition) {  //0 == BAD, 1 == NORMAL, 2 == GOOD
    case 0:
      ledStripColor = strip.Color(255, 0, 0);  //RGB
      break;
    case 1:
      ledStripColor = strip.Color(0, 255, 0);
      break;
    case 2:
      ledStripColor = strip.Color(0, 0, 255);
      break;
  }

  for (int i = 0; i <= strip.numPixels(); i++) {
    strip.setPixelColor(i, ledStripColor);
    strip.show();
  }
}
void controlLEDDim() {  //LED Strip 밝기 제어 함수
  static int controlLED_i;
  static int controlLEDOnOffTrigger;  //0일때 점점 밝게, 1 일때 점점 어둡게
  if (controlLEDOnOffTrigger == 0) {
    strip.setBrightness(controlLED_i);
    controlLEDColor();
    controlLED_i += maximumBrightness / 10;
    //Serial.println((String) "i = " + controlLED_i);
    if (controlLED_i >= maximumBrightness) {
      controlLEDOnOffTrigger = 1;
    }
  } else if (controlLEDOnOffTrigger == 1) {
    strip.setBrightness(controlLED_i);
    controlLEDColor();
    controlLED_i -= maximumBrightness / 10;
    //Serial.println((String) "i = " + controlLED_i);
    if (controlLED_i <= 1) {
      controlLEDOnOffTrigger = 0;
    }
  }
}
void piezoAlarm() {  //피에조 동작 함수
  static int toneChanger;
  if (toneChanger == 0) {
    tone(PiezoPin, 1760.00, 100);
    toneChanger++;
    piezoAlarmTicker.interval(150);
  } else {
    tone(PiezoPin, 1318.51, 100);
    toneChanger = 0;
    piezoAlarmTicker.interval(500);
  }
  piezoAlarm_i++;
}
