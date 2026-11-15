const int buttonPin = 2;

void setup()
{
  pinMode(buttonPin,INPUT);
  Serial.begin(115200); // baud rate setting
}

void loop()
{
  int buttonState = digitalRead(buttonPin);
  
  if(buttonState == HIGH)
  {
  	Serial.println("Pressed");    
  }
  
  else
  {
  	Serial.println("Not Pressed");
  }
  
  delay(2000);
}

//digitalWrite(pin,HIGH/LOW)