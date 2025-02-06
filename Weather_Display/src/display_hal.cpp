#include "display_hal.h"

uint8_t currentTextLine = 0;
const uint8_t maximumTextLines = 5;

// Sets the display to black and resets the text to the first line
void clearDisplay(TFT_eSPI display)
{
    display.fillScreen(TFT_BLACK);
    currentTextLine = 0;
}

// Prints a message 'text' with the status 'status' as 'color'
void printStatusMessage(TFT_eSPI display, String text, String status, uint16_t color)
{
    display.setCursor(0, 4 + (currentTextLine * 24), 4);

    display.print(text);
    display.setTextColor(color);
    display.println(status);
    display.setTextColor(TFT_WHITE);

    if (currentTextLine >= maximumTextLines)
    {
        clearDisplay(display);
    }
    else
    {
        currentTextLine++;
    }
}

// Displays the temperature and relative humidity for the in- or outside
void displaySensorReadings(TFT_eSPI display, bool isOutside, float temperature, float relativeHumidity)
{
    // Clear the screen
    clearDisplay(display);

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