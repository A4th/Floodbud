// Newest Feature: Non-hardcoded values
// Needs: Sensor Reading and Continuous Sending

#include "etherShield.h"
#include <string.h>

/*infrared sensor setting*/
#define INFRARED_IN 3
#define LED_STATUS 5
#define ENABLE_EXTERNAL1_INTERRUPT() (EIMSK |= (1 << INT1))
#define DISABLE_EXTERNAL1_INTERRUPT() (EIMSK &= ~(1 << INT1))

#define MODERATESENSOR A2
#define HIGHSENSOR A1
#define VERYHIGHSENSOR A0

#define MODERATETHRESHOLD 100
#define HIGHTHRESHOLD 300
#define VERYHIGHTHRESHOLD 300

#define MODERATELED 5
#define HIGHLED 6
#define VERYHIGHLED 7

#define LOW "LOW"
#define MODERATE "MODERATE"
#define HIGH "HIGH"
#define VERYHIGH "VERY HIGH"

int moderate_sensor_value = 0;
int high_sensor_value = 0;
int veryhigh_sensor_value = 0;

int level = 0;

int timer = 0;

// please modify the following lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
static uint8_t mymac[6] = {0xC0, 0xDE, 0xBA, 0xBE, 0x00, 0x01};
static uint8_t myip[4] = {192, 168, 1, 1};
static uint16_t my_port = 60; // client port

// client_ip - modify it when you have multiple client on the network
// for server to distinguish each ethershield client
static char client_ip[] = "192.168.1.1";

void(* resetFunc) (void) = 0;

// server settings - modify the service ip to your own server
static uint8_t dest_ip[4] = {192, 168, 1, 3};
static uint8_t dest_mac[6];

// global variables
int sensorValue = 0;
int contentLength;
char contentLengthArr[4];
char deviceID[] = "1";
char waterLevel[16];
char jsonObject[50];

enum CLIENT_STATE
{
  IDLE,
  ARP_SENT,
  ARP_REPLY,
  SYNC_SENT
};

static CLIENT_STATE client_state;

static uint8_t client_data_ready;

static uint8_t syn_ack_timeout = 0;

#define BUFFER_SIZE 500
static uint8_t buf[BUFFER_SIZE + 1];

EtherShield es = EtherShield();

void setup()
{
  Serial.begin(9600);
  getSensorData();
  pinMode(MODERATELED, OUTPUT);
  pinMode(HIGHLED, OUTPUT);
  pinMode(VERYHIGHLED, OUTPUT);

  /*initialize enc28j60*/
  es.ES_enc28j60Init(mymac);
  es.ES_enc28j60clkout(2); // change clkout from 6.25MHz to 12.5MHz
  delay(10);

  /* Magjack leds configuration, see enc28j60 datasheet, page 11 */
  // LEDA=greed LEDB=yellow
  //
  // 0x880 is PHLCON LEDB=on, LEDA=on
  // enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
  es.ES_enc28j60PhyWrite(PHLCON, 0x880);
  delay(500);
  //
  // 0x990 is PHLCON LEDB=off, LEDA=off
  // enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
  es.ES_enc28j60PhyWrite(PHLCON, 0x990);
  delay(500);
  //
  // 0x880 is PHLCON LEDB=on, LEDA=on
  // enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
  es.ES_enc28j60PhyWrite(PHLCON, 0x880);
  delay(500);
  //
  // 0x990 is PHLCON LEDB=off, LEDA=off
  // enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
  es.ES_enc28j60PhyWrite(PHLCON, 0x990);
  delay(500);
  //
  // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
  // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
  es.ES_enc28j60PhyWrite(PHLCON, 0x476);
  delay(100);

  // init the ethernet/ip layer:
  es.ES_init_ip_arp_udp_tcp(mymac, myip, 80);

  // intialize varible;
  syn_ack_timeout = 0;
  client_data_ready = 0;
  client_state = IDLE;

  ENABLE_EXTERNAL1_INTERRUPT();
  // tigger at INT1 rising edge
  EICRA = 0x0c;

  SREG |= 1 << SREG_I;
}

void loop()
{
  LED_process();
  if (client_data_ready == 1)
  {
    DISABLE_EXTERNAL1_INTERRUPT();
    client_process();
  }
  else
  {
    delay(100);
    ENABLE_EXTERNAL1_INTERRUPT();
  }
  if (millis() > 10000) {
    getSensorData();
    resetFunc();
  }
}

ISR(INT1_vect)
{
  client_data_ready = 1;
}

