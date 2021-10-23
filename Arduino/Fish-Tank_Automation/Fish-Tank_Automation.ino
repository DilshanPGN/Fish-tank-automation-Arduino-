#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,16,2); 


//defining buttons

#define bt_Options   A0
#define bt_up     A1
#define bt_down   A2



void setup()
{
  lcd.init(); 
  lcd.backlight();
  
  lcd.print("Hello, world!");
  delay(1500);
  lcd.clear();
}
void loop()
{
 
}