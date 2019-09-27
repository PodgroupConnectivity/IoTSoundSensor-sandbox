
#include <Arduino.h>
#include <Adafruit_ZeroI2S.h>
#include <SPI.h>
#include <stdio.h>
#include <math.h>

#define SAMPLERATE_HZ 44100

int32_t samplesxxx=128;

// Parameters of sampling algorithm
const int sample_delay = 1000;          // Delay between samples in ms  




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
  
  i2s.enableRx(); //We enable receiving pin
  
  pinMode(11,OUTPUT); //Enable the pin 11 for digital output because
                      //it's going to select the channel where we will
                      //receive the data
}

void loop() {

  
  digitalWrite(11, LOW);  //Selection of the left channel

  float sound;
  int32_t left,right;
  int i;
  int sample=0;
  // read a bunch of samples:
  int32_t samples[samplesxxx];
  
    for (int i = 0; i < samplesxxx; i++) 
    {
              
     i2s.read(&left, &right);
     delay(1);  //this delay prevents the oversizing of the buffer

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

    // subtract it from all samples to get a 'normalized' output
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

}
