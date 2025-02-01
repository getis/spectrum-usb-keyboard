/*
Arduino Pro Micro based USB keyboard driver

ZX Spectrum Version

*/

#include <Arduino.h>
#include <Keyboard.h>

// global variables
#define SERIAL_BAUDRATE     9600

#define NUM_ROWS 8
#define NUM_COLUMNS 5

#define DEBOUNCE_SCANS 5

// 8 row drivers - outputs
const uint8_t  row1 = 9;
const uint8_t  row2 = 8;
const uint8_t  row3 = 7;
const uint8_t  row4 = 6;
const uint8_t  row5 = 5;
const uint8_t  row6 = 4;
const uint8_t  row7 = 3;
const uint8_t  row8 = 2;

uint8_t  rowPins[NUM_ROWS] = {row1, row2, row3, row4, row5, row6, row7, row8};

// 5 column inputs - pulled high
const uint8_t  column1 = 10;
const uint8_t  column2 = 16;
const uint8_t  column3 = 14;
const uint8_t  column4 = 15;
const uint8_t  column5 = 18;

uint8_t  columnPins[NUM_COLUMNS] = {column1, column2, column3, column4, column5};

// reset button on Spectrum+ case 
const uint8_t resetButton = 19;

char keyMap[NUM_ROWS][NUM_COLUMNS] = {
  {'1','2','3','4','5'},
  {'q','w','e','r','t'},
  {'a','s','d','f','g'},
  {'0','9','8','7','6'},
  {'p','o','i','u','y'},
  {KEY_LEFT_SHIFT,'z','x','c','v'},
  {KEY_RETURN,'l','k','j','h'},
  {' ',KEY_LEFT_CTRL,'m','n','b'},
};


// class definitions

class KeyboardKey {

  public:

    int row;
    int column;
    int lastState;
    int debounceCount;
    char keyCode;

    // default constructor
    KeyboardKey(){}

    // init constructor;
    KeyboardKey(int rowNum, int colNum){
      // log matrix position
      row = rowNum;
      column = colNum;
      // get keycode from keyMap array
      keyCode = keyMap[row][column];
      // initialise state and debounce
      lastState = 0;
      debounceCount = 0;
    } 

    void updateKey(){
      // row driver set by matrix handler
      // scan key
      int thisState = digitalRead(columnPins[column]);
      if(thisState != lastState) {
        // state changing
        debounceCount ++;
        // is debounce finished
        if (debounceCount >= DEBOUNCE_SCANS){
          // key state has changed
          lastState = thisState;
          debounceCount = 0;
          // high = not pressed, low = pressed
          if(thisState == 1){
            //key released
            Keyboard.release(keyCode);
            //Serial.println(keyCode);
            //delay(10);
          }
          else {
            // key pressed
            Keyboard.press(keyCode);
            //Serial.println(keyCode);
            //delay(10);
          }
        }
      }
      else {
        // no change in state
        // make sure debounce reset
        debounceCount = 0;
      }

    }

}; // end class KeyboardKey

// global key handler array
// initialised in setup
KeyboardKey keyHandlers[NUM_ROWS][NUM_COLUMNS];


class MatrixDriver{

  public:

    MatrixDriver() {}

    void scanMatrix() {
      int row;
      int column;

      for(row = 0; row < NUM_ROWS; row ++){
        // trun on this row line
        activateRowLine(row);
        // do column keys
        for(column = 0; column < NUM_COLUMNS; column ++){
          keyHandlers[row][column].updateKey();
        }
      }

    }

    void activateRowLine(int rowNum){
      // rowNum is zero based to match arrays
      int row;
      for(row = 0; row < NUM_ROWS; row ++){
        if(row == rowNum){
          // turn on this row
          digitalWrite(rowPins[row], LOW);
        }
        else {
          // turn off this row
          digitalWrite(rowPins[row], HIGH);
        }
      }
    }

}; // end class Matrix Driver

// global matrix driver instance
MatrixDriver matrix = MatrixDriver();

void setup() {
  int row;
  int column;
  String message;

  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUDRATE);

  // reset button
  pinMode(resetButton, INPUT_PULLUP);
  delay(10);
  // wait for start command on reset button
  Serial.println("waiting for start");
  while(digitalRead(resetButton)){
    delay(100);
  }

  Serial.println("start button pressed");
  delay(10);

  // start keyboard
  Keyboard.begin();

  // init columns
  pinMode(column1, INPUT_PULLUP);
  pinMode(column2, INPUT_PULLUP);
  pinMode(column3, INPUT_PULLUP);
  pinMode(column4, INPUT_PULLUP);
  pinMode(column5, INPUT_PULLUP);

  // init rows as output and set high - inactive
  pinMode(row1, OUTPUT);
  digitalWrite(row1, HIGH);
  pinMode(row2, OUTPUT);
  digitalWrite(row2, HIGH);
  pinMode(row3, OUTPUT);
  digitalWrite(row3, HIGH);
  pinMode(row4, OUTPUT);
  digitalWrite(row4, HIGH);
  pinMode(row5, OUTPUT);
  digitalWrite(row5, HIGH);
  pinMode(row6, OUTPUT);
  digitalWrite(row6, HIGH);
  pinMode(row7, OUTPUT);
  digitalWrite(row7, HIGH);
  pinMode(row8, OUTPUT);
  digitalWrite(row8, HIGH);

  // setup key array
  for(row = 0; row < NUM_ROWS; row ++){
    for(column = 0; column < NUM_COLUMNS; column ++){
      keyHandlers[row][column] = KeyboardKey(row, column);
    }
  }

  Serial.println("finished setup");
  delay(10);

}

void loop() {
  // scan key matrix every 1 ms (+ execution time)
  //Serial.println("scanning");
  matrix.scanMatrix();
  delay(1);
}
