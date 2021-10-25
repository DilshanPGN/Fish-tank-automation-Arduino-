
#include <LiquidCrystal_I2C.h>


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

//Weight Sensor 
#define weightDT A0
#define weightSCK A1

/*------------------------------------------Variables---------------------------------*/

int intFoodPerServe=1234;

//set clock
int hr = 1, min = 30, sec = 55, day = 0, month = 0, year=0 ,  meridiemNumberSetTime=1;  // hour , minute , second ,date , month ,year
char *meridiemName[2] = { "AM", "PM"};

//Heat sensor
int minimumTemp=5;
int heaterOffTemp=15;

//Filter slots active = 1 , deactive = 0
int filterSlotsStatus[5] = {0,1,0,0,0};

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


/* Inturupts

*keep it short
*Don't use delay()
*Don't use serial prints
*Make variables shared with the main code volatile

*/


void setup()
{
  Serial.begin(9600);
  lcd.init(); 
  lcd.backlight();
  
  pinMode(btOk,INPUT_PULLUP);
  pinMode(btUp,INPUT_PULLUP);
  pinMode(btDown,INPUT_PULLUP); 
  pinMode(btMode,INPUT_PULLUP);

  //External inturrupts
  //attachInterrupt(digitalPinToInterrupt(btUp),pressUp,LOW);
  //attachInterrupt(digitalPinToInterrupt(btDown),pressDown,LOW);
  
  

  
  
  
}
void loop()
{
  printCalibrateScale();
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