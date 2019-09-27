

/*
  IoTSoundSensor. Copyright (c) 2019 Pod Group Ltd. http://podm2m.com
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3 as
  published by the Free Software Foundation
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
  Authors :
   - Alejandro Morón Mas <alejandro.moron@podgroup.com>
   - Juan Carlos <>
   - J. Félix Ontañón <felix.ontanon@podgroup.com>
*/

/*  This sketch connects to TheThings Network by OTAA using LoRa
    takes a bunch of samples, finds the average, the maximum and the minimum
    and sends the 3 variables through LoRa and falls into a sleep mode
*/

/*
You can freely modify the parameters below to adapt the behaviour of the
sound sensor. By default values provided below. Handle with care !

You can configure the sampling ratio with the three parameters below. The default behaviour is calculating min, max, avg
values from 60 samples along a minute (one sample a second) and then sleeping for 5 minutes.
That provides with 12 measurements an hour, 288 a day, 2016 a week (and so on).

WARNING: Manipulating this parameters can lead into more battery consumption !!
*/

// Parameters of sampling algorithm
const int sample_delay = 1000;// Delay between samples in ms, 1 second
const int N_samples = 60;// Number of samples per minute, 60 each minute
const int sleepytime = 300000;// Five minutes of deepsleep mode  



#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>
#include <math.h>

//libraries for connecting to TTN
#include <lmic.h>
#include <hal/hal.h>

//library for using I2S
#include <Adafruit_ZeroI2S.h>

//library for programming the deepSleep mode
#include <ArduinoLowPower.h>

#define SAMPLERATE_HZ 44100//I2S reading rate in Hz (44100 is the typical one)

// Vars for sound sampling

float maximum;// Max sound measured
float maximum2;// Second Max sound measured. When comes from deepsleep the first measure goes wrong 
float minimum;// Min sound measured
float average;// Average from all N_samples

// LoRa Variables
char TTN_response[30];// var for RX data from TTN
static osjob_t sendjob;// LMIC job variable

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 8,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = LMIC_UNUSED_PIN,
  .dio = {3, 6, LMIC_UNUSED_PIN},
};

// LoRa access
#include <my_credentials.h>

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = APPEUIS;
void os_getArtEui (u1_t* buf) {
  memcpy_P(buf, APPEUI, 8);
}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = DEVEUIS;
void os_getDevEui (u1_t* buf) {
  memcpy_P(buf, DEVEUI, 8);
}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
static const u1_t PROGMEM APPKEY[16] = APPKEYS;
void os_getDevKey (u1_t* buf) {
  memcpy_P(buf, APPKEY, 16);
}

int32_t samplesxxx=128;// Bunch of samples to get a right measure

Adafruit_ZeroI2S i2s;


// sampling function
void sampling(int sample_delay, int N_samples)
{  
  digitalWrite(11, LOW);//configure the pin for just receiving data from the left channel
  average = 0;

  float sound;
  int32_t left, right;
  int i, j;
  
  // read a bunch of samples:
  int32_t samples[samplesxxx];
  for (j = 0; j <= N_samples; j++)
  {
    for (int i = 0; i < samplesxxx; i++) 
    {
        i2s.read(&left, &right);
        delay(1);  
      // convert to 18 bit signed
      left >>= 14;
      samples[i] = left;
    }

    // we have the samples, get the mean (avg)
    float meanval = 0;
    for (int i = 0; i < samplesxxx; i++) 
    {
      meanval += samples[i];
    }
    meanval /= samplesxxx;

    // subtract it from all samples to get a 'normalized' output
    for (int i = 0; i < samplesxxx; i++) 
    {
      samples[i] -= meanval;
    }

    // find the 'peak to peak' max
    float maxsample, minsample;
    minsample = 100000;
    maxsample = -100000;
    for (int i = 0; i < samplesxxx; i++) 
    {
      minsample = min(minsample, samples[i]);
      maxsample = max(maxsample, samples[i]);
    }

    //obtain the final measure in dB
    sound = 10 * log(maxsample - minsample);

    Serial.println(sound);

    average += sound;

//Selection of the max and min measure
    if (j == 1)
    {
      // First measurement
      maximum = sound;
     maximum2 = sound;
      minimum = sound;
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
     else if (sound > maximum2 && sound < maximum)
    {
      // If the sound is higher than
      // the highest sound measured
      maximum2 = sound;
    }
    delay(sample_delay);
  }
  
  average=average/j;
  Serial.print("average = "); Serial.println(average);
  Serial.print("minimum = "); Serial.println(minimum);
  Serial.print("maximum = "); Serial.println(maximum2);


}


// Report function
void report_data(osjob_t* j) {
  
  // Var for sending the data
  // unsigned int of 8 bytes
  // 21 = 6B * 3 float + 2 ';' + \0
  // Init to {0,0,0...} important for the \0
  static uint8_t message[21] = "";

  // Buffer for storing the data
  // var for intermediate conversion
  String buff;
  char bufferance[21];

  Serial.print("report_data(): OS Timestamp [");
  Serial.print(os_getTime());
  Serial.print("]: ");
  Serial.println("taking samples from sensor");

  // Calling sampling function to read
  // sample and convert data from the sensor
  //Serial.println (average);
  sampling(sample_delay, N_samples);

  // We transform the data to String
  buff = (String) maximum2 + ";" + (String) minimum + ";" + (String) average;

  // Later we transform the String into a char array
  buff.toCharArray(bufferance, 21) ;
  memcpy(message, bufferance, sizeof(bufferance));

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println("OP_TXRXPEND: not sending");
    Serial.println("OP_TXRXPEND: There is already another TX/RX job running");
  } else {

    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, message, sizeof(message) - 1, 0);

    // This sends data uplink, when it finished, it will trigger
    // an EV_TXCOMPLETE event which will be handled by
    // onEvent() function

    // Serial debug
    Serial.print("report_data(): OS Timestamp [");
    Serial.print(os_getTime());
    Serial.print("]: ");
    Serial.print("uplink message payload : ");
    Serial.println(buff);
   
  }
}

