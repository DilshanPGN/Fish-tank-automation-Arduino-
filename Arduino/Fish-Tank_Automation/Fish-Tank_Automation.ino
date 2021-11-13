#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>



LiquidCrystal_I2C lcd(0x20,16,2); 

//defining buttons
#define btOk     5
#define btMode   4
#define btUp     3
#define btDown   2

//defining relay pins
#define relayFilter 12
#define relayHeater 11

//Buzzer
#define buzzer 13

//Tempurature sensor
#define ONE_WIRE_BUS 6  //pin 6 of arduino

//Value for blinking

bool blinkMode = true;
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

/*----------------------------------------------------------Variables---------------------------------------------*/

//-----------------------Variables to switch screens---------------------------------------------
int menuMode=0; //indicate menu or workingscreen
int menuSelectionOption=0;  //select main menu from list
int menuLevel1=0; //indicates which number of the submenu #1 selected
int menuLevel2=0; //indicates which number of the submenu #2 selected
int setTimePositionSettings =0; //set time position
int setTimePositionFeeder=0; //feeder set time position
int setDatePositionSettings = 0; ////set date position






//-----------------------Eeprom Store Variable (EEP variable adresses )--------------------------

//for intFoodPerServe - xx,xx
uint8_t eep_intFoodPerServeFisrtTwoNumbers = 1;
uint8_t eep_intFoodPerServeLastTwoNumbers = 2;

//set clockVariables
uint8_t eep_hr = 3, eep_min = 4, eep_sec = 5, eep_day = 6, eep_month = 7, eep_year=8 ,  eep_meridiemNumberSetTime=9;

//heat sensor settins
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
uint8_t eep_isSaved = 68;
uint8_t eep_calibratingFactor = 69;




//----------------------- Variable for saving value from EEPROM --------------------------
int intFoodPerServe=1234;
int calibratingFactor;

//set clock
int hr = 1, min = 30, sec = 55, day = 8, month = 11, year=21 ,  meridiemNumberSetTime=1;  // hour , minute , second ,date , month ,year
char *meridiemName[2] = { "AM", "PM"};

//Heat sensor settings
int minimumTemp=5;
int heaterOffTemp=15;


//Filter slots active = 1 , deactive = 0
int filterSlotsStatus[5] = {0,0,0,0,0};
int filterSlotHourFrom[5]={01,01,01,01,01};
int filterSlotHourTo[5]={01,01,01,01,01};
int filterSlotMinFrom[5]={30,30,30,30,30};
int filterSlotMinTo[5]={30,30,30,30,30};
int filterSlotsMeridiemFrom[5] = {0,0,0,0,0};//0=AM , 1=PM
int filterSlotsMeridiemTo[5] = {1,1,1,1,1};//0=AM , 1=PM

//Feed slots active = 1 , deactive = 0
int feedSlotsStatus[5] = {0,1,0,0,0};
int feedSlotHour[5]={01,01,01,01,01};
int feedSlotMin[5]={30,30,30,30,30};
int feedSlotsMeridiem[5] = {0,0,0,0,0};  //0=AM , 1=PM

//Feed AutoCalculation
int feedTimesPerDay = 0;
int firstFeedTimeHour = 0;
int firstFeedTimeMin = 0;
int firstFeedMeridiem = 0;

int manualMode = 0;
int isSaved = 1;


//----------------------- Values from sensors --------------------------

int foodWeight; //inGrams
float heatValueC; //inCelcius



