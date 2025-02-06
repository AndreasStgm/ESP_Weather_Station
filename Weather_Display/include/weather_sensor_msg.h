#ifndef WEATHER_SENSOR_MSG_H // https://en.wikipedia.org/wiki/Include_guard
#define WEATHER_SENSOR_MSG_H

// Structure for receiving data from outdoor sensor
struct WeatherSensorMessage
{
    float temperature;
    float relativeHumidity;
};

#endif // WEATHER_SENSOR_MSG_H