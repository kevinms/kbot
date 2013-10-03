#include <SPI.h>

#include <nRF24L01.h>
#include <RF24.h>
#include <PololuQik.h>

// ========================================================================
// Enable/disable debug information
// ========================================================================
#define DEBUG 1
#if DEBUG
  #define DB(x, ...) Serial.print(x, ##__VA_ARGS__)
  #define DBLN(x, ...) Serial.println(x, ##__VA_ARGS__)
#else
  #define DB(x, ...)
  #define DBLN(x, ...)
#endif

// Debug categories
#define DB_MOTOR 1
#define DB_RADIO 1
#define DB_RADIO_TIMING 1
#define DB_RADIO_SIGNAL 1

// ========================================================================
// Motor controller globals
// ========================================================================
#define QIK_ERR_DATA_OVERRUN (1<<2)
#define QIK_ERR_FRAME        (1<<3)
#define QIK_ERR_CRC          (1<<4)
#define QIK_ERR_FORMAT       (1<<5)
#define QIK_ERR_TIMEOUT      (1<<6)

PololuQik2s9v1 qik(7, 8, 3);

// ========================================================================
// Radio globals
// ========================================================================
#define PROTO_TOKEN    0x4b
#define PROTO_STOP     0
#define PROTO_FORWARD  1
#define PROTO_BACKWARD 2
#define PROTO_RIGHT    4
#define PROTO_LEFT     8

#define PROTO_PACKET_SIZE 4  // bytes
#define PROTO_INPUT_DELAY 50 // milliseconds

RF24 radio(4,10);

// RX addresses
uint8_t addr0[5] = {0xaa, 0xbb, 0xcc, 0xdd, 0x00};
uint8_t addr1[5] = {0xde, 0xee, 0xde, 0xee, 0x11};
uint8_t addr2[5] = {0xde, 0xee, 0xde, 0xee, 0x22};
uint8_t addr3[5] = {0xde, 0xee, 0xde, 0xee, 0x33};
uint8_t addr4[5] = {0xde, 0xee, 0xde, 0xee, 0x44};
uint8_t addr5[5] = {0xde, 0xee, 0xde, 0xee, 0x55};

unsigned long t_last_cmd;
unsigned char prev_seqnum = 0;

// ========================================================================
// Motor controller initialization
// ========================================================================
void init_motor_controller()
{
  DBLN("init motor controller");
  qik.init();
  qik.setConfigurationParameter(QIK_CONFIG_SHUT_DOWN_MOTORS_ON_ERROR, 0);
  
  DB("Firmware: ");
  Serial.write(qik.getFirmwareVersion());
  DBLN();
  
  DB("Errors: ");
  DBLN(qik.getErrors(), BIN);
  
  DBLN("Config Params:");
  DB("  id: ");DBLN(qik.getConfigurationParameter(QIK_CONFIG_DEVICE_ID));
  DB("  pwm: ");DBLN(qik.getConfigurationParameter(QIK_CONFIG_PWM_PARAMETER));
  DB("  soe: ");DBLN(qik.getConfigurationParameter(QIK_CONFIG_SHUT_DOWN_MOTORS_ON_ERROR));
  DB("  timeout: ");DBLN(qik.getConfigurationParameter(QIK_CONFIG_SERIAL_TIMEOUT));
  
  qik.setSpeeds(0,0);
}

void test_motor_controller()
{
  for (int i = 0; i <= 127; i++)
    qik.setSpeeds(i,i);
  for (int i = 127; i >= -127; i--)
    qik.setSpeeds(i,i);
  for (int i = -127; i <= 0; i++)
    qik.setSpeeds(i,i);
}

// ========================================================================
// Radio initialization
// ========================================================================
void init_radio()
{
  DBLN("init radio");
  radio.begin();
  radio.setRetries(7,15);
  radio.setCRCLength(RF24_CRC_16);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(120);
  radio.setPayloadSize(PROTO_PACKET_SIZE);
  //radio.setAutoAck(true);
  
  radio.openReadingPipe(0,addr0);
  //radio.openReadingPipe(1,addr1);
  //radio.openReadingPipe(2,addr2);
  //radio.openReadingPipe(3,addr3);
  //radio.openReadingPipe(4,addr4);
  //radio.openReadingPipe(5,addr5);
  radio.openWritingPipe(addr0);
  
#if DB_RADIO
  radio.printDetails();
#endif
  
  radio.startListening();
}

bool is_seqnum_in_order(unsigned char seqnum)
{
  bool ret = true;
  if(seqnum != (prev_seqnum+1))
    ret = false;
  prev_seqnum = seqnum;
  return ret;
}

void test_radio_read()
{
  uint8_t pipe;
  bool done = false;
  uint8_t buf[PROTO_PACKET_SIZE];

#if DB_RADIO_SIGNAL
  if(radio.testRPD())
    DBLN("signal on channel");
#endif

  if(radio.available(&pipe)) {
    if(pipe) {
      DB("pipe: ");
      DB(pipe, BIN);
    }
    DB("Radio_RX=");
    while(!done) {
      done = radio.read( buf, PROTO_PACKET_SIZE );
      DB((char *)&buf);
    }
    DBLN("");
  }
}

void do_radio_control()
{
  static bool executing = false;
  uint8_t pipe = 0;
  bool got_data = false;
  uint8_t buf[PROTO_PACKET_SIZE];
  
  if(radio.available(&pipe)) {
#if DB_RADIO
    if(pipe) {
      DB("pipe: ");
      DB(pipe, BIN);
    }
#endif
    while(!got_data) {
      got_data = radio.read( buf, PROTO_PACKET_SIZE );
    }
  }

  char m1speed = 120;
  char m2speed = 120;

  if(got_data && (buf[0] == PROTO_TOKEN)) {
    executing = true;
    t_last_cmd = millis();
    unsigned char seqnum = buf[1];
    unsigned char cmd    = buf[2];

#if DB_RADIO
    DB("t: ");
    DB(t_last_cmd);
    DB(", PSEQ: ");
    DB(prev_seqnum);
    if(!is_seqnum_in_order(seqnum))
      DB(", OOO");
    DB(", SEQ: ");
    DB(seqnum, DEC);
    DB(", CMD: ");
    DBLN(cmd, DEC);
#endif

    if(cmd == PROTO_STOP)
      qik.setSpeeds(0, 0);
    if(cmd == PROTO_FORWARD) {
      qik.setSpeeds(-m1speed, -m2speed);
      Serial.println("go...");
    }
    if(cmd == PROTO_BACKWARD)
      qik.setSpeeds(m1speed, m2speed);
    if(cmd == PROTO_RIGHT)
      qik.setSpeeds(m1speed, -m2speed);
    if(cmd == PROTO_LEFT)
      qik.setSpeeds(-m1speed, m2speed);
  } else {
    if(executing && ((millis() - t_last_cmd) > PROTO_INPUT_DELAY)) {
#if DB_RADIO
      DBLN("No commands - STOPPING");
#endif
      executing = false;
      qik.setSpeeds(0, 0);
    }
  }
}

// ========================================================================
// General functions and setup/loop
// ========================================================================
void do_check_errors()
{
  byte err = qik.getErrors();
#if DB_MOTOR
  if(err != 0) {
    DB("MOTOR_Error: ");
    DBLN(err);
  }
#endif
}

void setup()
{
#if DEBUG
  Serial.begin(57600);
  delay(2000);
#endif

  init_radio();
  init_motor_controller();
}

void loop()
{
  //test_motor_controller();
  //test_radio_read();
  do_radio_control();
  do_check_errors();
}
