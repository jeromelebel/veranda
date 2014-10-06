#include "application.h"
#include "OneWire.h"
#include "Sensor.h"
#include "DallasSensor.h"
#include "DHTSensor.h"

#define LED_PIN                     D7
#define MAGNET_SWITCH_PIN           D6
#define MAGNET_TESTER_PIN           D5
#define LIGHT_SENSOR_PIN            A7

#define RAIN_CHARGE_PIN             D0
#define RAIN_DISCHARGE_PIN          D1

#define MINI_PRO_SERIAL_SPEED       57600
#define MINI_PRO_RESET_PIN          D0

#define SENSOR_MAX                  10
OneWire onewire(D4);
Sensor *list[SENSOR_MAX];
DHTSensor dhtSensor(D1, DHTSensorType_22);

int rainDelay = 100;

int setDelay(String value)
{
    rainDelay = value.toInt();
    return 0;
}

int resetMiniPro(String value)
{
    digitalWrite(MINI_PRO_RESET_PIN, LOW);
    delay(500);
    digitalWrite(MINI_PRO_RESET_PIN, HIGH);
    return 0;
}

void setup(void)
{
    uint8_t address[8];
    int ii = 0;
    
    // MINI PRO
    Serial1.begin(MINI_PRO_SERIAL_SPEED);
    pinMode(MINI_PRO_RESET_PIN, OUTPUT);
    digitalWrite(MINI_PRO_RESET_PIN, HIGH);

    Serial.begin(115200);
    delay(2000);
    Serial.println("let's go\n");
    for (ii = 0; ii < SENSOR_MAX; ii++) {
        list[ii] = NULL;
    }
    onewire.reset_search();
    ii = 0;
    while (onewire.search(address)) {
        list[ii] = new DallasSensor(address, &onewire);
        list[ii]->begin();
        list[ii]->printAddress(&Serial);
        Serial.println(" ");
        ii++;
        if (ii == SENSOR_MAX) {
            break;
        }
    };
    Serial.print("dallas: ");
    Serial.println(ii);
    pinMode(LED_PIN, OUTPUT);
    pinMode(MAGNET_SWITCH_PIN, INPUT);
    pinMode(MAGNET_TESTER_PIN, INPUT);
    pinMode(LIGHT_SENSOR_PIN, INPUT);
    
    dhtSensor.begin();
    
    Spark.function("setdelay", setDelay);
    Spark.function("resetminipro", resetMiniPro);
}

int value = HIGH;

void loop()
{
    int ii;
    
    while (Serial.available()) {
        Serial1.write(Serial.read());
    }
    while (Serial1.available()) {
        Serial.write(Serial1.read());
    }
    for (ii = 0; ii < SENSOR_MAX; ii++) {
        if (list[ii]) {
            list[ii]->loop();
            list[ii]->printAddress(&Serial);
            Serial.print(" ");
            list[ii]->printValues(&Serial);
            Serial.println(" ");
        }
    }

    dhtSensor.loop();
    dhtSensor.printAddress(&Serial);
    Serial.print(" ");
    dhtSensor.printValues(&Serial);
    Serial.println(" ");
    
    Serial.print("switch ");
    if (digitalRead(MAGNET_SWITCH_PIN)) {
        Serial.println("HIGH");
    } else {
        Serial.println("LOW");
    }
    Serial.print("switch ");
    if (digitalRead(MAGNET_TESTER_PIN)) {
        Serial.println("HIGH");
    } else {
        Serial.println("LOW");
    }
    Serial.print("light sensor ");
    Serial.println(analogRead(LIGHT_SENSOR_PIN));
    
    Serial.println("--");
    delay(2000);
    digitalWrite(LED_PIN, value);
    if (value == LOW) {
        value = HIGH;
    } else {
        value = LOW;
    }
}
