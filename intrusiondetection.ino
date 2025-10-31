#include <Keypad.h>
#include <LiquidCrystal.h>

//LCD Pins
const int rs = 12, en = 11, d4 = 4, d5 = 5, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Other pins used
const int sensorpin = 13; //input from the motion sensor
const int ledpin = 10; //output for the LED
const int buttonpin = 9;
const int buzzerpin = 8;

//Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {7, 6, A5, A4};
byte colPins[COLS] = {A3, A2, A1, A0};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// System configuration
const char* mastercode = "13";    // The password to disarm the system
const long snoozeduration = 15000; // Snooze for 15 seconds

// System variables
enum Systemstate { DISARMED, ARMED, TRIGGERED };
Systemstate currentstate = ARMED; // System starts armed
String enteredcode = "";
bool isbuzzersnoozed = false;
unsigned long snoozestarttime = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("SafeHome System initializing...");

  pinMode (sensorpin,INPUT);
  pinMode (ledpin, OUTPUT);
  pinMode (buzzerpin, OUTPUT);
  pinMode (buttonpin, INPUT); 

  lcd.begin(16, 2);
  updateDisplay();
  lcd.print("System online");
}

void loop() {
  checkKeypad(); //always check for user input 
  switch (currentstate) {
    case DISARMED:
      stateDisarmed();
      break;
    case ARMED:
      stateArmed();
      break;
    case TRIGGERED:
      stateTriggered();
      break;
  }
  delay(100); // Small delay for stability
}

void stateDisarmed() {
  digitalWrite(ledpin, LOW);
  digitalWrite(buzzerpin, LOW);
}

void stateArmed() {
  if (digitalRead(sensorpin) == HIGH) {
    currentstate = TRIGGERED;
    isbuzzersnoozed = false;
    updateDisplay();
    Serial.println("EVENT: Motion Detected!");
  }
}

void stateTriggered() {
  if (digitalRead(buttonpin) == HIGH && !isbuzzersnoozed) {
    isbuzzersnoozed = true;
    snoozestarttime = millis();
    Serial.println("EVENT: Alarm Snoozed for 15s!");
    updateDisplay();
    delay(250);
  }

  digitalWrite(ledpin, HIGH);

  if (isbuzzersnoozed && (millis() - snoozestarttime >= snoozeduration)) {
    isbuzzersnoozed = false; // Snooze is over
    updateDisplay();
  }

  if (!isbuzzersnoozed) {
    //tone(buzzerpin, 2500, 1500);
    // Loop 3 times to create a short "wail"
    for (int i = 0; i < 3; i++) {
      tone(buzzerpin, 1500, 1500); // High pitch for 150ms
      delay(150);
      tone(buzzerpin, 1000, 1500);  // Low pitch for 150ms
      delay(150);
    }
  } else {
    digitalWrite(buzzerpin, LOW); // Ensure buzzer is quiet
  }
}

void checkKeypad() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Keypad registered: ");  //to see what is being typed
    Serial.println(key); //to see how keypad reacts to the touch

    if (key == '#') { // Enter key
      if (enteredcode == mastercode) {
        currentstate = DISARMED;
        Serial.println("EVENT: Disarmed by keypad!");
      }else {
        Serial.println("Incorrect password entered.");
        lcd.clear();
        lcd.print("Incorrect code");
        delay(2000); // Show message for 1.5 seconds
      }
      enteredcode = ""; // Clear code on attempt
    } else if (key == 'A') { // Arm key
      if (currentstate == DISARMED) {
        currentstate = ARMED;
        Serial.println("EVENT: Armed by keypad!");
      }
    } else {
      enteredcode += key;
    }
    updateDisplay();
  }
}

void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (currentstate) {
    case DISARMED:
      lcd.print("System Disarmed");
      lcd.setCursor(0, 1);
      lcd.print("Press 'A' to Arm");
      break;
    case ARMED:
      lcd.print("System Armed");
      lcd.setCursor(0, 1);
      lcd.print("Monitoring...");
      break;
    case TRIGGERED:
      lcd.print("!! ALARM !!");
      lcd.setCursor(0, 1);
      if (isbuzzersnoozed) {
        lcd.print("Buzzer Snoozed");
      } else {
        lcd.print("MOTION DETECTED");
        }
      break;
  }
}




