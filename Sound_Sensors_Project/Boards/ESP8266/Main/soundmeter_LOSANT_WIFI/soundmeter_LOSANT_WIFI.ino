#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Losant.h>
#include <EEPROM.h>
#include <my_credentials.h>

float sound;
float maximum;
float minimum;
float average;
float Allvalue;
int temp = 1;
int i;

int valeur ;
int sensor = A0;
// WiFi credentials.
const char* WIFI_SSID = WSSID;
const char* WIFI_PASS = WPASS;

// Losant credentials.
const char* LOSANT_DEVICE_ID = DEVICE_ID;
const char* LOSANT_ACCESS_KEY = ACCESS_KEY;
const char* LOSANT_ACCESS_SECRET = ACCESS_SECRET;
WiFiClientSecure wifiClient;

LosantDevice device(LOSANT_DEVICE_ID);



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


void report(double soundLevel, double maxi, double minim , double avg) 
{
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["soundLevel"] = soundLevel;
  root["maxi"] = maxi;
  root["minim"] = minim;
  root["avg"] = avg;
  device.sendState(root);   // send all the DATA
  Serial.println("Reported!");
  delay(5000);  // to make sure that the values are reported
}
void loop() 
{
    device.loop();

    
    int addr = 0;             //Read Temp in the EEPROM 
    EEPROM.begin(12);
    byte TempHi;
    byte TempLo;
    EEPROM.get(addr, TempHi);
    addr += 1;
    EEPROM.get(addr, TempLo);
    temp = ((TempHi <<8)+TempLo);
    EEPROM.commit();
    EEPROM.end();

    
    if(temp > 1)
    {
    EEPROM.begin(50);
    addr ++;

    byte *AllValread = (byte *)&Allvalue;   //Read Allvalue in the EEPROM
    for(i = 0; i < 4;i++)
    {
    EEPROM.get(addr, AllValread[i]);
    addr ++;
    }

    i = 0;
    byte *minimumread = (byte *)&minimum; //Read mimimum in the EEPROM
    for(i = 0; i < 4;i++)
    {
    EEPROM.get(addr, minimumread[i]);
    addr ++;
    }
    
    i = 0;
    byte *maximumread = (byte *)&maximum; //Read maximum in the EEPROM
    for(i = 0; i < 4;i++)
    {
    EEPROM.get(addr, maximumread[i]);
    addr ++;
    }
    
    EEPROM.commit();
    EEPROM.end();   //save in the EEPROM
    }
    
    valeur = analogRead(sensor); // Reads the value from the Analog PIN A0
    if (valeur < 3)
    {
       sound = 32; 
    }
    else
    {
      sound = 20*log10(valeur*91.3670324 -220.0092); 
    }
    
    if(temp == 1)
    {
      maximum = sound;     // For the first meausure
      minimum = sound;
    }
    else if (sound < minimum)   //If the actually sound is lower than the lowest sound measured 
    {
      minimum = sound; 
    }
    else if (sound > maximum) //If the actually sound is higher than the highest soudn measured
    {
      maximum = sound;
    }
    Allvalue = Allvalue + sound;
    average = Allvalue / temp;    //Makes the average sonce the begining
    

    //temp=0;
    //Allvalue=0;
    Serial.print("sound level: ");
    Serial.print(sound);
    Serial.print("DB");
    Serial.print("\t");
    Serial.print("minimum: ");
    Serial.print(minimum);
    Serial.print("\t");
    Serial.print("maximum: ");
    Serial.print(maximum);
    Serial.print("\t");
    Serial.print("average: ");
    Serial.print(average);
    Serial.print("\t");
    Serial.print("temp: ");
    Serial.print(temp);
    Serial.print("\t");
    Serial.print("Allvalue: ");
    Serial.print(Allvalue);
    Serial.print("\t");   //Write alle the values in the serial plotter 
    report(sound, maximum, minimum, average); // Report the values to the Losant platform 

    temp ++;
    //temp=0;
    //Allvalue=0;
    EEPROM.begin(50);
    addr=0;
    EEPROM.put(addr, highByte(temp));
    addr +=1;
    EEPROM.put(addr, lowByte(temp));
    addr +=1;                       // write temp

    i = 0;
    byte *AllVal = (byte *)&Allvalue; //Write Allvalue in the EEPROM
    for(i = 0; i < 4;i++)
    {
    EEPROM.put(addr, AllVal[i]);
    addr ++;
    }

    i = 0;
    byte *minimumwrite = (byte *)&minimum;  //Write minimum in the EEPROM
    for(i = 0; i < 4;i++)
    {
    EEPROM.put(addr, minimumwrite[i]);
    addr ++;
    }

    i = 0;
    byte *maximumwrite = (byte *)&maximum;  //Write maximum in the EEPROM
    for(i = 0; i < 4;i++)
    {
    EEPROM.put(addr, maximumwrite[i]);
    addr ++;
    }
    
    EEPROM.commit();
    EEPROM.end();   //save the values in the EEPROM 

    delay(5000);    // to make sure that the values are reported
    Serial.println("Going into deep sleep for 10 seconds"); //Write that it gonna in deep sleep mode
    ESP.deepSleep(10e6); // 10e6 is 10 seconds going to slep for 10 seconds           
    delay(5000); // to make sure that the board is awake
    
}
