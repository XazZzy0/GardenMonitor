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
#include <time.h>

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
const char* tzInfo     = "CST6CDT,M3.2.0,M11.1.0"; // Timezone information for Central Standard Time with Daylight Saving Time
const char* ntpServer1 = "pool.ntp.org";           // Primary NTP server for time synchronization
const char* ntpServer2 = "time.nist.gov";          // Secondary NTP server for time synchronization

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
#define UP_BUTTON_PIN 27
#define SELECT_BUTTON_PIN 26
#define DOWN_BUTTON_PIN 25

#define SCREEN_ADDRESS 0x3C

/*
 ██████╗ ██╗      ██████╗ ██████╗  █████╗ ██╗         ██╗   ██╗ █████╗ ██████╗ ██╗ █████╗ ██████╗ ██╗     ███████╗███████╗
██╔════╝ ██║     ██╔═══██╗██╔══██╗██╔══██╗██║         ██║   ██║██╔══██╗██╔══██╗██║██╔══██╗██╔══██╗██║     ██╔════╝██╔════╝
██║  ███╗██║     ██║   ██║██████╔╝███████║██║         ██║   ██║███████║██████╔╝██║███████║██████╔╝██║     █████╗  ███████╗
██║   ██║██║     ██║   ██║██╔══██╗██╔══██║██║         ╚██╗ ██╔╝██╔══██║██╔══██╗██║██╔══██║██╔══██╗██║     ██╔══╝  ╚════██║
╚██████╔╝███████╗╚██████╔╝██████╔╝██║  ██║███████╗     ╚████╔╝ ██║  ██║██║  ██║██║██║  ██║██████╔╝███████╗███████╗███████║
 ╚═════╝ ╚══════╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝      ╚═══╝  ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚═════╝ ╚══════╝╚══════╝╚══════╝

*/
volatile bool LEDState = LOW; // Internal LED State
volatile bool solenoidState = LOW; // Solenoid state variable 
volatile bool prevUpButtonState = false, prevDownButtonState = false, prevSelectButtonState = false; // Previous state of the UP and DOWN buttons for edge detection
volatile bool manualMode = true; // Set to true to enable manual control of the solenoid
enum { MAIN_DISPLAY, NETWORK_INFO, SENSOR_DATA, WATER_TIMER, SET_TIMER, SET_PERIOD, CHANGE_HOUR, CHANGE_MINUTE, HOLD_MINUTE, HOLD_SECOND, }; // Enum to represent different menu states
volatile int menuState = MAIN_DISPLAY; // integer variable to track the current menu state for display purposes [0 = main display, 1 = network info, 2 = sensor data, 3 = Water Timer]
enum { DEFAULT_SUBMENU, SUBMENU_1, SUBMENU_2, SUBMENU_3 }; // Enum to represent different submenu states
volatile int submenuState = DEFAULT_SUBMENU; // integer variable to track the current submenu state for display purposes [1 = default]
volatile int status = WL_IDLE_STATUS; // WiFi status variable
volatile float temperature = 0, humidity = 0, pressure = 0, altitude = 0; // Global variables
volatile int hourCount = 1, minuteCount = 0;
volatile int minuteholdCount = 2, secondholdCount = 0;

const int SCREEN_WIDTH = 128; // OLED display width, in pixels
const int SCREEN_HEIGHT = 64; // OLED display height, in pixels
const int OLED_RESET =  -1; // Reset pin # (or -1 if sharing Arduino reset pin)
const float SEA_LEVEL_PRESSURE_HPA = 1013.25F; // Standard sea level pressure in hPa
    
volatile uint32_t relayPeriod = 1000 * 60 * 60 * 1; //(1000 ms * X s * X min * X hour) - solenoid refresh timer
volatile uint32_t relayHold = 1000 * 60 * 1; //(1000 ms * X s * X min) - solenoid refresh timer
const int initrelayPeriod = 250 * 1 * 1; //(1000 ms * X s * X min) - solenoid refresh timer
const int initdataPeriod = 1000 * 5 * 1; // (1000 ms * X s * X min) - data refresh rate
const int initdisplayPeriod = 1000/10 * 1 * 1; // (100 ms * X s * X min) - display refresh rate (10 Hz)
const float tempLimit = 90.0; // Temperature threshold in Fahrenheit

