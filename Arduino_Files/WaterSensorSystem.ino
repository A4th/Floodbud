// This File is used to detect the water level within a singular sensor
// This will be used in case we would want to change "Trigger" Values in the actual system


#define POWER_PIN  7

int value1 = 0; // variable to store WaterSensor1's value
int value2 = 0; // variable to store WaterSensor1's value
int value3 = 0; // variable to store WaterSensor1's value

int level = 0; // variable to store the water level
int WaterSensor1 = A5;
int WaterSensor2 = A6;
int WaterSensor3 = A7; 


void setup() {
  Serial.begin(9600);
}

void loop() {
    value1 = analogRead(WaterSensor1);
    value2 = analogRead(WaterSensor2);
    value3 = analogRead(WaterSensor3);

    if (value1 <= 100){
        Serial.println("Water Level: Empty");
    }
    else {
        if (value1 >= 100 && value2 <= 100 && value3 <= 100){
            Serial.println("Water Level: Low")
        }
        else if (value1 >= 100 && value2 >= 100 && value3 <= 100){
            Serial.println("Water Level: Medium")
        }
        else if (value1 >= 100 && value2 >= 100 && value3 >= 100){
            Serial.println("Water Level: High")
        }
    }
    delay(1000);
}
