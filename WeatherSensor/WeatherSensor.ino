#include <SPI.h>
#include <Wire.h>

#define TINYWEATHER_ADDR  87
class TinyWeather {
  private:
    bool initialised = false;
    unsigned int rain = 0;
    unsigned int wind = 0;
    
  public:
    TinyWeather() {
      Wire.begin();
    }
    
    bool read() {
      bool ok = false;
      
      // Request three bytes
      Wire.requestFrom(TINYWEATHER_ADDR, 3);

      // If everthing went ok, we should have recieved 3 bytes
      if(Wire.available() == 3)
      {
        initialised = Wire.read();
        rain = (unsigned int)Wire.read();
        wind = (unsigned int)Wire.read();

        ok = true;
      }

      return ok;
    }

    bool isInitialised() {
      return initialised;
    }

    unsigned int getWind() {
      return wind;
    }

    unsigned int getRain() {
      return wind;
    }
};

// OLED Display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// BME sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define TEMPERATURE_ADDRESS 0x76
Adafruit_BME280 bme;

#define RAINWIND_SENSOR 87

// Light sensor
#include <hp_BH1750.h>
#define LIGHT_ADDRESS 0x23
hp_BH1750 BH1750;


// This sketch reads light, temperature, humidty and pressure values every
// 10 seconds, outputing the values to a display

void setup() {
  bool ok = true;
  Serial.begin(9600);
  Wire.begin();       // Do we need this if others are using it??

  // Initialise the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Failed to initialise the OLED display!"));
    ok = false;
  }
  
  if(!BH1750.begin(LIGHT_ADDRESS)) {
    Serial.println("Failed to initialise light sensor!");
    ok = false;
  }

  if(!bme.begin(TEMPERATURE_ADDRESS)) {
    Serial.println("Failed to initialise temperature sensor!");
    ok = false;
  }

  if(!ok)
    for(;;);
}

// Output a sensor value
void displaySensor(char* sensor, float value, char* unit)
{
  int buffer_size = 30;
  char buffer[buffer_size];
  snprintf(buffer, buffer_size, "%s %.2f %s", sensor, value, unit);
  Serial.println(buffer);
  display.println(buffer);
}

// Output TinyWeather functions
void displayTiny(char* text)
{
  Serial.println(text);
  display.println(text);
}

void displayTinyValue(char* text, byte value)
{
  int buffer_size = 30;
  char buffer[buffer_size];
  snprintf(buffer, buffer_size, "%s %d", text, value);
  Serial.println(buffer);
  display.println(buffer);
}

void displayTinyValues(char* text, byte total, byte since)
{
  int buffer_size = 30;
  char buffer[buffer_size];
  snprintf(buffer, buffer_size, "%s %d (%d since)", text, total, since);
  Serial.println(buffer);
  display.println(buffer);
}

void loop() {
  // Clear the display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  
  // Read from the light sensor
  BH1750.start();
  float lux=BH1750.getLux();
  displaySensor("Light", lux, "Lux");

  // Read from the BME280 sensor
  float temp, humd, pres;
  temp = bme.readTemperature();
  humd = bme.readHumidity();
  pres = bme.readPressure() / 100.0F;
  displaySensor("Temp", temp, "C");
  displaySensor("Pres", pres, "hPa");
  displaySensor("Humidity", humd, "%");
  

  // Update the display
  display.display();

  // Sleep for 10 seconds
  delay(7*1000);

  bool tiny_init = 0;
  byte tiny_windspeed = 0;
  byte tiny_wind = 0;
  byte tiny_rainfall = 0;
  byte tiny_rainsince = 0;
  byte tiny_rand = 333;
  

  // Read from the wind/rain sensor
  Serial.print("Requesting rain/weather from ");
  Serial.println(TINYWEATHER_ADDR);
  Wire.requestFrom(TINYWEATHER_ADDR, 6);
  int rtn = 0;
  rtn = Wire.available();
  Serial.print(" returned ");
  Serial.print(rtn);
  Serial.print(" ");
  if(rtn){
    tiny_init = (bool)Wire.read();
    tiny_windspeed = (byte)Wire.read();
    tiny_wind = (byte)Wire.read();
    tiny_rainfall = (byte)Wire.read();
    tiny_rainsince = (byte)Wire.read();
    tiny_rand = (byte)Wire.read(); 
    Serial.print(tiny_init ? "TinyWeather initialised (" : "TinyWeather not initialised (");
    Serial.print(tiny_rand);
    Serial.println(")");
  }

  // Display TinyWeather data
  display.clearDisplay();
  display.setCursor(0,0);
  if(tiny_init)
    displayTiny("Initialised");
  else
    displayTiny("Not Initialised");
  displayTinyValues("Wind", tiny_windspeed, tiny_wind);
  displayTinyValues("Rain", tiny_rainfall, tiny_rainsince);
  displayTinyValue("Random", tiny_rand);
  display.display();

  // Sleep for 3 seconds
  delay(3 * 1000);
}
