/*
 * CSC 413 Assignment 2
 * Code for Primary Arduino
 * Written by Amanda Anderson
 * October 28, 2024
 */
/********/
#include <Wire.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

// Pins
#define PROVBUTTON 2    //Single Button, must be inerrupt
#define GREENLED 3      //RGB LED, must be PWM       
#define DIRB 4          //Fan
#define ENABLE 5        //Fan, must be PWM
#define BLUELED 6       //RGB LED, must be PWM           
#define DIRA 7          //Fan
#define FARBUTTON 8     //Power Plant Type Button 1
#define SERVOSIGNAL 9   //Servo Motor
#define MIDDLEBUTTON 10 //Power Plant Type Button 2
#define REDLED 11       //RGB LED, must be PWM
#define CLOSEBUTTON 12  //Power Plant Type Button 3

// Data
float greenhouseEmissions[] = {0, 8.6, 1.6, 14.8, 12.5, 79.1, 157, 21.6, 75.9, 269.9, 64.3, 0.7, 1.4, 0.6}; // Units in Megatonnes of CO2 Equivalant (by province)
long powerGenerated[] = {0, 41942, 606, 8464, 11626, 212909, 148334, 29964, 24940, 73902, 71664, 701, 389, 279}; // Units in Gigawatt Hours (by province)
float percentWind[] = {0, 0.4, 99, 13, 6, 5, 8, 4, 4, 9, 3, 0.2, 3, 0}; // Units in percentage (/100) (by province)
float percentHydro[] = {0, 97, 0, 9, 23, 94, 24, 96, 10, 3, 89, 72, 36, 0};
float percentCoal[] = {0, 0, 0, 55, 13, 0, 0, 0, 41, 22, 0, 0, 0, 0};
float percentGas[] = {0, 0.6, 0, 18, 14, 0.1, 8, 0.2, 44, 63, 2.4, 16, 14, 99.8};
float percentOil[] = {0, 3, 0.8, 0.4, 0.4, 0.3, 0.1, 0.1, 0.1, 0.1, 0.1, 11, 47, 0};
float percentSolar[] = {0, 0, 0.1, 0, 0.1, 0.1, 5, 0.1, 0.1, 0.6, 0.1, 0.1, 0.5, 0.2};
float percentNuclear[] = {0, 0, 0, 0, 40, 0, 55, 0, 0, 0, 0, 0, 0, 0};

// Variables
/* Province Selection
 *  0 = Not Selected (default on start)
 *  1 = Newfoundland and Labrador
 *  2 = Prince Edward Island
 *  3 = Nova Scotia
 *  4 = New Brunswick
 *  5 = Quebec
 *  6 = Ontario
 *  7 = Manitoba
 *  8 = Saskatchewan
 *  9 = Alberta
 *  10 = British Columbia
 *  11 = Yukon
 *  12 = Northwest Territories
 *  13 = Nunavut
 */
int province = 13; 

/* Power Plant Type Selection
 *  0 = Not Selected (default on start)
 *  1 = Wind
 *  2 = Hydro
 *  3 = Coal
 *  4 = Gas
 *  5 = Oil
 *  6 = Solar
 *  7 = Nuclear
 */
int plantType = 0;

// Control Variables
int fanSpeed = 0;
int servorPosition = 0;
int ledColour[] = {0, 0, 0};
bool provinceStateChanged = false;
volatile unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500;
bool firstLoop = true;

void setup() {
  // Setup Servo
  myservo.attach(SERVOSIGNAL);

  // Setup RGB LED
  pinMode(REDLED,  OUTPUT);              
  pinMode(GREENLED, OUTPUT);
  pinMode(BLUELED, OUTPUT);

  // Setup Fan Motor
  pinMode(ENABLE, OUTPUT);
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);

  // Setup Single Button
  pinMode(PROVBUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PROVBUTTON), selectProvince, RISING);

  // Setup Power Plant Buttons
  pinMode(FARBUTTON, INPUT_PULLUP);   
  pinMode(MIDDLEBUTTON, INPUT_PULLUP);  
  pinMode(CLOSEBUTTON, INPUT_PULLUP); 

  // Start the I2C Bus as Primary
  Wire.begin();
}

