#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Losant.h>
#include <my_credentials.h>


float sound;
float maximum;
float minimum;
float average;
float Allvalue;
int temp = 1;
float P0 = 4540403.39793356;
float P1 = -2695328.88406384;
float P2 = 513679.63231002;
float P3 = -16110.00641618;
float MaxValue = 0;
float window[] = {0,0,0,0,0};

int valeur ;
int sensor = A0;
// WiFi credentials.
const char* WIFI_SSID = WSSID;
const char* WIFI_PASS =  WPASS;

// Losant credentials.
const char* LOSANT_DEVICE_ID = DEVICE_ID;
const char* LOSANT_ACCESS_KEY = ACCESS_KEY;
const char* LOSANT_ACCESS_SECRET = ACCESS_SECRET;
WiFiClientSecure wifiClient;

LosantDevice device(LOSANT_DEVICE_ID);

void UpdateMax (float Value)
{
  MaxValue = 0;
  for (int i = 4; i > 0; i--)
  {
    //Serial.print(i);
    window[i]= window[i-1];
  }
  window[0] = Value;
  
  for (int a = 0;a < 5;a ++)
  {
    if (window[a]>MaxValue)
    {
      MaxValue = window[a];
    }
  }
}

void setup() 
{
 // Connect to Wifi.
   Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);


  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long wifiConnectStart = millis();

  while (WiFi.status() != WL_CONNECTED) 
  {
    // Check to see if
    if (WiFi.status() == WL_CONNECT_FAILED) 
    {
      Serial.println("Failed to connect to WIFI. Please verify credentials: ");
      Serial.println();
      Serial.print("SSID: ");
      Serial.println(WIFI_SSID);
      Serial.print("Password: ");
      Serial.println(WIFI_PASS);
      Serial.println();
    }

    delay(500);
    Serial.println("...");
    // Only try for 5 seconds.
    if(millis() - wifiConnectStart > 5000) 
    {
      Serial.println("Failed to connect to WiFi");
      Serial.println("Please attempt to send updated configuration parameters.");
      return;
    }
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
    Serial.print("Authenticating Device...");
  HTTPClient http;
  http.begin("http://api.losant.com/auth/device");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["deviceId"] = LOSANT_DEVICE_ID;
  root["key"] = LOSANT_ACCESS_KEY;
  root["secret"] = LOSANT_ACCESS_SECRET;
  String buffer;
  root.printTo(buffer);

  int httpCode = http.POST(buffer);

  if(httpCode > 0) 
  {
      if(httpCode == HTTP_CODE_OK) 
      {
          Serial.println("This device is authorized!");
      } 
      else 
      {
        Serial.println("Failed to authorize device to Losant.");
        if(httpCode == 400) 
        {
          Serial.println("Validation error: The device ID, access key, or access secret is not in the proper format.");
        } 
        else if(httpCode == 401) 
        {
          Serial.println("Invalid credentials to Losant: Please double-check the device ID, access key, and access secret.");
        } 
        else 
        {
           Serial.println("Unknown response from API");
        }
      }
    } 
    else 
    {
        Serial.println("Failed to connect to Losant API.");

   }

  http.end();
  
// Connect to Losant.

  Serial.println();
  Serial.print("Connecting to Losant...");

   device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

    while(!device.connected()) 
    {
    delay(500);
    Serial.print(".");
    }
    Serial.println("Connected!");
    Serial.println();
    Serial.println("This device is now ready for use!");
}


void report(double maxi, double minim , double avg) 
{
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["maxi"] = maxi;
  root["minim"] = minim;
  root["avg"] = avg;
  device.sendState(root);   // send all the DATA
  Serial.println("Reported!");
}
void Sampling(int Sample_D, int n_Sample, int Sleep_t, int Mode)
{
    Allvalue = 0;
    maximum = 0;
    minimum = 0;
    average = 0;
    int i = 0;
    for (i = 0; i < n_Sample ; i++)
    {
      float OldvoltageValue;
      OldvoltageValue =  analogRead(sensor)*(3.3/1024);
      if (OldvoltageValue <= 0.039)
      {
        OldvoltageValue = 0.039;
      }
      UpdateMax(OldvoltageValue);
      sound = P0 * pow(MaxValue,3) + P1 * pow(MaxValue,2) + P2 * MaxValue + P3;
      sound = 20*log10(sound);
    
      if(i == 0)
      {
        maximum = sound;     // For the first meausure
        minimum = sound;
      }
      else if (sound < minimum)   //If the actually sound is lower than the lowest sound measured 
      {
        minimum = sound; 
      }
      else if (sound > maximum) //If the actually sound is higher than the highest sound measured
      {
        maximum = sound;
      }
      Allvalue = Allvalue + sound; //all the values for this sampling
      if (Mode == true)
      {
        average = Allvalue / (i + 1);    //Makes the average of the previous measures
        Serial.println("");
        Serial.print(average);
        Serial.print(",");
        Serial.print(maximum);
        Serial.print(",");
        Serial.print(minimum);
      }
      delay(Sample_D);
      device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);
    }
    average = Allvalue / n_Sample;    //Makes the average of this sample
    if (Mode == false)
    {
      Serial.println("");
      Serial.print(average);
      Serial.print(",");
      Serial.print(maximum);
      Serial.print(",");
      Serial.print(minimum);
    }
    report(maximum, minimum, average);
    delay(1000);  // to make sure that it is reported
    //ESP.deepSleep(Sleep_t); // going to sleep          
}
void loop() 
{
    device.loop();
    int sample_delay = 1000;
    int number_of_samples = 60;
    int Sleep_time = 20e6;
    int Mode = true;    // True = show serial monitor every samples, false = show serial monitor every reports
    Sampling(sample_delay, number_of_samples, Sleep_time, Mode);    //    
   
   // delay(5000);    // to make sure that the values are reported
}
