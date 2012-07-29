// Code for Adafruit GPS modules using MTK3329/MTK3339 driver
//
// This code shows how to listen to the GPS module in an interrupt
// which allows the program to have more 'freedom' - just parse
// when a new NMEA sentence is available! Then access data when
// desired.
//
// Tested and works great with the Adafruit Ultimate GPS module
// using MTK33x9 chipset
//    ------> http://www.adafruit.com/products/746
// Pick one up today at the Adafruit electronics shop 
// and help support open source hardware & software! -ada
//
// Also pick up an OLED Display at the Adafruit sore!
//    ------> http://www.adafruit.com/products/661
// Sketch by Jeremy Saglimbeni - 07/14/12
//    ------> http://thecustomgeek.com/2012/07/14/really-smalls-gps/
#define OLED_DC 4
#define OLED_CS 2
#define OLED_CLK 5
#define OLED_MOSI 6 // or "data" on the SSD1306
#define OLED_RESET 3
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(8, 9); // soft serial on pins 8 & 9
#define GPSECHO  true
Adafruit_GPS GPS(&mySerial);
boolean usingInterrupt = false;
int mode = 0; // used to select what screen we draw
int left = A5; // left or top button
int mid = A3; // middle button
int right = A4; // right or bottom button
int power = 0; // power flag
int tzhour; // time zone adjusted hour
int tzday; // time zone adjusted day
int fixflag = 0; // if the GPS has a fix or not
int timezone = 5; // enter the time zone! (EST = 5);
int dst = 1; // Is it daylight savings time? 0 for no, 1 for yes.
float maxspeed;
float maxalt;

void setup() {
  if (dst == 1) {
    timezone--;
  }
  pinMode(13, OUTPUT); // LED for button presses
  pinMode(A0, OUTPUT); // connected to the GPS "enabled" pin
  digitalWrite(A0, HIGH); // pull it high! Low is sleep
  pinMode(7, OUTPUT); // This is connected to the VIN of the OLED screen
  digitalWrite(7, HIGH); // turn the screen on!
  pinMode(left, INPUT);
  pinMode(mid, INPUT);
  pinMode(right, INPUT);
  digitalWrite(left, HIGH);
  digitalWrite(mid, HIGH);
  digitalWrite(right, HIGH);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("  Where am I anyway?");
  display.println(" ");
  display.println("  Jeremy Saglimbeni");
  display.print("  thecustomgeek.com");
  display.display();
  delay(1000);
  display.clearDisplay();
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  useInterrupt(true);
  delay(1000);
}

SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  if (GPSECHO)
    if (c) UDR0 = c;  
}

