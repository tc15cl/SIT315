//Variable declaration for pins - 8bit unsigned integers
//Constants used as pin numbers do not change during program execution
//Both variables initialised to their respective pin values 
const uint8_t LEDG_PIN = 13;
const uint8_t LEDY_PIN = 10;
const uint8_t PIR_PIN = 2;
const uint8_t BTN_PIN = 3;


void setup()
{
  //set PIR & button pins as input
  pinMode(PIR_PIN, INPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  //attach interrupt handler to PIR and button pins and call respective interrupt functions
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), btnInterrupt, CHANGE);
  
  //set LED pins as output
  pinMode(LEDG_PIN, OUTPUT);
  pinMode(LEDY_PIN, OUTPUT);
  
  //Initialise serial monitor
  Serial.begin(9600);
}

void loop()
{

}


//Function to handle PIR interrupts
void pirInterrupt()
{
  //write PIR state to Green LED output	
  digitalWrite(LEDG_PIN, digitalRead(PIR_PIN));

  //output PIR and LED status to serial monitor (will only output if state is changed)
  Serial.print("LED GREEN:");
  Serial.print(digitalRead(LEDG_PIN));
  Serial.print("  PIR:");
  Serial.println(digitalRead(PIR_PIN));
}

//New function to handle button interrupts
void btnInterrupt()
{
  //write button state to Yellow LED output  
  digitalWrite(LEDY_PIN, digitalRead(BTN_PIN));

  //output BTN and LED status to serial monitor (will only output if state is changed)
  Serial.print("LED YELLOW:");
  Serial.print(digitalRead(LEDY_PIN));
  Serial.print("  BTN:");
  Serial.println(digitalRead(BTN_PIN));
}
