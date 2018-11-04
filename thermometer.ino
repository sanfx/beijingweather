
#include "Dewpnt_heatIndx.h"
#include <ThingSpeak.h>
#include <Adafruit_DHT.h>
// https://www.hackster.io/monkbroc/from-0-to-iot-in-15-minutes-3e2607

// Sensor type
#define DHTTYPE DHT22    	// DHT 22 (AM2302)

// DHT22 sensor pinout:
// Pin 1 (on the left): +3.3V
// Pin 2: output
// Pin 4 (on the right): GROUND
#define DHT_5V_PIN D1
#define DHT_SENSOR_PIN D2
#define DHT_GROUND_PIN D4

// min/max values (sanity checks)
#define MIN_TEMPERATURE -30
#define MAX_TEMPERATURE 120

#define MIN_HUMIDITY 0
#define MAX_HUMIDITY 100
// which digital pin for the Photon/Spark Core/Electron LED
#define LEDPIN D7

// sensor sending interval (seconds)
#define SEND_INTERVAL 10

DHT dht(DHT_SENSOR_PIN, DHTTYPE);

int failed = 0;
// last time since we sent sensor readings
int lastUpdate = 0;

/* Thingspeak */
TCPClient client;
unsigned long myChannelNumber = 349697;
const char * myWriteAPIKey = "QPZNTP1R6SQ6B6PX";

void setup() {
    // Connect to ThingSpeak
    ThingSpeak.begin(client);

    // Give power to the sensor
    pinMode(DHT_5V_PIN, OUTPUT);
    pinMode(DHT_GROUND_PIN, OUTPUT);
    digitalWrite(DHT_5V_PIN, HIGH);
    digitalWrite(DHT_GROUND_PIN, LOW);

    // Wait for the sensor to stabilize
    delay(1000);

    // Initialize sensor
    dht.begin();
// run the first measurement
    loop();
}

void loop() {
    
    int now = Time.now();

    // only run every SEND_INTERVAL seconds
    if (now - lastUpdate < SEND_INTERVAL) {
        return;
    }

    // turn on LED when updating
    digitalWrite(LEDPIN, HIGH);

    lastUpdate = now;
    
    // Read Sensor
    double temperature = dht.getTempCelcius();
    double temperatureF = (temperature * 1.8) + 32;
    double humidity = dht.getHumidity();
    double heatIndex = Funcs::Dewpnt_heatIndx::heatIndex(temperatureF, humidity);
    
    
    if (temperature == NAN
        || humidity == NAN
        || temperature > MAX_TEMPERATURE
        || temperature < MIN_TEMPERATURE
        || humidity > MAX_HUMIDITY
        || humidity < MIN_HUMIDITY) {
        // if any sensor failed, bail on updates
        failed = 1;
    } else {
        // calculate the heat index
        heatIndex = Funcs::Dewpnt_heatIndx::heatIndex(temperatureF, humidity);
        // set all 3 fields first
        // Update the 2 ThingSpeak fields with the new data
        ThingSpeak.setField(1, (float)temperature);
        ThingSpeak.setField(2, (float)humidity);
        ThingSpeak.setField(3, (float)heatIndex);
        
        // Write the fields that you've set all at once.
        
        ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    }
    // done updating
    digitalWrite(LEDPIN, LOW);

    // Give time for the message to reach ThingSpeak
    delay(5000);

}
