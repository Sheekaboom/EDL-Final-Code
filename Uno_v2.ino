#include <LiquidCrystal.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define NUM_SENDS 50
#define LP_VAL 204
#define HP_VAL 75
#define UHP_VAL 40

RF24 radio(9,10);

LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

const uint64_t pipeSend = 0x123456ABCDEF;
const uint64_t pipeRecieve = 0xABCDEF123456;

const int LV = 1; // analog pin for v1
const int LH = 0; // analog pin for h1
const int LSel = 1; // digital selection pin for joystick 1

const int RV = 3; // analog pin for v2
const int RH = 2; // analog pin for h2
const int RSel = 0; // digital selection pin for joystick 2
const int batVal = 4;

unsigned batStat;
unsigned batDisp;
unsigned mode=0;
unsigned mapval=0;


unsigned start=0,endStart=0;

int LSpeed=0;
int LDir=0;

int RSpeed=0;
int RDir=0;

unsigned packet[4]={0,0,0,0};
unsigned sendPack[4]={0,0};
unsigned sendPackA[4],sendPackB[4];

long LVertical, LHorizontal, RVertical, RHorizontal;
int LSelect, RSelect;

void setup()
{
  pinMode(LSel,INPUT);
  pinMode(RSel,INPUT);
  digitalWrite(LSel,HIGH);
  digitalWrite(RSel,HIGH);
 // Serial.begin(9600);
  
  lcd.begin(16,2);
  radio.begin();
  radio.openWritingPipe(pipeSend);
  radio.openReadingPipe(1,pipeRecieve);
  wiimote();
}

void loop() 
{
  batStat = 0;

 for(int i=0;i<NUM_SENDS;++i){
   
switch(mode){
  case 0: mapval = LP_VAL;
          if(!LSelect&&RSelect)
            mode = 1;
          if(LSelect&&!RSelect)
            mode = 2;
          break;
  case 1: mapval = HP_VAL;
          if(!LSelect&&!RSelect)
            mode = 0;
          if(LSelect&&!RSelect)
            mode = 2;
           break;
  case 2:  mapval = UHP_VAL;
          if(!LSelect&&!RSelect)
            mode = 0;
          if(!LSelect&&RSelect)
            mode = 1;
          break;
  default: mode = 0;
}
  
  LVertical = analogRead(LV); // will be 0-1023
  LHorizontal = analogRead(LH); // will be 0-1023
  LSelect = digitalRead(LSel); // will be HIGH (1) if not pressed, and LOW (0) if pressed
  
  RVertical = analogRead(RV); // will be 0-1023
  RHorizontal = analogRead(RH); // will be 0-1023
  RSelect = digitalRead(RSel); // will be HIGH (1) if not pressed, and LOW (0) if pressed


  if(LVertical > 515){  //these values were choosen to fit the standing voltages
    LDir=1;
    LSpeed = map(LVertical, 515, 1023, 0, mapval);
  }
  else if(LVertical < 513){  //these values were choosen to fit the standing voltages
    LDir=0;
    LSpeed = map(LVertical, 513, 1, mapval, 0);
    LSpeed=mapval-LSpeed;
  }
  else{
    LDir=2;
    LSpeed=0;    
  }
  
  if(RVertical < 525){  //these values were choosen to fit the standing voltages
    RDir=1;
    RSpeed = map(RVertical, 525, 1, 0, mapval);
  }
  else if(RVertical > 527){  //these values were choosen to fit the standing voltages
    RDir=0;
    RSpeed = map(RVertical, 527, 1023, mapval, 0);
    RSpeed=mapval-RSpeed;
  }
  else{
    RDir=2;
    RSpeed=0;    
  }
  //start = micros();
  
  packet[0]=LDir;
  packet[1]=LSpeed;
  packet[2]=RDir;
  packet[3]=RSpeed;
  
  radio.write(packet, sizeof(packet));
  delay(5);
  //endStart = micros();
  batStat = batStat + analogRead(batVal);
 }
 radio.startListening();
 //get single packet
 while(!radio.available())//sending two increases accuracy and makes sure there is one available
 radio.read(sendPackA,sizeof(sendPack));
 radio.read(sendPackB,sizeof(sendPack));
 sendPack[0] = (sendPackA[0]+sendPackB[0])/2;
 sendPack[1] = (sendPackA[1]+sendPackB[1])/2;
 sendPack[2] = (sendPackA[2]+sendPackB[2])/2;
 sendPack[3] = (sendPackA[3]+sendPackB[3])/2;
// Serial.print(sendPack[0]);Serial.print(" , ");Serial.print(sendPack[1]);Serial.print(" , ");Serial.print(sendPack[2]);
// Serial.print(" , ");Serial.println(sendPack[3]);Serial.println(digitalRead(RSel));
 radio.stopListening();
 lcd.clear();
 batDisp = (unsigned)map(batStat/NUM_SENDS,716,920,0,100)*2;
 packet[3] = map(packet[3],0,1023,0,500);
 lcdDisplay(sendPack,mode,batDisp);

}  

void lcdDisplay(unsigned *packet,int mode,int CB){
  lcd.home();
  lcd.write("L: ");
  lcd.print((int)packet[0]);
  lcd.setCursor(6,0);
  lcd.write("R: ");
  lcd.print((int)packet[1]);
  if(mode==1){
    lcd.setCursor(14,0);
    lcd.write("HP");
  }
  if(mode==2){
    lcd.setCursor(13,0);
    lcd.write("UHP");
  }
  lcd.setCursor(0,1);
  lcd.write("D:");
  lcd.print(packet[3]);
  lcd.setCursor(6,1);
  lcd.write("BL:");
  lcd.print(CB);
  lcd.write("% ");
  lcd.print(packet[2]);
  lcd.write("%");
}

void wiimote(){
  //displays "Reminder: Securely fasten the wrist strap"
  lcd.print("Reminder: Securely fasten the wriststrap");
  delay(1000);
  int i;
  for(i = 0; i <= 23;i++){
    //while(1){
    lcd.scrollDisplayLeft();
    delay(250);
  }
}
