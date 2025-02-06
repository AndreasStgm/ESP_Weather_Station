#include <Arduino.h>
#include <TFT_eSPI.h>

// ===== Icons =====

#include "icon_thermostat.h"
#include "icon_humidity.h"
#include "icon_outside_small.h"
#include "icon_inside_small.h"

// Sets the display to black and the cursor to the top left
void clearDisplay(TFT_eSPI display);

// Prints a message 'text' with the status 'status' as 'color'
void printStatusMessage(TFT_eSPI display, String text, String status, uint16_t color);

// Displays the temperature and relative humidity for the in- or outside
void displaySensorReadings(TFT_eSPI display, bool isOutside, float temperature, float relativeHumidity);