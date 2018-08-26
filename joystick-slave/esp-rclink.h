#ifndef ESPRCLink_h
#define ESPRCLink_h
#include <Ticker.h>

#define ESPRC_MASTER 0
#define ESPRC_SLAVE 1

#define ESPRC_OFF 0 //input, high impedance
#define ESPRC_SERVO 1
#define ESPRC_PWM 2
#define ESPRC_BLINK 3 //500 on 500 off
#define ESPRC_STROBE 4 //100 on 100 off 100 on 700 off 
#define ESPRC_DIGITAL 6 //6 and 7 for digital low and high

typedef struct ESPRCLinkMasterPacket {
  uint8_t mode[9]; // 0 = disabled (input), 1 = servo, 2 = PWM, 3 = Blink, 4 = Position Strobe, 6 = digital Low, 7 = digital High
  uint8_t data[9]; //0-255 for the channels
} ESPRCLinkMasterPacket;

typedef struct ESPRCLinkSlavePacket {
  uint16_t analogRead:10; //battery
  uint8_t linkQuality:6; //0-64 link quality
} ESPRCLinkSlavePacket;


  class ESPRCLinkClass {
  public:
  
  ESPRCLinkClass();
  
  static void init(String mac, uint8_t mode, uint8_t channel = 1);
  
  static String macToString(const uint8_t* mac);

  static ESPRCLinkSlavePacket getSlaveState();
  
  static void setMode(uint8_t pin, uint8_t type );
  
  static void write(uint8_t pin, uint8_t value);
  
  static uint8_t getQuality();
  
  static String getMac();
  static uint8_t deviceMode();
 
 static ESPRCLinkMasterPacket masterState;
  static ESPRCLinkSlavePacket slaveState;
  
  static void (*on_receive)();

    private:

  static void receive_data(const uint8_t *mac, const uint8_t *data, uint8_t len);
  static void send_data(const uint8_t *mac, uint8_t status);
  
  static void (*on_send)(uint8_t status);

  static void uploader();
  static Ticker uploaderTicker;
  
  static uint8_t paired[6];

  static uint8_t channel;
  static uint8_t mode; //0 = master, 1 = slave
  static uint8_t quality[100];
  static uint8_t quality_counter;
  };
  
  extern ESPRCLinkClass ESPRCLink;
  
#endif
