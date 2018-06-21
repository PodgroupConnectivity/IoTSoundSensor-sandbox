#include <Arduino.h>
#include <SSD1306.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <stdio.h>
#include <math.h>

// Definitions for the LED screen
#define OLED_I2C_ADDR 0x3C
#define OLED_RESET 16
#define OLED_SDA 4
#define OLED_SCL 15

// Led screen into global var
SSD1306 display (OLED_I2C_ADDR, OLED_SDA, OLED_SCL);

// Calibration parameters
const float P0 = 4540403.39793356;
const float P1 = -2695328.88406384;
const float P2 = 513679.63231002;
const float P3 = -16110.00641618;

// Parameters of sampling algorithm
const int sample_delay = 1000;          // Delay between samples in ms
const int N_samples = 15;               // Number of samples to take

// Vars for sound sampling
int sensor = A0;                // Sensor to read data from
float maximum;                  // Max sound measured
float minimum;                  // Min sound measured
float average;                  // Average from all N_samples

// Vars for max moving filter
float MaxValue = 0;
float window[] = {0, 0, 0, 0, 0};

// LoRa Variables
char TTN_response[30];                  // var for RX data from TTN
static osjob_t sendjob;                 // LMIC job variable
const lmic_pinmap lmic_pins = {         // Pin mapping for heltec board
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 33, 32}
};

// LoRa credentials
// This EUI must be in little-endian format or LSB
// LSB
static const u1_t PROGMEM DEVEUI[8] = { ... };
static const u1_t PROGMEM APPEUI[8] = { ... };

// This key should be in big endian format or MSB
// MSB
static const u1_t PROGMEM APPKEY[16] = { ... };

// Implementation of LMIC functions
// for retrieving TTN credentials
void os_getDevEui (u1_t* buf) { 
    memcpy_P(buf, DEVEUI, 8);
}

void os_getArtEui (u1_t* buf) { 
    memcpy_P(buf, APPEUI, 8);
}

void os_getDevKey (u1_t* buf) { 
    memcpy_P(buf, APPKEY, 16);
}

// UpdateMax updates the max value for
// a window of 5 samples
// This a common max moving filter
// in DSP jargon
void UpdateMax (float Value)
{
    MaxValue = 0;
    for (int i = 4; i > 0; i--)
    {
        window[i] = window[i - 1];
    }
    
    window[0] = Value;

    for (int a = 0; a < 5; a ++)
    {
        if (window[a] > MaxValue)
        {
            MaxValue = window[a];
        }
    }
}

// sampling function
void sampling(int sample_delay, int N_samples)
{
    float sound;
    float sum_value = 0;
    int i = 0;
    
    maximum = 0;
    minimum = 0;
    average = 0;
    
    for (i = 0; i < N_samples ; i++)
    {
        float sensor_voltage;

        // Getting the new data from sensor
        sensor_voltage =  analogRead(sensor) * (3.3 / 1024);
        if (sensor_voltage <= 0.039)
        {
            // Minimum sound sensibility 
            // for the calibration done
            sensor_voltage = 0.039;
        }

        // Filtering the data
        UpdateMax(sensor_voltage);

        // Doing the calibration
        sound = P0 * pow(MaxValue, 3) + P1 * pow(MaxValue, 2) + P2 * MaxValue + P3;
        sound = 20 * log10(sound);
        
        // Updating max min and average
        // for each sample in N_samples
        if (i == 0)
        {
            // First measurement
            maximum = sound;
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

        // Doing the sum for later averaging
        sum_value = sum_value + sound; 
        delay(sample_delay);
        
    }

    // Doing the average
    average = sum_value / N_samples;

}


// Report function
void report_data(osjob_t* j){

    // Var for sending the data
    // unsigned int of 8 bytes
    static uint8_t message[7] = "";

    // Buffer for storing the data
    // var for intermediate conversion
    char buffer[7];

    // Drawing
    display.clear();
    display.drawString (0, 0, "report_data():");
    display.drawString (0, 50, "Sampling");
    display.display ();

    // Calling sampling function to read
    // sample and convert data from the sensor
    Serial.print("report_data(): OS Timestamp [");
    Serial.print(os_getTime());
    Serial.print("]: ");
    Serial.println("taking samples from sensor");
    
    sampling(sample_delay, N_samples);

    // From average var to buffer with format
    // [ 3 integer chipers, 2 fractionary chipers ]
    sprintf(buffer, "%3.2f", average);

    // From buffer to uint8_t array
    memcpy(message, buffer, sizeof(buffer));

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println("OP_TXRXPEND: not sending");
        Serial.println("OP_TXRXPEND: There is already another TX/RX job running");
    } else {

        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, message, sizeof(message)-1, 0);
        
        // This sends data uplink, when it finished, it will trigger
        // an EV_TXCOMPLETE event which will be handled by
        // onEvent() function

        // Serial debug
        Serial.print("report_data(): OS Timestamp [");
        Serial.print(os_getTime());
        Serial.print("]: ");
        Serial.print("uplink message: ");
        Serial.print("average = ");
        Serial.print(average);
        Serial.println(" dBA");

        // Drawing
        display.clear();
        display.drawString (0, 0, "report_data():");
        display.drawString (0, 50, "Sending data");
        display.display ();
    }
}


