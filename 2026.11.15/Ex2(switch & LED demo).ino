
const int buttonPin = 7;
const int ledPin = 2;


void setup()
{
  pinMode(buttonPin,INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200); // baud rate setting
  
}

void loop()
{
  int buttonState = digitalRead(buttonPin);
  
  if(buttonState == HIGH)
  {
  	Serial.println("Pressed"); 
    digitalWrite(ledPin, HIGH);
  }
  
  else
  {
  	Serial.println("Not Pressed");
    digitalWrite(ledPin, LOW);
  }
  
  delay(2000);
}
