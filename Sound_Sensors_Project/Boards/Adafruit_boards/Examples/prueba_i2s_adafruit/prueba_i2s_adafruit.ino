
#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <Adafruit_ZeroI2S.h>
#include <SPI.h>
#include <stdio.h>
#include <math.h>

#define SAMPLERATE_HZ 44100

int32_t samplesxxx=128;

// Parameters of sampling algorithm
const int sample_delay = 1000;          // Delay between samples in ms
const int N_samples = 60;   

float maximum;                  // Max sound measured
float minimum;                  // Min sound measured
float average;                  // Average from all N_samples

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
   digitalWrite(11, LOW);
  average = 0;
  float sound;
  int32_t left,right;
  int i = 0;
  int j;
  int sample=0;
  // read a bunch of samples:
  int32_t samples[samplesxxx];
  for (j = 0; j <= N_samples; j++)
  {
    for (int i = 0; i < samplesxxx; i++) 
    {
              
     i2s.read(&left, &right);
     delay(1);

     sample=left;
      // convert to 18 bit signed
      sample >>= 14;
      samples[i] = abs(sample);
    }
      //Serial.println(sample);
    // ok we hvae the samples, get the mean (avg)
    float meanval = 0;
    for (int i = 0; i < samplesxxx; i++) {
      meanval += samples[i];
    }
    meanval /= samplesxxx;
    //Serial.print("# average: " ); Serial.println(meanval);

    // subtract it from all sapmles to get a 'normalized' output
    for (int i = 0; i < samplesxxx; i++) {
      samples[i] -= meanval;
      //Serial.println(samples[i]);
    }

    // find the 'peak to peak' max
    float maxsample, minsample;
    minsample = 100000;
    maxsample = -100000;
    for (int i = 0; i < samplesxxx; i++) {
      minsample = min(minsample, samples[i]);
      maxsample = max(maxsample, samples[i]);
    }
    sound = 10 * log(maxsample - minsample);

    Serial.println(sound);

    //Serial.println(maxsample - minsample);

    average += sound;

    if (j == 0)
    {
      // First measurement
      maximum = sound;
      minimum = sound;
      Serial.print("max="); Serial.println(maximum);
    }
    else if (sound < minimum)
    {
      // If the sound is lower than
      // the lowest sound measured
      minimum = sound;
    }
    else if (sound > maximum)
    {
      // If the sound is higher than
      // the highest sound measured
      maximum = sound;
    }
    delay(sample_delay);
  }
  Serial.print("average = "); Serial.println(average / j);
  Serial.print("minimum = "); Serial.println(minimum);
  Serial.print("maximum = "); Serial.println(maximum);
}
