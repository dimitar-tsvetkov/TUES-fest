
#include <LiquidCrystal.h> 
#include <Keypad.h>
#include <Servo.h>
Servo ServoMotor;

#define buzzer 8
#define trigPin 9
#define echoPin 10
#define lcdBacklight 11

long duration;
int distance, initialDistance, currentDistance, i, sumDistance;
int screenOffMsg = 0;
int k = 0;
String password = "123";
String tempPassword;
boolean activated = false; 
boolean isActivated;
boolean activateAlarm = false;
boolean alarmActivated = false;
boolean enteredPassword; 
boolean passChangeMode = false;
boolean passChanged = false;


const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keypressed;

char keyMap[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {14, 15, 16, 17}; 
byte colPins[COLS] = {18, 19, 20, 21}; 

Keypad myKeypad = Keypad( makeKeymap(keyMap), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(1, 2, 4, 5, 6, 7); 

void setup() {
  //Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(buzzer, OUTPUT); 
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  analogWrite(lcdBacklight, 128);
  ServoMotor.attach(13);  
}

void loop() {
  if (activateAlarm) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm will be");
    lcd.setCursor(0, 1);
    lcd.print("activated in");

    int countdown = 5; 
    while (countdown != 0) {
      lcd.setCursor(13, 1);
      lcd.print(countdown);
      countdown--;
      tone(buzzer, 700, 100);
      delay(1000);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm Activated!");
    initialDistance = getDistance();
    activateAlarm = false;
    alarmActivated = true;
  }

  if (alarmActivated) {
    currentDistance = getDistance() + 10;
    int activated_counter = 0;
    while (activated_counter <= 5) {
      currentDistance = getDistance() + 10;
      if ( currentDistance < initialDistance) {
        activated_counter++;
      } else {
        activated_counter = 0;
      }
    }
    tone(buzzer, 1000); 
    lcd.clear();
    enterPassword();
    activated_counter = 0;

  }

  if (!alarmActivated) {
    if (screenOffMsg == 0 ) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("A - Activate");
      lcd.setCursor(0, 1);
      lcd.print("B - Change Pass");
      screenOffMsg = 1;
    }
    keypressed = myKeypad.getKey();
    if (keypressed == 'A') {      
      tone(buzzer, 1000, 200);
      activateAlarm = true;
      ServoMotor.write(100);

    }
    else if (keypressed == 'B') {
      lcd.clear();
      int i = 1;
      tone(buzzer, 2000, 100);
      tempPassword = "";
      lcd.setCursor(0, 0);
      lcd.print("Current Password");
      lcd.setCursor(0, 1);
      lcd.print(">");
      passChangeMode = true;
      passChanged = true;
      while (passChanged) {
        keypressed = myKeypad.getKey();
        if (keypressed != NO_KEY) {
          if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
              keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
              keypressed == '8' || keypressed == '9' ) {
            tempPassword += keypressed;
            lcd.setCursor(i, 1);
            lcd.print("*");
            i++;
            tone(buzzer, 2000, 100);
          }
        }
        if (i > 5 || keypressed == '#') {
          tempPassword = "";
          i = 1;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Current Password");
          lcd.setCursor(0, 1);
          lcd.print(">");
        }
        if ( keypressed == '*') {
          i = 1;
          tone(buzzer, 2000, 100);
          if (password == tempPassword) {
            tempPassword = "";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Set New Password");
            lcd.setCursor(0, 1);
            lcd.print(">");
            while (passChangeMode) {
              keypressed = myKeypad.getKey();
              if (keypressed != NO_KEY) {
                if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
                    keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
                    keypressed == '8' || keypressed == '9' ) {
                  tempPassword += keypressed;
                  lcd.setCursor(i, 1);
                  lcd.print("*");
                  i++;
                  tone(buzzer, 2000, 100);
                }
              }
              if (i > 5 || keypressed == '#') {
                tempPassword = "";
                i = 1;
                tone(buzzer, 2000, 100);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Set New Password");
                lcd.setCursor(0, 1);
                lcd.print(">");
              }
              if ( keypressed == '*') {
                i = 1;
                tone(buzzer, 2000, 100);
                password = tempPassword;
                passChangeMode = false;
                passChanged = false;
                screenOffMsg = 0;
              }
            }
          }
        }
      }
    }
  }
}

void enterPassword() {
  int k = 5;
  tempPassword = "";
  activated = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" *** ALARM *** ");
  lcd.setCursor(0, 1);
  lcd.print("Pass>");

  while (activated) {
    keypressed = myKeypad.getKey();
    if (keypressed != NO_KEY) {
      if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
          keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
          keypressed == '8' || keypressed == '9' ) {
        tempPassword += keypressed;
        lcd.setCursor(k, 1);
        lcd.print("*");
        k++;
      }
    }
    if (k > 9 || keypressed == '#') {
      tempPassword = "";
      k = 5;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" *** ALARM *** ");
      lcd.setCursor(0, 1);
      lcd.print("Pass>");
    }
    if ( keypressed == '*') {
      if ( tempPassword == password ) {
        activated = false;
        alarmActivated = false;
        noTone(buzzer);
        screenOffMsg = 0;
        ServoMotor.write(10);
      }
      else if (tempPassword != password) {
        lcd.setCursor(0, 1);
        lcd.print("Wrong! Try Again");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" *** ALARM *** ");
        lcd.setCursor(0, 1);
        lcd.print("Pass>");
      }
    }
  }
}

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  return distance;

}
