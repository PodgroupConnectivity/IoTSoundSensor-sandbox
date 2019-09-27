
#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <Adafruit_ZeroI2S.h>
#include <SPI.h>

#define SAMPLERATE_HZ 44100

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 8,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = LMIC_UNUSED_PIN,
  .dio = {3, 6, LMIC_UNUSED_PIN},
};

Adafruit_ZeroI2S i2s;

void setup() {
     // Configure serial port.
  Serial.begin(115200);
  Serial.println("Zero I2S Audio Tone Generator");

  // Initialize the I2S transmitter.
  if (!i2s.begin(I2S_32_BIT, SAMPLERATE_HZ)) {
    Serial.println("Failed to initialize I2S transmitter!");
    while (1);
  }

  i2s.enableRx();
  pinMode(11,OUTPUT);
}


void loop() {

     int32_t left;
     int32_t right;
     digitalWrite(11, LOW);
      int32_t sample;
     
     // i2s.read(sample,sample);
     i2s.read(&left, &right);
     /*if(digitalRead(0)==0)
      {sample=left;}
      else
      {sample=right;}*/
   
     Serial.println(left); 

     //delay(1000);
}
