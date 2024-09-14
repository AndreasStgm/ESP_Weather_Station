#include <Arduino.h>

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <esp_now.h>

// ===== Icons =====

#include "icon_thermostat.h"
#include "icon_humidity.h"
#include "icon_outside_small.h"
#include "icon_inside_small.h"

// ===== Structure/Enum Declarations =====

// Structure for receiving data from outdoor sensor
struct WeatherSensorMessage
{
  float temperature;
  float relativeHumidity;
};
// Enum for keeping track of the state of the buttons, mostly used for debouncing
enum ButtonState
{
  RELEASED,
  PRESSED
};
// Enum for keeping track of the state of the display, if it needs to be updated or not
enum DisplayState
{
  WAITING,
  UPDATE
};

// ===== Variable/Constant Declarations =====

TFT_eSPI display = TFT_eSPI();
WeatherSensorMessage incomingWeatherData;
ButtonState switchButtonState = ButtonState::RELEASED;
DisplayState outputDisplayState = DisplayState::WAITING;

const uint8_t buttonDebounceTime = 50;

// ===== Function Declarations =====

// Prints a message 'text' with the status 'status' as 'color'
void printStatusMessage(char *text, char *status, uint16_t color);
// Displays the temperature and relative humidity for the in- or outside
void displaySensorReadings(bool isOutside, float temperature, float relativeHumidity);

void setup()
{
  display.init();
  display.setRotation(1);
  display.fillScreen(TFT_BLACK);

  display.setCursor(0, 4, 4);
  display.setTextColor(TFT_WHITE);
  display.print("Starting");
  for (uint8_t i = 0; i < 3; i++)
  {
    delay(1000);
    display.print(".");
  }
  display.fillScreen(TFT_BLACK);
  display.setCursor(0, 4, 4);
  printStatusMessage("Display: ", "OK", TFT_GREEN);

  WiFi.mode(WIFI_MODE_STA);
  if (esp_now_init() != ESP_OK) // Check if ESP_NOW started successfully
  {
    printStatusMessage("ESP-Now: ", "FAILED", TFT_RED);
  }
  else
  {
    printStatusMessage("ESP-Now: ", "OK", TFT_GREEN);
  }

  display.println("Complete!");
  delay(2000);
}

void loop()
{
  displaySensorReadings(true, 69.69, 69.69);

  delay(5000);
}

// ===== Function Definitions =====

void printStatusMessage(char *text, char *status, uint16_t color)
{
  display.print(text);
  display.setTextColor(color);
  display.println(status);
  display.setTextColor(TFT_WHITE);
}

void displaySensorReadings(bool isOutside, float temperature, float relativeHumidity)
{
  // Clear the screen
  display.fillScreen(TFT_BLACK);

  // Rotate text to write sensor location
  display.setRotation(2);
  if (isOutside)
  {
    display.pushImage(0, 0, 32, 32, outside_small);
    display.setCursor(32, 8, 4);
    display.print("Outside");
  }
  else
  {
    display.pushImage(0, 0, 32, 32, inside_small);
    display.setCursor(32, 8, 4);
    display.print("Inside");
  }
  // Reset text rotation back to normal
  display.setRotation(1);

  // Display temperature with icon
  display.pushImage(0, 0, 64, 64, thermostat);
  display.setCursor(64, 16, 6);
  display.print(temperature);
  // Display humidity with icon
  display.pushImage(0, 64, 64, 64, humidity);
  display.setCursor(64, 80, 6);
  display.print(relativeHumidity);
}