WiFiClient espClient;
BME280<> BMESensor;
// TimerHandle_t relayTimer_solenoid, displayTimer, dataTimer; // Software timers for solenoid control and data logging
TimerHandle_t xTimers[ 3 ]; // Software timers for solenoid control and data logging
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Free RTOS semaphores for task synchronization
SemaphoreHandle_t i2cMutex; // Semaphore for synchronizing access to shared resources (e.g., sensor data, display updates)
SemaphoreHandle_t serialMutex; // Semaphore for synchronizing access to the serial console to prevent interleaved output from multiple tasks
TaskHandle_t relayRunner = NULL;

/*
███████╗██╗   ██╗███╗   ██╗ ██████╗████████╗██╗ ██████╗ ███╗   ██╗███████╗
██╔════╝██║   ██║████╗  ██║██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝
█████╗  ██║   ██║██╔██╗ ██║██║        ██║   ██║██║   ██║██╔██╗ ██║███████╗
██╔══╝  ██║   ██║██║╚██╗██║██║        ██║   ██║██║   ██║██║╚██╗██║╚════██║
██║     ╚██████╔╝██║ ╚████║╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║███████║
╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝

*/
void connectWifi() {
    int n = WiFi.scanNetworks(); // Trigger a WiFi scan to update the list of available networks
    Serial.printf("\n");
    Serial.printf("Searching for Networks... \n");
    display.setCursor(2, 20);
    display.print("Initializing RTC...");
    display.setCursor(2, 40);
    display.display();
    if(n == 0) {
        Serial.printf("No networks found. Retrying... \n");
    }
    else {
        Serial.printf("%d network(s) found:\n", n);
        Serial.printf("Nr | SSID                             | RSSI     | CH | Encryption\n");
        Serial.printf("------------------------------------------------------------------\n");

        for (int i = 0; i < n; ++i) {

            Serial.printf("%2d | %-32.32s | %4d dBm | %2d a| ", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
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
    display.printf("WiFi: \n %s", ssid);
    display.display();
    localIP = WiFi.localIP().toString();
    MacAddress = WiFi.macAddress();
    Serial.printf("Local IP Address: [%s] \n", localIP);
    Serial.printf("MAC Address: [%s] \n\n", MacAddress);
}

void printDateTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.printf("Failed to obtain time \n");
        return;
    }

    char formattedTime[64];
    strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.printf("Current Date and Time: %s \n\n", formattedTime);
}

void syncTime() {
    // --- Synchronize time with NTP servers ---
    configTime(0, 0, ntpServer1, ntpServer2); // Configure NTP time synchronization
    setenv("TZ", tzInfo, 1);                  // Set the timezone environment variable
    tzset();                                  // Apply the timezone settings

    time_t now = 0;
    Serial.printf("Waiting for NTP time synchronization...");
    while(time(&now) < 8 * 3600 * 2) { // Wait until the time is set (time should be greater than Jan 1, 1970)
        Serial.printf(".");
        delay(500);
    }
    Serial.printf("\n");
    Serial.printf("NTP time synchronized! \n"); 
    printDateTime(); // Print the current date and time after synchronization
}

void startUpLogo(void) {
    display.setTextSize(1);
    display.setTextWrap(false);
    display.setTextColor(BLACK);
    display.setCursor(2, 5);
    display.writeFillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT/4, WHITE); // Draw a filled rectangle as a background for the logo
    display.print("The Wood's Garden Hub");
    display.setTextColor(WHITE);
    display.setCursor(2, 20);
    display.display();
}

/*
████████╗ █████╗ ███████╗██╗  ██╗███████╗
╚══██╔══╝██╔══██╗██╔════╝██║ ██╔╝██╔════╝
   ██║   ███████║███████╗█████╔╝ ███████╗
   ██║   ██╔══██║╚════██║██╔═██╗ ╚════██║
   ██║   ██║  ██║███████║██║  ██╗███████║
   ╚═╝   ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚══════╝

*/

