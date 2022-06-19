const int Pin=2;
unsigned long newData = 0;
unsigned long oldData = 0;
float frequency = 0;
const int Interrupt_Pin = digitalPinToInterrupt(Pin);

void PwmRegist(){        //interrupt service routine
    oldData = newData;
    newData = micros();
  }

void setup(){
  Serial.begin(9600);
  pinMode(Pin,INPUT);
  attachInterrupt(Interrupt_Pin, PwmRegist, FALLING);
}

void loop(){

     detachInterrupt(0);                 //stop measurements and calculate frequency first
     frequency = 1000000 / (newData-oldData);
     Serial.println(frequency);
     attachInterrupt(Interrupt_Pin, PwmRegist, FALLING);                        //start measuring again
}

     
