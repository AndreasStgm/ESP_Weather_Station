#include "display_hal.h"

uint8_t currentTextLine = 0;
const uint8_t maximumTextLines = 5;

double ox = -999, oy = -999; // Force the origin coordinates to be off screen

// Sets the display to black and resets the text to the first line
void clearDisplay(TFT_eSPI display)
{
    display.fillScreen(TFT_BLACK);
    currentTextLine = 0;
}

// Prints a message 'text' with the status 'status' as 'color'
void printStatusMessage(TFT_eSPI display, String text, String status, uint16_t color)
{
    if (currentTextLine >= maximumTextLines)
    {
        clearDisplay(display);
    }
    display.setCursor(0, 4 + (currentTextLine * 24), 4);

    display.print(text);
    display.setTextColor(color);
    display.println(status);
    display.setTextColor(TFT_WHITE);

    currentTextLine++;
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

// Draws the graph axes
void drawGraph(TFT_eSPI &tft, double x, double y, byte dp,
               double xlo, double xhi, double xinc,
               double ylo, double yhi, double yinc,
               String title, String xlabel, String ylabel,
               bool &redraw)
{
    double gx = 30;                      // Offset for the units on the left
    double gy = DISPLAY_HEIGHT - 15;     // Offset for the units on the bottom
    double w = DISPLAY_WIDTH - gx - 32;  // Width of the grid
    double h = DISPLAY_HEIGHT - 15 - 32; // Height of the grid

    double ydiv,
        xdiv;
    double i;
    double temp;
    int rot, newrot;

    // gcolor = graph grid colors
    // acolor = axes line colors
    // tcolor = text color
    // bcolor = background color
    unsigned int gcolor = TFT_BLUE;
    unsigned int acolor = TFT_RED;
    unsigned int tcolor = TFT_WHITE;
    unsigned int bcolor = TFT_BLACK;

    if (redraw == true)
    {
        redraw = false;
        // initialize old x and old y in order to draw the first point of the graph
        // but save the transformed value
        // note my transform funcition is the same as the map function, except the map uses long and we need doubles
        // ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
        // oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

        tft.setTextDatum(MR_DATUM);

        // draw y scale
        for (i = ylo; i <= yhi; i += yinc)
        {
            // compute the transform
            temp = (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

            if (i == 0)
            {
                tft.drawLine(gx, temp, gx + w, temp, acolor);
                tft.setTextColor(acolor, bcolor);
                tft.drawString(xlabel, (int)(gx + w), (int)temp, 2);
            }
            else
            {
                tft.drawLine(gx, temp, gx + w, temp, gcolor);
            }
            // draw the axis labels
            tft.setTextColor(tcolor, bcolor);
            // precision is default Arduino--this could really use some format control
            tft.drawFloat(i, dp, gx - 4, temp, 1);
        }

        // draw x scale
        for (i = xlo; i <= xhi; i += xinc)
        {

            // compute the transform
            temp = (i - xlo) * (w) / (xhi - xlo) + gx;
            if (i == 0)
            {
                tft.drawLine(temp, gy, temp, gy - h, acolor);
                tft.setTextColor(acolor, bcolor);
                tft.setTextDatum(BC_DATUM);
                tft.drawString(ylabel, (int)temp, (int)(gy - h - 8), 2);
            }
            else
            {
                tft.drawLine(temp, gy, temp, gy - h, gcolor);
            }
            // draw the axis labels
            tft.setTextColor(tcolor, bcolor);
            tft.setTextDatum(TC_DATUM);
            // precision is default Arduino--this could really use some format control
            tft.drawFloat(i, dp, temp, gy + 7, 1);
        }

        // now draw the graph labels
        tft.setTextColor(tcolor, bcolor);
        tft.drawString(title, (int)(gx + w / 2), (int)(gy - h - 30), 4);
    }
}

// Draws the line on the graph
void drawLineOnGraph(TFT_eSPI &tft, double x, double y, byte dp,
                     double xlo, double xhi, double xinc,
                     double ylo, double yhi, double yinc,
                     bool &update, uint16_t color)
{
    double gx = 30;                      // Offset for the units on the left
    double gy = DISPLAY_HEIGHT - 15;     // Offset for the units on the bottom
    double w = DISPLAY_WIDTH - gx - 32;  // Width of the grid
    double h = DISPLAY_HEIGHT - 15 - 32; // Height of the grid

    double ydiv, xdiv;
    double i;
    double temp;
    int rot, newrot;

    // initialize old x and old y in order to draw the first point of the graph
    // but save the transformed value
    // note my transform funcition is the same as the map function, except the map uses long and we need doubles
    if (update)
    {
        update = false;

        ox = (x - xlo) * (w) / (xhi - xlo) + gx;
        oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

        if ((ox < gx) || (ox > gx + w))
        {
            update = true;
            return;
        }
        if ((oy < gy - h) || (oy > gy))
        {
            update = true;
            return;
        }
    }

    // the coordinates are now drawn, plot the data
    // the entire plotting code are these few lines...
    // recall that ox and oy are initialized above
    x = (x - xlo) * (w) / (xhi - xlo) + gx;
    y = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

    if ((x < gx) || (x > gx + w))
    {
        update = true;
        return;
    }
    if ((y < gy - h) || (y > gy))
    {
        update = true;
        return;
    }

    tft.drawLine(ox, oy, x, y, color);
    // it's up to you but drawing 2 more lines to give the graph some thickness
    // tft.drawLine(ox, oy + 1, x, y + 1, pcolor);
    // tft.drawLine(ox, oy - 1, x, y - 1, pcolor);
    ox = x;
    oy = y;
}

// Displays the historical temperature or relative humidity data from the last hour for the in- or outside
void drawHistoryGraph(TFT_eSPI &tft, double x, double y, bool &redrawGraph, bool &updateLine, WeatherSensorMessage history[], bool displayTemperature)
{
    double xMinValue = 0;
    double xMaxValue = HISTORY_SIZE;
    double xInterval = 10;

    double yMinValue = -1;
    double yMaxValue = 1;
    double yInterval = .25;

    drawGraph(tft, x, y, 1,
              xMinValue, xMaxValue, xInterval,
              yMinValue, yMaxValue, yInterval,
              "Temperature", "", "C",
              redrawGraph);

    for (x = 0; x < HISTORY_SIZE; x++)
    {
        // y = sin(x);
        y = history[(int)x].temperature;
        drawLineOnGraph(tft, x + 1, y, 1,
                        xMinValue, xMaxValue, xInterval,
                        yMinValue, yMaxValue, yInterval,
                        updateLine, TFT_YELLOW);
    }
}
