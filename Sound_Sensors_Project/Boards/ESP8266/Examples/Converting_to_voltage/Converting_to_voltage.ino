void setup()
{
Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
float val;
int treshold;
}
void loop()
{
float val;
val=analogRead(A0); //connect mic sensor to Analog 0
val = val * (0.0032226);
Serial.print("voltage Level: ");
Serial.println(val,5);//print the sound value to serial
//if(valeur>=threshold)
//  {
//    digitalWrite(led, HIGH); //Turn ON Led
//    delay(200);
//  }  
//  else
//  {
//    digitalWrite(led, LOW); // Turn OFF Led
//  }
delay(500);
}

