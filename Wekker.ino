/*
  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  =                                                         =
  =          Initialiseren van de bibliotheken              =
  =                                                         =
  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <DS1302.h>

#include <buttonPressed.h>

#include "blinkLCD.h"

const int kCePin   = 12;  // Chip Enable
const int kIoPin   = 11;  // Input/Output
const int kSclkPin = 10;  // Serial Clock

DS1302 rtc(kCePin, kIoPin, kSclkPin);
/*
  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  =                                                         =
  =            Custom characters lcd display                =
  =                                                         =
  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

byte pijlLinks[] = {
  B00000,
  B00100,
  B01100,
  B11111,
  B11111,
  B01100,
  B00100,
  B00000
};

byte pijlRechts[] = {
  B00000,
  B00100,
  B00110,
  B11111,
  B11111,
  B00110,
  B00100,
  B00000
};

byte brightnessLinks[] = {
  B11111,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B11111
};

byte brightnessRechts[] = {
  B11111,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B11111
};

byte brightnessMidden[] = {
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111
};

byte brightnessVol[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

//Bron : https://forum.arduino.cc/index.php?topic=124974.0
template <bool _Flag, typename _True, typename _False> struct If {
  typedef _True Result;
};
template <typename _True, typename _False> struct If<false, _True, _False> {
  typedef _False Result;
};

template< uint64_t _Number >
class BestFitUInt {
  protected:
    enum {
      T64 = _Number >> 0x20,
      T32 = _Number >> 0x10,
      T16 = _Number >> 0x08,
    };
    typedef typename If< T16, uint16_t, uint8_t >::Result TypeB;
    typedef typename If< T32, uint32_t, TypeB >::Result   TypeA;
  private:
  public:
    typedef typename If< T64, uint64_t, TypeA >::Result   Result;
};
#define BFUI(n) BestFitUInt< n >::Result

#define runEvery(t) for (static  BFUI(t) _lasttime;\
                         (BFUI(t))((BFUI(t))millis() - _lasttime) >= (t);\
                         _lasttime += (t))
//Einde bron

const int menuPin = 0;
const int selectPin = 1;
const int minPin = 8;
const int plusPin = 4;
const int snoozePin = 7;

menuButton Menu(menuPin);
menuButton Select(selectPin);
menuButton Min(minPin);
menuButton Plus(plusPin);
menuButton Snooze(snoozePin);

enum menus {M_klok, M_menu1, M_menu2, M_menu3, M_helderheid, M_alarm, M_alarm_aanuit, M_alarm_uur, M_alarm_minuut, M_tijd, M_tijd_uur, M_tijd_minuut, M_tijd_seconde};
menus currentMenu = M_klok;

BlinkLCD blinkAlarm;

bool alarm;
bool alarmGeluid;
bool alarmSnooze;
bool alarmStop;

int alarmSnoozeUur;
int alarmSnoozeMinuut;
int alarmSnoozeInterval = 5; // 5 minuten snooze interval

int alarmUur = 7;
int alarmMinuut = 30;

int tijdUur = 12;
int tijdMinuut = 23;
int tijdSeconde = 22;

int brightness[] = {0, 5, 15, 35, 75, 120, 150, 175, 200, 225, 255};
int brightnessLvl = 10;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Menu.init();
  Select.init();
  Min.init();
  Min.SetStateAndTime(HIGH, 500);
  Plus.init();
  Plus.SetStateAndTime(HIGH, 500);
  Snooze.init();
  Snooze.SetStateAndTime(HIGH, 2000);

  lcd.init();
  lcd.createChar(0, pijlLinks);
  lcd.createChar(1, pijlRechts);
  lcd.createChar(2, brightnessLinks);
  lcd.createChar(3, brightnessRechts);
  lcd.createChar(4, brightnessMidden);
  lcd.createChar(5, brightnessVol);

  lcd.backlight();

  setMenu(currentMenu);

  pinMode(9, OUTPUT);
  analogWrite(9, brightness[brightnessLvl]);

  Serial.begin(9600);
}

void loop() {
  if (Menu.pressed()) {
    actionMenuKnop();
  }
  if (Select.pressed()) {
    actionSelectKnop();
  }
  if (Min.pressed()) {
    actionMinKnop();
  }
  if (Plus.pressed()) {
    actionPlusKnop();
  }
  if (Snooze.pressed()) {
    actionSnoozeKnop();
  }

  updateTime();
  updateBlink();
  updateLongPressMin();
  updateLongPressPlus();
  updateLongPressSnooze();
}

void updateTime() { 
  runEvery(1000) {
    Time t = rtc.time();
    if (currentMenu == M_klok) {
      tijdUur = t.hr;
      tijdMinuut = t.min;
      tijdSeconde = t.sec;
      lcd.setCursor(4, 0);
      lcd.print(formatTime(t.hr));
      lcd.print(":");
      lcd.print(formatTime(t.min));
      lcd.print(":");
      lcd.print(formatTime(t.sec));
    }
    else if (currentMenu == M_tijd) {
      tijdUur = t.hr;
      tijdMinuut = t.min;
      tijdSeconde = t.sec;
  
      String tijd = "   " + formatTime(tijdUur) + ":" + formatTime(tijdMinuut) + ":" + formatTime(tijdSeconde);
      lcd.setCursor(0, 1);
      lcd.print(tijd);
    }
    else if (currentMenu == M_tijd_uur) {
      tijdMinuut = t.min;
      tijdSeconde = t.sec;
  
      String tijd = ":" + formatTime(tijdMinuut) + ":" + formatTime(tijdSeconde);
      lcd.setCursor(5, 1);
      lcd.print(tijd);
    }
    else if (currentMenu == M_tijd_minuut) {
      tijdUur = t.hr;
      tijdSeconde = t.sec;
  
      String tijd1 = formatTime(tijdUur) + ":";
      String tijd2 = ":" + formatTime(tijdSeconde);
      lcd.setCursor(3, 1);
      lcd.print(tijd1);
      lcd.setCursor(8 , 1);
      lcd.print(tijd2);
    }
    else if (currentMenu == M_tijd_seconde) {
      tijdUur = t.hr;
      tijdMinuut = t.min;
  
      String tijd = formatTime(tijdUur) + ":" + formatTime(tijdMinuut) + ":";
      lcd.setCursor(3, 1);
      lcd.print(tijd);
    }
  
    if ((tijdUur == alarmUur && tijdMinuut == alarmMinuut && tijdSeconde == 0) && !alarmGeluid && !alarmSnooze) {
      alarmGeluid = true;
      alarmSnooze = false;
    }
    if ((tijdUur == alarmSnoozeUur && tijdMinuut == alarmSnoozeMinuut) && alarm && alarmSnooze && !alarmStop) {
      alarmGeluid = true;
      alarmSnooze = false;
    }
    if (tijdUur == 0 && tijdMinuut == 0) {
      alarmStop = false;
    }  
    if (alarmGeluid && !alarmSnooze) {
      tone(5, 250, 500);
    }
  }
}

bool longPressBlink = false;

void updateBlink() {
  if ((currentMenu == M_tijd_uur
       || currentMenu == M_tijd_minuut
       || currentMenu == M_tijd_seconde
       || currentMenu == M_alarm_aanuit
       || currentMenu == M_alarm_uur
       || currentMenu == M_alarm_minuut) && !longPressBlink) {
    blinkAlarm.Update();
  }
}

bool lockMin = false;

void recoverBlink() {
  blinkAlarm.SetString(blinkAlarm.m_tekst);
  blinkAlarm.SetStartPosition(blinkAlarm.m_startPosition, blinkAlarm.m_row);
  blinkAlarm.SetLength(blinkAlarm.m_dataLength);
  blinkAlarm.SetBlinkRate(blinkAlarm.m_blinkRate);
}

void updateLongPressMin() {
  if (Min.checkButton(minPin) == HELD) {
    longPressBlink = true;
    if ((uint32_t(Min.GetHeldTime(MICROSECONDS)) % 200) == 0) {
      if ((currentMenu == M_tijd_uur) && !lockMin) {
        subTime(tijdUur, 23);
        lcd.setCursor(3, 1);
        lcd.print(formatTime(tijdUur));
      }
      if ((currentMenu == M_tijd_minuut) && !lockMin) {
        subTime(tijdMinuut, 59);
        lcd.setCursor(6, 1);
        lcd.print(formatTime(tijdMinuut));
      }
      if ((currentMenu == M_tijd_seconde) && !lockMin) {
        subTime(tijdSeconde, 59);
        lcd.setCursor(9, 1);
        lcd.print(formatTime(tijdSeconde));
      }
      if ((currentMenu == M_alarm_uur) && !lockMin) {
        subTime(alarmUur, 23);
        lcd.setCursor(8, 1);
        lcd.print(formatTime(alarmUur));
      }
      if ((currentMenu == M_alarm_minuut) && !lockMin) {
        subTime(alarmMinuut, 59);
        lcd.setCursor(11, 1);
        lcd.print(formatTime(alarmMinuut));
      }
      lockMin = true;
    }
    else {
      lockMin = false;
    }
  }
  else if (digitalRead(minPin) == 0) {
    longPressBlink = false;
  }
  if (Min.checkButton(minPin) == RELEASED) {
    longPressBlink = false;
    recoverBlink();
  }
}

bool lockPlus = false;

void updateLongPressPlus() {
  if (Plus.checkButton(plusPin) == HELD) {
    longPressBlink = true;
    if ((uint32_t(Plus.GetHeldTime(MICROSECONDS)) % 200) == 0) {
      if ((currentMenu == M_tijd_uur) && !lockPlus) {
        addTime(tijdUur, 23);
        lcd.setCursor(3, 1);
        lcd.print(formatTime(tijdUur));
      }
      if ((currentMenu == M_tijd_minuut) && !lockPlus) {
        addTime(tijdMinuut, 59);
        lcd.setCursor(6, 1);
        lcd.print(formatTime(tijdMinuut));
      }
      if ((currentMenu == M_tijd_seconde) && !lockPlus) {
        addTime(tijdSeconde, 59);
        lcd.setCursor(9, 1);
        lcd.print(formatTime(tijdSeconde));
      }
      if ((currentMenu == M_alarm_uur) && !lockMin) {
        addTime(alarmUur, 23);
        lcd.setCursor(8, 1);
        lcd.print(formatTime(alarmUur));
      }
      if ((currentMenu == M_alarm_minuut) && !lockMin) {
        addTime(alarmMinuut, 59);
        lcd.setCursor(11, 1);
        lcd.print(formatTime(alarmMinuut));
      }
      lockPlus = true;
    }
    else {
      lockPlus = false;
    }
  }
  else if (digitalRead(plusPin) == 0) {
    longPressBlink = false;
  }
  if (Plus.checkButton(plusPin) == RELEASED) {
    longPressBlink = false;
    recoverBlink();
  }
}

bool lockSnooze = false;
void updateLongPressSnooze() {
  if (Snooze.checkButton(snoozePin) == HELD) {
    if ((uint32_t(Snooze.GetHeldTime(MICROSECONDS)) % 2000) == 0) {
      if (!lockSnooze) {
        tone(5, 150, 200);        
        alarmSnooze = false;
        alarmGeluid = false;
        alarmStop = true;
      }
      lockSnooze = true;
    }    
  }
  else {
      lockSnooze = false;
  }
}

void addTime(int &getal, int maxG) {
  if (getal < maxG) {
    getal++;
    blinkAlarm.SetString(formatTime(getal));
  }
  else {
    getal = 0;
    blinkAlarm.SetString(formatTime(getal));
  }
}

void subTime(int &getal, int maxG) {
  if (getal > 0) {
    getal--;
    blinkAlarm.SetString(formatTime(getal));
  }
  else {
    getal = maxG;
    blinkAlarm.SetString(formatTime(getal));
  }
}

void setMenu(menus k) {
  Time t = rtc.time();
  String tijd = "   " + formatTime(tijdUur) + ":" + formatTime(tijdMinuut) + ":" + formatTime(tijdSeconde);
  switch (k) {
    case M_klok:
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print(formatTime(t.hr));
      lcd.print(":");
      lcd.print(formatTime(t.min));
      lcd.print(":");
      lcd.print(formatTime(t.sec));
      lcd.setCursor(0, 1);
      checkAlarm();
      break;
    case M_menu1:
      lcd.clear();
      lcd.write(1);
      lcd.print(" Alarm instellen");
      lcd.setCursor(0, 1);
      lcd.print("  Tijd instellen");
      break;
    case M_menu2:
      lcd.clear();
      lcd.print("  Alarm instellen");
      lcd.setCursor(0, 1);
      lcd.write(1);
      lcd.print(" Tijd instellen");
      break;
    case M_menu3:
      lcd.clear();
      lcd.print("  Tijd instellen");
      lcd.setCursor(0, 1);
      lcd.write(1);
      lcd.print(" Helderheid");
      break;
    case M_helderheid:
      lcd.clear();
      lcd.write(0);
      lcd.print(" Helderheid");
      brightnessLevel();
      break;
    case M_tijd:
      lcd.clear();
      lcd.write(0);
      lcd.print(" Tijd :");
      lcd.setCursor(0, 1);
      lcd.print(tijd);
      break;
    case M_tijd_uur:
      lcd.clear();
      lcd.write(0);
      lcd.print(" Tijd :");
      lcd.setCursor(0, 1);
      lcd.print(tijd);
      lcd.setCursor(0, 0);
      blinkAlarm.SetString(formatTime(tijdUur));
      blinkAlarm.SetStartPosition(3, 1);
      blinkAlarm.SetLength(2);
      blinkAlarm.SetBlinkRate(300);
      break;
    case M_tijd_minuut:
      lcd.clear();
      lcd.write(0);
      lcd.print(" Tijd :");
      lcd.setCursor(0, 1);
      lcd.print(tijd);
      lcd.setCursor(0, 0);
      blinkAlarm.SetString(formatTime(tijdMinuut));
      blinkAlarm.SetStartPosition(6, 1);
      blinkAlarm.SetLength(2);
      blinkAlarm.SetBlinkRate(300);
      break;
    case M_tijd_seconde:
      lcd.clear();
      lcd.write(0);
      lcd.print(" Tijd :");
      lcd.setCursor(0, 1);
      lcd.print(tijd);
      lcd.setCursor(0, 0);
      blinkAlarm.SetString(formatTime(tijdSeconde));
      blinkAlarm.SetStartPosition(9, 1);
      blinkAlarm.SetLength(2);
      blinkAlarm.SetBlinkRate(300);
      break;
    case M_alarm:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.write(0);
      lcd.print(" Alarm: " + formatBool(alarm));
      lcd.setCursor(0, 1);
      lcd.print("  Tijd: ");
      lcd.print(formatTime(alarmUur));
      lcd.print(":");
      lcd.print(formatTime(alarmMinuut));
      break;
    case M_alarm_aanuit:
      lcd.setCursor(2, 0);
      lcd.print("Alarm:");
      blinkAlarm.SetString(formatBool(alarm));
      blinkAlarm.SetStartPosition(9, 0);
      blinkAlarm.SetLength(3);
      blinkAlarm.SetBlinkRate(300);
      break;
    case M_alarm_uur:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.write(0);
      lcd.print(" Alarm: " + formatBool(alarm));
      lcd.setCursor(0, 1);
      lcd.print("  Tijd: ");
      blinkAlarm.SetString(formatTime(alarmUur));
      blinkAlarm.SetStartPosition(8, 1);
      blinkAlarm.SetLength(2);
      lcd.setCursor(10, 1);
      lcd.print(":" + formatTime(alarmMinuut));
      break;
    case M_alarm_minuut:
      lcd.setCursor(0, 1);
      lcd.print("  Tijd: " + formatTime(alarmUur) + ":");
      blinkAlarm.SetString(formatTime(alarmMinuut));
      blinkAlarm.SetStartPosition(11, 1);
      blinkAlarm.SetLength(2);
      break;
  }
  currentMenu = k;
}

void checkAlarm() {
  if (!alarm) {
    lcd.print("Geen alarm");
  }
  else {
    lcd.print("Alarm: " + formatTime(alarmUur) + ":" + formatTime(alarmMinuut));
  }
}

menus getCurrentMenu() {
  return currentMenu;
}

void actionMenuKnop() {
  switch (getCurrentMenu()) {
    case M_klok:
      setMenu(M_menu1);
      break;
    case M_helderheid:
      setMenu(M_menu3);
      break;
    case M_alarm:
      setMenu(M_menu1);
      break;
    case M_tijd:
      setMenu(M_menu2);
      break;
    default:
      setMenu(M_klok);
      break;
  }
}

void actionSelectKnop() {
  switch (getCurrentMenu()) {
    case M_menu1:
      setMenu(M_alarm);
      break;
    case M_menu2:
      setMenu(M_tijd);
      break;
    case M_menu3:
      setMenu(M_helderheid);
      break;
    case M_tijd:
      setMenu(M_tijd_uur);
      break;
    case M_tijd_uur:
      writeTime();
      setMenu(M_tijd_minuut);
      break;
    case M_tijd_minuut:
      writeTime();
      setMenu(M_tijd_seconde);
      break;
    case M_tijd_seconde:
      writeTime();
      setMenu(M_tijd);
      tone(5, 150, 200);
      break;
    case M_alarm:
      setMenu(M_alarm_aanuit);
      break;
    case M_alarm_aanuit:
      setMenu(M_alarm_uur);
      break;
    case M_alarm_uur:
      setMenu(M_alarm_minuut);
      break;
    case M_alarm_minuut:
      setMenu(M_alarm);
      alarmSnooze = false;
      alarmStop = false;
      alarmGeluid = false;
      tone(5, 150, 200);
      break;
  }
}

void writeTime() {
  rtc.writeProtect(false);
  rtc.halt(false);
  Time t(2020, 10, 04, tijdUur, tijdMinuut, tijdSeconde, Time::kSunday);
  rtc.time(t);
}

void actionMinKnop() {
  switch (getCurrentMenu()) {
    case M_menu2:
      setMenu(M_menu1);
      break;
    case M_menu3:
      setMenu(M_menu2);
      break;
    case M_helderheid:
      if (brightnessLvl > 0) {
        brightnessLvl -= 1;
      }
      if (brightnessLvl < 0) {
        brightnessLvl = 0;
      }
      analogWrite(9, brightness[brightnessLvl]);
      setMenu(M_helderheid);
      break;
    case M_tijd_uur:
      subTime(tijdUur, 23);
      break;
    case M_tijd_minuut:
      subTime(tijdMinuut, 59);
      break;
    case M_tijd_seconde:
      subTime(tijdSeconde, 59);
      break;
    case M_alarm_aanuit:
      alarm = !alarm;
      blinkAlarm.SetString(formatBool(alarm));
      break;
    case M_alarm_uur:
      subTime(alarmUur, 23);
      break;
    case M_alarm_minuut:
      subTime(alarmMinuut, 59);
      break;
  }
}

void actionPlusKnop() {
  switch (getCurrentMenu()) {
    case M_menu1:
      setMenu(M_menu2);
      break;
    case M_menu2:
      setMenu(M_menu3);
      break;
    case M_helderheid:
      if (brightnessLvl < 10) {
        brightnessLvl += 1;
      }
      if (brightnessLvl > 10) {
        brightnessLvl = 10;
      }
      analogWrite(9, brightness[brightnessLvl]);
      setMenu(M_helderheid);
      break;
    case M_tijd_uur:
      addTime(tijdUur, 23);
      break;
    case M_tijd_minuut:
      addTime(tijdMinuut, 59);
      break;
    case M_tijd_seconde:
      addTime(tijdSeconde, 59);
      break;
    case M_alarm_aanuit:
      alarm = !alarm;
      blinkAlarm.SetString(formatBool(alarm));
      break;
    case M_alarm_uur:
      addTime(alarmUur, 23);
      break;
    case M_alarm_minuut:
      addTime(alarmMinuut, 59);
      break;
  }
}

void actionSnoozeKnop() {
  if (alarmGeluid) {
    tone(5, 150, 200);
    alarmGeluid = false;
    alarmSnooze = true;
    alarmSnoozeUur = tijdUur;
    alarmSnoozeMinuut = tijdMinuut + alarmSnoozeInterval;
  }
}

void brightnessLevel() {
  lcd.setCursor(2, 1);
  lcd.write(2);
  for (int i = 1; i <= 8; i++) {
    lcd.write(4);
  }
  lcd.write(3);
  lcd.setCursor(2, 1);
  for (int i = 1; i <= brightnessLvl; i++) {
    lcd.write(5);
  }
}

String formatTime(int tijd123) {
  String text123;
  if (tijd123 < 10) {
    text123 = "0" + (String)tijd123;
  }
  else {
    text123 = (String)tijd123;
  }
  return text123;
}

String formatBool(bool &alarm) {
  if (alarm) return "AAN";
  return "UIT";
}
