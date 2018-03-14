 #include <DueTimer.h>

#define ENABLE 2
#define INT_A 4
#define INT_B 5
#define ENCODER_A 3 //OUTPUT (amarillo)
#define ENCODER_B 7 //OUTPUT (blanco)
#define PWM_1 34 // 6unido a 5
#define PWM_2 35 // 8unido a 4
#define PWM_DUTY_CYCLE 2100 
#define REVOLUCIONES 3591.84

#define FREQUENCY 20000 //funciona como un filtro paso bajo, por tanto, hay que subir la frecuencia (1k) porque es demasiado baja
#define time 600 // 600 ms de subida y 600 ms de bajada. Llega al régimen permanente

double dutyCycle = 0;
double dutyAct = 0;
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
  
  digitalWrite(ENABLE, HIGH);
  pinMode(PWM_1, OUTPUT);
  analogWrite(PWM_1, 0);
  
  PWM_Configuration();
}

//conectar puentes del PWM con diferentes canales 

void PWM_Configuration (){
    pmc_enable_periph_clk(PWM_INTERFACE_ID);
    PWMC_ConfigureClocks(42000000, 0, VARIANT_MCK); 
    
    PIO_Configure(
          g_APinDescription[PWM_1].pPort,
          g_APinDescription[PWM_1].ulPinType,
          g_APinDescription[PWM_1].ulPin,
          g_APinDescription[PWM_1].ulPinConfiguration);
          
    PIO_Configure(
          g_APinDescription[PWM_2].pPort,
          g_APinDescription[PWM_2].ulPinType,
          g_APinDescription[PWM_2].ulPin,
          g_APinDescription[PWM_2].ulPinConfiguration);   
          
    // PWHM1
    PWMC_ConfigureChannel(PWM_INTERFACE, 0 , 1, 0, 0);  
    PWMC_SetPeriod(PWM_INTERFACE, 0, PWM_DUTY_CYCLE); 
    PWMC_EnableChannel(PWM_INTERFACE, 0);
    PWMC_SetDutyCycle(PWM_INTERFACE, 0, 0); // entre -2100 , 2100 f= CLK_usado / PWM_DUTY_CYCLE=42000000/2100=20kHz

    // PWHM2
    PWMC_ConfigureChannel(PWM_INTERFACE, 1, 1, 0, 0);
    PWMC_SetPeriod(PWM_INTERFACE, 1, PWM_DUTY_CYCLE); 
    PWMC_EnableChannel(PWM_INTERFACE, 1);
    PWMC_SetDutyCycle(PWM_INTERFACE, 1, 0); // entre -2100 , 2100
     /*           
      PWMC_ConfigureChannel(PWM_INTERFACE, 0, PWM_CMR_CPRE_CLKA, 0, 0);
      PWMC_SetPeriod(PWM_INTERFACE, 0, PWM_MAX_DUTY_CYCLE);
      PWMC_SetDutyCycle(PWM_INTERFACE, 0, 100);
      PWMC_EnableChannel(PWM_INTERFACE, 0);
     */
      PWMC_SetDutyCycle(PWM_INTERFACE, 0, 200); // entre -2100 , 2100

     //Timer1.attachInterrupt(modeladoMotor).setPeriod(600).start();

}

void loop() {


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


