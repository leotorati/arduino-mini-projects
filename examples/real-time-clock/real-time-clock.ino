#include "DS3232RTC.h" // https://github.com/JChristensen/DS3232RTC
#include "DHT.h"       // https://github.com/adafruit/DHT-sensor-library
// DHT lib require: https://github.com/adafruit/Adafruit_Sensor
#include <LiquidCrystal_I2C.h> // https://github.com/johnrickman/LiquidCrystal_I2C

// #define RTC_ADDR 0x68
// #define LCD_ADDR 0x3F

#define DHTPIN A1
#define DHTTYPE DHT11

const int LCD_addr = 0x3F;
const int LCD_chars = 16;
const int LCD_lines = 2;
LiquidCrystal_I2C lcd(LCD_addr, LCD_chars, LCD_lines);

DHT dht(DHTPIN, DHTTYPE);

void enterSleep(void)
{
    /**
     * TODO:
     * - Set wake up by interrupt pin
     * - Add an alarm in the RTC to send signal in via SQW pin
     * - see: https://thekurks.net/blog/2018/2/5/wakeup-rtc-datalogger
     */

    // set_sleep_mode(SLEEP_MODE_IDLE);
    //sleep_enable();

    /* Disable all of the unused peripherals. This will reduce power
    * consumption further and, more importantly, some of these
    * peripherals may generate interrupts that will wake our Arduino from
    * sleep!
    */
    // power_adc_disable();
    // power_spi_disable();
    // power_timer0_disable();
    // power_timer2_disable();
    // power_twi_disable();  

    /* Now enter sleep mode. */
    // sleep_mode();
    
    /* The program will continue from here after the timer timeout*/
    // sleep_disable(); /* First thing to do is disable sleep. */
    
    /* Re-enable the peripherals. */
    // power_all_enable();
}

void setup() {
    Serial.begin(9600);
    dht.begin();

    lcd.init();
    lcd.clear();
    
    // Print a message to the LCD.
    lcd.backlight();
        
    // set the cursor and write some text
    lcd.setCursor(0,0);
    lcd.print("Time: "); 
    lcd.print("Temp: "); 

    setSyncProvider(RTC.get);   // the function to get the time from the RTC

    if(timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");

    /*time_t t;
    tmElements_t tm;

    tm.Year = CalendarYrToTm(2019);
    tm.Month = 7;
    tm.Day = 12;
    tm.Hour = 18;
    tm.Minute = 9;
    tm.Second = 0;
    t = makeTime(tm);
    RTC.set(t);        // use the time_t value to ensure correct weekday is set
    setTime(t);*/
}

void loop()
{
    updateLcd(getCurrentTemp(), getCurrentTime());
    delay(30000);
    // enterSleep();
}

void updateLcd(String line1, String line2)
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(line1);
    lcd.setCursor(0,1);
    lcd.print(line2);
}

String getCurrentTemp()
{
    // A leitura da temperatura e umidade pode levar 250ms!
    // O atraso do sensor pode chegar a 2 segundos.
    int h = dht.readHumidity();
    int t = dht.readTemperature();

    if (isnan(t) || isnan(h)) {
        return "x";
    }
    
    return String("") + "Temp: " + t + "o - " + h + '%';
}

String getCurrentTime()
{
    return String("") + hour() + ":" + minute();
}
