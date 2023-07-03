#include <Arduino.h>

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <esp_now.h>

// ===== Constant/Variable Declarations =====

const uint8_t outdoorSensorMac[] = {0x69, 0x69, 0x69, 0x69, 0x69, 0x69};

TFT_eSPI display = TFT_eSPI();

// ===== Function Declarations =====

// placeholder description
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
  display.println("");

  WiFi.mode(WIFI_MODE_STA);
  if (esp_now_init() != ESP_OK) // Check if ESP_NOW started successfully
  {
    printStatusMessage("ESP-NOW: ", "FAILED", TFT_RED);
  }
  else
  {
    printStatusMessage("ESP-NOW: ", "OK", TFT_GREEN);
  }

  esp_now_peer_info_t newPeer;
  newPeer.channel = 0;
  newPeer.encrypt = false;
  newPeer.ifidx = WIFI_IF_STA;
  memcpy(newPeer.peer_addr, outdoorSensorMac, sizeof(outdoorSensorMac));
  if (esp_now_add_peer(&newPeer) != ESP_OK) // Check if peer was successfully created
  {
    printStatusMessage("Outdoor Added: ", "FAILED", TFT_RED);
  }
  else
  {
    printStatusMessage("Outdoor Added: ", "OK", TFT_GREEN);
  }

  display.println("Complete!");
  delay(2000);
}

void loop()
{
  display.fillScreen(TFT_BLACK);
  display.setCursor(0, 4, 4);
  display.println("Outdoor: 69°C");
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