void setup()
{
  Serial.begin(9600);
  lcd.init(); 
  lcd.backlight();
 
  
  pinMode(btOk,INPUT_PULLUP);
  pinMode(btUp,INPUT_PULLUP);
  pinMode(btDown,INPUT_PULLUP); 
  pinMode(btMode,INPUT_PULLUP);
  
  //Weight sensor (Temporary)
  pinMode(weightPin,INPUT);

  //External inturrupts
  //attachInterrupt(digitalPinToInterrupt(btUp),buttonActivity,LOW);
  //attachInterrupt(digitalPinToInterrupt(btDown),buttonActivity,LOW);
  
  //writeEEPROM();
  //readEEPROM();
  
  lcd.print("Welcome");
  lcd.clear();

  //Starting Heat sensor
  tempuratureSensorSetup();

  
  
}
void loop()
{
 buttonActivity();
 changeScreens();
 toggleBlinkModeValue();  
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
  lcd.print("Feed Schedule   ");
}
void printMainMenu2(){ //MenuMode =1 & MenuOption=0

  
  lcd.setCursor(0,0);
  lcd.print("Settings        ");
  lcd.setCursor(0,1);
  lcd.print("Feed Schedule  >");
}
void printMainMenu3(){
  
  lcd.setCursor(0,0);  
  lcd.print("Filter Schedule>");
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
  lcd.setCursor(0,0);
  lcd.print("Slot| ");
  lcd.print(getDigit(filterSlotHourFrom[slotNumber-1],1));
  lcd.print(getDigit(filterSlotHourFrom[slotNumber-1],0));
  lcd.print(":");
  lcd.print(getDigit(filterSlotMinFrom[slotNumber-1],1));
  lcd.print(getDigit(filterSlotMinFrom[slotNumber-1],0));
  if(filterSlotsMeridiemFrom[slotNumber-1]==0){ //AM
  lcd.print("AM to  ");
  }else{
  lcd.print("PM to  ");
  }
  lcd.setCursor(0,1);
  lcd.print("#");
  lcd.print(slotNumber);
  lcd.print("  | ");
  lcd.print(getDigit(filterSlotHourTo[slotNumber-1],1));
  lcd.print(getDigit(filterSlotHourTo[slotNumber-1],0));
  lcd.print(":");
  lcd.print(getDigit(filterSlotMinTo[slotNumber-1],1));
  lcd.print(getDigit(filterSlotMinTo[slotNumber-1],0));
  if(filterSlotsMeridiemTo[slotNumber-1]==0){ //AM
  lcd.print("AM     ");
  }else{
  lcd.print("PM     ");
  }
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

/********Calculate Times Option****/
void printFeedTimesPerDay(){
  lcd.setCursor(0,0);
  lcd.print("Times Per Day:  ");    
  lcd.setCursor(0,1);
  lcd.print(feedTimesPerDay);
  lcd.print(" times          ");
}

void printFeedFirstTime(){
  lcd.setCursor(0,0);
  lcd.print("First Time :    ");
  lcd.setCursor(0,1);
  lcd.print(getDigit(firstFeedTimeHour,1));
  lcd.print(getDigit(firstFeedTimeHour,0));
  lcd.print(":");
  lcd.print(getDigit(firstFeedTimeMin,1));
  lcd.print(getDigit(firstFeedTimeMin,0));
  lcd.print(meridiemName[firstFeedMeridiem]);
  lcd.print("         ");
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
  lcd.print(getDigit(feedSlotHour[slotNumber-1],1));
  lcd.print(getDigit(feedSlotHour[slotNumber-1],0));
  lcd.print(":");
  lcd.print(getDigit(feedSlotMin[slotNumber-1],1));
  lcd.print(getDigit(feedSlotMin[slotNumber-1],0));
  if(feedSlotsMeridiem[slotNumber-1]==0){ //AM
  lcd.print("AM     ");
  }else{
  lcd.print("PM     ");
  }
  lcd.setCursor(0,1);
  lcd.print("#");
  lcd.print(slotNumber);
  lcd.print("  | ");
 
  if(feedSlotsStatus[slotNumber-1]==0){ //AM
  lcd.print("OFF    ");
  }else{
  lcd.print("ACTIVE ");
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


  EEPROM.write(eep_intFoodPerServeFisrtTwoNumbers,intFoodPerServe%100);
  EEPROM.write(eep_intFoodPerServeLastTwoNumbers,intFoodPerServe%100);
  //set clockVariables
  EEPROM.write(eep_hr,hr);
  EEPROM.write(eep_min,min);
  EEPROM.write(eep_sec,sec);
  EEPROM.write(eep_day,day);
  EEPROM.write(eep_month,month);
  EEPROM.write(eep_year,year);
  EEPROM.write(eep_meridiemNumberSetTime,meridiemNumberSetTime);
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
  EEPROM.write(eep_isSaved,isSaved);
}


void readEEPROM(){

  intFoodPerServe = (100*(EEPROM.read(eep_intFoodPerServeFisrtTwoNumbers))+(EEPROM.read(eep_intFoodPerServeLastTwoNumbers)));
  
  
  //set clockVariables
  hr = EEPROM.read(eep_hr);
  min = EEPROM.read(eep_min);
  sec = EEPROM.read(eep_sec);
  day = EEPROM.read(eep_day);
  month = EEPROM.read(eep_month);
  year = EEPROM.read(eep_year);
  meridiemNumberSetTime = EEPROM.read(eep_meridiemNumberSetTime);
  //heat sensor settins
  minimumTemp = EEPROM.read(eep_minimumTemp);
  heaterOffTemp = EEPROM.read(eep_heaterOffTemp);

  //Filter 
  for (int i = eep_filterSlotsStatus[0]; i<=(eep_filterSlotsStatus[4]); i++) {
    filterSlotsStatus[i]= (EEPROM.read(i));
  }
  for (int i = eep_filterSlotHourFrom[0]; i<=eep_filterSlotHourFrom[4]; i++) {
    filterSlotHourFrom[i] = (EEPROM.read(i));
  }
  for (int i = eep_filterSlotHourTo[0]; i<=eep_filterSlotHourTo[4]; i++) {
    filterSlotHourTo[i]=(EEPROM.read(i));
  }
  for (int i = eep_filterSlotMinFrom[0] = 0; i<=eep_filterSlotMinFrom[4]; i++) {
    filterSlotMinFrom[i]=(EEPROM.read(i));
  }
  for (int i = eep_filterSlotMinTo[0]; i<=eep_filterSlotMinTo[4]; i++) {
    filterSlotMinTo[i]=(EEPROM.read(i));
  }
  for (int i = eep_filterSlotsMeridiemFrom[0]; i<=eep_filterSlotsMeridiemFrom[4]; i++) {
    filterSlotsMeridiemFrom[i]=(EEPROM.read(i));
  }
  for (int i = eep_filterSlotsMeridiemTo[0]; i<= eep_filterSlotsMeridiemTo[4]; i++) {
    filterSlotsMeridiemTo[i] = (EEPROM.read(i));
  }
  
  //Feed
  for (int i = eep_feedSlotsStatus[0]; i<= eep_feedSlotsStatus[4]; i++) {
    feedSlotsStatus[i]= (EEPROM.read(i));
  }
  for (int i = eep_feedSlotHour[0]; i<=eep_feedSlotHour[4]; i++) {
    feedSlotHour[i]= (EEPROM.read(i));
  }
  for (int i = eep_feedSlotMin[0]; i<=eep_feedSlotMin[4]; i++) {
    feedSlotMin[i]= (EEPROM.read(i));
  }
  for (int i = eep_feedSlotsMeridiem[0]; i<=eep_feedSlotsMeridiem[4]; i++) {
    feedSlotsMeridiem[i]= (EEPROM.read(i));
  }

  //loading variables
  manualMode=EEPROM.read(eep_manualMode);
  isSaved=EEPROM.read(eep_isSaved);
  calibratingFactor = EEPROM.read(eep_calibratingFactor);
}

/*-----------------------------------------------Get values from Sensors-------------------------------*/
void getWeightSensorReadings(){
  int val = analogRead(weightPin);
  foodWeight = map(val, 0, 1023, minWeight, maxWeight);
  
  Serial.println(foodWeight);
}

void getTempuratureSensorReadings(){
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  //Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  //Serial.println("DONE");
  
  // It responds almost immediately. Let's print out the data
  printTemperature(insideThermometer); // Use a simple function to print out the data
}



/*-----------------------------------------------Tempurature Sensor methods-------------------------------*/
// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress){
 
  float tempC = sensors.getTempC(deviceAddress);
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    //Serial.println("Error: Could not read temperature data");
    return;
  }else{
    //It means there are sensor reading
    if (tempC!=84){
      heatValueC = tempC;
      Serial.print("Temp C: ");
      Serial.print(heatValueC);
    }

  }
  



  
  
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress){
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}


