#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <c_types.h>
#include <espnow.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <esp_now.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

#include "esp-rclink.h"

  void (*ESPRCLinkClass::on_receive)() = 0;
  void (*ESPRCLinkClass::on_send)(uint8_t status) = 0;
  
  uint8_t ESPRCLinkClass::paired[6]={};
  uint8_t ESPRCLinkClass::mode=0;
  uint8_t ESPRCLinkClass::channel=1;
  uint8_t ESPRCLinkClass::quality[100]={};
  uint8_t ESPRCLinkClass::quality_counter=0;
  Ticker ESPRCLinkClass::uploaderTicker;
  
  ESPRCLinkMasterPacket ESPRCLinkClass::masterState;
  ESPRCLinkSlavePacket ESPRCLinkClass::slaveState;
    
  ESPRCLinkClass::ESPRCLinkClass(){
  }

  String ESPRCLinkClass::macToString(const uint8_t* mac) {
    char buf[20];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
  }

  void ESPRCLinkClass::setMode(uint8_t pin, uint8_t type){
    ESPRCLinkClass::masterState.mode[pin]=type;
  }

  void ESPRCLinkClass::write(uint8_t pin, uint8_t value){
    ESPRCLinkClass::masterState.data[pin]=value;
  }
  
  uint8_t ESPRCLinkClass::getQuality(){
    uint8_t result=0;
    for (uint8_t i=0; i<64; i++) result += quality[i];
    return (float)result*100/64;
  }
  
  void ESPRCLinkClass::init(String mac, uint8_t mode, uint8_t channel){
    WiFi.mode(WIFI_AP);
    
    if (mode == ESPRC_MASTER) WiFi.softAP(WiFi.softAPmacAddress().c_str(), 0, channel, 0);
    else WiFi.softAP(WiFi.softAPmacAddress().c_str(), 0, channel, 1);

    //WiFi.disconnect();
    esp_now_init();
   #ifdef ESP8266
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
   #endif
    esp_now_register_send_cb(reinterpret_cast<esp_now_send_cb_t>(&ESPRCLinkClass::send_data));
    esp_now_register_recv_cb(reinterpret_cast<esp_now_recv_cb_t>(&ESPRCLinkClass::receive_data));

    mac.replace("-", "");
    mac.replace(":", "");
    mac.replace(" ", "");
    mac.toUpperCase();
    for (int i=0; i<6; i++) {
      paired[i] = ((mac[i*2] > '9') ? (mac[i*2]-'A'+0xA):(mac[i*2]-'0')) << 4 | ((mac[i*2+1] > '9') ? (mac[i*2+1]-'A'+0xA):(mac[i*2+1]-'0'));
    }

  #if defined(ESP8266)
    esp_now_add_peer(paired, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
  #elif defined(ESP32)
    esp_now_peer_info_t pi;
  
    memset(&pi, 0, sizeof(pi));
    memcpy(pi.peer_addr, paired, ESP_NOW_ETH_ALEN);
    pi.channel = channel;
    pi.ifidx = ESP_IF_WIFI_AP;
    esp_now_add_peer(&pi);
  #endif

    ESPRCLinkClass::mode = mode;
    memset(&ESPRCLinkClass::masterState, 0, sizeof(ESPRCLinkMasterPacket));
    ESPRCLinkClass::uploaderTicker.attach_ms(10, &ESPRCLinkClass::uploader);
  }
  
  String ESPRCLinkClass::getMac(){
    return WiFi.softAPmacAddress();
  }

  uint8_t ESPRCLinkClass::deviceMode(){
    return ESPRCLinkClass::mode;
  }

  void ESPRCLinkClass::uploader(){
    uint8_t data[13];
    memset (data, 0, 13);
    if (ESPRCLinkClass::mode) { //slave sending
      uint16_t adcread = analogRead(A0);
      data[0] = adcread;
      data[1] = (adcread>>8)&0b11000000;
      data[1] |= ESPRCLinkClass::slaveState.linkQuality&0b00111111;
      esp_now_send(ESPRCLinkClass::paired, data, 13);
    } else { //master sending
      uint8_t j=0;
      for (uint8_t i=0; i<9; i++){
        if (ESPRCLinkClass::masterState.mode[i]==1 || ESPRCLinkClass::masterState.mode[i]==2) data[ 4 + j++] = ESPRCLinkClass::masterState.data[i];
        else if (ESPRCLinkClass::masterState.mode[i]>=6 ) ESPRCLinkClass::masterState.mode[i] = ESPRC_DIGITAL + ESPRCLinkClass::masterState.data[i];
      }
      
      data[0] |= (ESPRCLinkClass::masterState.mode[0]& 0b111) << 5 ;
      data[0] |= (ESPRCLinkClass::masterState.mode[1]& 0b111) << 2 ;
      data[0] |= (ESPRCLinkClass::masterState.mode[2]& 0b110) >> 1 ;
      data[1] |= (ESPRCLinkClass::masterState.mode[2]& 0b001) << 7 ;
      data[1] |= (ESPRCLinkClass::masterState.mode[3]& 0b111) << 4 ;
      data[1] |= (ESPRCLinkClass::masterState.mode[4]& 0b111) << 1 ;
      data[1] |= (ESPRCLinkClass::masterState.mode[5]& 0b100) >> 2 ;
      data[2] |= (ESPRCLinkClass::masterState.mode[5]& 0b011) << 6 ;
      data[2] |= (ESPRCLinkClass::masterState.mode[6]& 0b111) << 3 ;
      data[2] |= (ESPRCLinkClass::masterState.mode[7]& 0b111) ;
      data[3] |= (ESPRCLinkClass::masterState.mode[8]& 0b111) <<5 ;

      esp_now_send(ESPRCLinkClass::paired, data, 4+j);
//      for (uint8_t i=0; i<9; i++) if (ESPRCLinkClass::masterState.mode[i]==7) ESPRCLinkClass::masterState.mode[i]=6;
//      Serial.println("===DATA-S===");
//      Serial.println(data[0], HEX);Serial.println(data[1], HEX);Serial.println(data[2], HEX);Serial.println(data[3], HEX);
//      Serial.println("===DATA-E===");
    }
  }
  
  void ESPRCLinkClass::receive_data(const uint8_t *mac, const uint8_t *data, uint8_t len) {
    if (ESPRCLinkClass::mode) { //master sent
      ESPRCLinkClass::masterState.mode[0] = (data[0]>>5) & 0b111;
      ESPRCLinkClass::masterState.mode[1] = (data[0]>>2) & 0b111;
      ESPRCLinkClass::masterState.mode[2] = (data[0]<<1) & 0b110 | (data[1]>>7) & 0b001;
      ESPRCLinkClass::masterState.mode[3] = (data[1]>>4) & 0b111;
      ESPRCLinkClass::masterState.mode[4] = (data[1]>>1) & 0b111;
      ESPRCLinkClass::masterState.mode[5] = (data[1]<<2) & 0b100 | (data[2]>>6) & 0b011;
      ESPRCLinkClass::masterState.mode[6] = (data[2]>>3) & 0b111;
      ESPRCLinkClass::masterState.mode[7] = (data[2]) & 0b111;
      ESPRCLinkClass::masterState.mode[8] = (data[3]>>5) & 0b111;
      uint8_t j=0;
      for (uint8_t i=0; i<9; i++){
        if (ESPRCLinkClass::masterState.mode[i]==ESPRC_SERVO || ESPRCLinkClass::masterState.mode[i]==ESPRC_PWM) ESPRCLinkClass::masterState.data[i]=data[ 4 + j++];
        else if (ESPRCLinkClass::masterState.mode[i]==ESPRC_DIGITAL+1) ESPRCLinkClass::masterState.data[i]=1;
//        else if (ESPRCLinkClass::masterState.mode[i]==ESPRC_DIGITAL) ESPRCLinkClass::masterState.data[i]=0; //ambigous
        else ESPRCLinkClass::masterState.data[i]=0;
      }
    } else { //slave sent
      ESPRCLinkClass::slaveState.analogRead = (uint16_t) data[0]<<2 | data[1]>>6;
      ESPRCLinkClass::slaveState.linkQuality = data[1]&0xF;
    }
    if (on_receive) on_receive();
//    Serial.print("Size: ");
//    Serial.println(len);
  }
  
  void ESPRCLinkClass::send_data(const uint8_t *mac, uint8_t status) {
    quality[quality_counter]=!status;
    quality_counter = (quality_counter+1)%64;
  }
