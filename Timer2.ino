#include <Arduino.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

//timer variables
int key = 0;
unsigned long buttonPressed = 0;
boolean stopped = true;
boolean timerRunning = false;

// button stuff
int longPress = 1000; // time in ms for long button press
boolean buttonShortPressed = false;
unsigned long startBounce;  //start of bounce time
int kpdelay=500; //keypad de-bounce (ms)
boolean bounceLockout = false;

int state = 0;  // 3 timer states, 0 = stopped (at zero), 1 = running, 2 = paused
int mode = 0; // 0 menu, 1 stop watch, 2 pomodoro, 3 counter

void resetValues();

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
unsigned long pomodoroTimer;
unsigned long pomodoroPaused;
int breakTime = (5*60)*1000; // 5 mins
int workTime = (25*60)*1000; // 25 mins
TM1637Display display(CLK, DIO);

void setup()
{
    pinMode(4, INPUT_PULLUP);
    display.setBrightness(0x0f);
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

    if (!bounceLockout) {
        startBounce = millis();
        bounceLockout = true;
        if (key == LOW) {
            if (!buttonPressed) { // if button is pressed record when
                buttonPressed = millis();
            }
            else if (millis() - buttonPressed > longPress) { // button long pressed
                if (mode != 0) {
                    mode = 0;
                }
                else {
                    mode = selected;
                    resetValues();
                }
                buttonPressed = 0;
            }
        }
        else if (buttonPressed > 0) { // button short pressed
            buttonShortPressed = true;
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
        selected %= 3 + 1; // cycle the 3 modes
        buttonShortPressed = false;
    }
    display.showNumberDec(selected, false);
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
        int rawSeconds = round((millis()-startMillis)/1000);
        display.showNumberDec(rawSeconds, false);
    }
    if (state == 0) { //reset the timer
        display.showNumberDec(0, false);  //reset LED display to zero
    }  
}

void pomodoro() {
    if (buttonShortPressed) {
        if (state != 1) {
            state = 1; // start timer
        }
        else {
            pomodoroPaused = millis();
            state = 2;
        }
        buttonShortPressed = false;
    }
    if (state == 0) { // timer setup from now
        pomodoroTimer = millis() + (count % 2 == 0 ? breakTime : workTime);
    }
    else if (state == 2) { // time left 
        int remaining = pomodoroTimer - pomodoroPaused;
        pomodoroTimer = millis() + remaining;
    }
    int pomodoroSeconds = round((pomodoroTimer - millis())/1000);
    display.showNumberDec(count, false);
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
}