#include <PS2X.h>
#include <WebServer.h>
#include "esp-rclink.h"

/*
PSB_SELECT       0x0001
PSB_L3           0x0002
PSB_R3           0x0004
PSB_START        0x0008
PSB_PAD_UP       0x0010
PSB_PAD_RIGHT    0x0020
PSB_PAD_DOWN     0x0040
PSB_PAD_LEFT     0x0080
PSB_L2           0x0100
PSB_R2           0x0200
PSB_L1           0x0400
PSB_R1           0x0800
PSB_GREEN        0x1000
PSB_RED          0x2000
PSB_BLUE         0x4000
PSB_PINK         0x8000
PSB_TRIANGLE     0x1000
PSB_CIRCLE       0x2000
PSB_CROSS        0x4000
PSB_SQUARE       0x8000
*/

int16_t off_rx=0, off_ry=0, off_lx=0, off_ly=0;

PS2X joystick;
WebServer server(80);

unsigned int interval;

unsigned int sinceTimer(const unsigned int millis_value)
{
  return millis()-millis_value;
}

void resetTimer(unsigned int &millis_value)
{
  millis_value = millis();
}


void dataIn(){
}

void httpLoop(void *pvParameters) {
  while (1){
  server.handleClient();
  delay(30);
  }
}

void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='2'/>\
    <title>ESP RC-Link</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>ESP RC-Link</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>Battery: %fV</p>\
    <p>Link: %d%%</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60, 3.3, ESPRCLink.getQuality()
          );
  server.send(200, "text/html", temp);
  
}

void setup() {
  Serial.begin(115200);

  resetTimer(interval);
  
  joystick.ConfigGamepad(12, 14, 26, 27);
  
  joystick.ReadGamepad();
  delay(100);
  joystick.ReadGamepad();

  off_rx = joystick.Analog(PSS_RX);
  off_ry = joystick.Analog(PSS_RY);
  off_lx = joystick.Analog(PSS_LX);
  off_ly = joystick.Analog(PSS_LY);
  
//  ESPRCLink.init("30:AE:A4:1E:8C:3D", ESPRC_SLAVE);
  ESPRCLink.init("5E:CF:7F:B2:E9:25", ESPRC_MASTER);
  ESPRCLink.on_receive = dataIn;
  Serial.println();
  Serial.println("Hi, I'm "+ESPRCLink.getMac());
  ESPRCLink.setMode(0, ESPRC_STROBE);
  ESPRCLink.setMode(4, ESPRC_SERVO);

  server.on("/", handleRoot);
  server.begin();
  xTaskCreatePinnedToCore(httpLoop, "HTTP", 8192 , NULL, 0, NULL, 0);
}

void loop() {
//    Serial.println("LOOP");
//  server.handleClient();
  
  joystick.ReadGamepad();
  if (joystick._buttons == 0xf0ff) { //calibration of joysticks when L1 L2 R1 R2 pressed
    off_rx = joystick.Analog(PSS_RX);
    off_ry = joystick.Analog(PSS_RY);
    off_lx = joystick.Analog(PSS_LX);
    off_ly = joystick.Analog(PSS_LY);
  }
  
//  ESPRCLink.write(0, !joystick.Button(PSB_CROSS));
  ESPRCLink.write(4, constrain(90+(joystick.Analog(PSS_RY)-off_ly)/1.4, 0, 180));

//    if (sinceTimer(interval)>100){
//      Serial.write(27);       // ESC command
//      Serial.print("[2J");    // clear screen command
//      Serial.write(27);
//      Serial.print("[H");     // cursor to home command
//      Serial.print(ESPRCLink.getQuality());
////      Serial.print("\n");
////      Serial.print(90+(joystick.Analog(PSS_RY)-off_ly)/1.4);
//      resetTimer(interval);
//    }


  /*
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  if (joystick._buttons >> 15 == 0 ) Serial.print(0);
  Serial.println(joystick._buttons, BIN);
  Serial.print(" U : "); Serial.println(joystick.Button(PSB_PAD_UP));
  Serial.print(" D : "); Serial.println(joystick.Button(PSB_PAD_DOWN));
  Serial.print(" L : "); Serial.println(joystick.Button(PSB_PAD_LEFT));
  Serial.print(" R : "); Serial.println(joystick.Button(PSB_PAD_RIGHT));
  Serial.print("/_\\: "); Serial.println(joystick.Button(PSB_TRIANGLE));
  Serial.print(" O : "); Serial.println(joystick.Button(PSB_CIRCLE));
  Serial.print(" X : "); Serial.println(joystick.Button(PSB_CROSS));
  Serial.print("[ ]: "); Serial.println(+joystick.Button(PSB_SQUARE));
  Serial.println(((joystick.Analog(PSS_LX) - off_lx)*(joystick.Analog(PSS_LX) - off_lx))/128);
  Serial.println(((joystick.Analog(PSS_LY) - off_ly)*(joystick.Analog(PSS_LY) - off_ly))/128);
  Serial.println(joystick.Analog(PSS_RX) - off_rx);
  Serial.println(joystick.Analog(PSS_RY) - off_ry);
  */
  delay(10);
}
