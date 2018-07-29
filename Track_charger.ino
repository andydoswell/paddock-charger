/*  Track Charger controller.
    (C) 2018 A.G.Doswell

    See andydoz.blogspot.com
    for details.
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// define some special charcters for battery level

const byte batt0[8] = {
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b11111
};
const byte batt1[8] = {
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b11111
};
const byte batt2 [8] = {
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b11111,
  0b11111
};
const byte batt3[8] = {
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};
const byte batt4[8] = {
  0b01110,
  0b10001,
  0b10001,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};
const byte batt5[8] = {
  0b01110,
  0b10001,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};
const byte batt6[8] = {
  0b01110,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

unsigned long loopCounter = 0;
unsigned long timer = 0;
float inputV = 0;
float outputV = 0;
float outputI = 0;
float cumulativeInputV = 0;
float cumulativeOutputV = 0;
float cumulativeOutputI = 0;
const float inputVCal = 0.015607;
const float outputVCal = 0.01540633857;
const float outputICal = 49.527;
float mAH = 0;
int outputIZero;
const int bypassPin = 4;
const int chargePin = 2;
const int beeperPin = 13;
const int inputVoltagePin = A2;
const int outputVoltagePin = A3;
const int outputIPin = A0;

LiquidCrystal_I2C  lcd(0x3F, 20, 4); // I2C address 0x3f, 20x4 display

void setup() {
  pinMode (beeperPin, OUTPUT);
  digitalWrite (beeperPin, HIGH); // beeper output set high (off)
  pinMode (chargePin, OUTPUT);
  digitalWrite (chargePin, LOW); // output relay off
  pinMode (bypassPin, OUTPUT);
  digitalWrite (bypassPin, HIGH); // bypass relay on
  delay(100);
  lcd.init();
  lcd.begin(20, 4); // set up the LCD as 20 x 4 display
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.setCursor(0, 0); //splash screen
  lcd.print("   Golden Griffen");
  lcd.setCursor(0, 1);
  lcd.print("   Track Charger");
  lcd.setCursor(0, 2);
  lcd.print("andydoz.blogspot.com");
  autozero (); // set zero on current sensor
  lcd.clear(); // write permanent display
  lcd.print ("IN      V");
  lcd.setCursor (10, 0);
  lcd.write(byte(0));
  lcd.setCursor (0, 1);
  lcd.print("OUT      V        ");
  lcd.setCursor (0, 2);
  lcd.print("           mAH");
  timer = millis();
}

void loop() {
  loopCounter ++;
  getData ();
  if (millis () >= timer + 1000) {
    calculateAndDisplay();
  }

  switch (loopCounter) {
    case 1:
      lcd.createChar (1, batt0);
      break;
    case 5:
      lcd.createChar (1, batt1);
      break;
    case 10:
      lcd.createChar (1, batt2);
      break;
    case 15:
      lcd.createChar (1, batt3);
      break;
    case 20:
      lcd.createChar (1, batt4);
      break;
    case 25:
      lcd.createChar (1, batt5);
      break;
    case 30:
      lcd.createChar (1, batt6);
      break;
  }

  if (outputV < 4.9) {
    lcd.setCursor( 0, 3);
    lcd.print("  Connect battery   ");
    digitalWrite (chargePin, LOW);
  }
  else digitalWrite (chargePin, HIGH);
  
  if (outputI < 20 && outputV > 4.8) {
    digitalWrite (chargePin, LOW);
    lcd.setCursor(0, 3);
    lcd.print("Disconnect output. ");
    return;
  }
}

void getData () {
  inputV = analogRead(inputVoltagePin);
  inputV *= inputVCal;
  cumulativeInputV += inputV;
  delay (5);
  outputV = analogRead(outputVoltagePin);
  outputV *= outputVCal;
  cumulativeOutputV += outputV;
  delay (5);
  outputI = analogRead(outputIPin);
  outputI -= outputIZero;
  outputI *= outputICal ;
  if (outputI < 0) {
    outputI = 0;
  }
  cumulativeOutputI += outputI;
  delay (5);
}

void calculateAndDisplay () {
  inputV = cumulativeInputV / loopCounter;  //calculate average
  outputV = cumulativeOutputV / loopCounter;
  outputI = cumulativeOutputI / loopCounter;
  if (outputI < 10) {
    outputI = 0;
  }

  if (outputI > 5500 && outputV > 4.8) {
    digitalWrite (3, HIGH); // turn on bypass relay
    lcd.setCursor(0, 3);
    lcd.print("Equalisation charge.");
  }
  else {
    digitalWrite (3, LOW); // bypass off
  }

  mAH += (outputI / 3600); // from mA to maS to mAH
  lcd.setCursor (3, 0);
  lcd.print(inputV, 1);
  lcd.setCursor (4, 1);
  lcd.print(outputV, 1);
  lcd.setCursor (11, 1);
  lcd.print (outputI, 0);
  lcd.print(" mA  ");
  lcd.setCursor (0, 2);
  lcd.print(mAH, 1);
  if (inputV <= 11.6) { // select input battery charge level symbol
    lcd.createChar (0, batt0);
    lcd.setCursor(0, 3);
    lcd.print(" INPUT BATT WARNING!");
    digitalWrite (13, LOW);
    delay(100);
    digitalWrite (13, HIGH);
  }
  else { // draw charging graphic
    lcd.setCursor(0, 3);
    lcd.print("      ");
    lcd.setCursor(6, 3);
    lcd.write(byte(0));
    lcd.setCursor (7, 3);
    lcd.print (" >>> ");
    lcd.setCursor(12, 3);
    lcd.write(byte(1));
    lcd.setCursor(13, 3);
    lcd.print("       ");
  }
  if (inputV <= 12.4 && inputV > 12.2) {
    lcd.createChar (0, batt5);
  }
  if (inputV <= 12.2 && inputV > 12) {
    lcd.createChar (0, batt4);
  }
  if (inputV <= 12.0 && inputV > 11.9) {
    lcd.createChar (0, batt3);
  }
  if (inputV <= 11.9 && inputV > 11.7) {
    lcd.createChar (0, batt2);
  }

  if (inputV <= 11.7) {
    lcd.createChar (0, batt1);
  }

  if (inputV >= 12.6) {
    lcd.createChar (0, batt6);
  }
  loopCounter = 0;
  timer = millis ();
  cumulativeInputV = 0;
  cumulativeOutputV = 0;
  cumulativeOutputI = 0;
}

void autozero () { // auto zero current sensor.
  long cumulativeIZeroCount = 0; // counts from ADC
  int p;
  lcd.setCursor(0, 3);
  lcd.print("    Calibrating.    ");
  for ( p = 0; p <= 1000; p ++) {
    outputI = analogRead (outputIPin); // get value from current sensor
    cumulativeIZeroCount += outputI;
    delay (10);
  }
  outputIZero = cumulativeIZeroCount / p;
  outputIZero ++;
  outputI = 0;
}

