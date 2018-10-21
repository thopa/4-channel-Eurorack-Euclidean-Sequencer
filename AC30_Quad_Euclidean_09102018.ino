//  ============================================================
//
//  Program: ArdCore Dual Euclidian Trigger Sequencer
//
//  Description: An implementation of a dual trigger sequencer
//               using Euclidian Rhythm concepts
//
//               Thanks to Robin Price (http://crx091081gb.net/)
//               for the use of his algorithm for Euclidean
//               Rhythm Generation.
//
//  I/O Usage:
//    A0: Steps for rhythm 1
//    A1: Steps for rhythm 2
//    A2: Steps for rhythm 3
//    A3: Steps for rhythm 4
//    A4: Pulses for rhythm 1
//    A5: Pulses for rhythm 2
//    A6: Pulses for rhythm 3
//    A7: Pulses for rhythm 4
//    Digital Out 1: Rhythm 1 output
//    Digital Out 2: Rhythm 2 output
//    Digital Out 3: Rhythm 3 output
//    Digital Out 4: Rhythm 4 output
//    Clock In: External clock input
//    Analog Out: Echoes the clock input
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  25 Sept 2012
//  Modified:
//
//  ============================================================
//
//  License:
//
//  This software is licensed under the Creative Commons
//  "Attribution-NonCommercial license. This license allows you
//  to tweak and build upon the code for non-commercial purposes,
//  without the requirement to license derivative works on the
//  same terms. If you wish to use this (or derived) work for
//  commercial work, please contact 20 Objects LLC at our website
//  (www.20objects.com).
//
//  For more information on the Creative Commons CC BY-NC license,
//  visit http://creativecommons.org/licenses/
//  ================= start of global section ==================

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[4] = {3, 4, 5, 6};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // 25 ms trigger timing

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[4] = {LOW, LOW, LOW, LOW};        // start with both set low
unsigned long digMilli[4] = {0, 0, 0, 0};  // a place to store millis()

//  variables used to control the current AIO output state
int anaState = LOW;
unsigned long anaMilli = 0;

// the euclidian rhythm settings
int inSteps[4];
int inPulses[4];
int inRotate[4];

int euArray[4][32];

unsigned long currPulse = 0;
int doCalc = 0;

//  ==================== start of setup() ======================
void setup()
{

  Serial.begin(9600);

  // set up the digital (clock) input
  pinMode(clkIn, INPUT);

  // set up the digital outputs
  for (int i = 0; i < 4; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }



  // get the analog reading to set up the system
  inSteps[0] = (analogRead(0) >> 5) + 1;
  inSteps[1] = (analogRead(1) >> 5) + 1;
  inSteps[2] = (analogRead(2) >> 5) + 1;
  inSteps[3] = (analogRead(3) >> 5) + 1;

  inPulses[0] = (analogRead(4) >> 5) + 1;
  inPulses[1] = (analogRead(5) >> 5) + 1;
  inPulses[2] = (analogRead(6) >> 5) + 1;
  inPulses[3] = (analogRead(7) >> 5) + 1;

  inRotate[0] = 0;
  inRotate[1] = 0;
  inRotate[2] = 0;
  inRotate[3] = 0;

  euCalc(0);
  euCalc(1);
  euCalc(2);
  euCalc(3);

  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
}


