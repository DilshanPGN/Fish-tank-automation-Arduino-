#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Wire.h>
#include <RTClib.h>
#include <HX711.h>
#include <Servo.h>

RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27,16,2); 

//defining buttons
#define btOk     3
#define btMode   2
#define btUp     4
#define btDown   5

//defining relay pins
#define relayFilter 12
#define relayHeater 11

//Buzzer
#define buzzer 13

//Motor
#define motorPin 10

//Tempurature sensor
#define ONE_WIRE_BUS 6  //pin 6 of arduino

//variable to save motor is On or OFF
bool isMotorOn = false;
int prevoiusQty=0; //this variable used in triggerFeederFunction
bool isMinutePassed = true; //this need in triggerFeederFunction
unsigned long previousTimeFeeder = 0;  //this value use for pass one minute
int blinkIntervalFeeder=60005;

//variable to save heater is On or OFF
bool isHeaterOn = false;

//variable to save filer is On or OFF
bool isFilterOn = false;


//RTC module
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
int rtcHr = 0;
int rtcMin = 0;
int rtcSec = 0;

int rtcDay = 0;
int rtcMonth = 0;
int rtcYear = 0;


//Value for blinking

bool blinkMode = false;
unsigned long previousTimeBlink = 0;  //this value use for blinking
int blinkInterval=200;

//weight details

const int maxWeight = 1000; //in grammes
const int minWeight = 0; //in grammes

//Weight sensor (Temporary)
#define weightPin  A3

/*---------------------------------------------------------Temp sensor objects---------------------------*/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

/*---------------------------------------------------------Weight sensor objects---------------------------*/
HX711 scale(7, 8); // ports DT, CLK    

/*---------------------------------------------------------Servo motor---------------------------*/
Servo servo;

/*----------------------------------------------------------Variables---------------------------------------------*/

//-----------------------Variables to switch screens---------------------------------------------
int menuMode=0; //indicate menu or workingscreen
int menuSelectionOption=0;  //select main menu from list
int menuLevel1=0; //indicates which number of the submenu #1 selected
int menuLevel2=0; //indicates which number of the submenu #2 selected
int menuLevel3=0; //indicates which number of the submenu #3 selected
int setTimePositionSettings =0; //set time position
int setTimePositionFeeder=0; //feeder set time position
int setDatePositionSettings = 0; ////set date position
int setTimePositionFilter =0;//set time position at filter





//-----------------------Eeprom Store Variable (EEP variable adresses )--------------------------

//for intFoodPerServe - xx,xx
uint8_t eep_intFoodPerServeFisrtTwoNumbers = 1;
uint8_t eep_intFoodPerServeLastTwoNumbers = 2;

//set clockVariables
//uint8_t eep_hr = 3, eep_min = 4, eep_sec = 5, eep_day = 6, eep_month = 7, eep_year=8 ,  eep_meridiemNumberSetTime=9;

//heat                    sensor settins
uint8_t eep_minimumTemp=10 ,eep_heaterOffTemp=11;

//Filter
int eep_filterSlotsStatus[5] = {12,13,14,15,16};
int eep_filterSlotHourFrom[5]={17,18,19,20,21};
int eep_filterSlotHourTo[5]={22,23,24,25,26};
int eep_filterSlotMinFrom[5]={27,28,29,30,31};
int eep_filterSlotMinTo[5]={32,33,34,35,36};
int eep_filterSlotsMeridiemFrom[5] = {37,38,39,40,41};
int eep_filterSlotsMeridiemTo[5] = {42,43,44,45,46};

//Feed
int eep_feedSlotsStatus[5] = {47,48,49,50,51};
int eep_feedSlotHour[5]={52,53,54,55,56};
int eep_feedSlotMin[5]={57,58,59,60,61};
int eep_feedSlotsMeridiem[5] = {62,63,64,65,66};

uint8_t eep_manualMode = 67;

uint8_t eep_calibratingFactor = 69;




//----------------------- Variable for saving value from EEPROM --------------------------
int intFoodPerServe=5;
double calibratingFactor=-16.70;



//set clock
int hr = 1, min = 30, sec = 55, day = 8, month = 11, year=21 ,  meridiemNumberSetTime=1;  // hour , minute , second ,date , month ,year
char *meridiemName[2] = { "AM", "PM"};

//Heat sensor settings
int minimumTemp=5;
int heaterOffTemp=15;


//Filter slots active = 1 , deactive = 0

int filterSlotsStatus[5] = {1,0,0,0,0};
int filterSlotHourFrom[5]={1,01,01,01,01};
int filterSlotMinFrom[5]={33,30,30,30,30};
int filterSlotsMeridiemFrom[5] = {0,0,0,0,0};//0=AM , 1=PM

int filterSlotHourTo[5]={1,01,01,01,01};
int filterSlotMinTo[5]={34,30,30,30,30};
int filterSlotsMeridiemTo[5] = {0,1,1,1,1};//0=AM , 1=PM

//Feed slots active = 1 , deactive = 0
int feedSlotsStatus[5] = {1,1,0,0,0};
int feedSlotHour[5]={1,01,01,01,01};
int feedSlotMin[5]={18,30,30,30,30};
int feedSlotsMeridiem[5] = {0,0,0,0,0};  //0=AM , 1=PM


int manualMode = 0;



//----------------------- Values from sensors --------------------------

float foodWeight; //inGrams
float heatValueC=0; //inCelcius

int remainings = 0; //how many number of times food can be served


void setup()
{
  Serial.begin(9600);
  lcd.init(); 
  lcd.backlight();
  //Wire.setClock(10000);
  rtcUpdateWithComputer(); //initialize RTC module
  
  pinMode(btOk,INPUT_PULLUP);
  pinMode(btUp,INPUT_PULLUP);
  pinMode(btDown,INPUT_PULLUP); 
  pinMode(btMode,INPUT_PULLUP);
  
  pinMode(motorPin,OUTPUT);
  pinMode(relayFilter,OUTPUT);
  pinMode(relayHeater,OUTPUT);
  pinMode(buzzer,OUTPUT);

  //attach servo
  servo.attach(motorPin);
  servo.write(3);

  //External inturrupts
  //attachInterrupt(digitalPinToInterrupt(btMode),innterruptModeButton,LOW);
  //attachInterrupt(digitalPinToInterrupt(btDown),buttonActivity,LOW);
  
  
  //readEEPROM();
  
  

  //weight sensor setup
  scale.set_scale();
  //scale.tare(); // reset the sensor to 0
  scale.set_scale(calibratingFactor); // apply calibration
  
  //tempurature sensor setup
  sensors.begin();

  lcd.print("Welcome");
  lcd.clear();
}
void loop()
{
  
  validation();
  updateRTCVariables();
  buttonActivity();
  changeScreens();
  toggleBlinkModeValue();  


  
  triggerHeater();

  
/*
  Serial.print("time = ");
  Serial.print(filterSlotHourFrom[0]);
  Serial.print(":");
  Serial.print(filterSlotMinFrom[0]);
  
  if(filterSlotsMeridiemFrom[0]==0){
    Serial.print("AM");
  }else if(filterSlotsMeridiemFrom[0]==1){
    Serial.print("PM");
  } 
  Serial.print("  ");

  if(filterSlotsStatus[0]==0){
    Serial.print("OFF");
  }else if(filterSlotsStatus[0]==1){
    Serial.print("ON");
  } 
  Serial.print("   ");

Serial.print("rtc = ");
  

  if(rtcHr>12){
    Serial.print(rtcHr-12);
  }else{
    Serial.print(rtcHr);
  }

  Serial.print(":");
  Serial.print(rtcMin);
  
  if(rtcHr>12){
    Serial.print("PM");
  }else{Serial.print("AM");}
*/

  triggerFilter();
  triggerBuzzer();
  

  
}

