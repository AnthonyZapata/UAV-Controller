
//imports

#include <SPI.h>
//#include <Wire.h>  
#include <LiquidCrystal_I2C.h>
#include <nRF24L01.h>
#include <RF24.h>


//lcd

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


//Antenna 

#define AntennaCEPin  9
#define AntennaCSNPin  10

////radio and transmissions

const uint64_t pipe = 0xE8E8F0F0E1LL; //sets transmission channel
RF24 radio (AntennaCEPin, AntennaCSNPin);

//buttons

#define buttonPin1  4    
#define buttonPin2  5
#define buttonPin3  6
#define buttonPin4  7

//Joysticks

#define joyStick1XPin  0
#define joyStick1YPin  1
#define joyStick2XPin  2
#define joyStick2YPin  3

#define DEADZONE    110         // distance from center for defining "deadzone"
#define DEADPULSE   1500      // default "center" pulse width
#define MINPULSE    1000      // minimum pulse width
#define MAXPULSE    2000      // maximum pulse width
#define CHANNELS    8         // number of channels
#define XBYTES      18        // number of bytes to be transmited


//controller initialize

boolean controllerOn = true;

//joystick values 

int jSValue1X = 0;
int jSValue1Y = 0;
int jSValue2X = 0;
int jSValue2Y = 0;

//button state

int buttonState1 = 0; 
int buttonState2 = 0;
int buttonState3 = 0;
int buttonState4 = 0;  


//Values to transmit
byte transmitBytes[XBYTES];        // 2 bytes per value
int  pulseValues[CHANNELS];          // actual pulse values {Throttle Y2, Roll X1, Pitch Y1, Yaw X2}
int  transmitDelay = 10000;   // microseconds between transmissions
unsigned long startTime;      // used to find elapsed time
unsigned long stopTime;       // used to find elapsed time

//Variables for calculating pulse widths
bool   reverseX1  = true;    // reverse axis direction roll
bool   reverseY1  = true;     // reverse axis direction pitch
bool   reverseX2  = false;    // reverse axis direction yaw
bool   reverseY2  = false;     // reverse axis direction throttle


void setup() {
  
  Serial.begin(9600);
  initializeDisplay(); 
  displayOnLine("Controller", 0, 1);
  displayOnLine("Prototype", 1, 1);
  delay(3000);
  while(digitalRead(1)){ //security 
    displayOnLine("set switches", 0, 1);
    displayOnLine("to Low", 1, 1);
  }

  radio.begin();
  radio.openWritingPipe(pipe); //begins trans
  
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  pinMode(buttonPin4, INPUT);
 
  
}
void loop() {
  
 
  while(controllerOn == true){

    pulseValues[0] = pulseCalculate2(joyStick1XPin, 1.25, 512, reverseX1); //Roll
    pulseValues[1] = pulseCalculate2(joyStick1YPin, 1.25, 512, reverseY1); //Pitch
    pulseValues[2] = pulseCalculate2(joyStick2YPin, 1.25, 512, reverseY2); //Throttle
    pulseValues[3] = pulseCalculate2(joyStick2XPin, 1.25, 512, reverseX2); //Yaw
//    pulseValues[4] = pulseCalculate(AUX1, false); //Aux1
//    pulseValues[5] = pulseCalculate(AUX2, false); //Aux2
//    pulseValues[6] = pinPulse(AUX3); //Aux3
//    pulseValues[7] = pinPulse(AUX4); //Aux4
    
    pulseToByteArray(pulseValues, transmitBytes);
    transmitBytes[XBYTES-2] = 1;
    transmitBytes[XBYTES-1] = 1;
    
    radio.write( transmitBytes, sizeof(transmitBytes) );

   runLCDJoyStickPositions();
   RunButtons();
  }
  
}



int pulseCalculate(int aPin, bool reverse)
{
  float input;
  int pulseOut;
  int readVal = reverseAxis(analogRead(aPin), reverse);
  input = (float)readVal;
  pulseOut = (int)(input*1000.0/1023.0);
  pulseOut += MINPULSE;
  return pulseOut;
}// end pulseCalculate()


void pulseToByteArray(int pulses[], byte bytes[])
{
  for (int i = 0; i < CHANNELS; i++)
  {
    bytes[2*i] = pulses[i]/256;
    bytes[2*i+1] = pulses[i]%256;
  }
}// end pulseToByteArray()




int pulseCalculate2(int aPin, float sensitivity, int center, bool reverse)
{
  int pulseOut;
  int readVal = reverseAxis(analogRead(aPin), reverse);
  if (readVal >= center - DEADZONE && readVal <= center + DEADZONE)
    pulseOut = DEADPULSE;
  if (readVal < center - DEADZONE)
    pulseOut =(int)(sensitivity * (readVal - (center - DEADZONE)) + DEADPULSE);
  if (readVal > center + DEADZONE)
    pulseOut =(int)(sensitivity * (readVal - (center + DEADZONE)) + DEADPULSE);
  if (pulseOut < MINPULSE)
    pulseOut = MINPULSE;
  if (pulseOut > MAXPULSE)
    pulseOut = MAXPULSE;
  return pulseOut;
}// end pulseCalculate()

int reverseAxis(int axis, bool reverse)
{
  if (reverse)
    axis = 1023 - axis;
  return axis;
} // ens reverseAxis()



void RunButtons(){

  
  
    buttonState1 = digitalRead(buttonPin1);
    buttonState2 = digitalRead(buttonPin2);
    buttonState3 = digitalRead(buttonPin3);
    buttonState4 = digitalRead(buttonPin4);
    
    // Buttons
    if (buttonState1 == HIGH) {//shutDown
      controllerOn = false;
      
    }
    if (buttonState2 == HIGH){
      
    }
    if (buttonState3 == HIGH){
      
    }
    if (buttonState4 == HIGH){
      
    }
  
  
}
void RunJoySticks(){
  while (controllerOn == true){
    jSValue1X = analogRead(joyStick1XPin);
    jSValue1Y = analogRead(joyStick1YPin);
    jSValue2X = analogRead(joyStick2XPin);
    jSValue2Y = analogRead(joyStick2YPin);
    delay(100);
  }
}

void runLCDJoyStickPositions()
{
displayOnLine("Roll:", 0, 0);
  lcd.print(pulseValues[0]);
  displayOnLine("Ptch:", 1, 0);
  lcd.print(pulseValues[1]);
  lcd.print("    ");
  displayOnLine("Thr: ", 2, 0);
  lcd.print(pulseValues[2]);
  displayOnLine("Yaw: ", 3, 0);
  lcd.print(pulseValues[3]);
  displayOnLine(" 1:", 0, 9);
  lcd.print((float)pulseValues[4]/1000);
  displayOnLine(" 2:", 1, 9);
  lcd.print((float)pulseValues[5]/1000);
  displayOnLine(" 3:", 2, 9);
    if (pulseValues[6]>1000)
      lcd.print("high");
    else
      lcd.print("low ");
  displayOnLine(" 4:", 3, 9);
    if (pulseValues[7]>1000)
      lcd.print("high");
    else
      lcd.print("low ");
}

  
void initializeDisplay (){
  lcd.begin(16,4);  
  lcd.backlight();
}

void displayOnLine(String script, int row, int col)
{
  lcd.setCursor(col, row);
  lcd.print(script);
}

