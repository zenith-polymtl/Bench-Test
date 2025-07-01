#include <Servo.h>
#include "Adafruit_HX711.h"

const uint8_t DATA_PIN = 52;  
const uint8_t CLOCK_PIN = 53; 

Servo ESC; 
Adafruit_HX711 hx711(DATA_PIN, CLOCK_PIN);

const int BUTTON_PIN_1 = 8;
const int BUTTON_PIN_2 = 9;
const int BUTTON_PIN_3 = 10;

bool safety = LOW;

const int ESC_PIN = 13; 

int lastState_1;             // will be set in setup()
int lastState_2;             // will be set in setup()
int lastState_3; 
            // will be set in setup()

const int LedPin = 3;    // you need the “=” between the name and the value
const int LedPin2 = 2;

int ledState1 = LOW;   // default off
int ledState2 = LOW;   // default off

const int PotPin = A1;

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

  lastState_1 = digitalRead(BUTTON_PIN_1); 
  lastState_2 = digitalRead(BUTTON_PIN_2); 
  lastState_3 = digitalRead(BUTTON_PIN_3); 

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
  int analogValue = analogRead(PotPin);
  float percent = floatMap(analogValue, 0, 1023, 100, 0);
  float pwm = floatMap(analogValue, 0, 1023, 2000, 1000);
  float weightA128 = hx711.readChannelBlocking(CHAN_A_GAIN_128);
  double calibration_factor = -1.0/47378.0;

  int currentState_1 = digitalRead(BUTTON_PIN_1);
  int currentState_3 = digitalRead(BUTTON_PIN_3);
  int currentState_2 = digitalRead(BUTTON_PIN_2);

  Serial.print(currentState_1);
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

  digitalWrite(LedPin, ledState1);
  digitalWrite(LedPin2, HIGH);

  if (safety == HIGH){
    ESC.write(pwm);
  } else {
    ESC.write(1000);
  }

  Serial.print("Load cell raw  :");
  Serial.print(weightA128);

  Serial.print(" , ");

  Serial.print("Force  :");
  Serial.println(weightA128*calibration_factor);

  if (lastState_1 == HIGH && currentState_1 == LOW) {
    if (ledState1 == LOW) {
      ledState1 = HIGH;
      safety = HIGH;
      digitalWrite(LedPin, ledState1);
      delay(150);
    } else {
      ledState1 = LOW;
      safety = LOW;
      digitalWrite(LedPin, ledState1);
      delay(300);
    }
  }

  delay(10);
}