uint16_t gen_client_request(uint8_t *buf)
{
  uint16_t plen;
  byte i;
  createJsonObject(jsonObject);
  plen = es.ES_fill_tcp_data_p(buf, 0, PSTR("POST / "));

  plen = es.ES_fill_tcp_data_p(buf, plen, PSTR("HTTP/1.1\r\n"));
  plen = es.ES_fill_tcp_data_p(buf, plen, PSTR("Host: 192.168.1.3\r\n"));
  plen = es.ES_fill_tcp_data_p(buf, plen, PSTR("Content-Type: application/json\r\n"));
  plen = es.ES_fill_tcp_data_p(buf, plen, PSTR("Content-Length: "));
  plen = es.ES_fill_tcp_data(buf, plen, contentLengthArr);
  plen = es.ES_fill_tcp_data_p(buf, plen, PSTR("\r\n\r\n"));
  plen = es.ES_fill_tcp_data(buf, plen, jsonObject);
  return plen;
}

//*****************************************************************************************
//
// Function : client_process
// Description : send temparature to web server, this option is disabled by default.
// YOU MUST install webserver and server script before enable this option,
// I recommented Apache webserver and PHP script.
// More detail about Apache and PHP installation please visit http://www.avrportal.com/
//
//*****************************************************************************************
void client_process(void)
{
  uint16_t plen;
  uint8_t i;

  if (client_data_ready == 0)
    return; // nothing to send

  if (client_state == IDLE)
  { // initialize ARP
    es.ES_make_arp_request(buf, dest_ip);

    client_state = ARP_SENT;
    return;
  }

  if (client_state == ARP_SENT)
  {

    plen = es.ES_enc28j60PacketReceive(BUFFER_SIZE, buf);

    // destination ip address was found on network
    if (plen != 0)
    {
      if (es.ES_arp_packet_is_myreply_arp(buf))
      {
        client_state = ARP_REPLY;
        syn_ack_timeout = 0;
        return;
      }
    }
    delay(10);
    syn_ack_timeout++;

    if (syn_ack_timeout == 100)
    { // timeout, server ip not found
      client_state = IDLE;
      client_data_ready = 0;
      syn_ack_timeout = 0;
      return;
    }
  }

  // send SYN packet to initial connection
  if (client_state == ARP_REPLY)
  {
    // save dest mac
    for (i = 0; i < 6; i++)
    {
      dest_mac[i] = buf[ETH_SRC_MAC + i];
    }

    es.ES_tcp_client_send_packet(
        buf,
        5050,
        my_port,
        TCP_FLAG_SYN_V, // flag
        1,              // (bool)maximum segment size
        1,              // (bool)clear sequence ack number
        0,              // 0=use old seq, seqack : 1=new seq,seqack no data : new seq,seqack with data
        0,              // tcp data length
        dest_mac,
        dest_ip);

    client_state = SYNC_SENT;
  }
  // get new packet
  if (client_state == SYNC_SENT)
  {
    plen = es.ES_enc28j60PacketReceive(BUFFER_SIZE, buf);

    // no new packet incoming
    if (plen == 0)
    {
      return;
    }

    // check ip packet send to avr or not?
    // accept ip packet only
    if (es.ES_eth_type_is_ip_and_my_ip(buf, plen) == 0)
    {
      return;
    }

    // check SYNACK flag, after AVR send SYN server response by send SYNACK to AVR
    if (buf[TCP_FLAGS_P] == (TCP_FLAG_SYN_V | TCP_FLAG_ACK_V))
    {

      // send ACK to answer SYNACK

      es.ES_tcp_client_send_packet(
          buf,
          5050,
          my_port,
          TCP_FLAG_ACK_V, // flag
          0,              // (bool)maximum segment size
          0,              // (bool)clear sequence ack number
          1,              // 0=use old seq, seqack : 1=new seq,seqack no data : new seq,seqack with data
          0,              // tcp data length
          dest_mac,
          dest_ip);
      // setup http request to server
      plen = gen_client_request(buf);
      // send http request packet
      // send packet with PSHACK
      es.ES_tcp_client_send_packet(
          buf,
          5050,                             // destination port
          my_port,                          // source port
          TCP_FLAG_ACK_V | TCP_FLAG_PUSH_V, // flag
          0,                                // (bool)maximum segment size
          0,                                // (bool)clear sequence ack number
          0,                                // 0=use old seq, seqack : 1=new seq,seqack no data : >1 new seq,seqack with data
          plen,                             // tcp data length
          dest_mac,
          dest_ip);
      return;
    }
    // after AVR send http request to server, server response by send data with PSHACK to AVR
    // AVR answer by send ACK and FINACK to server
    if (buf[TCP_FLAGS_P] == (TCP_FLAG_ACK_V | TCP_FLAG_PUSH_V))
    {
      plen = es.ES_tcp_get_dlength((uint8_t *)&buf);

      // send ACK to answer PSHACK from server
      es.ES_tcp_client_send_packet(
          buf,
          5050,           // destination port
          my_port,        // source port
          TCP_FLAG_ACK_V, // flag
          0,              // (bool)maximum segment size
          0,              // (bool)clear sequence ack number
          plen,           // 0=use old seq, seqack : 1=new seq,seqack no data : >1 new seq,seqack with data
          0,              // tcp data length
          dest_mac,
          dest_ip);
      ;

      es.ES_tcp_client_send_packet(
          buf,
          5050,                            // destination port
          my_port,                         // source port
          TCP_FLAG_FIN_V | TCP_FLAG_ACK_V, // flag
          0,                               // (bool)maximum segment size
          0,                               // (bool)clear sequence ack number
          0,                               // 0=use old seq, seqack : 1=new seq,seqack no data : >1 new seq,seqack with data
          0,
          dest_mac,
          dest_ip);
      return;
    }
    // answer FINACK from web server by send ACK to web server
    if (buf[TCP_FLAGS_P] == (TCP_FLAG_ACK_V | TCP_FLAG_FIN_V))
    {
      // send ACK with seqack = 1
      es.ES_tcp_client_send_packet(

          buf,
          5050,           // destination port
          my_port,        // source port
          TCP_FLAG_ACK_V, // flag
          0,              // (bool)maximum segment size
          0,              // (bool)clear sequence ack number
          1,              // 0=use old seq, seqack : 1=new seq,seqack no data : >1 new seq,seqack with data
          0,
          dest_mac,
          dest_ip);
      client_state = IDLE;   // return to IDLE state
      client_data_ready = 0; // client data sent
      
    }
  }
}

