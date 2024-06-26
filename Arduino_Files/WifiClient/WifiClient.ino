
#include <UIPEthernet.h>

// replace the MAC address below by the MAC address printed on a sticker on the Arduino Shield 2
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetClient client;

int HTTP_PORT = 80;
String HTTP_METHOD = "POST";
char SERVER_ADD[] = "shannentan.pythonanywhere.com";
String PATH_NAME = "/";

int moderate_sensor_value = 0;
int high_sensor_value = 0;
int veryhigh_sensor_value = 0;

#define MODERATESENSOR A2
#define HIGHSENSOR A1
#define VERYHIGHSENSOR A0

#define MODERATETHRESHOLD 100
#define HIGHTHRESHOLD     250
#define VERYHIGHTHRESHOLD 240

#define MODERATELED 5
#define HIGHLED 6
#define VERYHIGHLED 7

void (*resetFunc)(void) = 0;

int prevlevel = 0;
int levelint = 0;
int sensorValue = 0;
int contentLength;
char contentLengthArr[4];
char deviceID[] = "1";
char waterLevel[16];
char jsonObject[50];

void setup()
{
  Serial.begin(9600);
  // initialize the Ethernet shield using DHCP:
  pinMode(MODERATELED, OUTPUT);
  pinMode(HIGHLED, OUTPUT);
  pinMode(VERYHIGHLED, OUTPUT);


}

void loop()
{

  // if (millis() > 10000)
  // {
  //   Serial.println("resetting cuz 10s");
  //   resetFunc();
  // }
  getSensorData();
  LED_process();
  createJsonObject(jsonObject);

  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to obtaining an IP address using DHCP");
    while (true);
  }

  // connect to web server on port 80:
  if (client.connect(SERVER_ADD, HTTP_PORT))
  {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    client.println(HTTP_METHOD + " " + PATH_NAME + " HTTP/1.1");
    client.println("Host: " + String(SERVER_ADD));
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.print("Content-Length: "); // end HTTP header'
    client.println(contentLengthArr);
    client.println("");
    client.println(jsonObject);

    while (client.connected())
    {
      if (client.available())
      {
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
         //Serial.print(c);
      }
    }

    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  }
  else
  { // if not connected:
    Serial.println("connection failed");
  }

}

void LED_process(void)
{
  Serial.print(waterLevel);
  if (String(waterLevel) == "LOW")
  {
    Serial.print("WaterLevel == LOW");
    digitalWrite(MODERATELED, 0);
    digitalWrite(HIGHLED, 0);
    digitalWrite(VERYHIGHLED, 0);
  }

  else if (String(waterLevel) == "MODERATE")
  {
    Serial.print("WaterLevel == MODERATE");
    digitalWrite(MODERATELED, 1);
    digitalWrite(HIGHLED, 0);
    digitalWrite(VERYHIGHLED, 0);
  }

  else if (String(waterLevel) == "HIGH")
  {
    Serial.print("WaterLevel == HIGH");
    digitalWrite(MODERATELED, 0);
    digitalWrite(HIGHLED, 1);
    digitalWrite(VERYHIGHLED, 0);
  }

  else if (String(waterLevel) == "VERY HIGH")
  {
    Serial.print("WaterLevel == VERY HIGH");
    digitalWrite(MODERATELED, 0);
    digitalWrite(HIGHLED, 0);
    digitalWrite(VERYHIGHLED, 1);
  }
}

void getSensorData(void)
{
  moderate_sensor_value = analogRead(MODERATESENSOR);
  high_sensor_value = analogRead(HIGHSENSOR);
  veryhigh_sensor_value = analogRead(VERYHIGHSENSOR);
  Serial.println(moderate_sensor_value);
  Serial.println(high_sensor_value);
  Serial.println(veryhigh_sensor_value);
  if (moderate_sensor_value < MODERATETHRESHOLD && high_sensor_value < HIGHTHRESHOLD && veryhigh_sensor_value < VERYHIGHTHRESHOLD)
  {
    strncpy(waterLevel, "LOW", sizeof(waterLevel));
    levelint = 0;
  }
  else if (high_sensor_value < HIGHTHRESHOLD)
  {
    strncpy(waterLevel, "MODERATE", sizeof(waterLevel));
    levelint = 1;
  }
  else if (veryhigh_sensor_value < VERYHIGHTHRESHOLD)
  {
    strncpy(waterLevel, "HIGH", sizeof(waterLevel));
    levelint = 2;
  }
  else
  {
    strncpy(waterLevel, "VERY HIGH", sizeof(waterLevel));
    levelint = 3;
  }
  // printValues();
  // strncpy(waterLevel,HIGH,sizeof(waterLevel));
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