/*
██╗███╗   ███╗██████╗  ██████╗ ██████╗ ████████╗███████╗
██║████╗ ████║██╔══██╗██╔═══██╗██╔══██╗╚══██╔══╝██╔════╝
██║██╔████╔██║██████╔╝██║   ██║██████╔╝   ██║   ███████╗
██║██║╚██╔╝██║██╔═══╝ ██║   ██║██╔══██╗   ██║   ╚════██║
██║██║ ╚═╝ ██║██║     ╚██████╔╝██║  ██║   ██║   ███████║
╚═╝╚═╝     ╚═╝╚═╝      ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚══════╝

*/
#include <Wire.h>
#include <WiFi.h>

#include <BME280_t.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> 

/*
 ██████╗ ██████╗ ███╗   ██╗███████╗██╗ ██████╗ ██╗   ██╗██████╗  █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
██╔════╝██╔═══██╗████╗  ██║██╔════╝██║██╔════╝ ██║   ██║██╔══██╗██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
██║     ██║   ██║██╔██╗ ██║█████╗  ██║██║  ███╗██║   ██║██████╔╝███████║   ██║   ██║██║   ██║██╔██╗ ██║
██║     ██║   ██║██║╚██╗██║██╔══╝  ██║██║   ██║██║   ██║██╔══██╗██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
╚██████╗╚██████╔╝██║ ╚████║██║     ██║╚██████╔╝╚██████╔╝██║  ██║██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
 ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝     ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝

*/
const char* ssid       = "SpectrumSetup-39";       // WiFi SSID of your network
const char* password   = "quietengine538";         // WiFi password of your network
String localIP;                                    // Buffer to hold the local IP address as a string
String MacAddress;                                 // Buffer to hold the MAC address as a string

const char* device_id   = "garden_sensor";
const char* device_name = "Garden Sensor Node";

/*
██████╗ ██╗███╗   ██╗ ██████╗ ██╗   ██╗████████╗███████╗
██╔══██╗██║████╗  ██║██╔═══██╗██║   ██║╚══██╔══╝██╔════╝
██████╔╝██║██╔██╗ ██║██║   ██║██║   ██║   ██║   ███████╗
██╔═══╝ ██║██║╚██╗██║██║   ██║██║   ██║   ██║   ╚════██║
██║     ██║██║ ╚████║╚██████╔╝╚██████╔╝   ██║   ███████║
╚═╝     ╚═╝╚═╝  ╚═══╝ ╚═════╝  ╚═════╝    ╚═╝   ╚══════╝

*/
#define LED_PIN 2
#define SOLENOID_PIN 4
#define BUTTON_PIN 0

#define SCREEN_ADDRESS 0x3C

/*
 ██████╗ ██╗      ██████╗ ██████╗  █████╗ ██╗         ██╗   ██╗ █████╗ ██████╗ ██╗ █████╗ ██████╗ ██╗     ███████╗███████╗
██╔════╝ ██║     ██╔═══██╗██╔══██╗██╔══██╗██║         ██║   ██║██╔══██╗██╔══██╗██║██╔══██╗██╔══██╗██║     ██╔════╝██╔════╝
██║  ███╗██║     ██║   ██║██████╔╝███████║██║         ██║   ██║███████║██████╔╝██║███████║██████╔╝██║     █████╗  ███████╗
██║   ██║██║     ██║   ██║██╔══██╗██╔══██║██║         ╚██╗ ██╔╝██╔══██║██╔══██╗██║██╔══██║██╔══██╗██║     ██╔══╝  ╚════██║
╚██████╔╝███████╗╚██████╔╝██████╔╝██║  ██║███████╗     ╚████╔╝ ██║  ██║██║  ██║██║██║  ██║██████╔╝███████╗███████╗███████║
 ╚═════╝ ╚══════╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝      ╚═══╝  ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚═════╝ ╚══════╝╚══════╝╚══════╝

*/
bool LEDState = LOW; // Internal LED State
bool solenoidState = LOW; // Solenoid state variable 
bool buttonState = LOW; // Global variable to track the state of manual button
bool manualMode = false; // Set to true to enable manual control of the solenoid
int status = WL_IDLE_STATUS; // WiFi status variable
float temperature = 0, humidity = 0, pressure = 0, altitude = 0; // Global variables

