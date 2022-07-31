//Variable declaration for pins - 8bit unsigned integers
//Constants used as pin numbers do not change during program execution
//Both variables initialised to their respective pin values 
const uint8_t LED_PIN = 13;
const uint8_t PIR_PIN = 2;



void setup()
{
  //set PIR pin as input	
  pinMode(PIR_PIN, INPUT);

  //attach interrupt handler to PIR pin and call interrupt function
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirInterrupt, CHANGE);
  
  //set LED pin as output
  pinMode(LED_PIN, OUTPUT);
  
  //Initialise serial monitor
  Serial.begin(9600);
}

void loop()
{

}


//New function to handle interrupts
void pirInterrupt()
{
  //write PIR state to LED output	
  digitalWrite(LED_PIN, digitalRead(PIR_PIN));

  //output PIR and LED status to serial monitor (will only output if state is changed)
  Serial.print("LED:");
  Serial.print(digitalRead(LED_PIN));
  Serial.print("  PIR:");
  Serial.println(digitalRead(PIR_PIN));
  
}
