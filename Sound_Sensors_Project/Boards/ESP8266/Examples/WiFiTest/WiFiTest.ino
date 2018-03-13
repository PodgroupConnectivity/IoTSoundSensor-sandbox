#include <ESP8266WiFi.h>
#include <my_credentials.h>
int threshold = 100; //Change This
int volume;
int sensor = A0;
void setup()
{
  pinMode (sensor, INPUT);
  Serial.begin(9600);
  Serial.println();

  WiFi.begin(WSSID, WPASS);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());


}
//https://fr.macerobotics.com/developpeur/tutoriels/esp8266-controle-dune-led-par-wifi/
void loop() {}