// Event handler funtion for LMIC
void onEvent (ev_t ev) {
  Serial.print("onEvent(): OS Timestamp [");
  Serial.print(os_getTime());
  Serial.print("]: ");

  switch (ev) {
    case EV_TXCOMPLETE:
      Serial.println("EV_TXCOMPLETE (includes waiting for RX windows)");
      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println("TXRX_ACK: Received ack");
      }

      if (LMIC.dataLen) {
        int i = 0;
        // data received in rx slot after tx
        Serial.print("Data Received: ");
        Serial.write(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
        Serial.println();
        Serial.println(LMIC.rssi);

        for ( i = 0 ; i < LMIC.dataLen ; i++ )
          TTN_response[i] = LMIC.frame[LMIC.dataBeg + i];
        TTN_response[i] = 0;
      }
      // When event finished, call back to
      // and report function
      os_setCallback (&sendjob, report_data);

      //We put the board into the DeepSleep mode
      Serial.println("Going to sleep");
      LowPower.deepSleep(sleepytime); 
      Serial.println("Waking up!");
               
      break;

    case EV_JOINING:
      Serial.println("EV_JOINING: -> Joining...");
      break;

    case EV_JOINED:
      Serial.println("EV_JOINED");

      // Disable link check validation (automatically enabled
      // during join, but not supported by TTN at this time).
      LMIC_setLinkCheckMode(0);

      // Starting the  job of reading from sensor
      // and sent data uplink when finished
      // report_data(&sendjob); // This also works
        Serial.println("Hola, estoy JOINED");
      os_setCallback (&sendjob, report_data);
      
      break;

    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println("EV_RXCOMPLETE");
      break;

    case EV_LINK_DEAD:
      Serial.println("EV_LINK_DEAD");
      break;

    case EV_LINK_ALIVE:
      Serial.println("EV_LINK_ALIVE");
      break;

    default:
      Serial.println("Unknown event");
      break;
  }

}


void setup() {

  // Setting up serial
  Serial.begin(115200);
  delay(5000);
  Serial.println("Zero I2S Audio Tone Generator");

   // Initialize the I2S transmitter.
  if (!i2s.begin(I2S_32_BIT, SAMPLERATE_HZ)) {
    Serial.println("Failed to initialize I2S transmitter!");
    while (1);
  }
  //enable the receive pin
  i2s.enableRx();
  //pin 11 marked as output for control the mic channel
  pinMode(11,OUTPUT);

  Serial.println("setup(): Serial starting");

  // LMIC init
  os_init();

  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);


  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.

  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);

  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink
  // note: txpow seems to be ignored by the library

  //LMIC_setDrTxpow(DR_SF11,14);
  LMIC_setDrTxpow(DR_SF9, 14);

  // Start joining attemp to network
  LMIC_startJoining();

}

void loop() {

  // For more info read LMIC API
  os_runloop_once();
}
