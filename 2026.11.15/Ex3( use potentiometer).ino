const int potPin =A2;

void setup()
{
  Serial.begin(115200);
  
  
}

void loop()
{
  int value = analogRead(potPin);
  Serial.println(value);
  
  delay(50);
}