/*-------------------------------------------Button Functions---------------------------------------*/
void pressOK(){
  //do when ok butten pressed
  Serial.println("ok");
}
void pressMode(){
  //do when mode butten pressed
  Serial.println("mode");
}
void pressUp(){
  //do when up butten pressed
  Serial.println("up");
}
void pressDown(){
  //do when down butten pressed
  Serial.println("Down");
}


/*------------------------------------------------Screens------------------------------------------*/
/******************Initialization*************************/
void printHomeScreen(){
  
  lcd.setCursor(0,0);
  //time
  lcd.print(getDigit(rtcHr,1));
  lcd.print(getDigit(rtcHr,0));
  lcd.print(":");
  lcd.print(getDigit(rtcMin,1));
  lcd.print(getDigit(rtcMin,0));
  lcd.print(":");
  lcd.print(getDigit(rtcSec,1));
  lcd.print(getDigit(rtcSec,0));
  lcd.print(" "); //9

  //size
  float weightInKillo = foodWeight/1000.000;
  
  //Serial.print("Weight =");
  //Serial.print(weightInKillo);
  //Serial.println();
  lcd.print(getDigit(foodWeight,3));
  lcd.print(getDigit(foodWeight,2));
  lcd.print(getDigit(foodWeight,1));
  lcd.print(getDigit(foodWeight,0));
  lcd.print(" G");
  lcd.print(" ");
  

  lcd.setCursor(0,1);
  lcd.print(getDigit(heatValueC,1));
  lcd.print(getDigit(heatValueC,0));
  lcd.print("Celcius");
  lcd.print(" ");
  lcd.print(getDigit(remainings,2));
  lcd.print(getDigit(remainings,1));
  lcd.print(getDigit(remainings,0));
  lcd.print("Qty");
  
}

/***************************Home Screen*******************************/
void printCalibrateScale(){
  lcd.setCursor(0,0);
  lcd.print("Calibrate Scale:");
  lcd.setCursor(0,1);
  lcd.print("Set 0g->press OK");
}


/**************************Main Menu**********************************/
void printMainMenu1(){
  
  lcd.setCursor(0,0);
  lcd.print("Settings       >");
  lcd.setCursor(0,1);
  lcd.print("Filter Schedule ");
}
void printMainMenu2(){ //MenuMode =1 & MenuOption=0

  
  lcd.setCursor(0,0);
  lcd.print("Settings        ");
  lcd.setCursor(0,1);
  lcd.print("Filter Schedule>");
}
void printMainMenu3(){
  
  lcd.setCursor(0,0);  
  lcd.print("Feed Schedule > ");
  lcd.setCursor(0,1);  
  lcd.print("                ");

}

