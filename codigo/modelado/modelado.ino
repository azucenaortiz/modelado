#include <DueTimer.h>

#define ENABLE 2
#define ENCODER_A 3 //OUTPUT (amarillo)
#define ENCODER_B 7 //OUTPUT (blanco)
#define PWM_1 8 // 8 unido a 4 canal 5
#define PWM_2 6 // 6 unido a 5 canal 7
#define PWM_DUTY_CYCLE 2100 
#define REVOLUCIONES 3591.84

#define clock_A 42000000
#define Nexp 7
#define voltage 9

int channel1 = 0;//luego se le asignará un canal
int channel2 = 0;

volatile int pulses = 0;//contador de pulsos del encoder
int previous_state = 0;//variable para contar pulsos
volatile int iteraciones = 0;//contador de de muestras por experimento
volatile int experimentos = 0;//contador de experimentos realizados
volatile double valores[1201][Nexp+1];//array que guarda los valores


void setup() {
  Serial.begin(115200);
 
  //configuracion de pines
  pinMode(ENABLE, OUTPUT);
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  pinMode(PWM_1, OUTPUT);
  pinMode(PWM_2, OUTPUT);

  //Interrupciones de lectura de cambio de flancos en el encoder
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), getChange, CHANGE);//pin, funcion, LOW/HIGH/CHANGE/RISING/FALLING 
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), getChange, CHANGE);
  
  digitalWrite(ENABLE, HIGH);//activar enable 
  PWM_Configuration(); //Configuracion pwm
  Timer1.attachInterrupt(moving).setPeriod(1000).start();//interrupción cíclica que marca la velocidad
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

    channel1 = g_APinDescription[PWM_1].ulPWMChannel;//configuracion de canal pwm1
    channel2 = g_APinDescription[PWM_2].ulPWMChannel;//configuracion de canal pwm2
    // PWHM1
    PWMC_ConfigureChannel(PWM_INTERFACE, channel1, 1, 0, 0);  
    PWMC_SetPeriod(PWM_INTERFACE, channel1, PWM_DUTY_CYCLE); 
    PWMC_SetDutyCycle(PWM_INTERFACE, channel1, 0); // entre 0 , 2100 f = CLK_usado / PWM_DUTY_CYCLE=42000000/2100=20kHz
    PWMC_EnableChannel(PWM_INTERFACE, channel1);
    // PWHM2
    PWMC_ConfigureChannel(PWM_INTERFACE, channel2, 1, 0, 0);
    PWMC_SetPeriod(PWM_INTERFACE, channel2, PWM_DUTY_CYCLE); 
    PWMC_SetDutyCycle(PWM_INTERFACE, channel2, 0); // entre 0 , 2100 f = CLK_usado / PWM_DUTY_CYCLE=42000000/2100=20kHz
    PWMC_EnableChannel(PWM_INTERFACE, channel2);
     
}
// muestra: contador de muestras, contador de pulsos
void loop() {
  if (iteraciones >= 1200 && experimentos==Nexp-1){//ha terminado de medir todo 
    //Timer1.stop();
    print(); 
    //Serial.println("FIN");
    iteraciones = 0;
  }
}

//devuelve estado actual del encoder respecto a los dos pulsos
int getState(){
  int Enc_A = digitalRead(ENCODER_A);
  int Enc_B = digitalRead(ENCODER_B);

  if (Enc_A == LOW && Enc_B == LOW) return 1;
  else if (Enc_A == LOW && Enc_B == HIGH) return 2;
  else if (Enc_A == HIGH && Enc_B == HIGH) return 3;
  else if (Enc_A == HIGH && Enc_B == LOW) return 4;
}

//segun los cambios de estados, suma hacia un sentido y resta hacia el otro
void getChange(){
  int actual_state = getState();
  if( actual_state == 2 && previous_state == 1 || actual_state == 3 && previous_state == 2 || actual_state == 4 && previous_state == 3 || actual_state == 1 && previous_state == 4)
     pulses++;
  if( actual_state == 3 && previous_state == 4 || actual_state == 2 && previous_state == 3 || actual_state == 1 && previous_state == 2 || actual_state == 4 && previous_state == 1)
     pulses--;

  previous_state = actual_state;  
}

//Asigna un pwm proporcional a la tensión deseada, si es positivo por un canal y si es negativo por el otro
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
  if (iteraciones==1200 && experimentos==Nexp-1){//se han tomado todos los valores
    valores[iteraciones][experimentos] = pulses;
    setVoltage(0);    
    Timer1.stop();
  }
  else if (iteraciones==1200){//Se ha terminado un experimento y pasa al siguiente
    valores[iteraciones][experimentos] = pulses;
    experimentos ++;
    iteraciones=0;
    pulses=0;
  }
  else if (iteraciones <= 600) {//600 con tension
    setVoltage(voltage);
    if(iteraciones == 0 && pulses!=0)//quita la primera muestra del encoder en caso de ser distinta de 0
      iteraciones--;
    else
      valores[iteraciones][experimentos] = pulses;
  }
  else {//600 sin tension
    setVoltage(0);
    valores[iteraciones][experimentos] = pulses;
  }
    iteraciones++;
}

void print() {
  for(int i = 0; i < 1201 ; i++){//recorre iteraciones
    float media=0;
     for(int j=0; j<Nexp ; j++){//recorre exp
        media+=float(valores[i][j])/float(Nexp);
     }
     Serial.print(i);
     Serial.print(" ");
     Serial.println(media,6);     
  }
}
