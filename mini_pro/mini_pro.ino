#include <EEPROM.h>

#define RAIN_READER_PIN           A2
#define RAIN_POWER_PIN            A3
#define LED_PIN                   13

#define SERIAL_BUFFER_SIZE        32
#define SERIAL_SPEED              57600

#define SET_TRIGGER_VALUE_CMD     "trigger:"
#define SET_DELAY_VALUE_CMD       "delay:"
#define INFO_CMD                  "info"
#define VALUE_CMD                 "value"
#define EEPROM_VERSION            2

int triggerValue = 600;
int delayValue = 1000;
int beforeValue = 0;
int afterValue = 0;
unsigned long timeValue = 0;

static int readIntFromEEPROM(int address)
{
    int result;
    int ii, size = sizeof(int);
    byte *buffer;
    
    buffer = (byte *)&result;
    for (ii = 0; ii < size; ii++) {
        buffer[ii] = EEPROM.read(ii + address);
    }
    return result;
}

static void writeIntFromEEPROM(int address, int value)
{
    int ii, size = sizeof(int);
    byte *buffer;
    
    buffer = (byte *)&value;
    for (ii = 0; ii < size; ii++) {
        EEPROM.write(ii + address, buffer[ii]);
    }
}

static void readEEPROM(void)
{
    if (readIntFromEEPROM(0) == EEPROM_VERSION) {
        triggerValue = readIntFromEEPROM(sizeof(int));
        delayValue = readIntFromEEPROM(2 * sizeof(int));
    }
}

static void writeEEPROM(void)
{
    writeIntFromEEPROM(0, EEPROM_VERSION);
    writeIntFromEEPROM(sizeof(int), triggerValue);
    writeIntFromEEPROM(2 * sizeof(int), delayValue);
}

static void printInfo(void)
{
    Serial.print("version: ");
    Serial.print(EEPROM_VERSION);
    Serial.print(", trigger: ");
    Serial.print(triggerValue);
    Serial.print(", delay: ");
    Serial.println(delayValue);
}

static void printValue(void)
{
    Serial.print("value: ");
    Serial.print(timeValue);
    Serial.print(", before: ");
    Serial.print(beforeValue);
    Serial.print(", after: ");
    Serial.println(afterValue);
}

// the setup function runs once when you press reset or power the board
void setup()
{
    pinMode(RAIN_READER_PIN, INPUT);
    pinMode(RAIN_POWER_PIN, OUTPUT);
    Serial.begin(SERIAL_SPEED);
    readEEPROM();
    printInfo();
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(200);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
}

// the loop function runs over and over again forever
void loop()
{
    unsigned long current;
    int lastValue;
  
    if (Serial.available() > 0) {
        String line;
      
        line = Serial.readString();
        line.trim();
        if (line.startsWith(SET_TRIGGER_VALUE_CMD)) {
            String value;
            
            value = line.substring(strlen(SET_TRIGGER_VALUE_CMD));
            value.trim();
            triggerValue = value.toInt();
            writeEEPROM();
            printInfo();
        } else if (line.startsWith(SET_DELAY_VALUE_CMD)) {
            String value;
            
            value = line.substring(strlen(SET_DELAY_VALUE_CMD));
            value.trim();
            delayValue = value.toInt();
            writeEEPROM();
            printInfo();
        } else if (line == INFO_CMD) {
            printInfo();
        } else if (line == VALUE_CMD) {
            printValue();
        } else {
            Serial.print("unknown line");
        }
    }
    analogWrite(RAIN_POWER_PIN, 1023);
    delay(delayValue);
    beforeValue = analogRead(RAIN_READER_PIN);
    analogWrite(RAIN_POWER_PIN, 0);
    current = micros();
    while((afterValue = analogRead(RAIN_READER_PIN)) > triggerValue);
    timeValue = micros() - current;
}