/******************Settings*************************/
void printSettingsFoodServing(){
  lcd.setCursor(0,0);
  lcd.print("Food Per Serving");
  lcd.setCursor(0,1);
  lcd.print(getDigit(intFoodPerServe,3));
  lcd.print(getDigit(intFoodPerServe,2));
  lcd.print(getDigit(intFoodPerServe,1));
  lcd.print(getDigit(intFoodPerServe,0));
  lcd.print(" g");
  lcd.print("           ");

}
void printSettingSetTime(){
  //hr = 0, min = 0, sec = 0, day = 0, month = 0, year=0, meridiemNumberSetTime;
  lcd.setCursor(0,0);
  lcd.print("Set Time        ");
  lcd.setCursor(0,1);
  //--------------- set hr
  if(setTimePositionSettings==1){
    if(blinkMode==true){
      lcd.print(getDigit(hr,1));
      lcd.print(getDigit(hr,0)); //hour
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(hr,1));
    lcd.print(getDigit(hr,0)); //hour
  }
  //--------------- end of set hr
  lcd.print("."); 

  //--------------- set min
  if(setTimePositionSettings==2){
    if(blinkMode==true){
      lcd.print(getDigit(min,1));
      lcd.print(getDigit(min,0)); //minute
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(min,1));
    lcd.print(getDigit(min,0)); //minute
  }
//--------------- set min
  
  lcd.print(".");

  //--------------- set sec
  if(setTimePositionSettings==3){
    if(blinkMode==true){
      lcd.print(getDigit(sec,1));
      lcd.print(getDigit(sec,0)); //second
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(sec,1));
    lcd.print(getDigit(sec,0)); //second
  }
  //--------------- end of set hr

  //--------------- set meridian
  if(setTimePositionSettings==4){
    if(blinkMode==true){
      lcd.print(meridiemName[meridiemNumberSetTime]); //AM / PM
      lcd.print("      ");
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(meridiemName[meridiemNumberSetTime]); //AM / PM
    lcd.print("      ");
  }
  //--------------- set meridian
  
}

void printSettingSetDate(){
  //hr = 0, min = 0, sec = 0, day = 0, month = 0, year=0, meridiemNumberSetTime;

  //setDatePositionSettings
  lcd.setCursor(0,0);
  lcd.print("Set Date        ");
  lcd.setCursor(0,1);
  
  //--------------- set day
  if(setDatePositionSettings==1){
    if(blinkMode==true){
      lcd.print(getDigit(day,1));
      lcd.print(getDigit(day,0)); //day
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(day,1));
    lcd.print(getDigit(day,0)); //day
  }
  
  
  lcd.print("/"); 

  //--------------- set month
  if(setDatePositionSettings==2){
    if(blinkMode==true){
      lcd.print(getDigit(month,1));
      lcd.print(getDigit(month,0)); //month
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(month,1));
    lcd.print(getDigit(month,0)); //month
  }
  
  
  lcd.print("/20");

  //--------------- set month
  if(setDatePositionSettings==3){
    if(blinkMode==true){
      lcd.print(getDigit(year,1));
      lcd.print(getDigit(year,0)); //year
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(year,1));
    lcd.print(getDigit(year,0)); //year 
  }
  
  
  lcd.print("      ");
}

void printSettingMinimumTempurature(){
  lcd.setCursor(0,0);
  lcd.print("Set Minimum Temp");
  lcd.setCursor(0,1);
  lcd.print(getDigit(minimumTemp,1));
  lcd.print(getDigit(minimumTemp,0));
  lcd.print(" Celsius");
  lcd.print("      ");

}
void printSettingHeaterOffTempurature(){
  lcd.setCursor(0,0);
  lcd.print("Heater Off Temp");
  lcd.setCursor(0,1);
  lcd.print(getDigit(heaterOffTemp,1));
  lcd.print(getDigit(heaterOffTemp,0));
  lcd.print(" celcius");
  lcd.print("      ");

}

/******************Filter slot selection*************************/

void printFilterSlot1Menu(){
  lcd.setCursor(0,0);
  lcd.print("Slot #1>");
  lcd.print(getFilterStatus(1));
  lcd.setCursor(0,1);
  lcd.print("Slot #2 ");
  lcd.print(getFilterStatus(2));
}
void printFilterSlot2Menu(){
  lcd.setCursor(0,0);
  lcd.print("Slot #1 ");
  lcd.print(getFilterStatus(1));
  lcd.setCursor(0,1);
  lcd.print("Slot #2>");
  lcd.print(getFilterStatus(2));
}
void printFilterSlot3Menu(){
  lcd.setCursor(0,0);
  lcd.print("Slot #3>");
  lcd.print(getFilterStatus(3));
  lcd.setCursor(0,1);
  lcd.print("Slot #4 ");
  lcd.print(getFilterStatus(4));
}
void printFilterSlot4Menu(){
  lcd.setCursor(0,0);
  lcd.print("Slot #3 ");
  lcd.print(getFilterStatus(3));
  lcd.setCursor(0,1);
  lcd.print("Slot #4>");
  lcd.print(getFilterStatus(4));
}
void printFilterSlot5Menu(){
  lcd.setCursor(0,0);
  lcd.print("Slot #5>");
  lcd.print(getFilterStatus(5));
  lcd.setCursor(0,1);
  lcd.print("                ");
}

/****** set values filter*****/

void setValueFilterSlot(int slotNumber){
  /* (preview)
  Slot| 00:00AM To
  #1  | 00:00AM
  */
  //setTimePositionFilter

  lcd.setCursor(0,0);
  lcd.print("Slot| ");

  //--------------- set to hr
  if(setTimePositionFilter==1){
    if(blinkMode==true){
      lcd.print(getDigit(filterSlotHourFrom[slotNumber-1],1));
      lcd.print(getDigit(filterSlotHourFrom[slotNumber-1],0));
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(filterSlotHourFrom[slotNumber-1],1));
    lcd.print(getDigit(filterSlotHourFrom[slotNumber-1],0));
  }
  lcd.print(":");

  //--------------- set to MIN
  if(setTimePositionFilter==2){
    if(blinkMode==true){
      lcd.print(getDigit(filterSlotMinFrom[slotNumber-1],1));
      lcd.print(getDigit(filterSlotMinFrom[slotNumber-1],0));
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(filterSlotMinFrom[slotNumber-1],1));
    lcd.print(getDigit(filterSlotMinFrom[slotNumber-1],0));
  }

  
//--------------- set Meridiem From
  if(setTimePositionFilter==3){
    if(blinkMode==true){
      if(filterSlotsMeridiemFrom[slotNumber-1]==0){ //AM
        lcd.print("AM");
      }else{
        lcd.print("PM");
      }
    }else{
      lcd.print("  ");
    }
  }else{
    if(filterSlotsMeridiemFrom[slotNumber-1]==0){ //AM
      lcd.print("AM");
    }else{
      lcd.print("PM");
    }
  }
 

  lcd.print(" to  ");
  lcd.setCursor(0,1);
  lcd.print("#");
  lcd.print(slotNumber);
  lcd.print("  | ");


//--------------- set to HR
  if(setTimePositionFilter==4){
    if(blinkMode==true){
      lcd.print(getDigit(filterSlotHourTo[slotNumber-1],1));
      lcd.print(getDigit(filterSlotHourTo[slotNumber-1],0));
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(filterSlotHourTo[slotNumber-1],1));
    lcd.print(getDigit(filterSlotHourTo[slotNumber-1],0));
  }


  lcd.print(":");

  //--------------- set to HR
  if(setTimePositionFilter==5){
    if(blinkMode==true){
      lcd.print(getDigit(filterSlotMinTo[slotNumber-1],1));
      lcd.print(getDigit(filterSlotMinTo[slotNumber-1],0));
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(filterSlotMinTo[slotNumber-1],1));
    lcd.print(getDigit(filterSlotMinTo[slotNumber-1],0));
  }

  

  //--------------- set to HR
  if(setTimePositionFilter==6){
    if(blinkMode==true){
      if(filterSlotsMeridiemTo[slotNumber-1]==0){ //AM
        lcd.print("AM");
      }else{
        lcd.print("PM");
      }
    }else{
      lcd.print("  ");
    }
  }else{
    if(filterSlotsMeridiemTo[slotNumber-1]==0){ //AM
      lcd.print("AM");
    }else{
      lcd.print("PM");
    }
  }
  
  


  lcd.print("     ");
}

/****** toggle active/off filter*****/
void toggleStateFilterSlot(int slotNumber){
  /* (preview)
  Active Filter ?
  Slot #1 : ACTIVE
  */
  lcd.setCursor(0,0);
  lcd.print("Active Filter ? ");
  lcd.setCursor(0,1);
  lcd.print("Slot #");
  lcd.print(slotNumber);
  lcd.print(" : ");

  if(filterSlotsStatus[slotNumber-1]==0){ //Off
  lcd.print("OFF   ");
  }else{ //ACtive
  lcd.print("ACTIVE");
  }
  
}

/****************************************Feeding***************************************/

/********Feed Option selector****/
void printSelectSetFeedMethod1(){
  lcd.setCursor(0,0);
  lcd.print("Time Slots     >");
  lcd.setCursor(0,1);
  lcd.print("Calculate Times ");
}
void printSelectSetFeedMethod2(){
  lcd.setCursor(0,0);
  lcd.print("Time Slots      ");
  lcd.setCursor(0,1);
  lcd.print("Calculate Times>");
}


/********Feed Scedule slot Option****/
void printFeedSlot1Menu(){
  lcd.setCursor(0,0);
  lcd.print("Slot #1>");
  lcd.print(getFeedStatus(1));
  lcd.setCursor(0,1);
  lcd.print("Slot #2 ");
  lcd.print(getFeedStatus(2));
}
void printFeedSlot2Menu(){
  lcd.setCursor(0,0);
  lcd.print("Slot #1 ");
  lcd.print(getFeedStatus(1));
  lcd.setCursor(0,1);
  lcd.print("Slot #2>");
  lcd.print(getFeedStatus(2));
}
void printFeedSlot3Menu(){
  lcd.setCursor(0,0);
  lcd.print("Slot #3>");
  lcd.print(getFeedStatus(3));
  lcd.setCursor(0,1);
  lcd.print("Slot #4 ");
  lcd.print(getFeedStatus(4));
}
void printFeedSlot4Menu(){
  lcd.setCursor(0,0);
  lcd.print("Slot #3 ");
  lcd.print(getFeedStatus(3));
  lcd.setCursor(0,1);
  lcd.print("Slot #4>");
  lcd.print(getFeedStatus(4));
}
void printFeedSlot5Menu(){
  lcd.setCursor(0,0);
  lcd.print("Slot #5>");
  lcd.print(getFeedStatus(5));
  lcd.setCursor(0,1);
  lcd.print("                ");
}

/****** set values feed*****/
void printSetValueFeedSlot(int slotNumber){
  /* (preview)
  Slot| 00:00AM To
  #1  | ACTIVE
  */
  lcd.setCursor(0,0);
  lcd.print("Slot| ");

  //--------------- hour feed
  if(setTimePositionFeeder==1){
    if(blinkMode==true){
      lcd.print(getDigit(feedSlotHour[slotNumber-1],1));
      lcd.print(getDigit(feedSlotHour[slotNumber-1],0));
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(feedSlotHour[slotNumber-1],1));
    lcd.print(getDigit(feedSlotHour[slotNumber-1],0));
  }


  lcd.print(":");

  //--------------- minute feed
  if(setTimePositionFeeder==2){
    if(blinkMode==true){
      lcd.print(getDigit(feedSlotMin[slotNumber-1],1));
      lcd.print(getDigit(feedSlotMin[slotNumber-1],0));
    }else{
      lcd.print("  ");
    }
  }else{
    lcd.print(getDigit(feedSlotMin[slotNumber-1],1));
    lcd.print(getDigit(feedSlotMin[slotNumber-1],0));
  }

  

//--------------- mreridean feed
  if(setTimePositionFeeder==3){
    if(blinkMode==true){
      if(feedSlotsMeridiem[slotNumber-1]==0){ //AM
        lcd.print("AM");
      }else{
        lcd.print("PM");
      }
    }else{
      lcd.print("  ");
    }
  }else{
    if(feedSlotsMeridiem[slotNumber-1]==0){ //AM
        lcd.print("AM");
    }else{
      lcd.print("PM");
    }
  }

  lcd.print("     ");

  lcd.setCursor(0,1);
  lcd.print("#");
  lcd.print(slotNumber);
  lcd.print("  | ");

  //--------------- state feed
  if(setTimePositionFeeder==4){
    if(blinkMode==true){
      if(feedSlotsStatus[slotNumber-1]==0){ //AM
        lcd.print("OFF      ");
      }else{
        lcd.print("ACTIVE    ");
      }
    }else{
      lcd.print("       ");
    }
  }else{
    if(feedSlotsStatus[slotNumber-1]==0){ //AM
      lcd.print("OFF      ");
    }else{
      lcd.print("ACTIVE    ");
    }
  }


 
  
}
/***********************************Saving************************************/
void printSaving(){
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Saving..........");
}

/*------------------------------------------------Methods------------------------------------------*/
int getDigit(double value , int digit){
  int regiredDigit=digit-1;
  int finalValue=0;
  
  switch(regiredDigit){

    //3210.-1-2 format
    
    case 2:
      finalValue=((int)(value*0.1*(pow(10,-2)) )) %10;
      break;
    case 1:
      finalValue=((int)(value*0.1*(pow(10,-1)) )) %10;
      break;
    case 0:
      finalValue=((int)(value*0.1*(pow(10,0)) )) %10;
      break;
    case -1:
      finalValue=((int)(value*0.1*(pow(10,1)) )) %10;
      break;
      
    case -2:
      finalValue=((int)(value*0.1*(pow(10,2)) )) %10;
      break;
    case -3:
      finalValue=((int)(value*0.1*(pow(10,3)) )) %10;
      break;
  }  
  return finalValue;
}

String getFilterStatus(int slotNumber){
  int value = filterSlotsStatus[slotNumber-1];
  if(value==0){
    return "(OFF)   ";
  }else{
    return "(ACTIVE)";
  }
}

String getFeedStatus(int slotNumber){
  int value = feedSlotsStatus[slotNumber-1];
  if(value==0){
    return "(OFF)   ";
  }else{
    return "(ACTIVE)";
  }
}

/*-----------------------------------------------------------EEPROM-------------------------------------------------------*/

void writeEEPROM(){


  EEPROM.write(eep_intFoodPerServeFisrtTwoNumbers,((intFoodPerServe-(intFoodPerServe%100))/10));
  EEPROM.write(eep_intFoodPerServeLastTwoNumbers,intFoodPerServe%100);
 
  //heat sensor settins
  EEPROM.write(eep_minimumTemp,minimumTemp);
  EEPROM.write(eep_heaterOffTemp,heaterOffTemp);

  //weigh sensorSettins 
  EEPROM.write(eep_calibratingFactor,calibratingFactor);

  //Filter
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_filterSlotsStatus[i],filterSlotsStatus[i]);
  }
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_filterSlotHourFrom[i],filterSlotHourFrom[i]);
  }
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_filterSlotHourTo[i],filterSlotHourTo[i]);
  }
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_filterSlotMinFrom[i],eep_filterSlotMinFrom[i]);
  }
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_filterSlotMinTo[i],filterSlotMinTo[i]);
  }
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_filterSlotsMeridiemFrom[i],filterSlotsMeridiemFrom[i]);
  }
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_filterSlotsMeridiemTo[i],filterSlotsMeridiemTo[i]);
  }


  //Feed
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_feedSlotsStatus[i],feedSlotsStatus[i]);
  }
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_feedSlotHour[i],feedSlotHour[i]);
  }
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_feedSlotMin[i],feedSlotMin[i]);
  }
  for (int i = 0; i<5; i++) {
    EEPROM.write(eep_feedSlotsMeridiem[i],feedSlotsMeridiem[i]);
  }

  EEPROM.write(eep_manualMode,manualMode);

}


