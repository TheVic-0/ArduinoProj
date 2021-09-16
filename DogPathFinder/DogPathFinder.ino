#include <Adafruit_NeoPixel.h>
#include <Servo.h>

//pir sensor---------------------------------------------------------------------------------------------------------------
int PIR = 13; //pin
int readPIR = 0; //store values of PIR readings

//LED pins -----------------------------------------------------------------------------------------------------------------
int LED1 = 12; //pin
int LED2 = 11; //pin
int LED3 = 10; //pin
int LED4 = 9; //pin
int LED5 = 8; //pin

//ultrasonic distance sensor ------------------------------------------------------------------------------------------------
//two sensors will be used to make sure only a small dog will activate the system
// if the two sensors get obstructed it is a person, if only the sensor at a lower position gets obstructed it is the dog
//Sensor 1 would be positioned at chest height
//Sensor 2 would be positioned at shin/knee/tigh height

int USS1 = 7; //pin for higher sensor1
int USS2 = 6; //pin for lower sensor2
int USS3 = 3; //pin for exterior sensor3

int cm1 = 0; //sensor 1
int cm2 = 0; //sensor 2
int cm3 = 0; // exterior sensor 3

int SenseDist1 = 80;  // distance between sensor 1 and nearest wall/floor or immovable obstacle
int SenseDist2 = 80;  // distance between sensor 2 and nearest wall/floor or immovable obstacle
int SenseDist3 = 80;  // distance between sensor 3 and nearest wall/floor or immovable obstacle

long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}

//servo motor------------------------------------------------------------------------------------------------
int servo = 5; //pin
int pos = 0; // position 0 degrees
Servo myServo;

//NEOPIXEL---------------------------------------------------------------------------------------------------
//int NEO = 4; //pin
#define PIN 4   // input pin Neopixel is attached to
#define NUMPIXELS      60 // number of neopixels in strip series
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int r = 0;
int g = 0;
int b = 0;


//other variables--------------------------------------------------------------------------------------------
bool open = false; // door state
bool calibrated = false; // servo calibration to avoid random movement

void setup()
{
  Serial.begin(9600);
  
  // Initialize NeoPixel library
  pixels.begin();
  
  pinMode(LED1, OUTPUT); // POWER ON
  pinMode(LED2, OUTPUT); // ON when NEOPIXEL strip and door open
  pinMode(LED3, OUTPUT); // ON when PIR active
  pinMode(LED4, OUTPUT); // ON when sensor 1 active
  pinMode(LED5, OUTPUT); // ON when sensor 2 active
  
  pinMode(PIR, INPUT);

  myServo.attach(servo, 544, 2500);
}

void loop()
{
  readings();
  // resets LEDs
  digitalWrite(LED1, HIGH); //POWER LED always ON
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  
  // clears all neopixels
  pixels.clear(); 
  pixels.show();
  
  //calibrates and stops servo
  //this avoids weird and random movement from occurring
  if(calibrated == false) //runns only once 
  {
    servoCalibrationSequence();
    // resets door status
    open = false;
  }

  readings();
  
  handlePIR(); // checks for movement
  
  if(readPIR == HIGH) // if there is any movement
  {
    cm1 = 0.01723 * readUltrasonicDistance(7, 7);
    cm2 = 0.01723 * readUltrasonicDistance(6, 6);
    
    readings();

    // if cm2 (sensor2) smaller than cm1, it means there is a "short" obstacle that does not obstruct sensor 1, so its not a person
    if(cm1 >= SenseDist1 && cm2 < SenseDist2) // arbitrary total distance between sensor and wall
    {
      Serial.print("-------------DOGGO DETECTED-------------\n");
      digitalWrite(LED5, HIGH);
      
      cm3 = 0.01723 * readUltrasonicDistance(3, 3); // activates exterior sensor 3
      OpenDoorSequence();
      NeoON();
      readings();

      delay(5000); // some time for dog to exit
      
      while(open == true)
      {
        //wait for dog to come back
        //keeps checking sensor
        readings();
        Serial.print("-------------LOOPING-------------\n");
        cm3 = 0.01723 * readUltrasonicDistance(3, 3);
        
        if(cm3 < SenseDist3) // arbitrary total distance between sensor and wall/floor
        {
          Serial.print("-------------DOGGO IS BACK HOME-------------\n");
          delay(10000);
          CloseDoorSequence();
          delay(2000);
          NeoOFF();
        }
      }
      
      
    }
    else if(cm1 < SenseDist1) // if sensor 1 gets activated
    {
      Serial.print("---------Sensor 1 activated---------\n");
      readings();
      digitalWrite(LED4, HIGH);
      if(cm2 < SenseDist2) // if sensor 2 gets activated along with sensor 1
      {
        Serial.print("---------Sensor 2 activated along with 1---------\n");
        readings();
        digitalWrite(LED5, HIGH);
      }
      delay(1000);
    }
    else // no sensor has been activated, yet?
    {
      readings();
      Serial.print("---------------else-----------------\n");
    }
  }
  else // no movement detected by PIR
  {
    Serial.print("---------NO MOVEMENT---------\n");
  }
  
}

