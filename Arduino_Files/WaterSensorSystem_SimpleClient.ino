// This File is used to detect the water level within a singular sensor
// This will be used in case we would want to change "Trigger" Values in the actual system
#include <SPI.h>
#include <Ethernet.h>

#define POWER_PIN  7

#define MODERATESENSOR A5
#define HIGHSENSOR A6
#define VERYHIGHSENSOR A7

#define THRESHOLD 100

byte mac[] = {0xC0,0xDE,0xBA,0xBE,0x00,0x01};

IPAddress ip(192,168,1,2);
EthernetClient client;
String PostData = "someDataToPost";


int moderate_sensor_value = 0; // variable to store WaterSensor1's value
int high_sensor_value = 0; // variable to store WaterSensor1's value
int veryhigh_sensor_value = 0; // variable to store WaterSensor1's value

int level = 0; // variable to store the water level

void setup() {
  Serial.begin(9600);

  if (client.connect(server, 80)) {
    client.println("POST /Api/AddParking/3 HTTP/1.1");
    client.println("Host: 192.168.1.2");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println();
    client.println(PostData);
  }
}

void loop() {

    //moderate_sensor_value = analogRead(MODERATESENSOR);
    //high_sensor_value = analogRead(HIGHSENSOR);
    //veryhigh_sensor_value = analogRead(VERYHIGHSENSOR);
    
    if (moderate_sensor_value < THRESHOLD && level != 1){
        level = 1;
        printValues();

        moderate_sensor_value = 101;
    }
    else if (moderate_sensor_value >= THRESHOLD && high_sensor_value < THRESHOLD && veryhigh_sensor_value < THRESHOLD && level != 2) {
        level = 2;
        printValues();

        high_sensor_value = 101;
    }
    else if (moderate_sensor_value >= THRESHOLD && high_sensor_value >= THRESHOLD && veryhigh_sensor_value < THRESHOLD && level != 3) {
        level = 3;
        printValues();

        veryhigh_sensor_value = 101;
    }
    else if (moderate_sensor_value >= THRESHOLD && high_sensor_value >= THRESHOLD && veryhigh_sensor_value >= THRESHOLD && level != 4){
        level = 4;
        printValues();
    }
    
    delay(1000);
}

void printValues() {

    Serial.println("-----START OF READING-----");

    switch (level) {
      case 1:
        Serial.println("Water Level: Low");
        break;
      case 2:
        Serial.println("Water Level: Moderate");
        break;
      case 3:
        Serial.println("Water Level: High");
        break;
      case 4:
        Serial.println("Water Level: Very High");
        break;

    }

    Serial.print("Moderate Sensor: ");
    Serial.println(moderate_sensor_value);
    Serial.print("High Sensor: ");
    Serial.println(high_sensor_value);
    Serial.print("Very High Sensor: ");
    Serial.println(veryhigh_sensor_value);

    Serial.println("------END OF READING------");
}