void readEEPROM(){

  intFoodPerServe = (100*(EEPROM.read(eep_intFoodPerServeFisrtTwoNumbers))+(EEPROM.read(eep_intFoodPerServeLastTwoNumbers)));
  
  
  //heat sensor settins
  minimumTemp = EEPROM.read(eep_minimumTemp);
  heaterOffTemp = EEPROM.read(eep_heaterOffTemp);

  //Filter 
  for (int i = eep_filterSlotsStatus[0],  j=0; i<=(eep_filterSlotsStatus[4])&& j<5; i++ ,j++) {
    filterSlotsStatus[j]= (EEPROM.read(i));
  }

  for (int i = eep_filterSlotHourFrom[0] , j=0; i<=eep_filterSlotHourFrom[4]&& j<5 ; i++,j++ ) {
    filterSlotHourFrom[j] = (EEPROM.read(i));
  }
  for (int i = eep_filterSlotHourTo[0], j=0; i<=eep_filterSlotHourTo[4]&& j<5 ; i++,j++) {
    filterSlotHourTo[j]=(EEPROM.read(i));
  }
  for (int i = eep_filterSlotMinFrom[0] = 0, j=0; i<=eep_filterSlotMinFrom[4]&& j<5; i++,j++ ) {
    filterSlotMinFrom[j]=(EEPROM.read(i));
  }
 
  for (int i = eep_filterSlotMinTo[0], j=0; i<=eep_filterSlotMinTo[4]&& j<5 ; i++,j++) {
    filterSlotMinTo[j]=(EEPROM.read(i));
  }
  for (int i = eep_filterSlotsMeridiemFrom[0], j=0; i<=eep_filterSlotsMeridiemFrom[4]&& j<5 ; i++,j++) {
    filterSlotsMeridiemFrom[j]=(EEPROM.read(i));
  }
  for (int i = eep_filterSlotsMeridiemTo[0],j=0; i<= eep_filterSlotsMeridiemTo[4]&& j<5 ; i++,j++) {
    filterSlotsMeridiemTo[j] = (EEPROM.read(i));
  }

  
  //Feed
  for (int i = eep_feedSlotsStatus[0], j=0; i<= eep_feedSlotsStatus[4]&& j<5 ; i++,j++ ) {
    feedSlotsStatus[j]= (EEPROM.read(i));
  }
  for (int i = eep_feedSlotHour[0], j=0; i<=eep_feedSlotHour[4]&& j<5 ; i++,j++ ) {
    feedSlotHour[j]= (EEPROM.read(i));
  }
  for (int i = eep_feedSlotMin[0], j=0; i<=eep_feedSlotMin[4]&& j<5 ; i++,j++ ) {
    feedSlotMin[j]= (EEPROM.read(i));
  }
  for (int i = eep_feedSlotsMeridiem[0], j=0; i<=eep_feedSlotsMeridiem[4]&& j<5 ; i++,j++ ) {
    feedSlotsMeridiem[j]= (EEPROM.read(i));
  }

  //loading variables
  manualMode=EEPROM.read(eep_manualMode);
  
  calibratingFactor = EEPROM.read(eep_calibratingFactor);
}

