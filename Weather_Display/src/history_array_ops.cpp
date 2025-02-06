#include "history_array_ops.h"

// Shifts the last reading into the back of the array and shifts all readings one spot to the front, discarding the earliest reading
void shiftLastReadingInArray(WeatherSensorMessage history[], WeatherSensorMessage lastReading)
{
    // Shift the entire array one forwards so the new reading can be added at the back
    for (uint8_t i = 0; i < HISTORY_SIZE - 1; i++)
    {
        history[i] = history[i + 1];
    }

    history[HISTORY_SIZE - 1] = lastReading;
}