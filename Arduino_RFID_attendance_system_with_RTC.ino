/* This program Scans RFID cards and tags, and prints the student matriculation number with the time the student log in and stores in the arduino's memory
  It uses the following components: LCD(Vcc- 5v, GND-gnd, SDA- A4, SCL- A5), RC522(SDA(SS)-10, SCK-13, MOSI- 11, MISO-12, IRQ- not connected, RST- 9), 
  RTC module DS1302(VCC-5v, GND-gnd, CLK-6,DAT-7,RST(CE)-8). Arduino uno r3 was the microntroller board used.
  Library Used: 
    1. MFRC522 by GithubCommunity
    2. LiquidCrystal_I2C by Martin kubovcik, Frank de Brabander
    3. Rtc by Makuna by Michael C.Miller*/

// includes the library
#include <SPI.h>
#include<MFRC522.h>
#include<LiquidCrystal_I2C.h>
#include<ThreeWire.h>
#include<RtcDS1302.h>
#include<EEPROM.h>


ThreeWire wire(7,6,8); // assign the pins for the rtc module DAT,CLK, RST respectively
RtcDS1302<ThreeWire> rtc(wire);
MFRC522 rfid(10,9); // assign the pins used for SDA and RST of the RC522 module
LiquidCrystal_I2C lcd(0x27,16,2);


const byte MARKER = 0xA5;
const int ADDR_MARKER = 0;
const int ADDR_DAY= 1;
const int ADDR_TIMES = 3;
const byte NUM_STUDENTS = 8; //number of students

bool signedIn[NUM_STUDENTS];
RtcDateTime signInTime[NUM_STUDENTS];
uint16_t lastDay;
const byte studentUid[NUM_STUDENTS][4]={
  {0x2C, 0xD0, 0x18, 0x06},
  {0xD5, 0x41, 0x18, 0x06},
  {0xE5, 0xC0, 0x1B, 0x06},
  {0xCD, 0xF6, 0x18, 0x05},
  {0x4E, 0x3A, 0xF3, 0x05},
  {0xF3, 0x4C, 0x89, 0x2D},
  {0x3E, 0xFE, 0xA5, 0x05},
  {0xF1, 0x46, 0xF3, 0x05}
};

const char* studentMatric[NUM_STUDENTS]= {
  "xxx/xxx/xxx",
  "xxx/xxx/xxx",
  "xxx/xxx/xxx",
  "xxx/xxx/xxx",
  "xxx/xxx/xxx",
  "xxx/xxx/xxx",
  "xxx/xxx/xxx",
  "xxx/xxx/xxx"
  //insert as many matriculation number as you want but change the value of NUM_STUDENTS to the number of student you want
};


void printTwoDigits(byte number){
  if (number < 10){
    lcd.print(0);
  }
  lcd.print(number);
}

void showSignIn(RtcDateTime dt){
  lcd.setCursor(0,1);
  printTwoDigits(dt.Hour());
  lcd.print(":");
  printTwoDigits(dt.Minute());
  lcd.print(":");
  printTwoDigits(dt.Second());
  lcd.print(" ");
  printTwoDigits(dt.Day());
  lcd.print("/");
  printTwoDigits(dt.Month());
  lcd.print(" ");
}


int findStudent(){
  if(rfid.uid.size != 4){
    return -1;
  }
  for(byte row = 0; row <NUM_STUDENTS; row++){
    bool match = true;
    for(int col = 0; col <4; col++){
      if(rfid.uid.uidByte[col] != studentUid[row][col]){
        match = false;
      }
    }
    if(match){
      return row;
    }
  }
  return -1;
}


void showLine(byte row, const char* text){
  lcd.setCursor(0,row);
  lcd.print(text);
  for(byte i = strlen(text); i<16; i++){
    lcd.print(" ");
  }
}


void showAlready(RtcDateTime dt){
    lcd.setCursor(0, 1);
    lcd.print("Already ");
    printTwoDigits(dt.Hour());
    lcd.print(":");
    printTwoDigits(dt.Minute());
    lcd.print("   "); 
}

void saveSignIn(byte who, uint32_t secs){
  EEPROM.put(ADDR_TIMES + who * 4, secs);
}

void clearAllSignIns(uint16_t today){
  uint32_t zero =0;
  for(int i =0; i<NUM_STUDENTS; i++){
    EEPROM.put(ADDR_TIMES + i *4, zero);
    signedIn[i] = false; 
  }
  EEPROM.put(ADDR_DAY, today);
  lastDay = today;
}

void loadSignIns(){
  EEPROM.get(ADDR_DAY, lastDay);
  for(int i =0; i<NUM_STUDENTS; i++){
    uint32_t secs;
    EEPROM.get(ADDR_TIMES + i * 4, secs);
    if(secs != 0){
      signedIn[i] = true;
      signInTime[i] = RtcDateTime(secs);
    }
    else{
      signedIn[i] = false;
    }
  }
}

void checkNewDay(RtcDateTime now){
  if(now.TotalDays() != lastDay){
    clearAllSignIns(now.TotalDays());
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);

  lcd.init();
  lcd.backlight();

  rtc.Begin();
  RtcDateTime start = rtc.GetDateTime();
  
  if(EEPROM.read(ADDR_MARKER) != MARKER){
    clearAllSignIns(start.TotalDays());
    EEPROM.update(ADDR_MARKER, MARKER);
  }
  else{
    loadSignIns();
  }

  checkNewDay(start);
  
  if(rtc.GetIsWriteProtected()){
    rtc.SetIsWriteProtected(false);
  }
  if (!rtc.GetIsRunning()){
      rtc.SetIsRunning(true);
    }

    RtcDateTime compiled(__DATE__, __TIME__);
    RtcDateTime current = rtc.GetDateTime();

  if ((!rtc.IsDateTimeValid()) || (current < compiled)){
      rtc.SetDateTime(compiled);
  }
    
  showLine(0, "Scan your card");
  showLine(1, "");

}



void loop() {
  // put your main code here, to run repeatedly:
  if (!rfid.PICC_IsNewCardPresent()){
    return;
  }
  if(!rfid.PICC_ReadCardSerial()){
    return;
  }

  int who = findStudent();
  
  if(who == -1){
    showLine(0, "Unknown card");
    showLine(1, "");
  }
  else{
     RtcDateTime now = rtc.GetDateTime();

        // new day? clear everyone's flag
     checkNewDay(now); 

      if(!signedIn[who]){
            signedIn[who]   = true;
            signInTime[who] = now;
            saveSignIn(who, now.TotalSeconds());
            showLine(0, studentMatric[who]);
            showSignIn(now);
        }
    else{
    showLine(0, studentMatric[who]);
    showAlready(signInTime[who]);
    }
  }
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(2000);

  showLine(0, "Scan your card");
  showLine(1, "");
}   