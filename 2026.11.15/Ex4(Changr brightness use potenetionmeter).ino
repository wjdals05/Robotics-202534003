const int potPin =A2;
const int ledPin =9;

void setup()
{
  Serial.begin(115200);
  pinMode(ledPin,OUTPUT);
  
}

void loop()
{
  int value = analogRead(potPin);
  int ledValue = map(value,0,1023,0,255);  
  analogWrite(ledPin, ledValue);
  delay(50);
}