// ==========================================
// CORE 0 TASK: SENSOR READ LOOP
// ==========================================
void dataCollection(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount(); // Initialize the xLastWakeTime variable with the current tick count

    for(;;){
        bool readSuccess = false;

        // Schedule Sensor reading
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) { // Attempt to take the I2C mutex with a timeout of 10 ticks
            
            BMESensor.refresh(); // Refresh the sensor readings
            temperature = BMESensor.temperature; // Read temperature from the sensor
            humidity = BMESensor.humidity; // Read humidity from the sensor
            pressure = BMESensor.pressure; // Read pressure from the sensor
            altitude = BMESensor.pressureToAltitude(); // Calculate altitude based on pressure reading
            readSuccess = true; // Set read success flag to true if readings were successfully obtained

            xSemaphoreGive(i2cMutex); // Release the I2C mutex after reading is done
        }
        
        // Schedule Serial printing of the sensor data
        if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(50)) == pdTRUE) { // Attempt to take the serial mutex with a timeout of 10 ticks

            if (readSuccess) {
                Serial.printf("Temperature: %-.1f °F | Humidity: %-.1f% | Pressure: %-.3f atm | Altitude: %-.1f m \n", temperature * 1.8 + 32, humidity, pressure / 100.0F / SEA_LEVEL_PRESSURE_HPA, altitude);
            }
            else {
                Serial.printf("Failed to read from BME280 sensor! \n");
            }

            xSemaphoreGive(serialMutex); // Release the serial mutex after printing is done
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(initdataPeriod)); // Delay the task until the next scheduled time
    }
}

