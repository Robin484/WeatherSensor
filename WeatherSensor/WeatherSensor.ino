#include <SPI.h>
#include <Wire.h>

#define TINYWEATHER_ADDR  87
class TinyWeather {
  private:
    bool initialised = false;
    unsigned int rain = 0;
    unsigned int wind = 0;
    byte anemometer = 0;
    byte rain_bucket = 0;
    
  public:
    TinyWeather() {
      Wire.begin();
    }
    
    bool read() {
      bool ok = false;
      
      // Request three bytes
      Wire.requestFrom(TINYWEATHER_ADDR, 7);

      // If everthing went ok, we should have recieved 3 bytes
      if(Wire.available() == 7)
      {
        initialised = Wire.read();
        wind = (unsigned int) ((Wire.read() << 8) | Wire.read());
        rain = (unsigned int) ((Wire.read() << 8) | Wire.read());
        anemometer = Wire.read();
        rain_bucket = Wire.read();
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
      return rain;
    }

    byte getAnemometer() {
      return anemometer;
    }

    byte getRainBucket() {
      return rain_bucket;
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

#define INTERVAL_MIN  5         // 5 minute interval
#define INTERVAL_MS   INTERVAL_MIN *60000

// Light sensor
#include <hp_BH1750.h>
#define LIGHT_ADDRESS 0x23
hp_BH1750 BH1750;

TinyWeather tinyWeather;

unsigned long timeout;


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

  timeout = millis() + INTERVAL_MS;
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

void displayTinyValues(char* text, int total, byte since)
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

  Serial.println();
  

  // Update the display
  display.display();

  // Sleep for 5 seconds
  delay(5*1000);


  // Read the rain/wind sensor every 5 minutes
  if(millis() >= timeout) {
    timeout = millis() + INTERVAL_MS;
    if(!tinyWeather.read())
    {
      // If we failed to read from the TinyWeather sensor, wait a second and retry
      Serial.println("TinyWeather.read() returned false, retry...");
      delay(1000);
      if(!tinyWeather.read())
      {
        Serial.println("Error getting data from TinyWeather");
      }
    }
  }

  // Display TinyWeather data
  display.clearDisplay();
  display.setCursor(0,0);
  if(tinyWeather.isInitialised())
    displayTiny("Initialised");
  else
    displayTiny("Not Initialised");
  displayTinyValues("Wind", tinyWeather.getWind(), tinyWeather.getAnemometer());
  displayTinyValues("Rain", tinyWeather.getRain(), tinyWeather.getRainBucket());
  display.display();

  Serial.println();
  Serial.println();

  // Sleep for 3 seconds
  delay(3 * 1000);
}