void loop()
{
  int doClock = 0;

  // check to see if the clock as been set
  if (clkState == HIGH) {
    clkState = LOW;
    currPulse++;
    doClock = 1;
  }

  if (doClock) {
    int outPulse[4] = {0, 0, 0, 0};

    for (int i = 0; i < 4; i++) {
      int myPulse = (currPulse + inRotate[i]) % inSteps[i];
      outPulse[i] = euArray[i][myPulse];
    }

    for (int i = 0; i < 4; i++) {
      if (outPulse[i] > 0) {
        digState[i] = HIGH;
        digMilli[i] = millis();
        digitalWrite(digPin[i], HIGH);
      }
    }

    anaState = HIGH;
    anaMilli = millis();

  }

  // do we have to turn off any of the digital outputs?
  for (int i = 0; i < 4; i++) {
    if ((digState[i] == HIGH) && (millis() - digMilli[i] > trigTime)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
    }
  }

  // do we have to turn off the analog output?
  if ((anaState == HIGH) && (millis() - anaMilli > trigTime)) {
    anaState = LOW;

  }

  // reread the inputs in case we need to change
  /////////////////////////////////////////////////


  doCalc = 0;
  int tmp = (analogRead(0) >> 5) + 1;
  if (tmp != inSteps[0]) {
    inSteps[0] = tmp;
    doCalc = 1;
  }

  tmp = (analogRead(4) >> 5) + 1;
  if (tmp != inPulses[0]) {
    inPulses[0] = tmp;
    doCalc = 1;
  }

  if (doCalc) {
    euCalc(0); //////////seems to change depending on the index of inSteps & inPulses [0] -THIS WAS IT
  }

  ////////////////////////////////////////////

  doCalc = 0;
  tmp = (analogRead(1) >> 5) + 1;
  if (tmp != inSteps[1]) {
    inSteps[1] = tmp;
    doCalc = 1;
  }

  tmp = (analogRead(5) >> 5) + 1;
  if (tmp != inPulses[1]) {
    inPulses[1] = tmp;
    doCalc = 1;
  }

  if (doCalc) {
    euCalc(1);//////////seems to change depending on the index of inSteps & inPulses [1] -THIS WAS IT
  }

  /////////////////////////////////////////
  doCalc = 0;
  tmp = (analogRead(2) >> 5) + 1;
  if (tmp != inSteps[2]) {
    inSteps[2] = tmp;
    doCalc = 1;
  }

  tmp = (analogRead(6) >> 5) + 1;
  if (tmp != inPulses[2]) {
    inPulses[2] = tmp;
    doCalc = 1;
  }

  if (doCalc) {
    euCalc(2);//////////seems to change depending on the index of inSteps & inPulses [2] -THIS WAS IT
  }
  //////////////////////////////////
  doCalc = 0;
  tmp = (analogRead(3) >> 5) + 1;
  if (tmp != inSteps[3]) {
    inSteps[3] = tmp;
    doCalc = 1;
  }

  tmp = (analogRead(7) >> 5) + 1;
  if (tmp != inPulses[3]) {
    inPulses[3] = tmp;
    doCalc = 1;
  }

  if (doCalc) {
    euCalc(3); //////////seems to change depending on the index of inSteps & inPulses [3] -THIS WAS IT
  }
}


//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  clkState = HIGH;
}



//  euCalc(int) - create a Euclidean Rhythm array.
//
//  NOTE: Thanks to Robin Price for his excellent implementation, and for
//        making the source code available on the Interwebs.
//        For more info, check out: http://crx091081gb.net/
//  ----------------------------------------------------------------------
void euCalc(int ar) {
  int loc = 0;

  // clear the array to start
  for (int i = 0; i < 32; i++) {
    euArray[ar][i] = 0;
  }

  if ((inPulses[ar] >= inSteps[ar]) || (inSteps[ar] == 1)) {
    if (inPulses[ar] >= inSteps[ar]) {
      for (int i = 0; i < inSteps[ar]; i++) {
        euArray[ar][loc] = 1;
        loc++;
      }
    }
  } else {
    int offs = inSteps[ar] - inPulses[ar];
    if (offs >= inPulses[ar]) {
      int ppc = offs / inPulses[ar];
      int rmd = offs % inPulses[ar];

      for (int i = 0; i < inPulses[ar]; i++) {
        euArray[ar][loc] = 1;
        loc++;
        for (int j = 0; j < ppc; j++) {
          euArray[ar][loc] = 0;
          loc++;
        }
        if (i < rmd) {
          euArray[ar][loc] = 0;
          loc++;
        }
      }
    } else {
      int ppu = (inPulses[ar] - offs) / offs;
      int rmd = (inPulses[ar] - offs) % offs;

      for (int i = 0; i < offs; i++) {
        euArray[ar][loc] = 1;
        loc++;
        euArray[ar][loc] = 0;
        loc++;
        for (int j = 0; j < ppu; j++) {
          euArray[ar][loc] = 1;
          loc++;
        }
        if (i < rmd) {
          euArray[ar][loc] = 1;
          loc++;
        }
      }
    }
  }
}

//  ===================== end of program =======================rmd
