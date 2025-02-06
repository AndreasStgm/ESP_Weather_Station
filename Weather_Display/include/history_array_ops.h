#include <Arduino.h>

#include "weather_sensor_msg.h"

#define HISTORY_SIZE 60

// Shifts the last reading into the back of the array and shifts all readings one spot to the front, discarding the earliest reading
void shiftLastReadingInArray(WeatherSensorMessage history[], WeatherSensorMessage lastReading);