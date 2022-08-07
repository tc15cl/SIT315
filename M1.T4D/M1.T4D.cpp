//Variable declaration for pins - 8bit unsigned integers
//LED pins referenced to AVR port #s
const uint8_t LEDG_PIN = PB5;
const uint8_t LEDY_PIN = PB2;
const uint8_t LEDR_PIN = PB0;
const uint8_t LEDW_PIN = PB4;
const uint8_t BTN0_PIN = 4;
const uint8_t BTN1_PIN = 5;
const uint8_t BTN2_PIN = 6;


void setup()
{
  noInterrupts(); //switch off interrupts for setup
  
  //set LED pins as output
  DDRB |= (1 << LEDG_PIN);
  DDRB |= (1 << LEDY_PIN);
  DDRB |= (1 << LEDR_PIN);
  DDRB |= (1 << LEDW_PIN);
  
  //Switch on pin change interrupts for PORT D
  PCICR |= B00000100;

  //Flag interrupt pins 4,5,6 on PORT D
  PCMSK2 |= B01110000;

  //clear timer registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  //16Mhz/(1024 prescaler*0.5Hz) - 2s timer initially
  OCR1A = 31250; 
  
  TCCR1B |= (1 << WGM12);   //CTC mode
  TCCR1B |= (1 << CS12)|(1 << CS10); //Set 1024 prescaler 
  TIMSK1 |= (1 << OCIE1A);  //Timer compare interrupt
  
  //Initialise serial monitor
  Serial.begin(9600);
  
  interrupts();//switch interrupts back on
}

void loop()
{
  
}

//Interrupt function - checks state of each button and lights corresponding LED
ISR(PCINT2_vect)
{
  if(digitalRead(BTN0_PIN))
  {
    PORTB ^= (1 << LEDG_PIN);
    Serial.println("LED GREEN LIT");
  }
  else if(digitalRead(BTN1_PIN))
  {
    PORTB ^= (1 << LEDY_PIN);
    Serial.println("LED YELLOW LIT");
  }
  if(digitalRead(BTN2_PIN))
  {
    PORTB ^= (1 << LEDR_PIN);
    Serial.println("LED RED LIT");
  }
}

//flash LED timer pin
ISR(TIMER1_COMPA_vect)
{
  PORTB ^= (1 << LEDW_PIN);
  Serial.println("TIMER ACTIVE ON LED WHITE");
}
