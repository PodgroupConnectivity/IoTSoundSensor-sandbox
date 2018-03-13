int LED = 13;
const int pinA0 = A0;
// the setup function runs once when you press reset or power the board
void setup() {

  pinMode(pinA0,INPUT);
  Serial.begin(9600);
}

// the loop function runs over and over again forever
void loop() {
float val = 0;

    for(int i=0; i<250; i++)
    {
        val += analogRead(pinA0);
    }

    val = val/250;
    val = val * (0.0032226);
    Serial.print("voltage Level: ");
    Serial.println(val,5);//print the sound value to serial
    delay(500);
}

