#define ENABLE 2
#define INT_A 4
#define INT_B 5
#define ENCODER_A 3 //OUTPUT (amarillo)
#define ENCODER_B 7 //OUTPUT (blanco)
#define REVOLUCIONES 3591.84

#define FRECUENCY 20000 //funciona como un filtro paso bajo, por tanto, hay que subir la frecuencia (1k) porque es demasiado baja
int pulses = 0;
int n_pulses = 0;
int previous_state = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ENABLE, OUTPUT);
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), getChange, CHANGE);//pin, funcion, LOW/HIGH/CHANGE/RISING/FALLING 
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), getChange, CHANGE);
}

void loop() {

  digitalWrite(ENABLE, HIGH);
  if (n_pulses != pulses)
   {
      n_pulses = pulses;
      Serial.println(2*PI*pulses/REVOLUCIONES);
   }
  
}

int getState(){
  int Enc_A = digitalRead(ENCODER_A);
  int Enc_B = digitalRead(ENCODER_B);

  if (Enc_A == LOW && Enc_B == LOW) return 1;
  else if (Enc_A == LOW && Enc_B == HIGH) return 2;
  else if (Enc_A == HIGH && Enc_B == HIGH) return 3;
  else if (Enc_A == HIGH && Enc_B == LOW) return 4;

}

void getChange(){
  int actual_state = getState();
  if( actual_state == 2 && previous_state == 1 || actual_state == 3 && previous_state == 2 || actual_state == 4 && previous_state == 3 || actual_state == 1 && previous_state == 4)
     pulses++;
  if( actual_state == 3 && previous_state == 4 || actual_state == 2 && previous_state == 3 || actual_state == 1 && previous_state == 2 || actual_state == 4 && previous_state == 1)
     pulses--;

  previous_state = actual_state;  
}