void setColor()// picks only green color
{
  r = (0,0);
  g = (255,255);
  b = (0,0);
}

void handlePIR()
{
  readPIR = digitalRead(PIR);// reads PIR pin and stores value
  if(readPIR == HIGH)
  {
    Serial.println("**********Hey I got you**********");
    digitalWrite(LED3, HIGH);
  }
  
}

void NeoON()
{
  Serial.print("---------Turning NEOPIXELS ON---------\n");
  setColor();
  for (int i=0; i < NUMPIXELS; i++) 
  { 
    // pixels.Color takes RGB values - 0,0,0 to 255,255,255
    pixels.setPixelColor(i, pixels.Color(r, g, b));
    
    // This sends the updated pixel color to the hardware.
    pixels.show();
    delay(8.4);// takes approximately half a second to fully execute
  }
}

void NeoOFF()
{
  Serial.print("---------Turning NEOPIXELS OFF---------\n");
  setColor();
  for (int i=NUMPIXELS; i > 0; i--) 
  { 
    // clears NEO
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show();
    delay(8.4);
  }
}

void OpenDoorSequence()
{
  digitalWrite(LED2, HIGH);
  open = true; //indicates door is open
  // sweep the servo from 0 to 180 degrees in steps of 1 degrees
  // opens up pet door/pet flap
  for (pos = 0; pos <= 140; pos ++) 
  {
    // tell servo to go to position in variable 'pos'
    myServo.write(pos);
    // wait 10 ms for servo to reach the position
    delay(10); // Wait for 15 millisecond(s)
  }
}

void CloseDoorSequence()
{
  digitalWrite(LED2, LOW);
  //closes door/pet flap
  if(pos > 0)
  {
    for (pos = 140; pos > 0; pos --) 
    {
      // tell servo to go to position in variable 'pos'
      myServo.write(pos);
      // wait 10 ms for servo to reach the position
      delay(10); // Wait for 15 millisecond(s)
    }
  }
  open = false;
}

void servoCalibrationSequence()
{
  Serial.print("----THE SERVO IS BEING CALIBRATED----\n");
  for (pos = 0; pos < 30; pos ++) 
  {
    // tell servo to go to position in variable 'pos'
    myServo.write(pos);
    // wait 10 ms for servo to reach the position
    delay(10); // Wait for 10 millisecond(s)
  }
  for (pos = 30; pos > 0; pos --) 
    {
      // tell servo to go to position in variable 'pos'
      myServo.write(pos);
      // wait 10 ms for servo to reach the position
      delay(10); // Wait for 10 millisecond(s)
    }
  calibrated = true;
}

void readings()
{
  Serial.print("CM1: ");
  Serial.print(cm1);
  Serial.print("   ");
  Serial.print("CM2: ");
  Serial.print(cm2);
  Serial.print("   ");
  Serial.print("CM3: ");
  Serial.print(cm3);
  Serial.print("\n");


  Serial.print("readPIR: ");
  Serial.print(digitalRead(PIR));
  Serial.print("  ");
  Serial.print("pos: ");
  Serial.print(pos);
  Serial.print("\n");
}
