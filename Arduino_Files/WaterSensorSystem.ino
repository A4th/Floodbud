// This File is used to detect the water level within a singular sensor
// This will be used in case we would want to change "Trigger" Values in the actual system

#define POWER_PIN  7

#define MODERATESENSOR A5
#define HIGHSENSOR A6
#define VERYHIGHSENSOR A7

#define THRESHOLD 100

int moderate_sensor_value = 0; // variable to store WaterSensor1's value
int high_sensor_value = 0; // variable to store WaterSensor1's value
int veryhigh_sensor_value = 0; // variable to store WaterSensor1's value



int level = 0; // variable to store the water level

void setup() {
  Serial.begin(9600);
}

void loop() {
    //moderate_sensor_value = analogRead(MODERATESENSOR);
    //high_sensor_value = analogRead(HIGHSENSOR);
    //veryhigh_sensor_value = analogRead(VERYHIGHSENSOR);

    moderate_sensor_value = 100;
    high_sensor_value = 200;
    veryhigh_sensor_value = 100;

    if (moderate_sensor_value < THRESHOLD){
        Serial.println("Water Level: Low");
    }
    else {
        if (moderate_sensor_value >= THRESHOLD && high_sensor_value < THRESHOLD && veryhigh_sensor_value < THRESHOLD){
            Serial.println("Water Level: Moderate");
        }
        else if (moderate_sensor_value >= THRESHOLD && high_sensor_value >= THRESHOLD && veryhigh_sensor_value < THRESHOLD){
            Serial.println("Water Level: High");
        }
        else if (moderate_sensor_value >= THRESHOLD && high_sensor_value >= THRESHOLD && veryhigh_sensor_value >= THRESHOLD){
            Serial.println("Water Level: Very High");
        }
    }
    delay(1000);
}
