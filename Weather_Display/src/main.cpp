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

bool isCurrentlyDisplayingOutside = true;
WeatherSensorMessage lastOutsideWeatherData;
WeatherSensorMessage lastInsideWeatherData;
ButtonState switchButtonState = ButtonState::RELEASED;
DisplayState outputDisplayState = DisplayState::UPDATE; // Initial state is set to update so after initialization the readings are displayed

const uint8_t BUTTON_DEBOUNCE_TIME = 50;
const uint8_t SWITCH_BUTTON_PIN = 35;

// ===== Function Declarations =====

// Prints a message 'text' with the status 'status' as 'color'
void printStatusMessage(char *text, char *status, uint16_t color);
// Displays the temperature and relative humidity for the in- or outside
void displaySensorReadings(bool isOutside, float temperature, float relativeHumidity);
// ISR for handling when the switch button is pressed
void switchDisplayButtonISR();
// Handles the transition between the possible states
void stateHandler();
// Handles the incoming data from ESP-Now
void OnDataReceived(const uint8_t *senderMacAddress, const uint8_t *incomingData, int incomingDataLength);

// ===== Microcontroller Setup and Loop functions =====

void setup()
{
  // TODO: Remove after no longer necessary
  lastInsideWeatherData.temperature = 20.4;
  lastInsideWeatherData.relativeHumidity = 54.45;
  lastOutsideWeatherData.temperature = -0.15;
  lastOutsideWeatherData.relativeHumidity = 15.65;

  // Configure button as input and attach an intterupt service routing (ISR)
  pinMode(SWITCH_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SWITCH_BUTTON_PIN), switchDisplayButtonISR, CHANGE);

  // Start display, provide some delay for settling and clear
  display.init();
  display.setRotation(1);
  display.fillScreen(TFT_BLACK);

  display.setCursor(0, 4, 4);
  display.setTextColor(TFT_WHITE);
  display.println("ESP Weather Station\n          by AndreasStgm\n==========================\n");
  display.print("Starting");
  for (uint8_t i = 0; i <= 3; i++)
  {
    delay(1000);
    display.print(".");
  }
  display.fillScreen(TFT_BLACK);
  display.setCursor(0, 4, 4);

  // Show correctly functioning display
  printStatusMessage("Display: ", "OK", TFT_GREEN);

  // Start WiFi module in station mode
  if (WiFi.mode(WIFI_MODE_STA))
  {
    printStatusMessage("Wireless: ", "OK", TFT_GREEN);
  }
  else
  {
    printStatusMessage("Wireless: ", "FAILED", TFT_RED);
  }
  // Start ESP-Now
  if (esp_now_init() != ESP_OK)
  {
    printStatusMessage("ESP-Now: ", "FAILED", TFT_RED);
  }
  else
  {
    printStatusMessage("ESP-Now: ", "OK", TFT_GREEN);
  }

  // Register the receiving callback function for ESP-Now messages
  if (esp_now_register_recv_cb(OnDataReceived) != ESP_OK)
  {
    printStatusMessage("CB Register: ", "FAILED", TFT_RED);
  }
  else
  {
    printStatusMessage("CB Register: ", "OK", TFT_GREEN);
  }

  display.println("Complete!");
  delay(2000);
}

void loop()
{
  stateHandler();

  delay(1);
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
  display.setCursor(temperature < 0 ? 64 : 81, 16, 6); // Account for the possible minus when dealing with temps below zero
  display.print(temperature, 1);
  // Display humidity with icon
  display.pushImage(0, 64, 64, 64, humidity);
  display.setCursor(81, 80, 6);
  display.print(relativeHumidity, 1);
}

void switchDisplayButtonISR()
{
  // Assigning two longs between which the taken time in ms will be counted to debounce the button input
  static unsigned long last_cycle_interrupt_time = 0;
  unsigned long cycle_interrupt_time = millis();

  // If the current time is longer than the required debounce time
  if (cycle_interrupt_time - last_cycle_interrupt_time > BUTTON_DEBOUNCE_TIME)
  {
    // And the button was in a released state
    if (switchButtonState == ButtonState::RELEASED)
    {
      // The button now is being pressed (detecting the edge from unpressed to pressed)
      switchButtonState = ButtonState::PRESSED;
    }
    // And the button was in the pressed state
    else if (switchButtonState == ButtonState::PRESSED)
    {
      // The button now has been released again after pressing (detecting the edge from pressed to unpressed)
      switchButtonState = ButtonState ::RELEASED;

      // Switch from displaying outside to inside or vice versa
      isCurrentlyDisplayingOutside = !isCurrentlyDisplayingOutside;

      // The display now has to be updated, so the state is set
      outputDisplayState = DisplayState::UPDATE;
    }
  }

  // The ISR is now completed, the current time is now the last time for the next time the ISR is triggered
  last_cycle_interrupt_time = cycle_interrupt_time;
}

void stateHandler()
{
  switch (outputDisplayState)
  {
  case DisplayState::UPDATE:
    // Determine if outside or inside data should be displayed
    if (isCurrentlyDisplayingOutside)
    {
      // Display outside data
      displaySensorReadings(isCurrentlyDisplayingOutside, lastOutsideWeatherData.temperature, lastOutsideWeatherData.relativeHumidity);
    }
    else
    {
      // Display inside data
      displaySensorReadings(isCurrentlyDisplayingOutside, lastInsideWeatherData.temperature, lastInsideWeatherData.relativeHumidity);
    }
    // Complete the state by setting it back to waiting
    outputDisplayState = DisplayState::WAITING;
    break;
  case DisplayState::WAITING:
    // Do a new reading of the inside sensors every x amount of time
    break;
  default:
    printStatusMessage("State Handler: ", "ERROR", TFT_RED);
    break;
  }
}

void OnDataReceived(const uint8_t *senderMacAddress, const uint8_t *incomingData, int incomingDataLength)
{
  // Copy the received data into the data structure
  memcpy(&lastOutsideWeatherData, incomingData, sizeof(incomingData));

  // If the display is updating the outside values, update the display
  if (isCurrentlyDisplayingOutside)
  {
    outputDisplayState = DisplayState::UPDATE;
  }
}