// ==========================================
// CORE 1 TASK: Display Update Loop
// ==========================================
void displayTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount(); // Initialize the xLastWakeTime variable with the current tick count

    for(;;){
        // Check for button presses to navigate the menu and submenus
        if (digitalRead(UP_BUTTON_PIN)) {
            if (!prevUpButtonState){
                prevUpButtonState = true;
                if (menuState == WATER_TIMER || menuState == SET_TIMER || menuState == SET_PERIOD){
                    submenuState--;
                    if (submenuState < 0) {
                        submenuState = 3;
                    }
                }
                else if (menuState == CHANGE_HOUR){
                    if(hourCount > 24) {
                        hourCount = 0;
                    }
                    else {
                        hourCount += 1;
                    }                    
                }
                else if (menuState == CHANGE_MINUTE){
                    if(minuteCount > 59) {
                        minuteCount = 0;
                    }
                    else {
                        minuteCount += 1;
                    }  
                }
                else if (menuState == HOLD_MINUTE){
                    if(minuteholdCount > 29) {
                        minuteholdCount = 0;
                    }
                    else {
                        minuteholdCount += 1;
                    }  
                }
                else if (menuState == HOLD_SECOND){
                    if(secondholdCount > 59) {
                        secondholdCount = 0;
                    }
                    else {
                        secondholdCount += 1;
                    }  
                }
                else{
                    submenuState--;
                    if (submenuState < 0) {
                        submenuState = 2;
                    }
                }
            }
        }
        else {
            prevUpButtonState = false;
        }

        // Check for button presses to navigate the menu and submenus
        if (digitalRead(DOWN_BUTTON_PIN)) {
            if (!prevDownButtonState){
                prevDownButtonState = true;
                if (menuState == WATER_TIMER || menuState == SET_TIMER || menuState == SET_PERIOD){
                    submenuState++;
                    if (submenuState < 0) {
                        submenuState = 3;
                    }
                }
                else if (menuState == CHANGE_HOUR){
                    if(hourCount < 0) {
                        hourCount = 24;
                    }
                    else {
                        hourCount -= 1;
                    }                    
                }
                else if (menuState == CHANGE_MINUTE){
                    if(minuteCount < 0) {
                        minuteCount = 59;
                    }
                    else {
                        minuteCount -= 1;
                    }  
                }
                else if (menuState == HOLD_MINUTE){
                    if(minuteholdCount < 0) {
                        minuteholdCount = 29;
                    }
                    else {
                        minuteholdCount -= 1;
                    }  
                }
                else if (menuState == HOLD_SECOND){
                    if(secondholdCount < 0) {
                        secondholdCount = 59;
                    }
                    else {
                        secondholdCount -= 1;
                    }  
                }
                else{
                    submenuState++;
                    if (submenuState < 0) {
                        submenuState = 2;
                    }
                }
            }
        }
        else {
            prevDownButtonState = false;
        }

         // Check for button presses to navigate the menu and submenus
        if (digitalRead(SELECT_BUTTON_PIN)) {
            if (!prevSelectButtonState){
                // Handle select button press
                prevSelectButtonState = true;
                switch (menuState) {
                    case MAIN_DISPLAY:
                        // If we're on the main menu, selecting an option will take us to the corresponding submenu
                        menuState = submenuState + 1; // Set the main menu state to the currently selected submenu
                        submenuState = DEFAULT_SUBMENU; // Reset the submenu state to default whenever the select button is pressed
                        break;
                    case NETWORK_INFO:
                        menuState = MAIN_DISPLAY; // If we're in a submenu, selecting will take us back to the main menu
                        submenuState = DEFAULT_SUBMENU; // Reset the submenu state to default whenever the select button is pressed
                        break;
                    case SENSOR_DATA:
                        menuState = MAIN_DISPLAY; // If we're in a submenu, selecting will take us back to the main menu
                        submenuState = DEFAULT_SUBMENU; // Reset the submenu state to default whenever the select button is pressed
                        break;

                    case WATER_TIMER:
                        if (submenuState == DEFAULT_SUBMENU) { // Auto or Manual Mode toggle
                            manualMode = !manualMode; // Toggle between manual and automatic mode
                        }
                        else if (submenuState == SUBMENU_1) { // Toggle State
                            manualMode = true;
                            solenoidState = !solenoidState; // Toggle the solenoid state
                        }
                        else if (submenuState == SUBMENU_2) { // Timer option 
                            menuState = SET_TIMER; // Set the main menu state to the timer setting menu
                        }
                        else if (submenuState == SUBMENU_3) { // If the "Back" option is selected, return to the main menu
                            menuState = MAIN_DISPLAY; // Set the main menu state to the default main display
                        }
                        submenuState = DEFAULT_SUBMENU; // Reset the submenu state to default whenever the select button is pressed
                        break;

                    case SET_TIMER: 
                        if (submenuState == DEFAULT_SUBMENU) { //back
                            menuState = WATER_TIMER; // Set the main menu state to the default main display
                            submenuState = DEFAULT_SUBMENU; // Reset the submenu state to default whenever the select button is pressed
                        }
                        else if (submenuState == SUBMENU_1) { // Set button
                            relayPeriod = (1000 * 60 * minuteCount) + (1000 * 60 * 60 * hourCount);
                            submenuState = DEFAULT_SUBMENU; // Reset the submenu state to default whenever the select button is pressed
                            menuState = SET_PERIOD;
                        }
                        else if (submenuState == SUBMENU_2) { //minute
                            menuState = CHANGE_MINUTE;
                        }
                        else if (submenuState == SUBMENU_3) { //hour
                            menuState = CHANGE_HOUR;
                        }
                    break;

                    case SET_PERIOD: 
                        if (submenuState == DEFAULT_SUBMENU) { //back
                            menuState = WATER_TIMER; // Set the main menu state to the default main display
                            submenuState = DEFAULT_SUBMENU; // Reset the submenu state to default whenever the select button is pressed
                        }
                        else if (submenuState == SUBMENU_1) { // Set button
                            relayHold = (1000 * secondholdCount) + (1000 * 60 * minuteholdCount);
                            submenuState = DEFAULT_SUBMENU; // Reset the submenu state to default whenever the select button is pressed
                            menuState = WATER_TIMER;
                        }
                        else if (submenuState == SUBMENU_2) { //minute
                            menuState = HOLD_SECOND;
                        }
                        else if (submenuState == SUBMENU_3) { //hour
                            menuState = HOLD_MINUTE;
                        }
                    break;

                    case CHANGE_HOUR:
                        menuState = SET_TIMER;
                    break;

                    case CHANGE_MINUTE:
                        menuState = SET_TIMER;
                    break;

                    case HOLD_MINUTE:
                        menuState = SET_PERIOD;
                    break;
                    
                    case HOLD_SECOND:
                        menuState = SET_PERIOD;
                    break;

                    default:
                        menuState = MAIN_DISPLAY; // Set the main menu state to the default main display
                        submenuState = DEFAULT_SUBMENU; // Reset the submenu state to default whenever the select button is pressed
                        break;
                }
            }
            display.clearDisplay(); // Clear the display to prepare for the new menu content
        }
        else {
            prevSelectButtonState = false;
    }

        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) { // Attempt to take the I2C mutex with a timeout of 50 ticks
            
            startUpLogo(); // Display the startup logo on the OLED display

            switch(menuState) {

                case MAIN_DISPLAY:
                    display.setTextSize(1);
                    display.setCursor(0, 20);
                    if (submenuState == DEFAULT_SUBMENU) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("1. Network Settings");

                    display.setCursor(0, 30);
                    if (submenuState == SUBMENU_1) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("2. Sensor Data");
                    
                    display.setCursor(0, 40);
                    if (submenuState == SUBMENU_2) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("3. Water Timer");
                break;

                case NETWORK_INFO:
                    display.setTextSize(1);
                    display.setTextColor(WHITE, BLACK);
                    display.setTextWrap(false);
                    display.setCursor(0, 20);
                    display.printf("SSID: [%2d dBm] \n %s", WiFi.RSSI(0), ssid);
                    display.setCursor(0, 40);
                    display.printf("IP: \n %s", localIP.c_str());
                break;

                case SENSOR_DATA:
                    display.setTextSize(1);
                    display.setTextColor(WHITE, BLACK);
                    display.setCursor(0, 20);
                    display.printf("Temp: %-.1f F", temperature * 1.8 + 32);
                    display.setCursor(0, 30);
                    display.printf("Humidity: %-.1f%%", humidity);
                    display.setCursor(0, 40);   
                    display.printf("Pressure: %-.3f atm", pressure / 100.0F / SEA_LEVEL_PRESSURE_HPA);
                    display.setCursor(0, 50);
                    display.printf("Altitude: %-.1f m", altitude);
                break;

                case WATER_TIMER:
                    display.setTextSize(1);
                    display.setCursor(0, 20);
                    if (submenuState == DEFAULT_SUBMENU) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Solenoid: [%s]", manualMode ? "Manual" : "Auto");

                    display.setCursor(0, 30);
                    if (submenuState == SUBMENU_1) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("State: %s", solenoidState ? "ON " : "OFF");
                    
                    display.setCursor(0, 40);
                    if (submenuState == SUBMENU_2) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Timer");
                    display.setCursor(0, 50);
                    if (submenuState == SUBMENU_3) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Back");
                break; 

                case SET_TIMER:
                    display.setTextSize(1);
                    display.setTextColor(WHITE, BLACK);
                    display.setCursor(21, 20);
                    display.printf("Hour");
                    display.setCursor(69, 20);
                    display.printf("Minute");
                    display.setCursor(34, 54);
                    display.printf("|Period|");

                    display.setCursor(2, 54);
                    if (submenuState == DEFAULT_SUBMENU) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Back");

                    display.setTextSize(3);
                    display.setCursor(16, 30);
                    if (submenuState == SUBMENU_3) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", hourCount);
                    display.setCursor(52, 30);
                    display.setTextColor(WHITE, BLACK);
                    display.printf(":");
                    display.setCursor(70, 30);
                    if (submenuState == SUBMENU_2) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", minuteCount);

                    display.setTextSize(1);
                    display.setCursor(90, 54);
                    if (submenuState == SUBMENU_1) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Set");
                break;

                case SET_PERIOD:
                    display.setTextSize(1);
                    display.setTextColor(WHITE, BLACK);
                    display.setCursor(21, 20);
                    display.printf("Minute");
                    display.setCursor(69, 20);
                    display.printf("Second");
                    display.setCursor(38, 54);
                    display.printf("|Hold|");

                    display.setCursor(2, 54);
                    if (submenuState == DEFAULT_SUBMENU) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Back");

                    display.setTextSize(3);
                    display.setCursor(16, 30);
                    if (submenuState == SUBMENU_3) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", minuteholdCount);
                    display.setCursor(52, 30);
                    display.setTextColor(WHITE, BLACK);
                    display.printf(":");
                    display.setCursor(70, 30);
                    if (submenuState == SUBMENU_2) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", secondholdCount);

                    display.setTextSize(1);
                    display.setCursor(90, 54);
                    if (submenuState == SUBMENU_1) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Set");
                break;

                case CHANGE_HOUR:
                    display.setTextSize(1);
                    display.setTextColor(WHITE, BLACK);
                    display.setCursor(21, 20);
                    display.printf("Hour");
                    display.setCursor(69, 20);
                    display.printf("Minute");
                    display.setCursor(34, 54);
                    display.printf("|Period|");

                    display.setCursor(2, 54);
                    if (submenuState == DEFAULT_SUBMENU) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Back");

                    display.setTextSize(3);
                    display.setCursor(16, 30);
                    if (submenuState == SUBMENU_3) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", hourCount);
                    display.setCursor(52, 30);
                    display.setTextColor(WHITE, BLACK);
                    display.printf(":");
                    display.setCursor(70, 30);
                    if (submenuState == SUBMENU_2) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", minuteCount);

                    display.setTextSize(1);
                    display.setCursor(90, 54);
                    if (submenuState == SUBMENU_1) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Set");
                break;

                case CHANGE_MINUTE:
                    display.setTextSize(1);
                    display.setTextColor(WHITE, BLACK);
                    display.setCursor(21, 20);
                    display.printf("Hour");
                    display.setCursor(69, 20);
                    display.printf("Minute");
                    display.setCursor(34, 54);
                    display.printf("|Period|");

                    display.setCursor(2, 54);
                    if (submenuState == DEFAULT_SUBMENU) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Back");

                    display.setTextSize(3);
                    display.setCursor(16, 30);
                    if (submenuState == SUBMENU_3) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", hourCount);
                    display.setCursor(52, 30);
                    display.setTextColor(WHITE, BLACK);
                    display.printf(":");
                    display.setCursor(70, 30);
                    if (submenuState == SUBMENU_2) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", minuteCount);

                    display.setTextSize(1);
                    display.setCursor(90, 54);
                    if (submenuState == SUBMENU_1) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Set");
                break;

                case HOLD_MINUTE:
                    display.setTextSize(1);
                    display.setTextColor(WHITE, BLACK);
                    display.setCursor(21, 20);
                    display.printf("Minute");
                    display.setCursor(69, 20);
                    display.printf("Second");
                    display.setCursor(38, 54);
                    display.printf("|Hold|");

                    display.setCursor(2, 54);
                    if (submenuState == DEFAULT_SUBMENU) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Back");

                    display.setTextSize(3);
                    display.setCursor(16, 30);
                    if (submenuState == SUBMENU_3) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", minuteholdCount);
                    display.setCursor(52, 30);
                    display.setTextColor(WHITE, BLACK);
                    display.printf(":");
                    display.setCursor(70, 30);
                    if (submenuState == SUBMENU_2) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", secondholdCount);

                    display.setTextSize(1);
                    display.setCursor(90, 54);
                    if (submenuState == SUBMENU_1) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Set");
                break;

                case HOLD_SECOND:
                    display.setTextSize(1);
                    display.setTextColor(WHITE, BLACK);
                    display.setCursor(21, 20);
                    display.printf("Minute");
                    display.setCursor(69, 20);
                    display.printf("Second");
                    display.setCursor(38, 54);
                    display.printf("|Hold|");

                    display.setCursor(2, 54);
                    if (submenuState == DEFAULT_SUBMENU) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Back");

                    display.setTextSize(3);
                    display.setCursor(16, 30);
                    if (submenuState == SUBMENU_3) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", minuteholdCount);
                    display.setCursor(52, 30);
                    display.setTextColor(WHITE, BLACK);
                    display.printf(":");
                    display.setCursor(70, 30);
                    if (submenuState == SUBMENU_2) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("%2.2i", secondholdCount);

                    display.setTextSize(1);
                    display.setCursor(90, 54);
                    if (submenuState == SUBMENU_1) {     
                        display.setTextColor(BLACK, WHITE);   
                    }
                    else {
                        display.setTextColor(WHITE, BLACK);
                    }
                    display.printf("Set");
                break;
            }

            

            display.display(); // Update the OLED display with the new content
            xSemaphoreGive(i2cMutex); // Release the I2C mutex after updating the display
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(initdisplayPeriod)); // Delay the task until the next scheduled time
    }
}

