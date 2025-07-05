#include <Servo.h>
#include "Adafruit_HX711.h"

int SweepCounter;

const uint8_t DATA_PIN = 52;  
const uint8_t CLOCK_PIN = 53; 

Servo ESC; 
Adafruit_HX711 hx711(DATA_PIN, CLOCK_PIN);

float loop_begin_time;

const int BUTTON_PIN_1 = 8;
const int BUTTON_PIN_2 = 9;
const int BUTTON_PIN_3 = 10;
double calibration_factor;

float weightA128;

int PotValue;

float percent;
float pwm;

int increment = 5;

bool activated = LOW;
bool activable = HIGH;

bool armed = LOW;

const int ESC_PIN = 13; 

            // will be set in setup()

const int LedPin = 3;    // you need the “=” between the name and the value
const int LedPin2 = 2;


int ArmButtonPress = HIGH;
int currentState_3 = HIGH;
int currentState_2 = HIGH;

const int PotPin = A1;

bool SweepButtonPress = HIGH;
bool SweepActivable = HIGH;
bool SweepActivated = LOW;

float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
  Serial.begin(115200);

  pinMode(LedPin, OUTPUT);
  pinMode(LedPin2, OUTPUT);

  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);

  ArmButtonPress = digitalRead(BUTTON_PIN_1);
  currentState_3 = digitalRead(BUTTON_PIN_3);
  currentState_2 = digitalRead(BUTTON_PIN_2);

  hx711.begin();

  // read and toss 3 values each
  Serial.println("Tareing....");
  for (uint8_t t=0; t<2; t++) {
    hx711.tareA(hx711.readChannelRaw(CHAN_A_GAIN_128));
    hx711.tareA(hx711.readChannelRaw(CHAN_A_GAIN_128));
    hx711.tareB(hx711.readChannelRaw(CHAN_B_GAIN_32));
    hx711.tareB(hx711.readChannelRaw(CHAN_B_GAIN_32));
  }

  ESC.attach(ESC_PIN,1000,2000);
}

void loop() {
  loop_begin_time = millis();


  DataRead();
  ArmButtonHandler();
  SweepButtonHandler();
  Sweep();
  ESC_command();
  LED_output();
  PrintData();

  Enforce_max_loop_speed(2);
}

void Enforce_max_loop_speed(float MinDelay){
  if (MinDelay > millis() - loop_begin_time ){
    delay(1);
  }
  float loop_begin_time = millis();
}

void DataRead(){
  PotValue = analogRead(PotPin);
  percent = floatMap(PotValue, 0, 1023, 100, 0);
  pwm = floatMap(PotValue, 0, 1023, 2000, 1000);
  weightA128 = hx711.readChannelBlocking(CHAN_A_GAIN_128);
  calibration_factor = -1.0/47378.0;

  ArmButtonPress = digitalRead(BUTTON_PIN_1);
  SweepButtonPress = digitalRead(BUTTON_PIN_3);
  currentState_2 = digitalRead(BUTTON_PIN_2);
}

void LED_output(){
  digitalWrite(LedPin, activated);
  digitalWrite(LedPin2, SweepActivated);
}


void ESC_command(){
  if (armed == HIGH){
    ESC.write(pwm);
  } else {
    ESC.write(1000);
  }
}

void SweepButtonHandler(){

  if (SweepButtonPress == HIGH){
    SweepActivable = HIGH;
  }

  if (SweepActivable == HIGH && SweepButtonPress == LOW){
    SweepActivable = LOW;

    if (SweepActivated == LOW && armed == HIGH){
      SweepActivated = HIGH;
      SweepCounter = 1050;
    }
  } 
}

void Sweep(){
  if (SweepActivated) {
      pwm = SweepCounter;
      SweepCounter += increment;
  }
  if (SweepCounter > 1999){
    increment = increment * -1;
  } else if ((1040 > SweepCounter) && (SweepActivated)) {
    SweepActivated = LOW;
    pwm = 1000;
    SweepActivable = HIGH;
  }
}

void ArmButtonHandler(){

  if (ArmButtonPress == HIGH){
    activable = HIGH;
  }

  if (activable == HIGH && ArmButtonPress == LOW){
    activable = LOW;

    if (activated == HIGH){
      activated = LOW;
      armed = LOW;
      SweepActivated = LOW;
      pwm = 1000;
    } else if (pwm < 1020) {
      activated = HIGH;
      armed = HIGH;
    } else {
      Serial.print("THROTTLE DOWN MOTHERFUCKER");
    }
  } 
}

void PrintData() {
  Serial.print(ArmButtonPress);
  Serial.print(" , ");

  Serial.print(currentState_2);
  Serial.print(" , ");

  Serial.print(currentState_3);
  Serial.print(" , ");

  Serial.print("Percent: ");
  Serial.print(percent);
  Serial.print(" , ");

  Serial.print("Pwm: ");
  Serial.print(pwm);
  Serial.print(" , ");

  Serial.print("Load cell raw  :");
  Serial.println(weightA128);

  Serial.print(" , ");

  Serial.print("Force  :");
  Serial.println(weightA128*calibration_factor);
}