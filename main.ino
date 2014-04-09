#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>


// Säätö arvot
//
int powershift = 10; // Moottorin tehon säädössä käytettävä prosentti määrä
byte lowestpower = 102; // Teho taso jolla pyöriminen alkaa
unsigned int minpulse = 544; // Minimi pulssi servolle (vastapäivään)
unsigned int maxpulse = 2400; // Maksimi pulssi servolle (myötäpäivään)
byte startposition = 0; // Servon aloitus paikka työstössä (0-180)

// Pinnit ja laitteistoon liittyvien objection luonti.
//
const byte hallPin = 2;  // Kierrosluku sensori. Interrupt 0
const byte motorPin = 13; // PWM
const byte servoPin = 12; // PWM

Servo oscilator;

LiquidCrystal lcd(46, 47, 48, 49, 50, 51); // lcd(RS, Enable, D4, D5, D6, D7)

const byte ROWS = 4;
const byte COLS = 4;
byte rowPins[ROWS] = {32, 33, 34, 35}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {28, 29, 30, 31}; //connect to the column pinouts of the keypad
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


// Alku valinnoissa käytettävät muuttujat
//
byte mode = 0;
String input = "";
char key;
boolean print_dialog = true;
String dialogline0[] = {" Winding turns", "Turns per layer"};
String dialogline1[] = {"         #accept", "*back    #accept"};

// Työstössä käytettävät muuttujat
//
int power;

unsigned int oscilationturns;
byte oscilatorposition;
boolean reversedirection;

volatile int r_count;
unsigned int rpm;
unsigned int rpmcount;
unsigned int totalcount;
unsigned int targetcount;
unsigned long timeold;





void setup(){
  // Sensori
  pinMode(hallPin, INPUT);
  
  // Moottori
  power = 0;
  powershift = powershift * 255/100;
  pinMode(motorPin, OUTPUT);
  analogWrite(motorPin, power);
  
  //Servo
  oscilator.attach(servoPin, minpulse, maxpulse);
  
  // Set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.print(" Coil King 6600");
  delay(3000);
}

void loop(){
  delay(100);

  if ( print_dialog ) {
    clearLine(0);
    lcd.print(dialogline0[mode]);
    lcd.setCursor(0, 1);
    lcd.print(dialogline1[mode]);
    print_dialog = false;
  }

  key = keypad.getKey();
    
  if ( mode == 0 ) {
    
    if ( key >= '0' && key <= '9' ){
      if (input == "")
        clearLine(1);
      input = input + key;
      lcd.print(key);
    }
    else if (key == '#' && input != "" ) {
      mode++;
      targetcount = input.toInt();
      input = "";
      print_dialog = true;
    }
    
  }
  else if ( mode == 1 ) {
  
    if ( key >= '0' && key <= '9' ){
      if (input == "")
        clearLine(1);
      input = input + key;
      lcd.print(key);
    }
    else if (key == '#' && input != "" ) {
      mode++;
      oscilationturns = input.toInt();
      input = "";
      //print_dialog = true;
      startWork();
    }
    
  }
  /*
  else if ( mode == 2 ) {
  
  }
  else if ( mode == 3 ) {
  
  }
  */
  
  else { // Työstö
    
    totalcount = totalcount + r_count;
    rpmcount = rpmcount + r_count;
    r_count = 0;
    if ( millis() - timeold > 1000) { // rpmcount >= 50 ||
      rpm = 60000 * rpmcount / ( millis() - timeold ) ;
      timeold = millis();
      rpmcount = 0;
      lcd.setCursor(7, 1);
      lcd.print(String(" RPM ") + rpm + String(" "));
    }
    
    if (oscilationturns > 0) {
      setOscilatorPosition();
    }

    if (totalcount < targetcount) {
      lcd.setCursor(0, 1);
      lcd.print(String("T ") + totalcount);
      
      if ( key == '#' ) {
        power = power + powershift;
        if ( power > 255 ) {
          power = 255;
        }
        else if (power < lowestpower) {
          power = lowestpower;
        }
          
        analogWrite(motorPin, power);
      }
      else if ( key == '*' ) {
        power = power - powershift;
        if ( power < 0 )
          power = 0;
          
        analogWrite(motorPin, power);
      }
      else if ( key == '0' ) {
        power = 0;
        analogWrite(motorPin, power);
      }
      else if ( key == 'C' ) {
        endWork();
      }
    }
    else {
      endWork();
    }
  }
}

// Sensorin interrupt laskuri
void addToR() {
      r_count++;
}

void clearLine(byte line) {
  lcd.setCursor(0, line);
  lcd.print("                ");
  lcd.setCursor(0, line);
}

void startWork() {
  r_count = 0;
  totalcount = 0;
  rpmcount = 0;
  rpm = 0;
  power = 0;
  analogWrite(motorPin, power);
  attachInterrupt(0, addToR, FALLING);
  lcd.setCursor(0, 0);
  lcd.print("-*    0/C     #+");
  clearLine(1);
  timeold = millis();
  
  if (oscilationturns >= targetcount) {
    oscilationturns = 0;
    oscilatorposition = 90;
    oscilator.write(oscilatorposition);
  }
  else {
    reversedirection = false;
    oscilatorposition = startposition;
    oscilator.write(oscilatorposition);
  }
  
}

void endWork() {
  detachInterrupt(0);
  power = 0;
  analogWrite(motorPin, power);
  lcd.clear();
  mode = 0;
  print_dialog = true;
}

void setOscilatorPosition() {
  oscilatorposition = (totalcount % oscilationturns)/oscilationturns*180;
  
  if (reversedirection)
    oscilatorposition = 180 - oscilatorposition;
    
  if ( oscilatorposition == 0 )
    reversedirection = false;
  else if ( oscilatorposition == 180 )
    reversedirection = true;
    
  oscilator.write(oscilatorposition);
}