void createJsonObject(char *jsonPlaceholder)
{
  String jsonString = "{";

  // Add DeviceID
  jsonString += "\"DeviceID\": \"";
  jsonString += deviceID;
  jsonString += "\", ";

  // Add WaterLevel
  jsonString += "\"WaterLevel\": \"";
  jsonString += waterLevel;
  jsonString += "\"}";

  contentLength = jsonString.length();
  sprintf(contentLengthArr, "%d", contentLength);

  jsonString.toCharArray(jsonPlaceholder, 50);
  return;
}

void sendSensorDataPacket(void)
{
  // send finack to disconnect from web server
  es.ES_tcp_client_send_packet(
      buf,
      5050,                            // destination port
      my_port,                         // source port
      TCP_FLAG_FIN_V | TCP_FLAG_ACK_V, // flag
      0,                               // (bool)maximum segment size
      0,                               // (bool)clear sequence ack number
      0,                               // 0=use old seq, seqack : 1=new seq,seqack no data : >1 new seq,seqack with data
      0,
      dest_mac,
      dest_ip);
  return;
}

void getSensorData(void)
{
  moderate_sensor_value = analogRead(MODERATESENSOR);
  high_sensor_value = analogRead(HIGHSENSOR);
  veryhigh_sensor_value = analogRead(VERYHIGHSENSOR);
  if (moderate_sensor_value < MODERATETHRESHOLD && high_sensor_value < HIGHTHRESHOLD && veryhigh_sensor_value < VERYHIGHTHRESHOLD)
  {
    strncpy(waterLevel,LOW,sizeof(waterLevel)); 
  }
  else if (high_sensor_value < HIGHTHRESHOLD)
  {
    strncpy(waterLevel,MODERATE,sizeof(waterLevel)); 
  }
  else if (veryhigh_sensor_value < VERYHIGHTHRESHOLD)
  {
    strncpy(waterLevel,HIGH,sizeof(waterLevel)); 
  }
  else
  {
    strncpy(waterLevel,VERYHIGH,sizeof(waterLevel)); 
  }
  printValues();
  // strncpy(waterLevel,HIGH,sizeof(waterLevel)); 
}

void printValues() {

    Serial.println("-----START OF READING-----");

    Serial.print("Moderate Sensor: ");
    Serial.println(moderate_sensor_value);
    Serial.print("High Sensor: ");
    Serial.println(high_sensor_value);
    Serial.print("Very High Sensor: ");
    Serial.println(veryhigh_sensor_value);

    Serial.println("------END OF READING------");
}


void LED_process(void){
  Serial.print(waterLevel);
  if (String(waterLevel) == "LOW"){
    Serial.print("WaterLevel == LOW");
    digitalWrite(MODERATELED, 0);
    digitalWrite(HIGHLED, 0);
    digitalWrite(VERYHIGHLED, 0);
  }

  else if (String(waterLevel) == "MODERATE"){
    Serial.print("WaterLevel == MODERATE");
    digitalWrite(MODERATELED, 1);
    digitalWrite(HIGHLED, 0);
    digitalWrite(VERYHIGHLED, 0);
  }

  else if (String(waterLevel) == "HIGH"){
    Serial.print("WaterLevel == HIGH");
    digitalWrite(MODERATELED, 0);
    digitalWrite(HIGHLED, 1);
    digitalWrite(VERYHIGHLED, 0);
  }

  else if (String(waterLevel) == "VERY HIGH"){
    Serial.print("WaterLevel == VERY HIGH");
    digitalWrite(MODERATELED, 0);
    digitalWrite(HIGHLED, 0);
    digitalWrite(VERYHIGHLED, 1);
  }
}