float sound;
int sensor = A0;
float valeur;
void setup() 
{
  pinMode(sensor,INPUT);
  Serial.begin(9600);
}

void loop() 
{
    valeur = analogRead(sensor); // Reads the value from the Analog PIN A0
//    float val = 0;
//
//    for(int i=0; i<250; i++)
//    {
//        val += analogRead(A0);
//    }
//
//    valeur = val/250;
    if (valeur < 3)
    {
       sound = 32; 
    }
    
    else
    {
    sound = 20*log10(valeur*91.3670324 -220.0092); 
    }

    Serial.print("sound level ");
    Serial.print(sound,4);
    Serial.print(" ");
    Serial.print("  ");
    Serial.print("sensorvalue ");
    Serial.println(valeur);
    delay(100);
}
