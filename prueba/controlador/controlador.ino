#include <DueTimer.h>

#define ENABLE 2
#define ENCODER_A 3 //OUTPUT (amarillo)
#define ENCODER_B 7 //OUTPUT (blanco)
#define PWM_1 8 // 8 unido a 4 canal 5
#define PWM_2 6 // 6 unido a 5 canal 7
#define PWM_DUTY_CYCLE 2100 
#define REVOLUCIONES 3591.84

#define FREQUENCY 20000 //funciona como un filtro paso bajo, por tanto, hay que subir la frecuencia (1k) porque es demasiado baja
#define time 600 // 600 ms de subida y 600 ms de bajada. Llega al régimen permanente
#define clock_A 42000000


#define Nexp 7
#define voltage 8
#define Kp 15.0673
//Kp1=0.2009 Kp07=0.4100 Kp03=2.2322 Kp01=20.0897 //Con reductora Kp01=1506.7  Kp03=167.4145 Kp07=30.7496 Kp1=15.0673
#define Referencia (1)*PI

int channel1 = 0;
int channel2 = 0;

volatile int pulses = 0;
int n_pulses = 0;
int previous_state = 0;
volatile int iteraciones = 0; 
volatile int experimentos = 0; 
volatile double valores[1201][Nexp+1];

//int voltage = 5; // voltaje del experimento

void setup() {
  Serial.begin(115200);
 
  pinMode(ENABLE, OUTPUT);
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  pinMode(PWM_1, OUTPUT);
  pinMode(PWM_2, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), getChange, CHANGE);//pin, funcion, LOW/HIGH/CHANGE/RISING/FALLING 
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), getChange, CHANGE);
  
  digitalWrite(ENABLE, HIGH);
  PWM_Configuration();
  Timer1.attachInterrupt(moving).setPeriod(1000).start();
}

//conectar puentes del PWM con diferentes canales 

void PWM_Configuration (){
    pmc_enable_periph_clk(PWM_INTERFACE_ID);
    PWMC_ConfigureClocks(clock_A, clock_A, VARIANT_MCK); 
    
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

    channel1 = g_APinDescription[PWM_1].ulPWMChannel;
    channel2 = g_APinDescription[PWM_2].ulPWMChannel;
    // PWHM1
    PWMC_ConfigureChannel(PWM_INTERFACE, channel1, 1, 0, 0);  
    PWMC_SetPeriod(PWM_INTERFACE, channel1, PWM_DUTY_CYCLE); 
    PWMC_SetDutyCycle(PWM_INTERFACE, channel1, 0); // entre -2100 , 2100 f = CLK_usado / PWM_DUTY_CYCLE=42000000/2100=20kHz
    PWMC_EnableChannel(PWM_INTERFACE, channel1);
    // PWHM2
    PWMC_ConfigureChannel(PWM_INTERFACE, channel2, 1, 0, 0);
    PWMC_SetPeriod(PWM_INTERFACE, channel2, PWM_DUTY_CYCLE); 
    PWMC_SetDutyCycle(PWM_INTERFACE, channel2, 0); // de 0 a 2100
    PWMC_EnableChannel(PWM_INTERFACE, channel2);
     
}
// muestra: contador de muestras, contador de pulsos
void loop() {
  if (iteraciones >= 1200 && experimentos==Nexp-1){
    //Timer1.stop();
    print(); 
    //Serial.println("FIN");
    iteraciones = 0;
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

void setVoltage(double v){
  if (v > 0){
     PWMC_SetDutyCycle(PWM_INTERFACE, channel1, ((PWM_DUTY_CYCLE/12)*v)); 
     PWMC_SetDutyCycle(PWM_INTERFACE, channel2, 0); 
  }
  else if (v < 0){
     PWMC_SetDutyCycle(PWM_INTERFACE, channel2, ((PWM_DUTY_CYCLE/12)*-v)); 
     PWMC_SetDutyCycle(PWM_INTERFACE, channel1, 0); 
  }
  else {
    PWMC_SetDutyCycle(PWM_INTERFACE, channel1, 0); 
    PWMC_SetDutyCycle(PWM_INTERFACE, channel2, 0);     
  }
}

//interrupcion timer
void moving (){
  double error= Referencia - double(pulses)*2*PI/REVOLUCIONES;
  double realimentacion= error*Kp;
  if(realimentacion > 12) {
     realimentacion = 12;
     setVoltage(realimentacion);
  }else if (realimentacion < -12) {
     realimentacion = -12;
     setVoltage(realimentacion);
  }
  else
    setVoltage(realimentacion);
    
  if (iteraciones==1200 && experimentos==Nexp-1){
    valores[iteraciones][experimentos] = pulses;    
    Timer1.stop();
  }
  else if (iteraciones==1200){
    valores[iteraciones][experimentos] = pulses;
   //Serial.println("exp+1");
    experimentos ++;
    //Serial.println(experimentos);
    iteraciones=0;
    pulses=0;
  }
  else {
    if(iteraciones == 0 && pulses!=0)
      iteraciones--;
    else
      valores[iteraciones][experimentos] = pulses;
  }
    iteraciones++;
}

void print() {
  for(int i = 0; i < 1201 ; i++){//recorre iteraciones
    float media=0;
     for(int j=0; j<Nexp ; j++){//recorre exp
        media+=float(valores[i][j])/float(Nexp);
        /*
        Serial.print("exp ");
        Serial.print(j);
        Serial.print(" itr ");
        Serial.print(i);
        Serial.print(" ");
        Serial.println(valores[i][j]);
        */
     }
     
     Serial.print(i);
     Serial.print(" ");
     Serial.println(media,6);
     
  }
}