void useInterrupt(boolean v) {
  if (v) {
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } 
  else {
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

uint32_t timer = millis();

void loop() {
  if (digitalRead(mid) == LOW) {
    if (mode == 7) {
      maxspeed = GPS.speed * 1.150779;
    }
    if (mode == 8) {
      maxalt = GPS.altitude;
    }
    if (mode == 9) {
      power++;
      if (power == 2) {
        power = 0;
      }
      if (power == 0) {
        digitalWrite(A0, HIGH);
        digitalWrite(7, HIGH);
        digitalWrite(13, HIGH);
        display.begin(SSD1306_SWITCHCAPVCC);
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 8);
        display.setTextColor(WHITE);
        display.print("I'm Awake!");
        display.display();
        delay(1500);
        digitalWrite(13, LOW);
        display.clearDisplay();
        mode = 0;
      }
      if (power == 1) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 8);
        display.setTextColor(WHITE);
        display.print("Goodnight!");
        display.display();
        delay(1500);
        digitalWrite(A0, LOW);
        digitalWrite(7, LOW);
      }
      delay(250);
    }
  }

  if (digitalRead(left) == LOW) {
    if (mode == 9) {
      return;
    }
    mode++;
    if (mode == 9) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,8);
      display.print("Sleep Mode");
      display.display();
      delay(1000);
      return;
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,8);
    display.print("  Mode ");
    display.print(mode);
    display.display();
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    display.clearDisplay();
  }
  if (digitalRead(right) == LOW) {
    if (power == 1) {
      return;
    }
    if (mode == 0) {
      return;
    }
    mode--;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,8);
    display.print("  Mode ");
    display.print(mode);
    display.display();
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    display.clearDisplay();
  }

  if (! usingInterrupt) {
    char c = GPS.read();
    if (GPSECHO)
      if (c) UDR0 = c;
  }

  if (GPS.newNMEAreceived()) {

    if (!GPS.parse(GPS.lastNMEA()))
      return;
  }

  if (timer > millis())  timer = millis();
  if ((GPS.speed * 1.150779) > maxspeed) {
    maxspeed = GPS.speed * 1.150779;
  }
  if (GPS.altitude > maxalt) {
    maxalt = GPS.altitude;
  }

  if (millis() - timer > 500) {
    timer = millis(); // reset the timer

    if (mode == 0) {
      display.fillRect(0, 0, 127, 8, BLACK);
      display.setCursor(0, 0);
      display.setTextSize(1);
      if (GPS.hour >= 0 && GPS.hour < timezone) {
        tzhour = GPS.hour + 20;
      }
      else {
        tzhour = GPS.hour - timezone;
      }
      if (tzhour < 10) {
        display.print("0");
      }
      display.print(tzhour, DEC); 
      display.print(':');
      if (GPS.minute < 10) {
        display.print("0");
      }
      display.print(GPS.minute, DEC); 
      display.print(':');
      if (GPS.seconds < 10) {
        display.print("0");
      }
      display.print(GPS.seconds, DEC);    

      display.print("   ");
      if (GPS.month < 10) {
        display.print("0");
      }
      display.print(GPS.month, DEC);
      display.print("/");
      if (tzhour >= 20) {
        tzday = GPS.day - 1;
      }
      else {
        tzday = GPS.day;
      }
      if (tzday < 10) {
        display.print("0");
      }
      display.print(tzday, DEC);
      display.print("/");
      display.print("20");
      display.print(GPS.year, DEC);
      display.display();

      if (GPS.fix == 0) {
        display.fillRect(0, 16, 127, 16, BLACK);
        display.setTextSize(1);
        display.setCursor(0, 16);
        display.print("Acquiring Satellites!");
      }
      if (GPS.fix) {
        fixflag = 1;
        display.fillRect(0, 8, 128, 24, BLACK);
        display.setCursor(0, 8);
        display.print("Lat:");
        display.print(GPS.lat);
        display.print(GPS.latitude, 4);
        display.setCursor(0, 16);
        display.print("Lon:");
        display.print(GPS.lon);
        display.print(GPS.longitude, 4);
        display.setCursor(0, 24);
        display.print("Alt:");
        display.print(GPS.altitude * 3.28084);
        display.setCursor(92, 24);
        if (GPS.fix == 1) {
          display.print("Fix:");
          display.print(GPS.satellites);
        }
        if (GPS.fix == 0) {
          display.setTextColor(BLACK, WHITE);
          display.print("NO FIX");
          display.setTextColor(WHITE);
        }
      }
      display.display();
    }

    if (mode == 1) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      if (GPS.fix) {
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Lat:");
        display.print(GPS.lat);
        display.print(GPS.latitude, 4);
        display.setCursor(0, 8);
        display.print("Lon:");
        display.print(GPS.lon);
        display.print(GPS.longitude, 4);
        display.setCursor(0, 16);
        display.print("Speed(MPH):");
        display.print(GPS.speed * 1.150779); //*converts to MPH
        display.setCursor(0, 24);
        display.print("Bearing:");
        display.print(GPS.angle);
        display.setCursor(90, 24);
        display.print("Fix:");
        display.print(GPS.satellites);
      }
      display.display();
    }
    
  if (mode == 3) {
    display.clearDisplay();
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    if (GPS.hour >= 0 && GPS.hour < timezone) {
      tzhour = GPS.hour + 20;
    }
    else {
      tzhour = GPS.hour - timezone;
    }
    if (tzhour < 10) {
      display.print("0");
    }
    display.print(tzhour, DEC); 
    display.print(':');
    if (GPS.minute < 10) {
      display.print("0");
    }
    display.print(GPS.minute, DEC);
    display.display();
  }
  if (mode == 2) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    if (GPS.hour >= 0 && GPS.hour < timezone) {
      tzhour = GPS.hour + 20;
    }
    else {
      tzhour = GPS.hour - timezone;
    }
    if (tzhour < 10) {
      display.print("0");
    }
    display.print(tzhour, DEC); 
    display.print(':');
    if (GPS.minute < 10) {
      display.print("0");
    }
    display.print(GPS.minute, DEC);
    display.print(":");
    if (GPS.seconds < 10) {
      display.print("0");
    }
    display.print(GPS.seconds, DEC);
    display.setCursor(0, 16);
    if (GPS.month < 10) {
      display.print("0");
    }
    display.print(GPS.month, DEC);
    display.print("/");
   if (tzhour >= 20) {
        tzday = GPS.day - 1;
      }
      else {
        tzday = GPS.day;
      }
      if (tzday < 10) {
        display.print("0");
      }
      display.print(tzday, DEC);
    display.print("/");
    display.print("20");
    display.print(GPS.year, DEC);
    display.display();
  }

  if (mode == 4) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print(GPS.speed * 1.150779);
    display.print(" MPH");
    display.setCursor(0, 16);
    display.print(GPS.angle);
    display.print(" DEG");
    display.display();
  }
  if (mode == 9) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("Power Down");
    display.setCursor(0, 16);
    display.print(" Really?");
    display.display();
  }
  if (mode == 5) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print(GPS.speed * 1.150779);
    display.print(" MPH");
    display.setCursor(0, 16);
    display.print(GPS.altitude * 3.28084);
    display.print("Ft");
    display.display();
  }
    
    if (mode == 7) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("Max Speed");
    display.setCursor(0, 16);
    display.print(maxspeed);
    display.print("MPH");
    display.display();
  }
  if (mode == 8) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("Max Alt-Ft");
    display.setCursor(0, 16);
    display.print(maxalt);
    display.display();
  }
  if (mode == 6) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
        display.setTextSize(1);
        display.setCursor(0, 0);
        if (GPS.hour >= 0 && GPS.hour < timezone) {
        tzhour = GPS.hour + 20;
      }
      else {
        tzhour = GPS.hour - timezone;
      }
      if (tzhour < 10) {
        display.print("0");
      }
      display.print(tzhour, DEC); 
      display.print(':');
      if (GPS.minute < 10) {
        display.print("0");
      }
      display.print(GPS.minute, DEC); 
      display.print(':');
      if (GPS.seconds < 10) {
        display.print("0");
      }
      display.print(GPS.seconds, DEC);    

      display.print("   ");
      if (GPS.month < 10) {
        display.print("0");
      }
      display.print(GPS.month, DEC);
      display.print("/");
      if (tzhour >= 20) {
        tzday = GPS.day - 1;
      }
      else {
        tzday = GPS.day;
      }
      if (tzday < 10) {
        display.print("0");
      }
      display.print(tzday, DEC);
      display.print("/");
      display.print("20");
      display.print(GPS.year, DEC);
        display.setCursor(0, 8);
        display.print("Speed(MPH):");
        display.print(GPS.speed * 1.150779); //*converts to MPH
        display.setCursor(0, 16);
        display.print("Alt:");
        display.print(GPS.altitude * 3.28084);
        display.setCursor(0, 24);
        display.print("Bearing:");
        display.print(GPS.angle);
        display.setCursor(90, 24);
        display.print("Fix:");
        display.print(GPS.satellites);
  }
  /*
      else {
        display.fillRect(0, 16, 127, 16, BLACK);
        display.setTextSize(1);
        display.setCursor(0, 16);
        display.print("Acquiring Satellites!");
      }
      */
      display.display();
    }
}
