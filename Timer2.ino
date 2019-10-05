#include <Arduino.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

//timer variables
int key = 0;
boolean stopped = true;
boolean timerRunning = false;

unsigned long startMillis;  //start of timer
unsigned long startBounce;  //start of bounce time
int kpdelay=500; //keypad de-bounce (ms)
boolean bounceLockout = false;

int state = 0;  // 3 timer states, 0 = stopped (at zero), 1 = running, 2 = paused

TM1637Display display(CLK, DIO);

void setup()
{
    pinMode(4, INPUT_PULLUP);
    display.setBrightness(0x0f);
    //reset LED display to zero
    display.showNumberDec(0, false);
}

void stopWatch();

void loop()
{
    key = digitalRead(4);   // read the key state

    

    delay(10);
}

void stopWatch() {
    //start timer if stopped, timer not running and keypressed
  if ((state == 0) && (key == LOW) && (!bounceLockout)) 
  {
    startMillis = millis();
    startBounce = millis();
    state = 1;
    bounceLockout = true;
  }

 //if the bounce time is exceeded then reset bounce
    if ((bounceLockout) && (millis()>(startBounce+kpdelay)))
    {
      bounceLockout = false;
    }  

//display the time
  if (state == 1) 
  {
    //display seconds on the LED display
    display.showNumberDec(round((millis()-startMillis)/1000), false);
    // send some debug info to the serial port (9600 baud default)
    Serial.print("seconds : ");
    Serial.print(round((millis()-startMillis)/1000));
    Serial.print(" lockout : ");
    Serial.println(bounceLockout);
    //if 9999 seconds exceeded then reset timer (add code to enable this functionality!)
  }

//pause the time if key pressed
 if ((state == 1) && (key == LOW) && (!bounceLockout)) 
  {
    startBounce = millis();
    state = 2;
    bounceLockout = true;
  }

//reset the timer if paused
 if ((state == 2) && (key == LOW) && (!bounceLockout)) 
  {
    startBounce = millis();
    //reset LED display to zero
    display.showNumberDec(0, false);
    state = 0;
    bounceLockout = true;
  }  
}