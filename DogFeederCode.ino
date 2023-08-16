#include <Wire.h>
#include <TimeLib.h>
#include <LiquidCrystal.h>
#include <DS1307RTC.h>
#include <IRremote.h>

//github edit comment


//Motor control pin
#define motor 2

//Buzzer Control Pin
#define buzzer A3

//Food Sensor (HC-SR04 Sensor)
#define echo 8
#define trig 9

//IR Remote
#define irPin 10

//SDA and SCL Clock
#define SDA A4
#define SCL A5


LiquidCrystal lcd(13, 3, 4, 5, 6, 7);
IRrecv ir(irPin);

decode_results results;

bool timePM;
bool settingMenu = false;
bool minuteSet = false;
bool setFirstTime = false;


//set two feeding times here (military time):
static uint8_t firstHour = 9;
static uint8_t firstMinute = 0;

static uint8_t secondHour = 18;
static uint8_t secondMinute= 0;

uint8_t newHour;
uint8_t menu = 0;

String menuOptions[4] = {"Set 1st time", 
                               "Set 2nd time", 
                               "Manual dispense"};

tmElements_t tm;












                   
void setup() {
  Serial.begin(9600);

  Wire.begin();
  lcd.begin(16,2);

  ir.enableIRIn();

  pinMode(buzzer, OUTPUT);
  pinMode(motor,OUTPUT);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  
  bool parse=false;
  bool config=false;
  
  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
    parse = true;
    // and configure the RTC with this info
    if (RTC.write(tm)) {
      config = true;
    }
  }

  if (parse && config) {
    Serial.print("DS1307 configured Time=");
    Serial.print(__TIME__);
    Serial.print(", Date=");
    Serial.println(__DATE__);
  } else if (parse) {
    Serial.println("DS1307 Communication Error :-{");
    Serial.println("Please check your circuitry");
  } else {
    Serial.print("Could not parse info from the compiler, Time=\"");
    Serial.print(__TIME__);
    Serial.print("\", Date=\"");
    Serial.print(__DATE__);
    Serial.println("\"");
  }

  
}











void loop() {
   decode_results results;

   feedingTime();

   detectFoodLevel();
        
   if(RTC.read(tm)) {
    convertTo12Hour();
     } 

  if(settingMenu){
    menuSet();
     }

  else{
    if(ir.decode(&results)){

    Serial.println(results.value);

    lcd.clear();
    menuSelect(results.value);
    lcd.setCursor(2,1);

    if(menu == 2){
      lcd.setCursor(0,1);

    }

    lcd.print(menuOptions[menu]);

    ir.resume();
 
 if(results.value == 16753245){
    switch(menu){

        case 0:
   
          lcd.clear();
          lcd.setCursor(0,1);
          lcd.print("Set  ");

          settingMenu = true;
          setFirstTime = true;

          break;

          

        case 1:
          lcd.clear();
          lcd.setCursor(0,1);
          lcd.print("Set: ");

          settingMenu = true;
          setFirstTime = false;
          
          break;

          
        case 2:
        lcd.setCursor(0,1);
        lcd.print(" Dispensing...  ");
        digitalWrite(motor,HIGH);
        delay(15000);
        digitalWrite(motor,LOW);
        lcd.clear(); 
        
        break;
              }
  
           }
   
        }
  
     }

     /*

    Serial.print("First Time = ");
    Serial.print(firstHour);
    Serial.print(":");
    Serial.print(firstMinute);
    Serial.print("   Second Time = ");
    Serial.print(secondHour);
    Serial.print(":");
    Serial.println(secondMinute);

    */
    
}










void detectFoodLevel(){

  long duration;
  int distance;

  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  duration = pulseIn(echo, HIGH);
  distance = duration * 0.034 / 2;

if(distance < 23 && distance > 5){
    lcd.setCursor(13,0);
    lcd.write("LOW");
   }
  else{
    lcd.setCursor(13,0);
    lcd.write("   ");
    }

}