/*-----------------------------------------------Get values from Sensors-------------------------------*/
void getWeightSensorReadings(){
  double units; 
  double initialWeight = 659.39;
  double containerWeight = 141.75;
  double error = 0;

  for(int i=0; i<10; i++) units =+ scale.get_units(), 10; // make measurements 10 times
   units / 10; // divide values by 10
   foodWeight = (units * 0.035274)+ initialWeight -containerWeight +error; // convert values into grams    


   
  remainings = foodWeight / intFoodPerServe;
  //Serial.println(foodWeight);
}


void getTempuratureSensorReadings(){
  sensors.requestTemperatures();
  heatValueC = sensors.getTempCByIndex(0);
}

void clearAllScreenVariables(){
  menuMode=0;
  menuSelectionOption=0;
  menuLevel1=0;
  menuLevel2=0;
  menuLevel3=0;
  setTimePositionSettings=0;
  setTimePositionFeeder=0;
}


void debugVariables(){
  //Serial.println(foodWeight);
  
  /*
  Serial.print("isSaved= ");
  Serial.print(isSaved);
  Serial.print("  menuMode= ");
  Serial.print(menuMode);
  Serial.print("  menuSelectionOption= ");
  Serial.print(menuSelectionOption);
  Serial.print("  menuLevel1= ");
  Serial.print(menuLevel1);
  Serial.print("  menuLevel2= ");
  Serial.print(menuLevel2);
  Serial.print("  setTimePositionFilter = ");
  Serial.print(setTimePositionFilter);
  //Serial.print("  setTimePositionFeeder= ");
  //Serial.print(setTimePositionFeeder);
  Serial.println();
  /*/
}

void changeScreens(){


  debugVariables();
  

  //Desktop
  if(menuMode==0 && menuSelectionOption==0 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    printHomeScreen();

    
  }

  //------------------------------------------Settings List-------------------------------------------------//
 
  //Settings menu 1 list
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    printMainMenu1();
  }


  //settings menu 2 list
  if(menuMode==1 && menuSelectionOption==2 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    printMainMenu2();
  }  


  //settings menu 3 list
  if(menuMode==1 && menuSelectionOption==3 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    printMainMenu3();
  } 

  //------------------------------------------Settings Sub list//

  //-------------Food Per Serving
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==1 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    printSettingsFoodServing();
  }

  

  //-------------Set time settings
  //set time hr
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==1 && setTimePositionFeeder==0){
    printSettingSetTime();
  }  

  //set time min
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==2 && setTimePositionFeeder==0){
    printSettingSetTime();
  } 

  //set time sec
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==3 && setTimePositionFeeder==0){
    printSettingSetTime();
  } 

  //set time meridiem
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==4 && setTimePositionFeeder==0){
    printSettingSetTime();
  }

  
  //-------------Set date settings
  //set date day
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==3 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0 &&  setDatePositionSettings == 1){
    printSettingSetDate();
  }

  //set date month
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==3 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0 &&  setDatePositionSettings == 2){
    printSettingSetDate();
  }

  //set date year
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==3 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0 &&  setDatePositionSettings == 3){
    printSettingSetDate();
  }

  //-------------Set minimum Temp
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==4 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    printSettingMinimumTempurature();
  }
  
  //-------------Set Heater off Temp
  if(menuMode==1 && menuSelectionOption==1 && menuLevel1==5 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    printSettingHeaterOffTempurature();
  }

  
  //------------------------------------------Filter Schedule Sub list//

  //------------Filter Slot Selection

  //Slot 1
  if(menuMode==1 && menuSelectionOption==2 && menuLevel1==1 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    printFilterSlot1Menu();
  }
  //Slot 2
  if(menuMode==1 && menuSelectionOption==2 && menuLevel1==2 && menuLevel2==0 ){
    printFilterSlot2Menu();
  }
  //Slot 3
  if(menuMode==1 && menuSelectionOption==2 && menuLevel1==3 && menuLevel2==0 ){
    printFilterSlot3Menu();
  }
  //Slot 4
  if(menuMode==1 && menuSelectionOption==2 && menuLevel1==4 && menuLevel2==0 ){
    printFilterSlot4Menu();
  }
  //Slot 5
  if(menuMode==1 && menuSelectionOption==2 && menuLevel1==5 && menuLevel2==0 ){
    printFilterSlot5Menu();
  }

  //------------Filter Slot Set time

  if(menuMode==1 && menuSelectionOption==2 && menuLevel1!=0 && menuLevel2==1 && menuLevel3 ==0 && setTimePositionFilter!=0){
    setValueFilterSlot(menuLevel1);
  }

  //------------Filter Slot Active
  if(menuMode==1 && menuSelectionOption==2 && menuLevel1!=0 && menuLevel2!=0 && menuLevel3 ==1 && setTimePositionFilter==0){
    toggleStateFilterSlot(menuLevel2);
  }
  

  //------------------------------------------Feed Schedule Sub list//
  //------------Feed Slot Selection
  //Slot 1
  if(menuMode==1 && menuSelectionOption==3 && menuLevel1==1 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    printFeedSlot1Menu();
  }
  //Slot 2
  if(menuMode==1 && menuSelectionOption==3 && menuLevel1==2 && menuLevel2==0 ){
    printFeedSlot2Menu();
  }
  //Slot 3
  if(menuMode==1 && menuSelectionOption==3 && menuLevel1==3 && menuLevel2==0 ){
    printFeedSlot3Menu();
  }
  //Slot 4
  if(menuMode==1 && menuSelectionOption==3 && menuLevel1==4 && menuLevel2==0 ){
    printFeedSlot4Menu();
  }
  //Slot 5
  if(menuMode==1 && menuSelectionOption==3 && menuLevel1==5 && menuLevel2==0 ){
    printFeedSlot5Menu();
  }

  //------------Feeder Slot Set time

  if(menuMode==1 && menuSelectionOption==3 && menuLevel1!=0 && menuLevel2==1 && menuLevel3 ==0 && setTimePositionFeeder!=0){
    printSetValueFeedSlot(menuLevel1);
  }



}



