//Variable declaration for pins - 8bit unsigned integers
//Constants used as pin numbers do not change during program execution
//Both variables initialised to their respective pin values 
const uint8_t LED_PIN = 13;
const uint8_t PIR_PIN = 2;


//Initialise serial monitor and set output LED pin
void setup()
{
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop()
{

  //boolean variable to hold state of PIR input
  bool pirState = 0;
 
  //Read pir pin and allocate value to pir state variable
  pirState =  digitalRead(PIR_PIN);

  //light LED based on state of PIR sensor
  digitalWrite(LED_PIN, pirState);
  
  //output PIR and LED status to serial monitor
  Serial.print("LED:");
  Serial.print(digitalRead(LED_PIN));
  Serial.print("  PIR:");
  Serial.println(pirState);
 
  // loop logic every 100ms
  delay(100); 
}