void menuSet(){
  static bool setFirst = false;
  static bool hourSet = true;
  static bool settingMinute = false;
  static int moveCursor = 0;
  static int firstNum;
  static int secondNum;
  
  if (ir.decode(&results)) {
    if(!setFirst){
            Serial.println(results.value);
            firstNum = numberSelect(results.value);
            lcd.setCursor(5 + moveCursor,1);

            lcd.print(firstNum);
            setFirst = true;

            ir.resume();

    }
    else{
            ir.resume();

            lcd.setCursor(5 + moveCursor,1);
            lcd.print(firstNum);
            Serial.println(results.value);
            lcd.setCursor(6 + moveCursor,1);
            secondNum = numberSelect(results.value);

            lcd.print(secondNum);
            lcd.setCursor(7 + moveCursor,1);
            
            if(hourSet)
            lcd.print(":");
            
            setFirst = false;
        
            if(hourSet){
              if(setFirstTime)
              firstHour = firstNum * 10 + secondNum;
              else
              secondHour = firstNum * 10 + secondNum;

              hourSet = false;
              moveCursor = 3;
            }
            else{
              if(setFirstTime)
              firstMinute = firstNum * 10 + secondNum;
              else
              secondMinute = firstNum * 10 + secondNum;
              hourSet = true;
              moveCursor = 0;
              settingMenu = false;
              
              
            }

            
    }
    
  }

  
}








void feedingTime(){
  int feedTime = 15000;
  static bool fed = false;

  if( ((tm.Hour == firstHour && tm.Minute == firstMinute) || (tm.Hour == secondHour && secondMinute == tm.Minute)) && !fed ) {
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("   Feeding...   ");
      digitalWrite(motor, HIGH);
      delay(feedTime);
      digitalWrite(motor,LOW);
      lcd.clear();
      fed = true;
    }

    else
      if(tm.Minute == tm.Minute + 1){
        fed = false;
      }
}









int timeAMPM(int number){
    if(number >=13){
      timePM = true;
      return number - 12;
    }
    else if(number == 0){
      return number + 12;
    }

    timePM = false;
    return number;
  }









void convertTo12Hour(){
   lcd.setCursor(4,0);

    newHour = timeAMPM(tm.Hour);
    print2digits(newHour);
    lcd.print(newHour);
    lcd.print(":");
    print2digits(tm.Minute);
    lcd.print(tm.Minute);

    if(timePM)
      lcd.print("PM");
    else
       lcd.print("AM");
}










void menuSelect(int entered){
  switch(entered){
        case 16724175:
          Serial.println("One pressed");
          break;
          
        case 16738455:
          Serial.println("Zero pressed");
          break;
        
        case 16718055:
          Serial.println("Two pressed");
          break;
          
        case 16743045:
          Serial.println("Three pressed");
          break;
          
        case 16716015:
          Serial.println("Four pressed");
          break;
          
        case 16726215:
          Serial.println("Five pressed");
          break;
          
        case 16734885:
          Serial.println("Six pressed");
          break;
          
        case 16728765:
          Serial.println("Seven pressed");
          break;
          
        case 16730805:
          Serial.println("Eight pressed");
          break;
          
        case 16732845:
          Serial.println("Nine pressed");
          break;

        case 16769055:
          Serial.println("Down");
          if(menu==0){
            menu = 0;
          }
          else{
          menu--;
          }

          Serial.println(menuOptions[menu]);
          lcd.clear();
          
          break;

        case 16748655:
          Serial.println("Up");
          if(menu>=2){
            menu = 2;
          }
          else{
          menu++;
          }
          
          lcd.clear();
          break;

        default:
          break;
        
    }


}









int numberSelect(int entered){
  int number;
  bool goodValue = false;
  switch(entered){
        case 16724175:
          Serial.println("One pressed");
          number = 1;
          break;
          
        case 16738455:
          Serial.println("Zero pressed");
          number = 0;
          break;
        
        case 16718055:
          Serial.println("Two pressed");
          number = 2;
          break;
          
        case 16743045:
          Serial.println("Three pressed");
          number = 3;
          break;
          
        case 16716015:
          Serial.println("Four pressed");
          number = 4;
          break;
          
        case 16726215:
          Serial.println("Five pressed");
          number = 5;
          break;
          
        case 16734885:
          Serial.println("Six pressed");
          number = 6;
          break;
          
        case 16728765:
          Serial.println("Seven pressed");
          number = 7;
          break;
          
        case 16730805:
          Serial.println("Eight pressed");
          number = 8;
          break;
          
        case 16732845:
          Serial.println("Nine pressed");
          number = 9;
          break;

        default:
          goodValue = false;
          break;

  }
   
    return number;
}











bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}












bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
    }

}








  

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    lcd.print("0");
  }
}
