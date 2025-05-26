#include <Arduino.h>
#include <NimBLEDevice.h>
#include <DHT.h>

NimBLEServer* pServer;
NimBLECharacteristic* pCharacteristic1;
NimBLECharacteristic* sensorChar;
DHT dht(4, DHT11); // DHT sensor on GPIO 4 - Temperature and Humidity

#define SERVICE_UUID          "223862d6-64a5-40c0-ba56-b32451c2aa27"
#define CHARACTERISTIC_UUID_1 "b3898a5a-5d50-4636-a814-d804afb43274"
#define CHARACTERISTIC_UUID_2 "85c62e72-7735-463c-a5ce-5c5f7a703acc"
#define LED_PIN          2
#define R_LED           19
#define G_LED           18
#define B_LED           5
#define LIGHT_SENSOR    15

void LED_Blink( void ) {
  digitalWrite(LED_PIN, HIGH), ledcWrite(2, 255), delay(100); 
  digitalWrite(LED_PIN, LOW), ledcWrite(2, 0), delay(100);
}

int RGB_val[3] = {0, 0, 0};

// Read From DHT Sensor
struct DHT_values {
    float t; // Read temperature as Celsius
    float h; // Read humidity
};
DHT_values dht_values = {0.0f, 0.0f}; // Read temperature and humidity  

void sensorUpdate(void) {
    dht_values = {dht.readTemperature(), dht.readHumidity()}; // Read temperature and humidity
    if (isnan(dht_values.t) || isnan(dht_values.h)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    
}

void setup() {
  Serial.begin(115200); // initiallize serial communication
  while (!Serial) { delay(100); }  // wait for serial port to connect

  pinMode(LED_PIN, OUTPUT); // Set LED pin as output
  pinMode(LIGHT_SENSOR, INPUT); // Set Light Sensor pin as Input
  ledcAttachPin(R_LED, 0);
  ledcSetup(0, 5000, 8);
  ledcWrite(0, 0);
  ledcAttachPin(G_LED, 1);
  ledcSetup(1, 5000, 8);
  ledcWrite(1, 0);
  ledcAttachPin(B_LED, 2);
  ledcSetup(2, 5000, 8);
  ledcWrite(2, 0);

  
  dht.begin(); // Initialize DHT sensor
  dht_values = {dht.readTemperature(), dht.readHumidity()}; // Read temperature and humidity

  // Initialize NimBLE
  NimBLEDevice::init("Garden Monitor");
  pServer = NimBLEDevice::createServer();

  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic1 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_1,
                      NIMBLE_PROPERTY::READ |
                      NIMBLE_PROPERTY::WRITE
                    );

  sensorChar = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_1,
                      NIMBLE_PROPERTY::READ 
                    );

  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("BLE Advertising Started");

  // Flash LED twice to indicate startup is successful
  for (int i = 0; i < 3; i++) {
    LED_Blink();
  }
}

void loop() {
    sensorUpdate();
    float lightValue = analogRead(LIGHT_SENSOR)/4095.0 * 3.3; // Read light sensor value in V [3.2 (Dark) - 0.15V (Light)] - 10K Voltage Divider for std daylight

    if (pServer->getConnectedCount() > 0) {
        Serial.println("Client connected"); 
        NimBLEDevice::getAdvertising()->stop(); // Stop advertising if a client is connected
        String value = pCharacteristic1->getValue(); // Take the string value from the pCharacteristic

        if (value.isEmpty()){
            // do nothing
        } else {
            RGB_val[0] = value.substring(0, value.length()).toInt();
            RGB_val[1] = value.substring(3, value.length()).toInt();
            RGB_val[2] = value.substring(7, value.length()).toInt();

            ledcWrite(0, RGB_val[0]); // Set Red LED
            ledcWrite(1, RGB_val[1]); // Set Green LED  
            ledcWrite(2, RGB_val[2]); // Set Blue LED
        } 
        
        Serial.print(">Light V: ");
        Serial.println(lightValue, 2);

    } else {
        Serial.println("No clients connected");
        Serial.print("print this: ");
        Serial.println(dht.readTemperature(true));
        NimBLEDevice::getAdvertising()->start(); // Start advertising if no client is connected
    }

    delay(100); // Delay or sleep for 2 seconds
}