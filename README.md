# ESP Weather Station

This project uses multiple ESP32's. One is supposed to be located inside the house and the other outside.
The inside module is powered through USB-C and the outside module is powered by batteries.

## Power

The outside module is but into a hibernation state after sending out its measurement to preserve the most amount of battery life.

When the wireless module is active power draw is around 120mA. When sleeping it should be around 5µA.

## Communication

Communication is done with ESP-Now. This is a very low-level communication method that uses MAC addresses of the devices to send messages wirelessly.

To ensure good communication an external antenna is recommended as the PCB antennas of ESP32 have very low range (especially in concrete homes).

## UI

The display shows the temperature and relative humidity of the outside and inside modules. Which values are shown can be selected by pressing the button.

![PXL_20240915_124104659](https://github.com/user-attachments/assets/dfbb65c6-23b9-4056-8619-d9b4b6e5b640)

## Features to be added:

| Order | Description                                        | State       |
| ----- | -------------------------------------------------- | ----------- |
| 1     | Measurement of outdoor sensor                      | DONE        |
| 2     | Sending data from outdoor sensor to indoor display | DONE        |
| 3     | Outdoor sensor hibernation                         | DONE        |
| 4     | Measurement of indoor sensor                       | NOT STARTED |
| 5     | Creation of custom PCB                             | NOT STARTED |
| 6     | Creation of housing for outdoor mounting           | NOT STARTED |
