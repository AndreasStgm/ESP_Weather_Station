#include <Arduino.h>

#include <Adafruit_AHTX0.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <esp_now.h>

#include "display_hal.h"

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
Adafruit_AHTX0 insideSensor = Adafruit_AHTX0();

bool isCurrentlyDisplayingOutside = true;
WeatherSensorMessage lastOutsideWeatherData;
WeatherSensorMessage lastInsideWeatherData;
ButtonState switchButtonState = ButtonState::RELEASED;
DisplayState outputDisplayState = DisplayState::UPDATE; // Initial state is set to update so after initialization the readings are displayed

const uint8_t BUTTON_DEBOUNCE_TIME = 50;
const uint8_t SWITCH_BUTTON_PIN = 35;

const uint16_t millisecondsDelayBetweenMeasurements = 60000;
uint16_t currentDelay = 60000;

// ===== Function Declarations =====

// ISR for handling when the switch button is pressed
void switchDisplayButtonISR();
// Handles the transition between the possible states
void stateHandler();
// Handles the incoming data from ESP-Now
void onDataReceived(const uint8_t *senderMacAddress, const uint8_t *incomingData, int incomingDataLength);

// ===== Microcontroller Setup and Loop functions =====

void setup()
{
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
  clearDisplay(display);

  // Show correctly functioning display
  printStatusMessage(display, "Display: ", "OK", TFT_GREEN);

  // Start WiFi module in station mode
  if (!WiFi.mode(WIFI_MODE_STA))
  {
    printStatusMessage(display, "Wireless: ", "FAILED", TFT_RED);
  }
  else
  {
    printStatusMessage(display, "Wireless: ", "OK", TFT_GREEN);
  }
  // Start ESP-Now
  if (esp_now_init() != ESP_OK)
  {
    printStatusMessage(display, "ESP-Now: ", "FAILED", TFT_RED);
  }
  else
  {
    printStatusMessage(display, "ESP-Now: ", "OK", TFT_GREEN);
  }

  // Register the receiving callback function for ESP-Now messages
  if (esp_now_register_recv_cb(onDataReceived) != ESP_OK)
  {
    printStatusMessage(display, "CB Register: ", "FAILED", TFT_RED);
  }
  else
  {
    printStatusMessage(display, "CB Register: ", "OK", TFT_GREEN);
  }

  // Start the AHT20 sensor
  if (!insideSensor.begin())
  {
    printStatusMessage(display, "AHT20: ", "FAILED", TFT_RED);
  }
  else
  {
    printStatusMessage(display, "AHT20: ", "OK", TFT_GREEN);
  }

  delay(2000);
}

void loop()
{
  stateHandler();

  delay(1);
}

// ===== Function Definitions =====

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
      displaySensorReadings(display, isCurrentlyDisplayingOutside, lastOutsideWeatherData.temperature, lastOutsideWeatherData.relativeHumidity);
    }
    else
    {
      // Display inside data
      displaySensorReadings(display, isCurrentlyDisplayingOutside, lastInsideWeatherData.temperature, lastInsideWeatherData.relativeHumidity);
    }
    // Complete the state by setting it back to waiting
    outputDisplayState = DisplayState::WAITING;
    break;
  case DisplayState::WAITING:
    // Do a new reading of the inside sensors every x amount of time
    if (currentDelay >= millisecondsDelayBetweenMeasurements)
    {
      sensors_event_t temperature, relativeHumidity;
      if (!insideSensor.getEvent(&relativeHumidity, &temperature))
      {
        printStatusMessage(display, "AHT20 Sensor Read: ", "FAILED", TFT_RED);
      }
      else
      {
        // Set the readings in the data structure for sending
        lastInsideWeatherData.temperature = temperature.temperature;
        lastInsideWeatherData.relativeHumidity = relativeHumidity.relative_humidity;

        // If the display is showing the inside values, update the display
        if (!isCurrentlyDisplayingOutside)
        {
          outputDisplayState = DisplayState::UPDATE;
        }
      }
      currentDelay = 0;
    }
    else
    {
      currentDelay++;
    }
    break;
  default:
    printStatusMessage(display, "State Handler: ", "ERROR", TFT_RED);
    break;
  }
}

void onDataReceived(const uint8_t *senderMacAddress, const uint8_t *incomingData, int incomingDataLength)
{
  // Copy the received data into the data structure
  memcpy(&lastOutsideWeatherData, incomingData, sizeof(lastOutsideWeatherData));

  // If the display is showing the outside values, update the display
  if (isCurrentlyDisplayingOutside)
  {
    outputDisplayState = DisplayState::UPDATE;
  }
}