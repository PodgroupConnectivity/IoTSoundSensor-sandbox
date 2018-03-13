int led = 13;
int threshold = 100; //Change This
int volume;
int sensor = A0;

void setup() {                
  Serial.begin(9600); // For debugging
  pinMode(led, OUTPUT);
  pinMode (sensor, INPUT);   
}

void loop() {
  
  volume = analogRead(sensor); // Reads the value from the Analog PIN A0
 // Serial.print("Sound Level: ");
  Serial.println(volume);//print the value
  
  if(volume>=threshold)
  {
    digitalWrite(led, HIGH); //Turn ON Led
    delay(200);
  }  
  else
  {
    digitalWrite(led, LOW); // Turn OFF Led
  }
delay(100);
}
