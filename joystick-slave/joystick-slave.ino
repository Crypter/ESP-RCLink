#include <Servo.h>
#include "esp-rclink.h"

int16_t off_rx=0, off_ry=0, off_lx=0, off_ly=0;

Servo servos[9];

uint8_t oldModes[9];

uint8_t pinMappings[9] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};

unsigned int timeout, blink;
uint8_t reset = 0;

unsigned int sinceTimer(const unsigned int millis_value)
{
  return millis()-millis_value;
}

void resetTimer(unsigned int &millis_value)
{
  millis_value = millis();
}


void dataIn(){
  resetTimer(timeout);
  
  for (int i=0; i<9; i++){
    if(oldModes[i] != ESPRCLink.masterState.mode[i] || reset){
      oldModes[i] = ESPRCLink.masterState.mode[i];
      if (ESPRCLink.masterState.mode[i]==ESPRC_OFF){
        if (servos[i].attached()) servos[i].detach();
        pinMode(pinMappings[i], INPUT);
      } else if (ESPRCLink.masterState.mode[i]==ESPRC_SERVO) {
        servos[i].attach(pinMappings[i]);
      } else if (ESPRCLink.masterState.mode[i]>ESPRC_SERVO) {
        if (servos[i].attached()) servos[i].detach();
        pinMode(pinMappings[i], OUTPUT);
      }
    }
    if (ESPRCLink.masterState.mode[i] == ESPRC_SERVO){
      servos[i].write(ESPRCLink.masterState.data[i]);
    } else if (ESPRCLink.masterState.mode[i] == ESPRC_PWM){
      analogWrite(pinMappings[i], ESPRCLink.masterState.data[i]);
    } else if (ESPRCLink.masterState.mode[i] >= ESPRC_DIGITAL){
      digitalWrite(pinMappings[i], ESPRCLink.masterState.data[i]);
    }
  }
  reset=0;
}

void setup() {
  analogWriteRange(255);
  Serial.begin(115200);

  resetTimer(timeout);
  resetTimer(blink);
  
  ESPRCLink.init("30:AE:A4:1E:8C:3D", ESPRC_SLAVE);
//  ESPRCLink.init("5E:CF:7F:B2:E9:25", ESPRC_MASTER);
  ESPRCLink.on_receive = dataIn;
  Serial.println();
  Serial.println("Hi, I'm "+ESPRCLink.getMac());
}

void loop() {
  if (sinceTimer(timeout)>5000) {
    reset=1;
    for (int i=0; i<9; i++){
      if (ESPRCLink.masterState.mode[i]!=ESPRC_BLINK && ESPRCLink.masterState.mode[i]!=ESPRC_STROBE){
        if (servos[i].attached()) servos[i].detach();
        pinMode(pinMappings[i], INPUT);
      }
    }
  }

  unsigned int toBlink = sinceTimer(blink);
  if ( (toBlink<=100) || (toBlink>200 && toBlink<=300) ) {
    for (int i=0; i<9; i++){
      if (ESPRCLink.masterState.mode[i]==ESPRC_BLINK || ESPRCLink.masterState.mode[i]==ESPRC_STROBE){
        digitalWrite (pinMappings[i], LOW);
      }
    }
  }
  if ( (toBlink>100 && toBlink<=200) || (toBlink>300) ) {
    for (int i=0; i<9; i++){
      if (ESPRCLink.masterState.mode[i]==ESPRC_STROBE){
        digitalWrite (pinMappings[i], HIGH);
      }
    }
  }
  if ( toBlink>=500 ) {
    for (int i=0; i<9; i++){
      if (ESPRCLink.masterState.mode[i]==ESPRC_BLINK || ESPRCLink.masterState.mode[i]==ESPRC_STROBE){
        digitalWrite (pinMappings[i], HIGH);
//        Serial.println("DEBUG");
      }
    }
  }
  
  if (toBlink >= 1000) {
    resetTimer(blink);
    
      Serial.println("==BEGIN==");
    for (int i=0; i<9; i++){
      Serial.print(i);
      Serial.print(" => ");
      Serial.println(ESPRCLink.masterState.mode[i]);
    }
      Serial.println("==END==");
  }
}