void buttonActivity(){
  bool btnOkValue = digitalRead(btOk);
  bool btnModeValue = digitalRead(btMode);
  bool btnUpValue = digitalRead(btUp);
  bool btnDownValue = digitalRead(btDown);

  //Desktop
  if( menuMode==0 && menuSelectionOption==0 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){

   //get tempurature sensor readings
    getTempuratureSensorReadings();
    //get weight sensor readings
    getWeightSensorReadings();

    //trigger feeder
    triggerFeeder();
    
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      //do nothin
    }
    if(btnDownValue==LOW){ //Press Down button
      //do nothing
    }
    if(btnModeValue==LOW){ //Press Mode button
      
      delay(500);
      menuMode=1;
      menuSelectionOption=1;
    }
    if(btnOkValue==LOW){  //Press Ok buton
      //do nothing
    }
  }

  /*------------------------------------------Settings List-------------------------------------------------*/
 
  //Settings menu 1 list
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
 
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuSelectionOption=3;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuSelectionOption=2;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      delay(500);
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel1 = 1; 
      delay(500);
    }
  }


  //settings menu 2 list
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
 
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuSelectionOption=1;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuSelectionOption=3;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      delay(500);
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel1 = 1; 
      delay(500);
    }
  }  

  
  //settings menu 3 list
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){

    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuSelectionOption=2;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuSelectionOption=1;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
       delay(500);
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel1 = 1; 
      delay(500);
    }
  } 

  //------------------------------------------Settings Sub list//

  //-------------Food Per Serving
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==1 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){

    //Key change
    if(btnUpValue==LOW){ //Press Up button
      intFoodPerServe++;
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      intFoodPerServe--;
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      delay(500);
      menuLevel1=2; 
      setTimePositionSettings=1;
      updateSetClockVariables();

    }
  }
 

  //-------------Set time settings
  //set time hr
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==1 && setTimePositionFeeder==0){
    
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      hr++;
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      hr--;
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionSettings=2;
      delay(500);
    }
  }  

  //set time min
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==2 && setTimePositionFeeder==0){
    
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      min++;
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      min--;
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionSettings=3;
      delay(500);
    }
  } 

  //set time sec
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==3 && setTimePositionFeeder==0){
   
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      sec++;
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      sec--;
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionSettings=4;
      delay(500);
    }
    
  } 

  //set time meridiem
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==4 && setTimePositionFeeder==0){
    
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      meridiemNumberSetTime ++;
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      meridiemNumberSetTime --;
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionSettings=0;
      setDatePositionSettings = 1;
      menuLevel1 = 3;
      delay(500);
    }
  }

  
  //-------------Set date settings
  //set date day
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==3 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0 &&  setDatePositionSettings == 1){
    
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      day++;
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      day--;
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setDatePositionSettings++;
      delay(500);
    }
  }

  //set date month
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==3 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0 &&  setDatePositionSettings == 2){
    
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      month++;
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      month--;
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setDatePositionSettings++;
      delay(500);
    }
  }

  //set date year
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==3 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0 &&  setDatePositionSettings == 3){

    //Key change
    if(btnUpValue==LOW){ //Press Up button
      year++;
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      year--;
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setDatePositionSettings=0;
      menuLevel1 = 4;
      updateRTC();
      delay(500);
    }
  }
  //-------------Set minimum Temp
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==4 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      minimumTemp++;
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      minimumTemp--;
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel1=5;
      //Save all details
      delay(500);
    }
  }

  //-------------Set Heater off Temp
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==5 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      heaterOffTemp++;
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      heaterOffTemp--;
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel1=0;
      //Save all details
      writeEEPROM();
      delay(500);
    }
  }


  //------------------------------------------Filter Schedule Sub list//

  //------------Filter Slot Selection

  //Slot 1
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1==1 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuLevel1=5;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuLevel1=2;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel2=1;
      setTimePositionFilter=1;
      delay(500);
    }
  }

  //Slot 2
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuLevel1=1;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuLevel1=3;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel2=1;
      setTimePositionFilter=1;
      delay(500);
    }
  }

  //Slot 3
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1==3 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuLevel1=2;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuLevel1=4;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel2=1;
      setTimePositionFilter=1;
      delay(500);
    }
  }

  //Slot 4
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1==4 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuLevel1=3;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuLevel1=5;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel2=1;
      setTimePositionFilter=1;
      delay(500);
    }
  }

  //Slot 5
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1==5 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuLevel1=4;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuLevel1=1;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel2=1;
      setTimePositionFilter=1;
      delay(500);
    }
  }

  //------------Filter Slot Set time
  
  //From hh 
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1!=0 && menuLevel2==1 && setTimePositionFilter==1){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      filterSlotHourFrom[menuLevel1-1]= filterSlotHourFrom[menuLevel1-1]+1;      
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      filterSlotHourFrom[menuLevel1-1]= filterSlotHourFrom[menuLevel1-1]-1;   
      delay(200);  
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionFilter =2;
      delay(500);
    }
  }
  
  //From min
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1!=0 && menuLevel2==1 && setTimePositionFilter==2){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      filterSlotMinFrom[menuLevel1-1] = filterSlotMinFrom[menuLevel1-1]+1;      
      delay(200);
    }
    if(btnDownValue==LOW){ //Press Down button
      filterSlotMinFrom[menuLevel1-1] = filterSlotMinFrom[menuLevel1-1]-1;    
      delay(200); 
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionFilter =3;
      delay(500);
    }
  }


  //From Meridiem
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1!=0 && menuLevel2==1 && setTimePositionFilter==3){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      filterSlotsMeridiemFrom[menuLevel1-1] = filterSlotsMeridiemFrom[menuLevel1-1]+1;     
      delay(200); 
    }
    if(btnDownValue==LOW){ //Press Down button
      filterSlotsMeridiemFrom[menuLevel1-1] = filterSlotsMeridiemFrom[menuLevel1-1]-1; 
      delay(200);    
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionFilter =4;
      delay(500);
    }
  }


  //to hr
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1!=0 && menuLevel2==1 && setTimePositionFilter==4){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      filterSlotHourTo[menuLevel1-1] = filterSlotHourTo[menuLevel1-1]+1;  
      delay(200);    
    }
    if(btnDownValue==LOW){ //Press Down button
      filterSlotHourTo[menuLevel1-1] = filterSlotHourTo[menuLevel1-1]-1;  
      delay(200);   
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionFilter =5;
      delay(500);
    }
  }


  //to min
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1!=0 && menuLevel2==1 && setTimePositionFilter==5){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      filterSlotMinTo[menuLevel1-1] = filterSlotMinTo[menuLevel1-1]+1;    
      delay(200);  
    }
    if(btnDownValue==LOW){ //Press Down button
      filterSlotMinTo[menuLevel1-1] = filterSlotMinTo[menuLevel1-1]-1;     
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionFilter = 6;
      delay(500);
    }
  }

  //to meridian
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1!=0 && menuLevel2==1 && setTimePositionFilter==6){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      filterSlotsMeridiemTo[menuLevel1-1] = filterSlotsMeridiemTo[menuLevel1-1]+1;    
      delay(200);  
    }
    if(btnDownValue==LOW){ //Press Down button
      filterSlotsMeridiemTo[menuLevel1-1] = filterSlotsMeridiemTo[menuLevel1-1]-1;   
      delay(200);  
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionFilter = 0;
      menuLevel2 = menuLevel1;
      menuLevel3 = 1;
      delay(500);
    }
  }
 
  //------------Filter Slot Active
  else if(menuMode==1 && menuSelectionOption==2 && menuLevel1!=0 && menuLevel2!=0 && menuLevel3 ==1 && setTimePositionFilter==0){
    if(btnUpValue==LOW){ //Press Up button
      filterSlotsStatus[menuLevel2-1] = filterSlotsStatus[menuLevel2-1]+1;  
      delay(200);    
    }
    if(btnDownValue==LOW){ //Press Down button
      filterSlotsStatus[menuLevel2-1] = filterSlotsStatus[menuLevel2-1]-1; 
      delay(200);    
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      writeEEPROM(); //save
      menuLevel1 = 0;
      menuLevel2 = 0;
      menuLevel3 = 0;
      delay(500);
    }
  }




  //------------------------------------------Feed Schedule Sub list//

  //------------Filter Slot Selection

  //Slot 1
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1==1 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuLevel1=5;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuLevel1=2;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel2=1;
      setTimePositionFeeder=1;
      delay(500);
    }
  }

  //Slot 2
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuLevel1=1;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuLevel1=3;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel2=1;
      setTimePositionFeeder=1;
      delay(500);
    }
  }

  //Slot 3
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1==3 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuLevel1=2;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuLevel1=4;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel2=1;
      setTimePositionFeeder=1;
      delay(500);
    }
  }

  //Slot 4
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1==4 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuLevel1=3;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuLevel1=5;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel2=1;
      setTimePositionFeeder=1;
      delay(500);
    }
  }

  //Slot 5
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1==5 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuLevel1=4;
      delay(500);
    }
    if(btnDownValue==LOW){ //Press Down button
      menuLevel1=1;
      delay(500);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel2=1;
      setTimePositionFeeder=1;
      delay(500);
    }
  }

  //------------Feeder Slot Set time
  
  //From hh 
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1!=0 && menuLevel2==1 && setTimePositionFeeder==1){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      feedSlotHour[menuLevel1-1]= feedSlotHour[menuLevel1-1]+1;
      delay(200);      
    }
    if(btnDownValue==LOW){ //Press Down button
      feedSlotHour[menuLevel1-1]= feedSlotHour[menuLevel1-1]-1;   
      delay(200);  
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionFeeder =2;
      delay(500);
    }
  }
  
  //From min
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1!=0 && menuLevel2==1 && setTimePositionFeeder==2){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      feedSlotMin[menuLevel1-1] = feedSlotMin[menuLevel1-1]+1;   
      delay(200);   
    }
    if(btnDownValue==LOW){ //Press Down button
      feedSlotMin[menuLevel1-1] = feedSlotMin[menuLevel1-1]-1;    
      delay(200); 
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionFeeder =3;
      delay(500);
    }
  }


  //From Meridiem
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1!=0 && menuLevel2==1 && setTimePositionFeeder==3){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      feedSlotsMeridiem[menuLevel1-1] = feedSlotsMeridiem[menuLevel1-1]+1;  
      delay(200);    
    }
    if(btnDownValue==LOW){ //Press Down button
      feedSlotsMeridiem[menuLevel1-1] = feedSlotsMeridiem[menuLevel1-1]-1;     
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setTimePositionFeeder =4;
      delay(500);
    }
  }

  //feeder status
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1!=0 && menuLevel2==1 && setTimePositionFeeder==4){
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      feedSlotsStatus[menuLevel1-1] = feedSlotsStatus[menuLevel1-1]+1; 
      delay(200);     
    }
    if(btnDownValue==LOW){ //Press Down button
      feedSlotsStatus[menuLevel1-1] = feedSlotsStatus[menuLevel1-1]-1;     
      delay(200);
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
    
      setTimePositionFeeder =0;
      menuLevel1=0;
      menuLevel2=0;
      writeEEPROM(); //save
      delay(500);
    }
  }
  
}



