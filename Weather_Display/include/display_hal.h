#include <Arduino.h>
#include <TFT_eSPI.h>

#include "weather_sensor_msg.h"
#include "history_array_ops.h"

// ===== Icons =====

#include "icon_thermostat.h"
#include "icon_humidity.h"
#include "icon_outside_small.h"
#include "icon_inside_small.h"

// ===== Display Size Definition =====

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 135

// Sets the display to black and the cursor to the top left
void clearDisplay(TFT_eSPI display);

// Prints a message 'text' with the status 'status' as 'color'
void printStatusMessage(TFT_eSPI display, String text, String status, uint16_t color);

// Displays the temperature and relative humidity for the in- or outside
void displaySensorReadings(TFT_eSPI display, bool isOutside, float temperature, float relativeHumidity);

// Displays the historical temperature or relative humidity data from the last hour for the in- or outside
// The graph drawing code was lifted and modified from the examples of the TFT_eSPI library: https://github.com/Bodmer/TFT_eSPI/blob/master/examples/480%20x%20320/Graph_2/Graph_2.ino
void drawHistoryGraph(TFT_eSPI &tft, bool isOutside, WeatherSensorMessage history[], bool displayTemperature);
