#include <MD_MAX72xx.h>
#include <SPI.h>
#include <DS3231.h>
//#include "Font_Data.h


DS3231 rtc(SDA, SCL);     
const byte LDR_PIN = A2;

#define MAX_DEVICES  4
// Define pins
#define CLK_PIN   13  
#define DATA_PIN  11 
#define CS_PIN    10  
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define USE_NEW_FONT 1

#define BUF_SIZE      20  
#define CHAR_SPACING  1   

char buf[BUF_SIZE], secs[4];
uint8_t hh, mm, ss, dots;

// Definition of the small fonts:
uint8_t Font3x5 [ 10 ][ 3 ]={ 
  { 248, 136, 248},   
  {144, 248, 128},   
  {200, 168, 184},   
  {136, 168, 248},   
  {112, 72, 224},    
  {184, 168, 232},   
  {248, 168, 232},   
  {8, 232, 24},      
  {248, 168, 248},   
  {184, 168, 248}};   

char months[12][4]= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
char* wday;
MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

const byte WAIT = 100;
const byte SPACER = 1;
byte FONT_WIDTH;

bool timeset=false;

void adjustClock(String data) {
  byte _day = data.substring(0,2).toInt();
  byte _month = data.substring(3,5).toInt();
  int _year = data.substring(6,10).toInt();
  byte _hour = data.substring(11,13).toInt();
  byte _min = data.substring(14,16).toInt();
 byte _sec = data.substring(17,19).toInt();
  rtc.setTime(_hour, _min ,_sec);
  rtc.setDate(_day,_month,_year);
  Serial.println(F(">> Datetime successfully set!"));
  timeset=true;
}

byte ledintensitySelect(int light) {
  byte _value = 0;
  if (light >= 0 && light <= 127) {
    _value = 12;
  } else if (light >= 128 && light <= 319) {
    _value = 3; 
  } else if (light >= 320 && light <= 512) {
    _value = 0;
  }
  return _value;
};

void printText(uint8_t modStart, uint8_t modEnd, char *pMsg)
{
  uint8_t   state = 0;
  uint8_t   curLen;
  uint16_t  showLen;
  uint8_t   cBuf[FONT_WIDTH];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;

  matrix.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     
  {
    switch(state)
    {
      case 0: 
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  
          state = 2;
          break;
        }
        showLen = matrix.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
      
      case 1: 
        matrix.setColumn(col--, cBuf[curLen++]);

        
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: 
        curLen = 0;
        state++;
        // fall through

      case 3:  
        matrix.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;

      default:
        col = -1;   
    }
  } while (col >= (modStart * COL_SIZE));

  matrix.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void setup() {
  pinMode(LDR_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  pinMode(8, OUTPUT);        
  Serial.println(F(">> Use <dd/mm/yyyy hh:mm:ss> format to set clock's date and hour!"));
  rtc.begin();
  matrix.begin();
  matrix.clear();
  FONT_WIDTH= 5 + SPACER;   
  matrix.control(MD_MAX72XX::INTENSITY, 2);; 
  rtc.setDOW();    
}

void getDate()
{ 

  String dts = rtc.getDateStr();    
  String dds = dts.substring(0,2);    
  String mms = dts.substring(3,5);    
  int mm = mms.toInt();               
  dds.concat(" ");
  dds.concat(String(months[mm-1]));  
  dds.toCharArray(buf,sizeof(buf)); 
  wday = rtc.getDOWStr(2);
}

void getHour()
{
  String dts = rtc.getTimeStr();    
  String hhs=dts.substring(0,2);    
  String mms=dts.substring(3,5);    
  hh=hhs.toInt();               
  mm=mms.toInt();               
  ss=(dts.substring(6,8)).toInt();  
  
  if (hh >= 0 && hh < 10)  dots = 7;
  if (hh > 9 && hh < 20)  dots = 11; 
  if (hh > 19 && hh < 25) dots = 13;
  if (hh%10 == 1)         dots-=2; 
  
  //String outmsg=dts.substring(0,5);    
  String outmsg=String(hh);              
  // outmsg.concat(":");                   
  outmsg.concat(char(124));              
  outmsg.concat(dts.substring(3,5));     
  outmsg.toCharArray(buf,BUF_SIZE);
}

void showsec(uint8_t secs)
{ uint8_t secs1=secs%10;
  uint8_t secs2=secs/10;
  for (uint8_t k=0; k<3; k++){
    matrix.setColumn(MAX_DEVICES*8-26-k,Font3x5 [secs2][k]); 
    matrix.setColumn(MAX_DEVICES*8-30-k,Font3x5 [secs1][k]);  
  }
}

void loop() {

  byte ledIntensity = ledintensitySelect(analogRead(LDR_PIN));
  matrix.control(MD_MAX72XX::INTENSITY, ledIntensity);
 
  getHour();                      
  printText(0,MAX_DEVICES-1,buf); 
  matrix.setColumn(MAX_DEVICES*8-dots,0); 
  unsigned long inst =millis(); 

  while (ss < 53){                
    while (millis() - inst > 1000){
      inst =millis();
      ss++;                       
      showsec(ss);              
      for (uint8_t i = 0; i < 2; i++){
        matrix.setColumn(MAX_DEVICES*8-dots,36); 
        delay(240);
        matrix.setColumn(MAX_DEVICES*8-dots,0);
        delay(240);
      }     
    }
  }

  for (uint8_t i=0; i<8; i++){
    matrix.transform(MD_MAX72XX::TSU);
    delay(3*WAIT);
  }


  getDate();
  printText(0,MAX_DEVICES-1,buf);
  delay(20*WAIT);

  
  int temp = rtc.getTemp();
  temp=temp-1;                            
  String outmsg=String(temp);
  outmsg.concat(" C"); 
  outmsg.toCharArray(buf,BUF_SIZE);
  printText(0,MAX_DEVICES-1,buf);
  delay(20*WAIT);
  
  // Time setting in RTC
  if (Serial.available() > 0 && timeset==false) {
    adjustClock(Serial.readString());
        
      char data= Serial.read(); 
      switch(data)
      {
        //case 'a': digitalWrite(8, HIGH);break; 
        //case 'd': digitalWrite(8, LOW);break; 
        default : break;
      }
      Serial.println(data);
   }
   delay(50);
  
}
