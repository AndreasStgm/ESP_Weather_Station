#include <Arduino.h>

#include <Adafruit_AHTX0.h>
#include <WiFi.h>
#include <esp_now.h>

// ===== Structure/Enum Declarations =====

// Structure for sending data to indoor display
struct WeatherSensorMessage
{
    float temperature;
    float relativeHumidity;
};

// ===== Variable/Constant Declarations =====

Adafruit_AHTX0 sensor = Adafruit_AHTX0();

WeatherSensorMessage outgoingWeatherData;

const uint8_t macAddressDisplay[] = {0x24, 0x62, 0xAB, 0xDC, 0x49, 0xEC};

// ===== Function Declarations =====

// Prints a message 'text' for the specific module 'module' with the status 'status'
void printStatusMessage(String module, String text, String status);
// Handles the incoming data from ESP-Now
void onDataSent(const uint8_t *sendToMacAddress, esp_now_send_status_t status);

void setup()
{
    // Start the serial debug connection
    Serial.begin(115200);
    Serial.println("ESP Weather Sensor\nby AndreasStgm\n==================");
    printStatusMessage("Serial", "Initialisation", "OK");

    // Start WiFi module in station mode
    if (!WiFi.mode(WIFI_MODE_STA))
    {
        printStatusMessage("Wireless", "Initialisation", "FAILED");
    }
    else
    {
        printStatusMessage("Wireless", "Initialisation", "OK");
    }
    // Start ESP-Now
    if (esp_now_init() != ESP_OK)
    {
        printStatusMessage("ESP-Now", "Initialisation", "FAILED");
    }
    else
    {
        printStatusMessage("ESP-Now", "Initialisation", "OK");
    }

    // Creating a new peer and adding it so we can send messages to it
    esp_now_peer_info_t displayPeer;
    // Copying given address into peer_addr
    memcpy(displayPeer.peer_addr, macAddressDisplay, 6);
    displayPeer.channel = 0;
    displayPeer.encrypt = false;
    displayPeer.ifidx = WIFI_IF_STA;
    if (esp_now_add_peer(&displayPeer) != ESP_OK)
    {
        printStatusMessage("ESP-Now", "Peer Registration", "FAILED");
    }
    else
    {
        printStatusMessage("ESP-Now", "Peer Registration", "OK");
    }

    // Register the sending callback function for ESP-Now messages
    if (esp_now_register_send_cb(onDataSent) != ESP_OK)
    {
        printStatusMessage("ESP-Now", "Callback Registration", "FAILED");
    }
    else
    {
        printStatusMessage("ESP-Now", "Callback Registration", "OK");
    }

    // Start the AHT20 sensor
    if (!sensor.begin())
    {
        printStatusMessage("AHT20", "Initialisation", "FAILED");
    }
    else
    {
        printStatusMessage("AHT20", "Initialisation", "OK");
    }

    Serial.println("Complete!");
}

void loop()
{
    // Read the sensor's data weather data
    sensors_event_t temperature, relativeHumidity;
    if (!sensor.getEvent(&relativeHumidity, &temperature))
    {
        printStatusMessage("AHT20", "Sensor Read", "FAILED");
    }
    else
    {
        printStatusMessage("AHT20", "Sensor Read", "OK");

        // Set the readings in the data structure for sending
        outgoingWeatherData.temperature = temperature.temperature;
        outgoingWeatherData.relativeHumidity = relativeHumidity.relative_humidity;
        // Show readings on serial port
        printStatusMessage("AHT20", "Temperature", (String)outgoingWeatherData.temperature);
        printStatusMessage("AHT20", "Relative Humidity", (String)outgoingWeatherData.relativeHumidity);

        esp_now_send(macAddressDisplay, (uint8_t *)&outgoingWeatherData, sizeof(outgoingWeatherData));
    }
    delay(30000);
}

// ===== Function Definitions =====

void printStatusMessage(String module, String text, String status)
{
    Serial.printf("<%s> %s: %s\n", module.c_str(), text.c_str(), status.c_str());
}

void onDataSent(const uint8_t *sendToMacAddress, esp_now_send_status_t status)
{
    if (status != ESP_OK)
    {
        printStatusMessage("ESP-Now", "Send Message", "FAILED");
    }
    else
    {
        printStatusMessage("ESP-Now", "Send Message", "OK");
    }
}