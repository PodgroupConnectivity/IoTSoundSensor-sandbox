void setup()
{
Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
float VeryOldSensor;
float OldSensor;
float NewSensor;
}
float VeryOldSensor;
float OldSensor;
float NewSensor;
void loop()
{
VeryOldSensor = analogRead(A0); //read Analog 0
OldSensor = analogRead(1); //read Analog 1
NewSensor = analogRead(2); //read Analog 2
VeryOldSensor = VeryOldSensor * (0.003223);
OldSensor = OldSensor * (0.003223);
NewSensor = NewSensor * (0.003223);
Serial.print("voltage Very old sensor: ");
Serial.println(VeryOldSensor);//print the sound value to serial
Serial.print("\t");
Serial.print("voltage Old Sensor: ");
Serial.println(OldSensor);//print the sound value to serial
Serial.print("\t");
Serial.print("voltage New Sensor: ");
Serial.println(NewSensor);//print the sound value to serial
Serial.print("\t");
delay(1000);
}