/*-----------------------------------------------Tempurature Sensor setup-------------------------------*/
void tempuratureSensorSetup(){
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");
  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();
  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9);
 
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();
}


void clearAllScreenVariables(){
  menuMode=0;
  menuSelectionOption=0;
  menuLevel1=0;
  menuLevel2=0;
  setTimePositionSettings=0;
  setTimePositionFeeder=0;
}


void debugVariables(){
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
  Serial.print("  setTimePositionSettings= ");
  Serial.print(setTimePositionSettings);
  Serial.print("  setTimePositionFeeder= ");
  Serial.print(setTimePositionFeeder);
  Serial.println();
}

void changeScreens(){


  debugVariables();
  

  //Desktop
  if(isSaved==1 && menuMode==0 && menuSelectionOption==0 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    lcd.setCursor(0,0);
    lcd.print("Desktop         ");
    lcd.setCursor(0,1);
    lcd.print("                ");
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



}








void buttonActivity(){
  bool btnOkValue = digitalRead(btOk);
  bool btnModeValue = digitalRead(btMode);
  bool btnUpValue = digitalRead(btUp);
  bool btnDownValue = digitalRead(btDown);

  //Desktop
  if(isSaved==1 && menuMode==0 && menuSelectionOption==0 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
   

    //Key change
    if(btnUpValue==LOW){ //Press Up button
      //do nothin
    }
    if(btnDownValue==LOW){ //Press Down button
      //do nothing
    }
    if(btnModeValue==LOW){ //Press Mode button
      //do mothing
     
      menuMode=1;
      menuSelectionOption=1;
    }
    if(btnOkValue==LOW){  //Press Ok buton
      delay(500);
    }
  }

  /*------------------------------------------Settings List-------------------------------------------------*/
 
  //Settings menu 1 list
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
 
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuSelectionOption=4;
    }
    if(btnDownValue==LOW){ //Press Down button
      menuSelectionOption=2;
    }
    if(btnModeValue==LOW){ //Press Mode button
      //clearAllScreenVariables();
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
    }
    if(btnDownValue==LOW){ //Press Down button
      menuSelectionOption=3;
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel1 = 2; 
      delay(500);
    }
  }  

  
  //settings menu 3 list
  else if(menuMode==1 && menuSelectionOption==3 && menuLevel1==0 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){

    //Key change
    if(btnUpValue==LOW){ //Press Up button
      menuSelectionOption=2;
    }
    if(btnDownValue==LOW){ //Press Down button
      menuSelectionOption=1;
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel1 = 3; 
      delay(500);
    }
  } 

  //------------------------------------------Settings Sub list//

  //-------------Food Per Serving
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==1 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){

    //Key change
    if(btnUpValue==LOW){ //Press Up button
      intFoodPerServe++;
    }
    if(btnDownValue==LOW){ //Press Down button
      intFoodPerServe--;
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      delay(500);
      menuLevel1=2; 
      setTimePositionSettings=1;
    }
  }

  

  //-------------Set time settings
  //set time hr
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==2 && menuLevel2==0 && setTimePositionSettings==1 && setTimePositionFeeder==0){
    
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      hr++;
    }
    if(btnDownValue==LOW){ //Press Down button
      hr--;
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
    }
    if(btnDownValue==LOW){ //Press Down button
      min--;
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
    }
    if(btnDownValue==LOW){ //Press Down button
      sec--;
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
    }
    if(btnDownValue==LOW){ //Press Down button
      meridiemNumberSetTime --;
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
    }
    if(btnDownValue==LOW){ //Press Down button
      day--;
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
    }
    if(btnDownValue==LOW){ //Press Down button
      month--;
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
    }
    if(btnDownValue==LOW){ //Press Down button
      year--;
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      setDatePositionSettings=0;
      menuLevel1 = 4;
      delay(500);
    }
  }
//-------------Set minimum Temp
  else if(menuMode==1 && menuSelectionOption==1 && menuLevel1==4 && menuLevel2==0 && setTimePositionSettings==0 && setTimePositionFeeder==0){
    
    //Key change
    if(btnUpValue==LOW){ //Press Up button
      minimumTemp++;
    }
    if(btnDownValue==LOW){ //Press Down button
      minimumTemp--;
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
    }
    if(btnDownValue==LOW){ //Press Down button
      heaterOffTemp--;
    }
    if(btnModeValue==LOW){ //Press Mode button
      clearAllScreenVariables();
    }
    if(btnOkValue==LOW){  //Press Ok buton
      menuLevel1=0;
      //Save all details
      
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
