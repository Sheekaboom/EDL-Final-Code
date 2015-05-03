#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define VERBOSE
#define LEFT_TRIM 10
#define RIGHT_TRIM 8
#define NUM_RECS 50

//speed control settings
#define INITSPEED 150

const int onOff = 6;      //On/off switch  
const int CW_L = 7;       //Clockwise control left
const int CCW_L = 8;      //CCW control left
const int REF_L = 9;      //left ref voltage
const int REF_R = 10;      //right reference control
const int CW_R = 11;      //Clockwise right control
const int CCW_R = 12;     //counterclock right control
const int batVal = 0;

const int ultrasonic = A1;

unsigned batStat = 0;

unsigned start = 0;
unsigned stopval = 0;

unsigned joystick[4];
unsigned sendPack[4];
unsigned rightSpeed = 0, leftSpeed = 0;
int k;

RF24 radio(48,49);

const uint64_t pipeRecieve = 0x123456ABCDEF;
const uint64_t pipeSend = 0xABCDEF123456;

void setup(void)
{
  Serial.begin(9600);
  
  radio.begin();
  radio.openWritingPipe(pipeSend);
  radio.openReadingPipe(1,pipeRecieve);
  radio.startListening();
  pinMode(ultrasonic,INPUT);
  
  pinMode(onOff,INPUT);
  pinMode(CW_L, OUTPUT);
  pinMode(CCW_L, OUTPUT);
  pinMode(REF_L, OUTPUT);
  pinMode(CW_R, OUTPUT);
  pinMode(CCW_R, OUTPUT);
  pinMode(REF_R,OUTPUT);
  digitalWrite(CW_L, LOW);
  digitalWrite(CCW_L, LOW);
  digitalWrite(CW_R, LOW);
  digitalWrite(CCW_R, LOW); //stop the robot to start
  analogWrite(REF_L, INITSPEED);
  analogWrite(REF_R, INITSPEED);
}

void loop(void)
{
 for(int i=0;i<NUM_RECS;++i){
  if ( radio.available() )
  {
    // Dump the payloads until we've gotten everything
    bool done = false;
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( joystick, sizeof(joystick) );
      #ifdef VERBOSE
      Serial.print(joystick[0]);
      Serial.print(", ");
      Serial.print(joystick[1]);
      Serial.print(", ");
      Serial.print(joystick[2]);
      Serial.print(", ");
      Serial.println(joystick[3]);
      #endif
      
     //trim using this hopefully wont take too long
     if(((int)joystick[1]-LEFT_TRIM)<0)
       leftSpeed = 0;
     else 
       leftSpeed = joystick[1] - LEFT_TRIM;
       
     if(((int)joystick[3]-RIGHT_TRIM)<0)
       rightSpeed = 0;
     else 
       rightSpeed = joystick[3] - RIGHT_TRIM;
       
    }
    //left wheel control
    if(joystick[0]==1){
      digitalWrite(CW_L,LOW);
      digitalWrite(CCW_L,HIGH);
      analogWrite(REF_L,leftSpeed);
    }
    else if(joystick[0]==0){
      digitalWrite(CW_L,HIGH);
      digitalWrite(CCW_L,LOW);
      analogWrite(REF_L,leftSpeed);
    }
    else{
      digitalWrite(CW_L,LOW);
      digitalWrite(CCW_L,LOW);
    }
    //right wheel control
    if(joystick[2]==1){
      digitalWrite(CW_R,HIGH);
      digitalWrite(CCW_R,LOW);
      analogWrite(REF_R,rightSpeed);
    }
    else if(joystick[2]==0){
      digitalWrite(CW_R,LOW);
      digitalWrite(CCW_R,HIGH);
      analogWrite(REF_R,rightSpeed);
    }
    else{
      digitalWrite(CW_R,LOW);
      digitalWrite(CCW_R,LOW);
    }
      
  }
  batStat = batStat+analogRead(batVal);
 }
 sendPack[0] = leftSpeed;//ultrasonic();
 sendPack[1] = rightSpeed;
 sendPack[2] = map(batStat/NUM_RECS,716,1028,0,100)*2;
 sendPack[3] = analogRead(ultrasonic);
 radio.stopListening();
 radio.write(sendPack,sizeof(sendPack));//sending two increases response time
 radio.write(sendPack,sizeof(sendPack));
 radio.startListening();
 batStat = 0;
 ++k;
}

