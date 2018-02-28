#include <DueTimer.h>

#define PWM_FREQ 20000
#define encoderPinA  3
#define encoderPinB  7
#define PWM_enableA  2
#define dutyCycle_1a 0
#define dutyCycle_2a 28 //Máximo 255
//#define PWM_in1a     6  //Puentearlo con el 5
#define PWM_in1a     5 
#define PWM_in2a     8  //Puentearlo con el 4
#define Tmuestreo_ms  1
#define t_pulso_alto_ms  600
#define t_pulso_alto_enMuestras   t_pulso_alto_ms/Tmuestreo_ms
//Hemos quitado la resolución, suponiendo que write tiene la misma que el PWM por defecto (8 bits = 0-255)
#define pulsos_por_experimento  24

volatile int PWM_enable = 0;
volatile static uint8_t pinEnabled[PINS_COUNT];
volatile boolean alto = true;
volatile boolean terminado = false;
volatile int contador_muestras = 0;
volatile int contador_pulsos = 0;
volatile long encoder_position = 0; 
// Volatile > porque la variable puede ser modificada en una interrupción

void setup() {

  Serial.begin (115200); 
  Serial.print(0);
  Serial.print(" ");
  Serial.println(0);
  
  pinMode (PWM_in1a, OUTPUT);
  analogWrite (PWM_in1a, 0);
  
//INICIAMOS LA PWM POR HARDWARE
  void PWM_iniciar_HW();

//CONFIGURACIÓN PARA LA INTERRUPCIÓN DEL PERÍODO DE MUESTREO
  Timer.getAvailable().attachInterrupt(handler).start(1000);

//INTERRUPCIONES DEL ENCODER: en los flancos de A y  en los flancos de B
  attachInterrupt(encoderPinA, countA, CHANGE); 
  attachInterrupt(encoderPinB, countB, CHANGE);  

}

void loop() {
  if(terminado){ 
    PWMwrite (PWM_in2a, 0,PWM_FREQ);
    return;
    }
}


/*FUNCIÓN PARA INICIAR LA PWM A LA FRECUENCIA DESEADA*/
void PWM_iniciar_HW(){
   //analogOutputInit() incializa a 0 el array pinEnabled[PINS_COUNT] que contiene información sobre si todos los pines del arduino están enabled (1) o no (0)
  analogOutputInit();
  //Fijarse en que el PIN sea de los de PWM y no de los timers
  pinMode (PWM_in2a, OUTPUT);
  pinMode (PWM_enableA, OUTPUT);
   //Analog write con la frecuencia que hemos establecido PWMC_ConfigureClocks
  PWMwrite (PWM_in2a, dutyCycle_2a,PWM_FREQ);
  digitalWrite (PWM_enableA, HIGH);
}

/*FUNCIÓN PARA GENERAR LA PWM CON LA FRECUENCIA ESTABLECIDA*/
void PWMwrite(uint32_t ulPin, uint32_t dCycle, int pwm_freq) {    
    if (!PWM_enable) {
      // PWM Startup code
        pmc_enable_periph_clk(PWM_INTERFACE_ID);
    //VARIANT_MCK: C:\Users\Teresina\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.6\variants\arduino_due_x\variant.h
        PWMC_ConfigureClocks(pwm_freq * PWM_MAX_DUTY_CYCLE, 0, VARIANT_MCK);
       PWM_enable = 1;
    }
    uint32_t chan = g_APinDescription[ulPin].ulPWMChannel;
    if (!pinEnabled[ulPin]) {
      // Setup PWM for this pin
      PIO_Configure(g_APinDescription[ulPin].pPort,
          g_APinDescription[ulPin].ulPinType,
          g_APinDescription[ulPin].ulPin,
          g_APinDescription[ulPin].ulPinConfiguration);
      PWMC_ConfigureChannel(PWM_INTERFACE, chan, PWM_CMR_CPRE_CLKA, 0, 0);
      PWMC_SetPeriod(PWM_INTERFACE, chan, PWM_MAX_DUTY_CYCLE);
      PWMC_SetDutyCycle(PWM_INTERFACE, chan, dCycle);
      PWMC_EnableChannel(PWM_INTERFACE, chan);
      pinEnabled[ulPin] = 1;
    }

    PWMC_SetDutyCycle(PWM_INTERFACE, chan, dCycle);
    return;
}

/*INTERRUPCIONES ASOCIADAS A LOS ENCODERS*/
//Tras un flanco de reloj en A: si las dos señales son iguales (HIGH o LOW), sentido negativo
void countA() {
  if (digitalRead(encoderPinA) == digitalRead(encoderPinB)) encoder_position--; else encoder_position++;
}
//Tras un flanco de reloj en B: si las dos señales son iguales (HIGH o LOW), sentido positivo
void countB() {
  if (digitalRead(encoderPinA) == digitalRead(encoderPinB)) encoder_position++; else encoder_position--;
}


/*MANEJADOR DE LA INTERRUPCIÓN*/
void handler(){

  contador_muestras++;
  //Con alto las subidas, con !alto las bajadas
    Serial.print(contador_muestras);
    Serial.print(" ");
    Serial.println(encoder_position);
  if(contador_muestras==t_pulso_alto_enMuestras){
    alto = !alto;
    contador_pulsos++;
    Serial.print("NUMERO DE PULSO:");
    Serial.println(contador_pulsos);
    contador_muestras = 0; 
    if(!alto){
      PWMwrite (PWM_in2a, 0,PWM_FREQ);
    }else{
      PWMwrite (PWM_in2a, dutyCycle_2a,PWM_FREQ);
      encoder_position = 0; 
    }
  }

  if(contador_pulsos == pulsos_por_experimento){
    terminado = true;
    exit(0);
  }
  }