const int SCREEN_WIDTH = 128; // OLED display width, in pixels
const int SCREEN_HEIGHT = 64; // OLED display height, in pixels
const int OLED_RESET =  -1; // Reset pin # (or -1 if sharing Arduino reset pin)
const float SEA_LEVEL_PRESSURE_HPA = 1013.25F; // Standard sea level pressure in hPa
    
const int initrelayPeriod = 1000 * 30 * 1; //(1000 ms * X s * X min) - solenoid refresh timer
const int initdataPeriod = 1000 * 1 * 1; // (1000 ms * X s * X min) - data refresh rate
const float tempLimit = 90.0; // Temperature threshold in Fahrenheit

WiFiClient espClient;
BME280<> BMESensor;
TimerHandle_t relayTimer_solenoid, dataTimer; // Software timers for solenoid control and data logging
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*
 ██████╗ █████╗ ██╗     ██╗     ██████╗  █████╗  ██████╗██╗  ██╗███████╗
██╔════╝██╔══██╗██║     ██║     ██╔══██╗██╔══██╗██╔════╝██║ ██╔╝██╔════╝
██║     ███████║██║     ██║     ██████╔╝███████║██║     █████╔╝ ███████╗
██║     ██╔══██║██║     ██║     ██╔══██╗██╔══██║██║     ██╔═██╗ ╚════██║
╚██████╗██║  ██║███████╗███████╗██████╔╝██║  ██║╚██████╗██║  ██╗███████║
 ╚═════╝╚═╝  ╚═╝╚══════╝╚══════╝╚═════╝ ╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚══════╝

 */
void relayCallback(TimerHandle_t xTimer) {
    // Skip automatic control if manual mode is enabled - Off by default, the system will automatically control the solenoid based on temperature readings.
    if (manualMode) {
        Serial.printf("Manual mode enabled. Skipping automatic solenoid control.");
        return; 
    }

    // Temperature limit so you dont kill the plants
    if (temperature <= tempLimit) {
        solenoidState = HIGH; // toggle the solenoid state to on
    }

    Serial.printf("Solenoid state: %s\n", solenoidState ? "ON" : "OFF");
}

void dataCallback(TimerHandle_t xTimer) {
    BMESensor.refresh();
    temperature = BMESensor.temperature; // Update global temperature variable
    humidity = BMESensor.humidity; // Update global humidity variable
    pressure = BMESensor.pressure; // Update global pressure variable
    altitude = BMESensor.pressureToAltitude(); // Update global altitude variable

    Serial.printf("Temperature: %-.1f °F | Humidity: %-.1f% | Pressure: %-.2f atm | Altitude: %-.1f m \n", temperature * 1.8 + 32, humidity, pressure / 100.0F / SEA_LEVEL_PRESSURE_HPA, altitude);

    LEDState = !LEDState; // toggle the LED state
    digitalWrite(LED_PIN, LEDState); // turn on the LED
}

