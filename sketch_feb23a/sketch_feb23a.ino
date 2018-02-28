// encoder drive stepper motor
//


#define encoder_a 2 //keep this on and interrupt pin
#define encoder_b 3 //keep this on and interrupt pin
#define motor_step 4 //can be any pin
#define motor_direction 5 //can be any pin

//#include <delay.h>
volatile long motor_position, encoder;

void setup () {
  //set up the various outputs
  pinMode(motor_step, OUTPUT);
  pinMode(motor_direction, OUTPUT);
  
  // then the encoder inputs
  pinMode(encoder_a, INPUT);
  pinMode(encoder_b, INPUT);
  // enable pullup as we are using an open collector encoder
  digitalWrite(encoder_a, HIGH); 
  digitalWrite(encoder_b, HIGH); 
  
  // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, encoderPinChangeA, CHANGE);
  // encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, encoderPinChangeB, CHANGE);
  encoder = 0; //reseet the encoder position to 0
}

void loop() {
  //do stuff dependent on encoder position here
  //such as move a stepper motor to match encoder position
  //if you want to make it 1:1 ensure the encoder res matches the motor res by dividing/multiplying
  if (encoder > 0) {
    digitalWrite(motor_direction, HIGH);// move stepper in reverse
    digitalWrite(motor_step, HIGH);
    digitalWrite(motor_step, LOW);
    delayMicroseconds(600); //_delay_us(200); //modify to alter speed
    motor_position++;
    encoder = 0; //encoder--;
  }
  else if (encoder < 0) {
    digitalWrite (motor_direction, LOW); //move stepper forward
    digitalWrite (motor_step, HIGH);
    digitalWrite (motor_step, LOW);
    delayMicroseconds(600); //_delay_us(200); //modify to alter speed
    motor_position--;
    encoder = 0; //encoder++;
  }
}

void encoderPinChangeA() {
  if (digitalRead(encoder_a)==digitalRead(encoder_b)) {
    encoder--;
  }
  else{
      encoder++;
  }
}

void encoderPinChangeB() {
  if (digitalRead(encoder_a) != digitalRead(encoder_b)) {
    encoder--;
  }
  else {
    encoder++;
  }
}

