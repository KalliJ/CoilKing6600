#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>


// Säätö arvot
//
int powershift = 10; // Moottorin tehon säädössä käytettävä prosentti määrä
const byte lowestpower = 102; // Teho taso jolla pyöriminen alkaa


// Pinnit ja laitteistoon liittyvien objection luonti.
//
const byte hallPin = 2;  // Kierrosluku sensori. Interrupt 0
const byte motorPin = 13; // PWM
const byte servoPin = ; // PWM
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
Servo oscilator;


// Muuttujat
byte mode = 0;
boolean print_dialog = true;

int power;
volatile int r_count;
unsigned int rpm;
unsigned int rpmcount;
unsigned int totalcount;
unsigned int targetcount;
unsigned long timeold;

String dialog[] = {" Set coil turns"};
String input = "";

char key;

void setup(){
  // Sensori
  pinMode(hallPin, INPUT);
  //attachInterrupt(1, addToR, FALLING);
  // Moottori
  pinMode(motorPin, OUTPUT);
  analogWrite(motorPin, 0);
  powershift = powershift * 255/100;
  //Servo
  oscilator.attach(servoPin);
  //oscilator.write(1500);
  // Set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.print(" Coil King 6600");
  delay(3000);
}

void loop(){
  delay(100);
  key = keypad.getKey();
  
  if ( print_dialog ) {
    clearLine(0);
    lcd.print(dialog[mode]);
    lcd.setCursor(0, 1);
    lcd.print("  and press #");
    delay(2500);
    clearLine(1);
    print_dialog = false;
  }
    
  if ( mode == 0 ) {
    
    if ( key >= '0' && key <= '9' ){
      Serial.println(key);
      //lcd.scrollDisplayRight();
      input = input + key;
      lcd.print(key);
      
    }
    else if (key == '#' && input != "" ) {
      mode++;
      targetcount = input.toInt();
      input = "";
      //print_dialog = true;
      
      startWork();
    }
  }
  /*
  else if (mode == 1) {
  
  }
  else if (mode == 2) {
  
  }
  else if (mode == 3) {
  
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
    else { // sammuta
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
}

void endWork() {
  detachInterrupt(0);
  //targetcount = 0;
  power = 0;
  analogWrite(motorPin, power);

  lcd.clear();
  mode = 0;
  print_dialog = true;
}
