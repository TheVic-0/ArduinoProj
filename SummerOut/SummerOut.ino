#include <Servo.h>
#include <LiquidCrystal.h>

// LCD object -- Parameters: (RS, E, D4, D5, D6, D7):
LiquidCrystal lcd = LiquidCrystal(2, 3, 4, 5, 6, 7);

//outside temp LED  pins
int LEDt1 = 8;
int LEDt2 = 9;
int LEDt3 = 10;

//Door/window status LED  pins
int LEDs1 = 11;
int LEDs2 = 12;

// Servo
int servo = 13;
int pos;
Servo MyServo;
bool DoorStat = false;
bool calibrated = false;

//temperature sensor
int tmp = A0;
int celsius = 0;



void setup()
{
  Serial.begin(9600);

  // LCD's number of columns and rows------------------------------
  lcd.begin(16, 2);

  //temp sensor----------------------------------------------------
  pinMode(tmp, INPUT);

  //All LEDs-------------------------------------------------------
  pinMode(LEDt1, OUTPUT);
  pinMode(LEDt2, OUTPUT);
  pinMode(LEDt3, OUTPUT);
  pinMode(LEDs1, OUTPUT);
  pinMode(LEDs2, OUTPUT);

  //Sets up servo---------------------------------------------------
  MyServo.attach(servo, 544, 2500);

  
}

void loop()
{
  servoCalibrationSequence();
}

void GetTemp()
{
  celsius = map(((analogRead(tmp) - 20) * 3.04), 0, 1023, -40, 125);
}

void PrintToLCD()
{
  // Set the cursor on the third column and the first row
  lcd.setCursor(2, 0);
  lcd.print("Temp Outside: ");
  lcd.println(celsius);
  // Set the cursor on the third column and the second row:
  lcd.setCursor(2, 1);
  lcd.print("Door/Window Status: "); 
  GetDoorStat();
}

void GetDoorStat()
{
  if(DoorStat == false)
  {
    lcd.print("Closed");
  }
  else if (DoorStat == true)
  {
    lcd.print("Open");
  }
  else
  {
    lcd.print("Door Status Unknown");
    lcd.clear();
  }
}

void servoCalibrationSequence()
{
  Serial.print("----THE SERVO IS BEING CALIBRATED----\n");
  if(calibrated == false)
  {
    for (pos = 0; pos < 30; pos ++) 
    {
      // tell servo to go to position in variable 'pos'
      MyServo.write(pos);
      // wait 10 ms for servo to reach the position
      delay(10); // Wait for 10 millisecond(s)
    }
    for (pos = 30; pos > 0; pos --) 
    {
      // tell servo to go to position in variable 'pos'
      MyServo.write(pos);
      // wait 10 ms for servo to reach the position
      delay(10); // Wait for 10 millisecond(s)
    }
    calibrated = true; 
  }
}

void OpenDoorSequence()
{
  digitalWrite(LEDs2, HIGH);
  DoorStat = true; //indicates door is open
  // sweep the servo from 0 to 180 degrees in steps of 1 degrees
  // opens up pet door/pet flap
  for (pos = 0; pos <= 140; pos ++) 
  {
    // tell servo to go to position in variable 'pos'
    MyServo.write(pos);
    // wait 15 ms for servo to reach the position
    delay(15); // Wait for 15 millisecond(s)
  }
}
