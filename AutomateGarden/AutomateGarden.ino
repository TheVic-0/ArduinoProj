#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

// Utilizing Arduino Mega

//Real Time Clock ------------------------------------------------
RTC_DS3231 rtc;

// Soil Humidity Sensor ------------------------------------------
const int dry = 280;
const int soilSensor = A2;

// Water Pump ----------------------------------------------------
const int pumpPin = 12;

// KY-013 Analog Temperature Sensor Module -----------------------
int ThermistorPin = A0;
int Vo;
float R1 = 10000; // value of R1 on board
float logR2, R2, T;
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; //steinhart-hart coeficients for thermistor

// LCD object -- Parameters: (RS, E, D4, D5, D6, D7):-------------
LiquidCrystal lcd = LiquidCrystal(2, 3, 4, 5, 6, 7);

// Slide Switch Pin ----------------------------------------------------
int SwitchPin = 8;
int SwitchState = 0;
bool SwitchOn = false;

// LED -----------------------------------------------------------
int LED1 = 9; // Slide Switch - Regular Watering
int LED2 = 11; // Slide Switch - Drip Watering


// Rain Sensor FC-37/YL-83/HM-RD ---------------------------------------------------
int RainD = 10; // Digital pin of Rain Sensor
int RainA = A1; // Analog pin of Rain Sensor
int RainAnalogVal;
int RainDigitalVal;
int ThresholdValue = 500;
int HasRainedDay = 0;
int HasRainedMonth = 0;
bool HasRained = false;

// other variables ----------------------------------------------------------------
int milli = 0; // milliseconds for watering calculation  

// drip watering system variables
//bool HasDrippedAlready;
int DripLastHour = 0;
int DripLastMinute = 0;
int DripLastDay = 0;
int DripLastMonth = 0;

// ----------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Water Pump ------------------
  pinMode(pumpPin, OUTPUT);

  // Soil sensor ------------------
  pinMode(soilSensor, INPUT); 

  // Rain Sensor -----------------
  pinMode(RainD, INPUT);
  pinMode(RainA, INPUT);

  // LCD -------------------------
  lcd.begin(16, 2);

  // LED ------------------------
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  // Switch Pins ----------------
  pinMode(SwitchPin, INPUT);
  bool SwitchState = false;

  // Sets up RTC Module ---------
  Wire.begin();
  if (! rtc.begin()) 
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1);
  }

  // RTC ------------------------
  // automatically sets the RTC to the date & time on PC this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // manually sets the RTC with an explicit date & time, for example to set
  // January 21, 2021 at 3am you would call:
  // rtc.adjust(DateTime(2021, 1, 21, 3, 0, 0));

}
void loop() 
{
  //GetSerialDateTime();
  CalcWateringTime();
  GetSwitchState();
  GetRainStat();

  WaterPlantsNormal();
  Drip_ishWatering();
  
  
  PrintToLCD();
  //lcd.clear();
}




// ------------------------------------------------------------------------------------------------------
void GetSwitchState()
{
  SwitchState = digitalRead(SwitchPin);
  
  if(SwitchState == 1)
  {
    digitalWrite(LED2, LOW);
    digitalWrite(LED1, HIGH); // regular watering ON
    SwitchOn = true;
  }
  else
  {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH); //Drip Watering ON
    SwitchOn = false;
  }
}

void GetRainStat()
{
  DateTime now = rtc.now();
  RainDigitalVal = digitalRead(RainD);
  if(digitalRead(RainD) == LOW) // Output goes down when it rains
  {
    Serial.println("Digital value : wet");
    HasRained = true;
    HasRainedDay = now.day();
    HasRainedMonth = now.month();
        
    delay(10); 
  }
  else
  {
    Serial.println("Digital value : dry");
    delay(10); 
  }
  // zeroes variables in the next day after rain
  if(HasRainedDay < now.day() || HasRainedMonth < now.month())
  {
    HasRainedDay = 0;
    HasRained = false;
  }
  /*
  //Another way of doing it using the Analog pin
  RainAnalogVal = analogRead(RainA);
  Serial.print(RainAnalogVal);
  if(RainAnalogVal < ThresholdValue){
    Serial.println(" - It's wet");
    //LED ?
  }
  else {
    Serial.println(" - It's dry");
    // LED ?
  }
  
  */
}

void GetTempRead()
{
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0); //calculate resistance on thermistor
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // temperature in Kelvin
  T = T - 273.15; //convert Kelvin to Celcius
  // T = (T * 9.0)/ 5.0 + 32.0; //convert Celcius to Farenheit

  Serial.print("Temperature: "); 
  Serial.print(T);
  Serial.println(" C");
}

void CalcWateringTime()
{
  GetTempRead();
  if(T >= 32)
  {
    milli = 30000; // 30 seconds
  }
  else if(T<32 && T>25)
  {
    milli = 10000; // 10 seconds
  }
  else if(T < 25 && T > 10)
  {
    milli = 5000; // 5 seconds
  }
}

void WaterPlantsNormal()
{
  if(SwitchOn == true && HasRained == false)
  {
    // read current moisture
    int moisture = analogRead(soilSensor);
    Serial.println(moisture);
    
    if (moisture <= dry)
    {
      Serial.println("Watering starts now..moisture is " + String(moisture));
      digitalWrite(pumpPin, LOW);
  
      // keep watering for X sec
      delay(milli);
  
      // turn off water
      digitalWrite(pumpPin, HIGH);
      Serial.println("Done watering.");
    }
    else 
    {
      Serial.println("No watering needed " + String(moisture));
    }
  }
}

void PrintToLCD()
{
  // Set the cursor on the third column and the first row
  lcd.setCursor(2, 0);
  lcd.print("Temp: ");
  lcd.print(T);
  // Set the cursor on the third column and the second row:
  lcd.setCursor(2, 1);
  lcd.print("Mode: ");
  if(SwitchOn == true)
  {
    lcd.print("Regular");
  }
  else if(SwitchOn == false)
  {
    lcd.print("Drip");
  }
}

void GetSerialDateTime()
{
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print("  ");

  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.println(now.minute(), DEC);
  /*if(now.hour() > 0 && now.hour() < 12)
  {
    Serial.println(" AM");
  }
  else if(now.hour() >= 12)
  {
    Serial.println(" PM");
  }*/
}

void Drip_ishWatering()
{
  DateTime now = rtc.now();
  if(SwitchOn == false && HasRained == false)
  {
    // resets variables when month goes by
    if(DripLastMonth < now.month())
    {
      DripLastHour = 0;
      DripLastMinute = 0;
      DripLastDay = 0;
      DripLastMonth = 0;
    }
    // water plants if
    if((DripLastHour <= now.hour() && DripLastMinute < now.minute() && DripLastDay <= now.day()) || (DripLastHour == 0 && DripLastMinute == 0 && DripLastDay == 0))
    {
      if((DripLastHour <= now.hour() && now.minute() >= DripLastMinute + 20) || (DripLastHour == 0 && DripLastMinute == 0)) // drip watering in 20 min intervals pr if no
      {
        digitalWrite(pumpPin, LOW);
  
        // keep watering for 0.5 sec
        delay(500);
      
        // turn off water
        digitalWrite(pumpPin, HIGH);
        Serial.println("Done watering.");
        
        DripLastHour = now.hour();
        DripLastMinute = now.minute();
        DripLastDay = now.day();
        DripLastMonth = now.month();
      }
    }
  }
}
