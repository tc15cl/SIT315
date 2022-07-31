// C++ code
//

//set pin numbers
const byte LED_PIN = 13;
const byte METER_PIN = A4;

double timerFrequency = 0; //variable to hold frequency

void setup()
{
   
  //set pin types
  pinMode(LED_PIN, OUTPUT);
  pinMode(METER_PIN, INPUT);
      
  Serial.begin(9600); //serial output for debug
  noInterrupts(); //switch off interrupts for setup
  
  //clear timer registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  
  //16Mhz/(1024 prescaler*0.5Hz) - 2s timer initially
  OCR1A = 31250; 
  
  TCCR1B |= (1 << WGM12);   //CTC mode
  TCCR1B |= (1 << CS12)|(1 << CS10); //Set 1024 prescaler 
  TIMSK1 |= (1 << OCIE1A);  //Timer compare interrupt
  
  interrupts();//switch interrupts back on
  
}

void loop()
{ 
  double potVal = analogRead(METER_PIN); // reads ADC value between 0 and 1023 (0 and 5V)
  
  //debug info
  Serial.print("Freq: ");
  Serial.println(potVal);
              
  startTimer(potVal);//call start timer function
}



void startTimer(double timerFrequency)
{
  //set output register to new value based on pot position ratioed to clock speed
  OCR1A = 16000000 / (timerFrequency);
}

//output to LED pin
ISR(TIMER1_COMPA_vect){
   digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);
}