// ==========================================
// CORE 1 TASK: Relay Task
// ==========================================

void relayTimer(void *parameter){
    TickType_t xLastWakeTime = xTaskGetTickCount(); // Initialize the xLastWakeTime variable with the current tick count

    for(;;){
        /*
        // Automatic control logic based on temperature readings can be implemented here
        if (temperature > tempLimit) {
            solenoidState = HIGH; // toggle the solenoid state to on
        } else {
            solenoidState = LOW; // toggle the solenoid state to off
        }*/
       
        //The task goes into a deep sleep here during the automatic phase. It uses 0% CPU until displayTask calls xTaskNotifyGive()
        if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(50)) == pdTRUE) { // Attempt to take the serial mutex with a timeout of 10 ticks
            Serial.printf("Mode: %u and %u \n", xLastWakeTime, xTaskGetTickCount());
            xSemaphoreGive(serialMutex); // Release the serial mutex after printing is done
        }

        if ((xTaskGetTickCount() - xLastWakeTime) >= pdMS_TO_TICKS(relayPeriod) && !manualMode) {
            solenoidState = true;
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(relayHold)); // Delay the task until buffer period ended
            solenoidState = false;

            xLastWakeTime = xTaskGetTickCount(); // set lastWakeTime to now
        } else {
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(initrelayPeriod)); // Delay the task until the next scheduled time
        }
    }
}

void relayTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;){

        digitalWrite(SOLENOID_PIN, solenoidState); // turn on/off the solenoid  
        LEDState = solenoidState; // toggle the LED state
        digitalWrite(LED_PIN, LEDState); // turn on the LED   

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(initrelayPeriod)); // Delay the task until the next scheduled time
    }
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
    
    display.clearDisplay();
    startUpLogo(); // Display the startup logo on the OLED display

    // --- Initalize pins ---
    pinMode(LED_PIN, OUTPUT);
    pinMode(SOLENOID_PIN, OUTPUT);
    pinMode(UP_BUTTON_PIN, INPUT_PULLDOWN); // Set button pin as input with pull-up resistor
    pinMode(DOWN_BUTTON_PIN, INPUT_PULLDOWN); // Set button pin as input with pull-up resistor
    pinMode(SELECT_BUTTON_PIN, INPUT_PULLDOWN); // Set button pin as input with pull-up resistor

    // --- Connect to WiFi ---
    WiFi.mode(WIFI_STA);  // Set WiFi to station mode
    connectWifi();        // Connect to the specified WiFi network

    // --- Establish NTP time synchronization ---
    syncTime();             // Synchronize time with NTP servers
    WiFi.disconnect();      // Disconnect from WiFi after time synchronization to save power
    display.clearDisplay(); // Clear the display after showing the startup logo and WiFi information
    startUpLogo();          // Display the startup logo on the OLED display

    // --- Timers ---
    i2cMutex = xSemaphoreCreateMutex(); // Create a mutex for synchronizing access to I2C resources
    serialMutex = xSemaphoreCreateMutex(); // Create a mutex for synchronizing access to the serial console

    if (i2cMutex == NULL || serialMutex == NULL) {
        Serial.printf("Failed to create mutexes! \n");
        while(1); // Halt execution if mutex creation fails
    }

    xTaskCreatePinnedToCore(
        dataCollection, 
        "Data Collection Task", 
        4096, 
        NULL, 
        1, 
        NULL, 
        0
    ); // Create a task for collecting sensor data on core 0

    xTaskCreatePinnedToCore(
        relayTimer, 
        "Relay Timer", 
        2048, 
        NULL, 
        1, 
        &relayRunner, 
        0
    ); // Create a task for collecting sensor data on core 0

    xTaskCreatePinnedToCore(
        displayTask, 
        "Display Task", 
        4096, 
        NULL, 
        1, 
        NULL, 
        1
    ); // Create a task for updating the display on core 1

    xTaskCreatePinnedToCore(
        relayTask, 
        "Relay Task", 
        2048, 
        NULL, 
        1, 
        NULL, 
        1
    ); // Create a task for updating the display on core 1

}

/*
███╗   ███╗ █████╗ ██╗███╗   ██╗    ██╗      ██████╗  ██████╗ ██████╗
████╗ ████║██╔══██╗██║████╗  ██║    ██║     ██╔═══██╗██╔═══██╗██╔══██╗
██╔████╔██║███████║██║██╔██╗ ██║    ██║     ██║   ██║██║   ██║██████╔╝
██║╚██╔╝██║██╔══██║██║██║╚██╗██║    ██║     ██║   ██║██║   ██║██╔═══╝
██║ ╚═╝ ██║██║  ██║██║██║ ╚████║    ███████╗╚██████╔╝╚██████╔╝██║
╚═╝     ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝    ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝

*/
void loop() {
    vTaskDelete(NULL);
}