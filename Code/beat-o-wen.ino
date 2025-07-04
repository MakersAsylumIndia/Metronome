// Importing config file
#include "config.h"
// Importing ticker library for creating beat timings
#include <Ticker.h>

#define motorPin 14 // GPIO14 => D5

// Setting up the feeds
AdafruitIO_Feed *onoffFeed = io.feed("VIB_onoff");
AdafruitIO_Feed *bpmFeed = io.feed("VIB_bpm");
AdafruitIO_Feed *pulsewidthFeed = io.feed("VIB_pulsewidth");

// Defining variables
bool on = false;
int bpm = 80;
int pulseWidth = 100;
unsigned long interval = 60000UL / 80;

// Defining Tickers
Ticker beatTicker;
Ticker pulseOffTimer;

void setup() {
  // Defining motor connections and ensuring motor is off in the start
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);
  
  // Starting the serial connection
  Serial.begin(115200);

  // Connecting to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // Setting up message handlers
  onoffFeed->onMessage(handleOnOff);
  bpmFeed->onMessage(handleBpmChange);
  pulsewidthFeed->onMessage(handlePulsewidthChange);

  // Waiting for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // On connect
  Serial.println("CONNECTED");
  Serial.println(io.statusText());

  // Getting initial values of feeds
  onoffFeed->get();
  bpmFeed->get();
  pulsewidthFeed->get();

  // Starting beat with initial conditions
  beatTicker.attach_ms(interval, pulseBeat);
}

void pulseBeat() {
  digitalWrite(motorPin, HIGH);                // Turn on motor
  pulseOffTimer.once_ms(pulseWidth, endPulse); // Schedule turning it off after pulseWidth has passed
}

void endPulse() {
  digitalWrite(motorPin, LOW); // Turn off motor
}

void loop() {
  // Keep Adafruit connection alive
  io.run();
}

// Callback functions
void handleOnOff(AdafruitIO_Data *data) {
  if(data->toInt() == 1) {
    Serial.println("Metronome ON");
    on = true;
    beatTicker.detach();
    beatTicker.attach_ms(interval, pulseBeat);
  } else {
    Serial.println("Metronome OFF");
    on = false;
    beatTicker.detach();
  }
}

void handleBpmChange(AdafruitIO_Data *data) {
  Serial.println("Changing BPM to: ")
  Serial.print(data->toInt());
  bpm = data->toInt();
  interval = 60000UL / bpm;
  if (on) {
    beatTicker.detach(); // Stop the previous ticker
    beatTicker.attach_ms(interval, pulseBeat); // Attach with new interval
  }
}

void handlePulsewidthChange(AdafruitIO_Data *data) {
  Serial.println("Pulsewidth changed to: ")
  Serial.print(data->toInt());
  pulseWidth = data->toInt();
}
