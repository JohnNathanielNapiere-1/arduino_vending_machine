#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Define the Arduino pin connected to the coin acceptor's "COIN" output
const int coinPin = 2;

// Define pin assignments for buttons
const int buttonPin1 = 3;
const int buttonPin2 = 4;
const int buttonPin3 = 5;

// Define pin assignments for servos
const int servoPin1 = 9;
const int servoPin2 = 10;
const int servoPin3 = 11;

// Product prices (in pesos)
const int price1 = 10;
const int price2 = 5;
const int price3 = 10;

// Variable to keep track of the balance (in pesos)
volatile unsigned int balance = 0;

// Debouncing variables for coin acceptor
volatile bool coinDetected = false;
unsigned long lastCoinDebounceTime = 0;
const unsigned long coinDebounceDelay = 50; // milliseconds

// Debouncing variables for buttons
unsigned long lastButtonDebounceTime1 = 0;
unsigned long lastButtonDebounceTime2 = 0;
unsigned long lastButtonDebounceTime3 = 0;
const unsigned long buttonDebounceDelay = 200; // milliseconds

// Previous button states
bool lastButtonState1 = HIGH;
bool lastButtonState2 = HIGH;
bool lastButtonState3 = HIGH;

// LCD variables
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display
unsigned long lastScrollTime = 0;
const unsigned long scrollDelay = 400; // milliseconds
int scrollPosition = 0;
String priceMessage = "Product 1: 10P | Product 2: 5P | Product 3: 10P | ";
int messageLength = 0;

// Servo objects
Servo servo1;
Servo servo2;
Servo servo3;

// Servo operation time
const unsigned long servoRunTime = 1000; // Time for servo to run (in milliseconds)
unsigned long servoStartTime = 0;
bool servoActive = false;
int activeServo = 0;

void setup() {
  // Initialize LCD
  lcd.init();  
  lcd.backlight();
  
  // Set the price message length
  messageLength = priceMessage.length();
  
  // Display initial message
  updateLCD();
  
  // Set button pins as input with pull-up resistors
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  
  // Set the coin pin as an input with a pull-up resistor
  pinMode(coinPin, INPUT_PULLUP);
  
  // Attach servos to pins
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo3.attach(servoPin3);
  
  // Set servos to stop position (90 for continuous rotation servos)
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  
  // Attach an interrupt to detect the signal from the coin acceptor
  attachInterrupt(digitalPinToInterrupt(coinPin), coinDetectedISR, FALLING);
}

void loop() {
  // Handle coin insertion
  if (coinDetected) {
    // Debounce the input
    if (millis() - lastCoinDebounceTime > coinDebounceDelay) {
      // A 10 peso increment has been detected
      balance += 10;
      
      // Update LCD with new balance
      updateLCD();
      
      // Reset the flag
      coinDetected = false;
    }
  }
  
  // Handle servo operation if active
  if (servoActive) {
    if (millis() - servoStartTime >= servoRunTime) {
      // Stop the running servo
      if (activeServo == 1) servo1.write(90);
      else if (activeServo == 2) servo2.write(90);
      else if (activeServo == 3) servo3.write(90);
      
      servoActive = false;
      activeServo = 0;
    }
  }
  // If no servo is active, check buttons
  else {
    // Check button 1
    bool buttonState1 = digitalRead(buttonPin1);
    if (buttonState1 != lastButtonState1) {
      lastButtonDebounceTime1 = millis();
    }
    
    if ((millis() - lastButtonDebounceTime1) > buttonDebounceDelay) {
      if (buttonState1 == LOW) { // Button is pressed
        processProductSelection(1, price1);
      }
    }
    lastButtonState1 = buttonState1;
    
    // Check button 2
    bool buttonState2 = digitalRead(buttonPin2);
    if (buttonState2 != lastButtonState2) {
      lastButtonDebounceTime2 = millis();
    }
    
    if ((millis() - lastButtonDebounceTime2) > buttonDebounceDelay) {
      if (buttonState2 == LOW) { // Button is pressed
        processProductSelection(2, price2);
      }
    }
    lastButtonState2 = buttonState2;
    
    // Check button 3
    bool buttonState3 = digitalRead(buttonPin3);
    if (buttonState3 != lastButtonState3) {
      lastButtonDebounceTime3 = millis();
    }
    
    if ((millis() - lastButtonDebounceTime3) > buttonDebounceDelay) {
      if (buttonState3 == LOW) { // Button is pressed
        processProductSelection(3, price3);
      }
    }
    lastButtonState3 = buttonState3;
  }
  
  // Update scrolling text every scrollDelay milliseconds
  if (millis() - lastScrollTime > scrollDelay) {
    scrollPriceMessage();
    lastScrollTime = millis();
  }
}

// Interrupt Service Routine (ISR) to set the coinDetected flag
void coinDetectedISR() {
  coinDetected = true;
  lastCoinDebounceTime = millis();
}

// Process product selection
void processProductSelection(int product, int price) {
  if (balance >= price) {
    // Deduct price from balance
    balance -= price;
    
    // Display dispensing message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dispensing...");
    lcd.setCursor(0, 1);
    lcd.print("Balance: ");
    lcd.print(balance);
    lcd.print("P");
    
    // Activate the appropriate servo
    activateServo(product);
  } else {
    // Display insufficient balance message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Insufficient");
    lcd.setCursor(0, 1);
    lcd.print("Balance: ");
    lcd.print(balance);
    lcd.print("P");
    
    delay(2000); // Show error message for 2 seconds
    updateLCD(); // Return to normal display
  }
}

// Activate a servo to dispense a product
void activateServo(int product) {
  servoActive = true;
  activeServo = product;
  servoStartTime = millis();
  
  // Set servo speed (0/180 for continuous rotation servos)
  // Adjust the values (0 or 180) based on the direction needed for your servos
  if (product == 1) servo1.write(0); // Full speed in one direction
  else if (product == 2) servo2.write(0);
  else if (product == 3) servo3.write(0);
}

// Update the LCD display with current information
void updateLCD() {
  lcd.clear();
  
  // First row: Scrolling price message (handled by scrollPriceMessage)
  scrollPosition = 0; // Reset scroll position
  scrollPriceMessage();
  
  // Second row: Balance
  lcd.setCursor(0, 1);
  lcd.print("Balance: ");
  lcd.print(balance);
  lcd.print("P");
}

// Scroll the price message on the first row of the LCD
void scrollPriceMessage() {
  lcd.setCursor(0, 0);
  
  // Calculate end position for substring
  int endPos = scrollPosition + 16;
  if (endPos > messageLength) {
    // If we need to wrap around
    String firstPart = priceMessage.substring(scrollPosition);
    String secondPart = priceMessage.substring(0, endPos - messageLength);
    lcd.print(firstPart + secondPart);
  } else {
    // No wrapping needed
    lcd.print(priceMessage.substring(scrollPosition, endPos));
  }
  
  // Increment scroll position and reset if necessary
  scrollPosition++;
  if (scrollPosition >= messageLength) {
    scrollPosition = 0;
  }
}