/*-------------------- Altrenative method for delaying (Millis)-----------------*/
bool delayMillis(int previousMillis , int interval){
  
  unsigned long currentMillis = millis();
  
  if(currentMillis - previousMillis >= interval ){
    return true;
  }else{
    return false;
  }
  
}



//method to toggle blinkMode variable value
void toggleBlinkModeValue(){
  if(delayMillis(previousTimeBlink,blinkInterval)){

    if(blinkMode==true){
      blinkMode=false;
    }else{
      blinkMode=true;
    }
    previousTimeBlink = millis();
  } 
}



//----------------------------------------------------RTC module functions---------------------------------------
void rtcUpdateWithComputer(){
  if (! rtc.begin()) 
  {
    lcd.print("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) 
  {
    lcd.print("RTC is NOT running!");
  }
  
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//auto update from computer time
    //;// to set the time manualy 
}


void updateRTCVariables(){
    DateTime now = rtc.now();
    rtcHr = now.hour();
    rtcMin = now.minute();
    rtcSec = now.second();

    rtcDay = now.day();
    rtcMonth = now.month();
    rtcYear = (now.year()-2000);
    
    //Serial.println(rtcYear);
}

void updateSetClockVariables(){

  if(rtcHr>12){
    hr = rtcHr-12;
  }else{
    hr = rtcHr; 
  }
  
  min = rtcMin; 
  sec = rtcSec; 
  day = rtcDay; 
  month = rtcMonth; 
  year= rtcYear; 

  if(rtcHr>12){
    meridiemNumberSetTime=1;
  }else{
    meridiemNumberSetTime=0;
  } 
  
}

void updateRTC(){
  int fullYear = 2000+year;
  int fullHr =0;
  if(meridiemNumberSetTime==1){
    fullHr = 12+hr;
  }else{
    fullHr = hr;
  } 
  rtc.adjust(DateTime(fullYear, month, day, fullHr, min, sec));
}


/*--------------------------------------------------  Triggering methods--------------------------------------*/

/*-------------------Trigger feeder------------*/
void triggerFeeder(){
  
  //Serial.print("remainings = ");
  //Serial.print(remainings);
  //Serial.print("      prevoiusQty = ");
  //Serial.print(prevoiusQty);
  //Serial.print("      isMunutePassed = ");
  //Serial.print(isMinutePassed);
  //Serial.print("      prevoiusQty>remainings = ");
  //Serial.print(prevoiusQty>remainings);
  //Serial.print("      isMotorOn = ");
  //Serial.print(isMotorOn);
  //Serial.println();
  //delay(200);

// && digitalRead(motorPin)==LOW
  if(isFeederTimeArrive() && isMotorOn==false && isMinutePassed==true){
    
    previousTimeFeeder = millis();
    isMinutePassed = false;

    prevoiusQty = remainings;
    //digitalWrite(motorPin,HIGH);
    isMotorOn=true;
  }

  if(isMotorOn==true && prevoiusQty>remainings){

    //digitalWrite(motorPin,LOW);
    isMotorOn=false;

  }

  if(delayMillis(previousTimeFeeder , blinkIntervalFeeder)){
    isMinutePassed = true;
  }

  triggerMotor(); //trigger motor due to the value
}

//method to trigger motor on and off
void triggerMotor(){
  if(isMotorOn==true){
    Serial.println("here");
    runServo();
  }
  
}




//trigger feeder sub method

bool isFeederTimeArrive(){

  //getting values to compare
  int hr=0;
  int min = rtcMin;
  int meridean = 0;

  bool isFound=false;

  if(rtcHr==0){
    hr = 12;
  }
  else if(rtcHr>12){
    hr = rtcHr-12;
  }else{hr = rtcHr;}
  
  if(rtcHr>12){
    meridean=1;
  }else{meridean=0;}

  for(int i = 0; i<5 ; i++){

    if(feedSlotHour[i]==hr && feedSlotMin[i]==min && feedSlotsMeridiem[i]==meridean && feedSlotsStatus[i]==1){
      isFound=true;
      break;
    }
  }

  return isFound;
}


/*-------------------TriggerHeater------------*/
void triggerHeater(){


  if(heatValueC<=minimumTemp   ){
    
    isHeaterOn = true;
    digitalWrite(relayHeater,HIGH);
    
  }else if((heatValueC>minimumTemp  &&  heatValueC<heaterOffTemp  && isHeaterOn==true)){
    isHeaterOn = true;
    digitalWrite(relayHeater,HIGH);
    
  }
  else if((heatValueC>minimumTemp  &&  heatValueC<heaterOffTemp  && isHeaterOn==false)){
    isHeaterOn = false;
    digitalWrite(relayHeater,LOW);
  }
  else if(heatValueC>=heaterOffTemp){
    isHeaterOn = false;
    digitalWrite(relayHeater,LOW);
  }
}

/*-------------------Trigger filter------------*/


//trigger filter sub methods

void triggerFilter(){
  changeFilterVariables();
  if(isFilterOn==true){
    digitalWrite(relayFilter,HIGH);
  }else if(isFilterOn==false){
    digitalWrite(relayFilter,LOW);
  }
}


void changeFilterVariables(){

  if(isFilterOnTimeArrive() && isFilterOn==false){
    isFilterOn = true;
  }
  
  if(isFilterOffTimeArrive() && isFilterOn==true){
    isFilterOn = false;
  }
}

bool isFilterOnTimeArrive(){

  //----getting values from rtc to compare
  int hr=0;
  int min = rtcMin;
  int meridean = 0;
  
  bool isFound=false;

  if(rtcHr==0){
    hr = 12;
  }
  else if(rtcHr>12){
    hr = rtcHr-12;
  }else{hr = rtcHr;}
  
  if(rtcHr>12){
    meridean=1;
  }else{meridean=0;}
  //End of getting values from rtc to compare----


  for(int i = 0; i<5 ; i++){

    if(filterSlotHourFrom[i]==hr && filterSlotMinFrom[i]==min && filterSlotsMeridiemFrom[i]==meridean && filterSlotsStatus[i]==1){
      
      isFound=true;
      break;
    }
  }

  return isFound;
  
}


bool isFilterOffTimeArrive(){

  //----getting values from rtc to compare
  int hr=0;
  int min = rtcMin;
  int meridean = 0;
  
  bool isFound=false;

  if(rtcHr==0){
    hr = 12;
  }
  else if(rtcHr>12){
    hr = rtcHr-12;
  }else{hr = rtcHr;}
  
  if(rtcHr>12){
    meridean=1;
  }else{meridean=0;}
  //End of getting values from rtc to compare----


  for(int i = 0; i<5 ; i++){

    if(filterSlotHourTo[i]==hr && filterSlotMinTo[i]==min && filterSlotsMeridiemTo[i]==meridean && filterSlotsStatus[i]==1){
      isFound=true;
      break;
    }
  }

  return isFound;
}



/*--------------------- Trigger Servo -----------------------------*/
void runServo(){
  
  servo.write(90);
  delay(500);
  servo.write(3);
  delay(500);
}


/*--------------------- Trigger buzzer -----------------------------*/
void triggerBuzzer(){
  if(remainings<6 && isMotorOn){
    playBuzzer();
  }
}

void playBuzzer(){
  tone(buzzer, 1000); // Send 1KHz sound signal...
  delay(1000);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec
}


/*----------------- Intrurrupt functions---------------*/

void innterruptModeButton(){
  
  if(menuMode==0 && menuSelectionOption==0 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){ //it means the main screen
    if(digitalRead(btMode)==LOW){ //Press Mode button          
      menuMode=1;
      menuSelectionOption=1;
    }
  }else{
    clearAllScreenVariables();
  }
}









/*---------------------Validation part---------------------*/

void validation(){
  //intFoodPerServe

  //food weight per serve
  if(intFoodPerServe<0){intFoodPerServe = 1000;}else if(intFoodPerServe>1000){intFoodPerServe = 0;}

  //set time variables
  if(hr<1){hr = 12;}else if(hr>12){hr = 1;}
  if(min<0){min = 59;}else if(min>59){min = 0;}
  if(sec<0){sec = 59;}else if(sec>59){sec = 0;}
  if(meridiemNumberSetTime<0){meridiemNumberSetTime = 1;}else if(meridiemNumberSetTime>1){meridiemNumberSetTime = 0;}

  //set date variables
  if(day<1){day = 31;}else if(day>31){day = 1;}
  if(month<1){month= 12;}else if(month>12){month = 1;}
  if(year<21){year= 40;}else if(year>40){ year = 21;}


  //remainings
  if(remainings<0){
    remainings=0;
  }


  //validating arrays
  for(int i = 0; i<5 ; i++){

    //feeder array validation
    if(feedSlotHour[i]<1){feedSlotHour[i]=12;}else if(feedSlotHour[i]>12){feedSlotHour[i]=1;}
    if(feedSlotsStatus[i]<0){feedSlotsStatus[i]=1;}else if(feedSlotsStatus[i]>1){feedSlotsStatus[i]=0;}
    if(feedSlotMin[i]<0){feedSlotMin[i]=59;}else if(feedSlotMin[i]>59){feedSlotMin[i]=0;}
    if(feedSlotsMeridiem[i]<0){feedSlotsMeridiem[i]=1;}else if(feedSlotsMeridiem[i]>1){feedSlotsMeridiem[i]=0;}

    //filter array validation
    if(filterSlotsStatus[i]<0){filterSlotsStatus[i]=1;}else if(filterSlotsStatus[i]>1){filterSlotsStatus[i]=0;}
    
    if(filterSlotHourFrom[i]<1){filterSlotHourFrom[i]=12;}else if(filterSlotHourFrom[i]>12){filterSlotHourFrom[i]=1;}
    if(filterSlotMinFrom[i]<0){filterSlotMinFrom[i]=59;}else if(filterSlotMinFrom[i]>59){filterSlotMinFrom[i]=0;}
    if(filterSlotsMeridiemFrom[i]<0){filterSlotsMeridiemFrom[i]=1;}else if(filterSlotsMeridiemFrom[i]>1){filterSlotsMeridiemFrom[i]=0;}

    if(filterSlotHourTo[i]<1){filterSlotHourTo[i]=12;}else if(filterSlotHourTo[i]>12){filterSlotHourTo[i]=1;}
    if(filterSlotMinTo[i]<0){filterSlotMinTo[i]=59;}else if(filterSlotMinTo[i]>59){filterSlotMinTo[i]=0;}
    if(filterSlotsMeridiemTo[i]<0){filterSlotsMeridiemTo[i]=1;}else if(filterSlotsMeridiemTo[i]>1){filterSlotsMeridiemTo[i]=0;}
  } 



  
}
