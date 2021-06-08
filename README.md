# WeatherSensor
A simple weather sensor built around a Wemos D1 and an ATtiny85

## ATTinyWeatherSensor
ATTinyWeatherSensor is an Arduino sketch for an ATTiny used to monitor windspeed and rainfall.
Arduino build settings used for this sketch are
Setting | Value
------- | -----
Board: | ATtiny25/45/85
Processor: | ATtiny85
Clock: | Internal 8 MHz

### I2C
I2C is used to communicated between the Wemos and the ATtiny, when data is requested from the ATtiny. By default the address 87 is used, this is defined by *I2C_ADDRESS*. When requested, the following bytes are returned:

Byte | Example | Descirption
---- | ------- | -
0 | true | **Initialised (True/False)** when the ATtiny starts up it will not report wind speed or rainfall until it has been running and collected enough data and initialised (by default this is 5 minutes and defined by *INTERVAL_MIN*)
1 | 0x00 | **Wind speed MSB** Windspeed is measured in meters per hour and stored in an unsigned integer. The ATtiny calculates this as an average over time. Until the ATtiny has initialised this will be zero. This is the MSB of the integer.
2 | 0xFF | **Wind speed LSB** This is the LSB of the windspeed integer
3 | 0x00 | **Rainfall MSB** Rainfall in millilitres since the last data was requested and stored in an unsigned integer. Until the ATtiny has initialised this will be zero. This is the MSB of the integer
4 | 0xFF | **Rainfall LSB** This is the LSB of the rainfall integer
5 | 255 | **Anemometer rotations** This is the number of rotations the anemometer has bade since the last *INTERVAL_MIN*
6 | 255 | **Rainfall counter** This is the number of times the rain bucket has tipped since the last data was requested. If the ATtiny has initialised this will be reset each time data is requested
