#include <TinyWireS.h>

#include <avr/io.h>
#include <limits.h>
#include <avr/interrupt.h>

#include "Dataset.h"

#define INTERRUPT_PIN1 PCINT1     // This is PB1 per the schematic
#define INTERRUPT_PIN2 PCINT3
#define INT_PIN1 PB1              // Interrupt pin of choice: PB1 (same as PCINT1) - Pin 6
#define INT_PIN2 PB3              //     PB3 (PCINT3) - Pin 2
#define PCINT_VECTOR PCINT0_vect  // This step is not necessary - it's a naming thing for clarit

#define RADIUS 14             // Radius of the anemometer
#define ROTATION  (2 * RADIUS * 3.141)

#define BUCKET_MM 8.8         // mm required to tip the bucket

#define INTERVAL_MIN  5         // 5 minute interval
#define INTERVAL_MS   INTERVAL_MIN *60000
#define I2C_ADDRESS   87        // i2c address


bool initialised;
volatile unsigned int dbg = 0;
volatile unsigned int wind = 0;
volatile unsigned int rain_since = 0;

volatile Dataset dWind;

// ISR increment wind counter and rainfall
// Every 5 mins work out wind speed

unsigned long timeout;

// the setup function runs once when you press reset or power the board
void setup() {

  cli();                            // Disable interrupts during setup
  PCMSK |= (1 << INTERRUPT_PIN1);   // Enable interrupt handler (ISR) for our chosen interrupt pin (PCINT1/PB1/pin 6)
  PCMSK |= (1 << INTERRUPT_PIN2);    
  GIMSK |= (1 << PCIE);             // Enable PCINT interrupt in the general interrupt mask
  pinMode(INT_PIN1, INPUT_PULLUP);   // Set our interrupt pin as input with a pullup to keep it stable
  pinMode(INT_PIN2, INPUT_PULLUP);
  sei();                            //last line of setup - enable interrupts after setup

  initialised = false;

  // Setup i2c as a slave
  TinyWireS.begin(I2C_ADDRESS);
  TinyWireS.onRequest(request);

  // The ATtiny will work out the windspeed at intervals set by INTERVAL_MS
  // Set the timeout
  timeout = millis() + INTERVAL_MS;
}

void loop() {

    if(millis() >= timeout) {
      // Workout the next timeout
      timeout = millis() + INTERVAL_MS;

      // Calculate the windspeed (convert rotations to distance, multiply to find value per hour)
      dWind.add((int)(wind * ROTATION));
      
      // A reading has been taken, record that we have initialised
      initialised = true;
    }

    // If there's nothing to do sleep, hopefully this will reduce power consumption
    delay(200);
}


// Interupt service routine, increment the wind or rainfall counters
ISR(PCINT_VECTOR)
{
  if( digitalRead(INT_PIN1) == HIGH ) {
    dbg = 1;
    if(wind < UINT_MAX)
      wind++;
  }
  if( digitalRead(INT_PIN2) == HIGH ) {
    dbg = 2;
    if(rain_since < UINT_MAX) {
      rain_since++;
    }
  }
}


// Function called when a request is recieved for data
void request()
{
  unsigned int windspeed = (int)(dWind.average() * (60 / INTERVAL_MIN));
  unsigned int rain = (int)(rain_since * BUCKET_MM);
  
  TinyWireS.write((byte)initialised);                   // First byte indicates if the sensor has initialised (i.e. has a windspeed)
  TinyWireS.write((byte)(initialised ? highByte(windspeed) : 0)); // Windspeed MSB
  TinyWireS.write((byte)(initialised ? lowByte(windspeed) : 0));  // Windspeed LSB
  TinyWireS.write((byte)(initialised ? highByte(rain) : 0));      // Rainfall MSB
  TinyWireS.write((byte)(initialised ? lowByte(rain) : 0));       // Rainfall LSB
  TinyWireS.write((byte)(wind > MAX_BYTE ? MAX_BYTE : wind));     // Anemometer rotations
  TinyWireS.write((byte)(dWind.average() > MAX_BYTE ? MAX_BYTE : (byte)dWind.average()));
  TinyWireS.write((byte)(rain_since > MAX_BYTE ? MAX_BYTE : rain_since)); // Rain guage tips

  // If we have initialised, reset the rainfall
  if (initialised)
  {
    rain_since = 0;
  }
}