/*
███████╗███████╗████████╗██╗   ██╗██████╗
██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
███████╗█████╗     ██║   ██║   ██║██████╔╝
╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝
███████║███████╗   ██║   ╚██████╔╝██║
╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝

*/
void setup() {
  // --- Begin Serial and I2C communication ---/
  
  Wire.begin();
  Serial.begin(115200);
  delay(1000); // let the serial connection initiate with some downtime 

  if(!BMESensor.begin()) { // Initialize the BME280 sensor
    Serial.printf("Could not find a valid BME280 sensor, check wiring! \n"); 
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Initialize the OLED display
    Serial.printf("SSD1306 allocation failed \n");
  }
  
  display.display();

  // --- Initalize pins ---
  pinMode(LED_PIN, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN); // Set button pin as input with pull-up resistor
  
  // --- Connect to WiFi ---
  WiFi.mode(WIFI_STA);
  
  int n = WiFi.scanNetworks(); // Trigger a WiFi scan to update the list of available networks
  Serial.printf("\n");
  Serial.printf("Searching for Networks... \n");
  if(n == 0) {
    Serial.printf("No networks found. Retrying... \n");
  }
  else {
    Serial.printf("%d network(s) found:\n", n);
    Serial.printf("Nr | SSID                             | RSSI     | CH | Encryption\n");
    Serial.printf("------------------------------------------------------------------\n");

    for (int i = 0; i < n; ++i) {

        Serial.printf("%2d | %-32.32s | %4d dBm | %2d | ", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
        switch (WiFi.encryptionType(i)) {
            case WIFI_AUTH_OPEN:
                Serial.printf("OPEN");
                break;
            case WIFI_AUTH_WEP:
                Serial.printf("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                Serial.printf("WPA_PSK");
                break;
            case WIFI_AUTH_WPA2_PSK:
                Serial.printf("WPA2_PSK");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                Serial.printf("WPA_WPA2_PSK");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                Serial.printf("WPA2_ENTERPRISE");
                break;
            default:
                Serial.printf("UNKNOWN");
        }
        Serial.printf("\n");

    }
}
  Serial.printf("\n");
  Serial.printf("Attempting to connect to SSID: [%s]", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf(".");
    delay(500);
  }

  Serial.printf("\n");
  Serial.printf("Wifi connection to [%-s] successful! \n", ssid);
  localIP = WiFi.localIP().toString();
  MacAddress = WiFi.macAddress();
  Serial.printf("Local IP Address: [%s] \n", localIP);
  Serial.printf("MAC Address: [%s] \n\n", MacAddress);

  // --- Timers ---
  relayTimer_solenoid = xTimerCreate("waterSolenoid", pdMS_TO_TICKS(initrelayPeriod), pdTRUE, (void*)0, relayCallback);
  xTimerStart(relayTimer_solenoid, 0);
  dataTimer = xTimerCreate("dataLogger", pdMS_TO_TICKS(initdataPeriod), pdTRUE, (void*)0, dataCallback);
  xTimerStart(dataTimer, 0);
}

/*
███╗   ███╗ █████╗ ██╗███╗   ██╗    ██╗      ██████╗  ██████╗ ██████╗
████╗ ████║██╔══██╗██║████╗  ██║    ██║     ██╔═══██╗██╔═══██╗██╔══██╗
██╔████╔██║███████║██║██╔██╗ ██║    ██║     ██║   ██║██║   ██║██████╔╝
██║╚██╔╝██║██╔══██║██║██║╚██╗██║    ██║     ██║   ██║██║   ██║██╔═══╝
██║ ╚═╝ ██║██║  ██║██║██║ ╚████║    ███████╗╚██████╔╝╚██████╔╝██║
╚═╝     ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝    ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝

*/
void loop() {\
    buttonState = digitalRead(0); // Read the state of the button connected to GPIO 0

    if(solenoidState) {
        digitalWrite(SOLENOID_PIN, solenoidState); // turn on the solenoid
        delay(1000 * 10 * 1); // keep the solenoid on for 20 seconds
        solenoidState = LOW; // turn off the solenoid
    }
    else {
        digitalWrite(SOLENOID_PIN, solenoidState); // turn off the solenoid
    }

    //xTimerChangePeriod(relayTimer_solenoid, pdMS_TO_TICKS(1000 * 60 * 1), 0); // change the period of the solenoid timer to 1 minute
    //xTimerChangePeriod(dataTimer, pdMS_TO_TICKS(1000), 0
}