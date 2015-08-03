/**
PIN CONNECTIONS
ARDUINO D10  FUTABA SIGNAL-IN PIN (PIN6)
ARDUINO GND  FUTANA GND PIN (PIN2)

Futaba pinouts can be found here: http://static.rcgroups.net/forums/attachments/1/3/7/6/0/2/a6769970-151-Futaba%20trainer%20pinout.gif?d=1400215867

*This code can also work on any drone transmitter with TRAINER port


Prepared by Jimeno Juan.
**/

#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

#define out_channel 10  //set the number of chanels
#define default_servo_value 1490  //set the default servo value
#define PPM_FrLen 22000  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PPM_Pulsrec_elen 420  //set the pulse length
#define onState 0  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 10  //set PPM signal output pin on the arduino

String Axes = "";
int xAxis = 1450;
int yAxis = 1450;

char floatbuf[32];
String inString ="";
int ppm[out_channel];

int x;
int y;

unsigned long prevTime;
unsigned long currTime;
int timeDiff;
int stampDiff;
int prevX;
int currX;
int xDiff;

void setup() {
  prevTime = millis();
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  mySerial.begin(57600);

  for(int i=0; i<out_channel; i++){
    ppm[i]= default_servo_value;
  }

  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, !onState);  //set the PPM signal pin to the default state (off)
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;
  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
}

void loop() {
  //parse serial data from osvr clieng
  while (Serial.available())
  {
    char data = Serial.read(); // receive a character from BT port
    Axes.concat(data); // add the received character to buffer 'Axes'
    if (data == 'x') //if delimeter 'x' is read, assign x-axis value
    {
      y = Axes.toInt();
      Axes = "";
    }
    else if (data == 'g') //if delimeter 'y' is read, assign y-axis value
    {
      x = Axes.toInt();
      Axes = "";
    }
  }

  currTime = millis();
  timeDiff = currTime - prevTime;

  //check head angle for every 700ms
  if(timeDiff > 700)
  {
    //get yaw
    currX = x;
    //check if there's a change in yaw
    int xDiff = currX - prevX;
    timeDiff = 0;
    //get yaw for next loop
    prevTime = currTime;
    prevX = currX;
    mySerial.println(xDiff);

    //the head is tilting left if it is less than 0
    if((xDiff < -4) && (xDiff > -20))
    {
      //send the pwm signal to the transmitter (1100uS @ channel 7)
      ppm[6] = 1100;//x
    }
    //the head is tilting right if it is more than 0
    else if((xDiff > 4) && (xDiff < 20))
    {
      //send the pwm signal to the transmitter (1900uS @ channel 7)
      ppm[6] = 1900;//x
    }
    else
    {
      //if head isn't tilting send to neutral (1450uS @ channel7)
      ppm[6] = 1450;//x
    }
  }

  //convert pitch( -90 to 90 degrees) to pwm (2150uS to 750us)
  int y_ppm = map(y, 90, -90, 2150, 750);
  //semd [wm signal to the transmitter
  ppm[5] = y_ppm;

}

//leave this alone( signal encoder interrupts)
ISR(TIMER1_COMPA_vect){
  static boolean state = true;

  TCNT1 = 0;

  if(state) {  //start pulse
    digitalWrite(sigPin, onState);
    OCR1A = PPM_Pulsrec_elen * 2;
    state = false;
  }
  else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;

    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= out_channel){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PPM_Pulsrec_elen;//
      OCR1A = (PPM_FrLen - calc_rest) * 2;
      calc_rest = 0;
    }
    else{
      OCR1A = (ppm[cur_chan_numb] - PPM_Pulsrec_elen) * 2;
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }
  }
}
