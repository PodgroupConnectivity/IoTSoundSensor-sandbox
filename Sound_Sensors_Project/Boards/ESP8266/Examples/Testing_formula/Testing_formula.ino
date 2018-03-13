int led = 13;
int threshold = 100; //Change This
int sensor = A0;
int valeur ;
float mV = 4.882;
float uPA;
float Decibel;

void setup() {                
  Serial.begin(9600); // For debugging
  pinMode(led, OUTPUT);
  //pinMode (sensor, INPUT);   
}

void loop() {
  
  valeur = analogRead(sensor); // Reads the value from the Analog PIN A0
  //uPA = valeur * mV * (1/2.238) * 100000;
  Decibel = 20*log10((valeur*0.004882*158.489319)/20)+40; 
  if (valeur==0)
  {
    Decibel = -73+40;
  }
  
  
  
  Serial.print("Sound Level: ");
  Serial.println(Decibel);//print the value
  
  
  if(valeur>=threshold)
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
