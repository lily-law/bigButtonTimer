#include <Arduino.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

//timer variables
int key = 0;
boolean stopped = true;
boolean timerRunning = false;

// button stuff
int longPress = 1000; // time in ms for long button press
boolean buttonShortPressed = false;
boolean buttonLongPressed = false;
unsigned long buttonPressed = 0; // time button was pressed
unsigned long startBounce;  //start of bounce time
int kpdelay=500; //keypad de-bounce (ms)
boolean bounceLockout = false;
void debounceButton(int ms);

int state = 0;  // 3 timer states, 0 = stopped (at zero), 1 = running, 2 = paused
int mode = 0; // 0 menu, 1 stop watch, 2 pomodoro, 3 counter

void resetValues();
void displayTime(unsigned long ms, boolean flash);
unsigned long lastTimeToDisp = 0; // for display time to compare change for flash
unsigned long dotsFlashedOnMsAgo = 0; // for testing when last dots flashed
void beep();
uint8_t data[] = { 0x19, 0x1a, 0x1b, 0x1c }; 
uint8_t dataSTOP[] = { 0x6D, 0x78, 0x3F, 0x73 }; 
uint8_t dataPOMO[] = { 0x73, 0x3F, 0x55, 0x3F }; 
uint8_t dataCONT[] = { 0x39, 0x3F, 0x54, 0x78 }; 

/*
 * 0x06: 1, 0x07: 7, 0x08: _, 
 */

// menu
void menu();
int selected = 1;

// stopWatch 
void stopWatch();
unsigned long startMillis;  //start of timer

// counter 
void counter();
int count = 0;

// pomodoro
void pomodoro();
unsigned long lastPomoTime = 0;
unsigned long breakTime = 60000*5; // 5 mins
unsigned long longBreakTime = 60000*30; // 30 mins
unsigned long workTime = 60000*25; // 25 mins
unsigned long pomodoroTimer = workTime;

TM1637Display display(CLK, DIO);

void setup()
{
    pinMode(4, INPUT_PULLUP);
    pinMode(5, OUTPUT);
    //display.setBrightness(0x0f);
    display.setBrightness(2);
    //reset LED display to zero
    display.showNumberDec(0, false);
}

void loop()
{
    key = digitalRead(4);   // read the key state

    //if the bounce time is exceeded then reset bounce
    if ((bounceLockout) && (millis()>(startBounce+kpdelay)))
    {
      bounceLockout = false;
    }  

    if (!buttonShortPressed && !bounceLockout) {
        if (key == LOW) {
            if (!buttonPressed) { // record when first pressed
                buttonPressed = millis();
            }
            else if (millis() - buttonPressed > longPress && !buttonLongPressed) { // button long pressed
                buttonLongPressed = true;
                beep(1, 100, 100);
                if (mode != 0) {
                    mode = 0;
                }
                else {
                    resetValues();
                    mode = selected;
                }
                debounceButton(100);
            }
        }
        else if (millis() - buttonPressed > longPress) {
            buttonPressed = 0;
            buttonLongPressed = false;
        }
        else if (buttonPressed > 0 && !bounceLockout) { // button short pressed
            debounceButton(50);
            buttonShortPressed = true;
            beep(1, 50, 50);
            buttonPressed = 0;
        }
    }

    switch(mode) {
        case 0:
        menu();
        break;
        case 1:
        stopWatch();
        break;
        case 2:
        pomodoro();
        break;
        case 3:
        counter();
        break;
    }
   
    delay(10);
}

void menu() {
    if (buttonShortPressed) {
        selected %= 3; // cycle the 3 modes
        selected += 1;
        buttonShortPressed = false;
    }
   // display.showNumberDec(selected, false);
   switch(selected) {
    case 1:
    display.setSegments(dataSTOP);
    break;
    case 2:
    display.setSegments(dataPOMO);
    break;
    case 3:
    display.setSegments(dataCONT);
    break;
   }
}

void stopWatch() {
    if (buttonShortPressed) {
        state++;
        state %= 3; // start, pause, stop
        if (state == 1) {
            startMillis = millis();
        }
        buttonShortPressed = false;
    }
    if (state == 1) { //display the time
        //display seconds on the LED display
        unsigned long rawMs = millis()-startMillis;
        displayTime(rawMs, true);
    }
    if (state == 0) { //reset the timer
        displayTime(0, false);  //reset LED display to zero
    }  
}

void pomodoro() {
    unsigned long msElapsed = millis() - lastPomoTime;
    if (buttonShortPressed) {
        if (state != 1) {
            state = 1; // start timer
        }
        else {
            state = 2;
        }
        buttonShortPressed = false;
    }
    if (msElapsed > 100) {
    if (state == 0) { // stopped, set timer start
        pomodoroTimer = ((count % 2) == 0) ? workTime : (count > 1 && ((count + 1) % 8) == 0) ? longBreakTime : breakTime;
        displayTime(pomodoroTimer, false);
    }
    else if (state == 1) { // running
        if (pomodoroTimer < 1000) { // timer finished
          state = 0;
          count += 1;
          beep(15, 50, 25);
        }
        else {
          pomodoroTimer -= msElapsed;
          displayTime(pomodoroTimer, true);
        }
    }
    lastPomoTime = millis();
    }
}

void counter() {
    if (buttonShortPressed) {
        count++;
        buttonShortPressed = false;
    }
    display.showNumberDec(count, false);
}

void resetValues() {
    count = 0;
    state = 0;
}

void displayTime(unsigned long ms, boolean flash) {
  unsigned long seconds = round(ms/1000);
  int secs = seconds % 60;
  int mins = (seconds > 59) ? (floor(seconds / 60)*100) : 0;
  int timeToDisp = mins + secs;
  timeToDisp = timeToDisp > 9999 ? 9999 : timeToDisp;
  int showDots = 0;
  if (lastTimeToDisp != timeToDisp || !flash) {
     showDots = 1;
     dotsFlashedOnMsAgo = millis();
  }
  else if (millis() - dotsFlashedOnMsAgo < 500) {
    showDots = 1;
  }
  display.showNumberDecEx(timeToDisp, (0x80 >> showDots), true);
  lastTimeToDisp = timeToDisp;
}

void debounceButton(int ms) {
  if (!bounceLockout) {
    kpdelay = ms;
    startBounce = millis();
    bounceLockout = true;
  }
}

void beep(int num, int len, int gap) {
  for (int i=0; i<num; i++) {
    analogWrite(5, 127);
    delay(len);
    analogWrite(5, 0);
    delay(gap);
  }
}