void loop() {
  /*
   * First Iteration of Loop? Match up secondary Arduino
   */
   if (firstLoop) {
    Wire.beginTransmission(9);
    Wire.write(3);
    Wire.endTransmission();
    firstLoop = false;
   }
  
  /*
   * Check Power Plant Selection (polling)
   */
  int buttonState1 = digitalRead(FARBUTTON);
  int buttonState2 = digitalRead(MIDDLEBUTTON);
  int buttonState3 = digitalRead(CLOSEBUTTON);
  if (buttonState1 == HIGH && buttonState2 == HIGH && buttonState3 == HIGH) { 
    plantType = 7;
  }
  if (buttonState1 == HIGH && buttonState2 == HIGH && buttonState3 == LOW) { 
    plantType = 5;
  }
  if (buttonState1 == HIGH && buttonState2 == LOW && buttonState3 == HIGH) { 
    plantType = 4;
  }
  if (buttonState1 == LOW && buttonState2 == HIGH && buttonState3 == HIGH) { 
    plantType = 2;
  }
  if (buttonState1 == HIGH && buttonState2 == LOW && buttonState3 == LOW) { 
    plantType = 3; 
  }
  if (buttonState1 == LOW && buttonState2 == HIGH && buttonState3 == LOW) { 
    plantType = 1;
  }
  if (buttonState1 == LOW && buttonState2 == LOW && buttonState3 == HIGH) { 
    plantType = 6;
  }

  /*
   * Display Province based Data
   */
  // Fan, turn on with value between 100 to 255 
  //  represents relative power generation with value from 279 to 212909 GWh
  fanSpeed = map(powerGenerated[province], 270, 212909, 100, 255);
  digitalWrite(DIRA, HIGH); 
  digitalWrite(DIRB, LOW);
  analogWrite(ENABLE, fanSpeed);

  // LED, turn on with value?
  //  represents relative CO2 emissions
  ledColour[0] = map(greenhouseEmissions[province], 0, 270, 0, 255);
  ledColour[1] = map(greenhouseEmissions[province], 0, 270, 255, 0);
  setColor(ledColour[0], ledColour[1], ledColour[2]);

  /*
   * Display Power Plant based Data
   */
  // Servor, turn on with value between 0 to 180
  //  represents percent of total power generated by plant type
  switch(plantType){
    case 1:
      servorPosition = map(percentWind[province], 0, 100, 0, 180);
      break;
    case 2:
      servorPosition = map(percentHydro[province], 0, 100, 0, 180);
      break;
    case 3:
      servorPosition = map(percentCoal[province], 0, 100, 0, 180);
      break;
    case 4:
      servorPosition = map(percentGas[province], 0, 100, 0, 180);
      break;
    case 5:
      servorPosition = map(percentOil[province], 0, 100, 0, 180);
      break;
    case 6:
      servorPosition = map(percentSolar[province], 0, 100, 0, 180);
      break;
    case 7:
      servorPosition = map(percentNuclear[province], 0, 100, 0, 180);
      break;
    default:
      servorPosition = 0;
      break;    
  }

  if(provinceStateChanged) {
    Wire.beginTransmission(9);
    Wire.write(1);
    Wire.endTransmission();
    provinceStateChanged = false;;
  }

  myservo.write(servorPosition);  
  
  delay(50);
}

/*
 * Interrupt Routine
 */
void selectProvince() {
  if((millis() - lastDebounceTime) > debounceDelay) {
    if (!provinceStateChanged) {
      province = (province++ % 13) + 1;
      provinceStateChanged = true; 
    }
    lastDebounceTime = millis();
  }
}

/*
 * Functions
 */
void setColor(int redValue, int greenValue,  int blueValue) {
  analogWrite(REDLED, redValue);
  analogWrite(GREENLED, greenValue);
  analogWrite(BLUELED, blueValue);
}
