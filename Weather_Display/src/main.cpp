#include <Arduino.h>

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <esp_now.h>

// Icons

#include "thermostat.h"

// ===== Constant/Variable Declarations =====

TFT_eSPI display = TFT_eSPI();

// ===== Function Declarations =====

// Prints a message 'text' with the status 'status' as 'color'
void printStatusMessage(char *text, char *status, uint16_t color);

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
  display.fillScreen(TFT_BLACK);
  display.setCursor(0, 4, 4);
  display.println("Outdoor: 69\u00B0C");

  display.pushImage(20, 20, 96, 96, thermostat);
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