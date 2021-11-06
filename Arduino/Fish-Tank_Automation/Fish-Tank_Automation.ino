#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>



LiquidCrystal_I2C lcd(0x20,16,2); 

//defining buttons
#define btOk   5
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




//----------------------- Variable for saving value from EEPROM --------------------------
int intFoodPerServe=1234;

//set clock
int hr = 1, min = 30, sec = 55, day = 0, month = 0, year=0 ,  meridiemNumberSetTime=1;  // hour , minute , second ,date , month ,year
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
  //attachInterrupt(digitalPinToInterrupt(btUp),pressUp,LOW);
  //attachInterrupt(digitalPinToInterrupt(btDown),pressDown,LOW);
  
  
  readEEPROM();
  
  lcd.print("Welcome");
  lcd.clear();

  //Starting Heat sensor
  tempuratureSensorSetup();

  
  
}
void loop()
{
  //getWeightSensorReadings();
  getTempuratureSensorReadings();
  
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

/******************Main Menu*************************/
void printMainMenu1(){
  
  lcd.setCursor(0,0);
  lcd.print("Settings       >");
  lcd.setCursor(0,1);
  lcd.print("Feed Schedule   ");
}
void printMainMenu2(){
  
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
  lcd.print(getDigit(hr,1));
  lcd.print(getDigit(hr,0)); //hour
  lcd.print("."); 
  lcd.print(getDigit(min,1));
  lcd.print(getDigit(min,0)); //minute
  lcd.print(".");
  lcd.print(getDigit(sec,1));
  lcd.print(getDigit(sec,0)); //second
  lcd.print(meridiemName[meridiemNumberSetTime]); //AM / PM
  lcd.print("      ");
}
void printSettingMinimumTempurature(){
  lcd.setCursor(0,0);
  lcd.print("Set Minimum Temp");
  lcd.setCursor(0,1);
  lcd.print(getDigit(minimumTemp,1));
  lcd.print(getDigit(minimumTemp,0));
  lcd.print(" celcius");
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
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  // It responds almost immediately. Let's print out the data
  printTemperature(insideThermometer); // Use a simple function to print out the data
}



/*-----------------------------------------------Tempurature Sensor methods-------------------------------*/
// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress){
 
  float tempC = sensors.getTempC(deviceAddress);
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
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
