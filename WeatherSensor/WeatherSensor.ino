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

      Serial.print("Wire.available() = ");
      Serial.println(Wire.available());

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

      if(ok)
      {
        Serial.print("initialised = ");
        Serial.println(initialised);
  
        Serial.print("wind = ");
        Serial.println(wind);
  
        Serial.print("rain = ");
        Serial.println(rain);
  
        Serial.print("anemometer = ");
        Serial.println(anemometer);
  
        Serial.print("rain_bucket = ");
        Serial.println(rain_bucket);
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

// ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti WiFiMulti;

const char* ssid = "";
const char* password = "";

const char* update_endpoint = "http://192.168.4.1:5001/update";

char id[5];

// BME sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define TEMPERATURE_ADDRESS 0x76
Adafruit_BME280 bme;

#define MINS 60000000
#define MAX_TINYWEATHER_ATTEMPTS 5
#define MAX_ATTEMPTS 3

// Light sensor
#include <hp_BH1750.h>
#define LIGHT_ADDRESS 0x23
hp_BH1750 BH1750;

TinyWeather tinyWeather;

void setup() {
  bool ok = true;
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Setup called");

  // Use the chip ID to identify the sensor
  snprintf(id, 5, "%X", ESP.getChipId());
  Serial.print("Chip ID = ");
  Serial.println(id);
  
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
}

// Output TinyWeather functions
void displayTiny(char* text)
{
  Serial.println(text);
}

void displayTinyValues(char* text, int total, byte since)
{
  int buffer_size = 30;
  char buffer[buffer_size];
  snprintf(buffer, buffer_size, "%s %d (%d since)", text, total, since);
  Serial.println(buffer);
}


float lux = 0.0;
float temp = 0.0;
float humd = 0.0;
float pres = 0.0;

#define WEATHER_STATUS_OK 0
#define WEATHER_STATUS_TINYWEATHER 1

void loop() {
  int attempts;
  bool success = false;
  int error = WEATHER_STATUS_OK;
  Serial.println("loop called");

  for(attempts = 0; attempts < MAX_TINYWEATHER_ATTEMPTS; attempts++)
  {
    if(tinyWeather.read())
    {
      error = WEATHER_STATUS_OK;
      break;
    }
    
    // If we failed to read from the TinyWeather sensor, wait a second and retry
    Serial.println("TinyWeather.read() returned false, retry...");
    error = WEATHER_STATUS_TINYWEATHER;
    delay(3000);
  }
  
  // Read from the light sensor
  BH1750.start();
  lux=BH1750.getLux();

  // Read from the BME280 sensor
  temp = bme.readTemperature();
  humd = bme.readHumidity();
  pres = bme.readPressure() / 100.0F;
  
  displaySensor("Light", lux, "Lux");
  displaySensor("Temp", temp, "C");
  displaySensor("Pres", pres, "hPa");
  displaySensor("Humidity", humd, "%");
  Serial.println();

  for(attempts = 0; attempts < MAX_ATTEMPTS; attempts++)
  {
    // Conntect to WIFI
    Serial.print("Connecting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int count=60;
    while(WiFi.status() != WL_CONNECTED && count > 0) {
      delay(1000);
      Serial.print(".");
      count--;
    }
  
    if(WiFi.status()!=WL_CONNECTED) {
        Serial.println(" Failed to connect!");
    }
    else {
      Serial.print(" Connected, IP address: ");
      Serial.println(WiFi.localIP());
  
      HTTPClient http;
      int status = 0;

      if(http.begin(update_endpoint)){
        char data[255];
        sprintf(data, "id=%X&t=%02.02f&h=%02.02f&p=%02.02f&l=%02.02f&w=%d&r=%d&e=%d", id, temp, humd, pres, lux, tinyWeather.getWind(), tinyWeather.getRain(), error);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
        Serial.print("REQUEST: ");
        Serial.print(update_endpoint);
        Serial.print("  ");
        Serial.println(data);
        
        status = http.POST(data);
  
        if(status == 200) {
          Serial.print("RESPONSE: ");
          String payload = http.getString();
          Serial.print(status);
          Serial.print("  ");
          Serial.println(payload);
          success = true;
        }
        else {
          Serial.print("ERROR ");
          Serial.print(status);
          Serial.print("  ");
          Serial.println(http.errorToString(status));
        }
      }

      http.end();

      if(success)
        break;
    }
  }

  // Stop WIFI and sleep
  Serial.println("Sleeping...");
  ESP.deepSleep(30 * MINS);
}