// Event handler funtion for LMIC
void onEvent (ev_t ev) {
    Serial.print("onEvent(): OS Timestamp [");
    Serial.print(os_getTime());
    Serial.print("]: ");
    
    switch(ev) {
        case EV_TXCOMPLETE:
            Serial.println("EV_TXCOMPLETE (includes waiting for RX windows)");
            if (LMIC.txrxFlags & TXRX_ACK) {
                Serial.println("TXRX_ACK: Received ack");
            }

            if (LMIC.dataLen) {
                int i = 0;
                // data received in rx slot after tx
                Serial.print("Data Received: ");
                Serial.write(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
                Serial.println();
                Serial.println(LMIC.rssi);

                for ( i = 0 ; i < LMIC.dataLen ; i++ )
                    TTN_response[i] = LMIC.frame[LMIC.dataBeg+i];
                TTN_response[i] = 0;
            }

            // Drawing
            display.clear();
            display.drawString (0, 0, "onEvent():");
            display.drawString (0, 50, "EV_TXCOMPLETE    :)");
            display.display ();

            // When event finished, call back to 
            // and report function
            os_setCallback (&sendjob, report_data);
            break;

        case EV_JOINING:
            Serial.println("EV_JOINING: -> Joining...");
            
            // Drawing
            display.clear();
            display.drawString (0, 0, "onEvent():");
            display.drawString (0, 50, "EV_JOINING");
            display.display ();
            break;

        case EV_JOINED: 
            Serial.println("EV_JOINED");

            // Drawing
            display.clear();
            display.drawString (0, 0, "onEvent():");
            display.drawString (0, 50, "EV_JOINED");
            display.display ();

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);

            // Starting the  job of reading from sensor
            // and sent data uplink when finished
            // report_data(&sendjob); // This also works
            os_setCallback (&sendjob, report_data);
            break;

        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println("EV_RXCOMPLETE");
            
            // Drawing
            display.clear();
            display.drawString (0, 0, "onEvent():");
            display.drawString (0, 50, "EV_RXCOMPLETE");
            display.display ();
            break;

        case EV_LINK_DEAD:
            Serial.println("EV_LINK_DEAD");

            // Drawing
            display.clear();
            display.drawString (0, 0, "onEvent():");
            display.drawString (0, 50, "EV_LINK_DEAD :(");
            display.display ();
            break;

        case EV_LINK_ALIVE:
            Serial.println("EV_LINK_ALIVE");

            // Drawing
            display.clear();
            display.drawString (0, 0, "onEvent():");
            display.drawString (0, 50, "EV_LINK_ALIVE   =)");
            display.display ();
            break;

        default:
            Serial.println("Unknown event");

            // Drawing
            display.clear();
            display.drawString (0, 0, "onEvent():");
            display.drawString (0, 50, "Unknown event   :S");
            display.display ();
            break;
    }

}


void setup() {

    // Setting up serial
    Serial.begin(115200);
    delay(2500);

    Serial.println("setup(): Serial starting");

    // OLED screen settup
    pinMode(OLED_RESET, OUTPUT);
    digitalWrite(OLED_RESET, LOW);
    delay(50);
    digitalWrite(OLED_RESET, HIGH);

    // Init the screen
    display.init ();
    display.flipScreenVertically ();
    display.setFont (ArialMT_Plain_10);
    display.setTextAlignment (TEXT_ALIGN_LEFT);

    // Drawing
    display.clear();
    display.drawString (0, 0, "Void Setup...");
    display.drawString (0, 50, "Init Board");
    display.display ();

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
    LMIC_setDrTxpow(DR_SF9,14);

    // Start joining attemp to network
    LMIC_startJoining();

}

void loop() {

    // For more info read LMIC API
    os_runloop_once();

}
