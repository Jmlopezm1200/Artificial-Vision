//------------------------------Pines Dirreccion Motores----------------------
#define PIN_MOTOR_R_FWD 19
#define PIN_MOTOR_R_BWD 18
#define PIN_MOTOR_L_FWD 5
#define PIN_MOTOR_L_BWD 17
//------------------------------Pines Velocidades Motores----------------------
#define PIN_ENABLE_R 27
#define PIN_ENABLE_L 14
//------------------------------Pines Encoders----------------------
#define Encoder_Der 15
#define Encoder_Izq 4

//------------------------------Variables Generales----------------------
#define PI 3.14159265359
unsigned long previousMillis = 0;
long interval = 100;
float diametro_rueda = 65; 
float ranuras_encoder = 20;

//------------------------------Variables Por Motor----------------------
volatile int Cont_Der = 0;
volatile int Cont_Izq = 0;
volatile int RPM_Der = 0;
volatile int RPM_Izq = 0;
float VEL_Der = 0;
float VEL_Izq = 0;

//------------------------------Variables PID---------------------


void IRAM_ATTR IntEncDer(){
  Cont_Der++;

}

void IRAM_ATTR IntEncIzq(){
  Cont_Izq++;

}

enum Comandos { CMD_FORWARD = 'w', CMD_BACKWARD ='s', CMD_RIGHT ='d', CMD_LEFT = 'a', CMD_STOP = 'q' }; // Enumeración de comandos recibidos



void byteReceived() {
  
    
    char byteRecibido = (char)Serial.read();

    switch(byteRecibido){

      case CMD_FORWARD:
       Serial.println("Forward");
       digitalWrite(PIN_MOTOR_R_FWD, HIGH);
       digitalWrite(PIN_MOTOR_R_BWD, LOW);
       digitalWrite(PIN_MOTOR_L_FWD, LOW);
       digitalWrite(PIN_MOTOR_L_BWD, HIGH);

      break;

      case CMD_BACKWARD:

      Serial.println("Backward");
       digitalWrite(PIN_MOTOR_R_FWD, LOW);
       digitalWrite(PIN_MOTOR_R_BWD, HIGH);
       digitalWrite(PIN_MOTOR_L_FWD, HIGH);
       digitalWrite(PIN_MOTOR_L_BWD, LOW);
      
      break;

      case CMD_RIGHT:

       Serial.println("Right");
       digitalWrite(PIN_MOTOR_R_FWD, LOW);
       digitalWrite(PIN_MOTOR_R_BWD, HIGH);
       digitalWrite(PIN_MOTOR_L_FWD, LOW);
       digitalWrite(PIN_MOTOR_L_BWD, HIGH);

      break;


      case CMD_LEFT:
       Serial.println("Left");
       digitalWrite(PIN_MOTOR_R_FWD, HIGH);
       digitalWrite(PIN_MOTOR_R_BWD, LOW);
       digitalWrite(PIN_MOTOR_L_FWD, HIGH);
       digitalWrite(PIN_MOTOR_L_BWD, LOW);


      break;
      case CMD_STOP:

        Serial.println("Stop ");
      
        digitalWrite(PIN_MOTOR_R_FWD, LOW);
        digitalWrite(PIN_MOTOR_R_BWD, LOW);
        digitalWrite(PIN_MOTOR_L_FWD, LOW);
        digitalWrite(PIN_MOTOR_L_BWD, LOW);

      break;

      default: break;
            
    }
  
}


void setup() {
  pinMode(Encoder_Der,INPUT);
  pinMode(Encoder_Izq,INPUT);

  pinMode(PIN_MOTOR_R_FWD, OUTPUT);
  pinMode(PIN_MOTOR_R_BWD, OUTPUT);
  pinMode(PIN_MOTOR_L_FWD, OUTPUT);
  pinMode(PIN_MOTOR_L_BWD, OUTPUT);

  pinMode(PIN_ENABLE_R, OUTPUT);
  pinMode(PIN_ENABLE_L, OUTPUT);

  analogWrite(PIN_ENABLE_R, 255);
  analogWrite(PIN_ENABLE_L, 255);
  
  digitalWrite(PIN_MOTOR_R_FWD, LOW);
  digitalWrite(PIN_MOTOR_R_BWD, LOW);
  digitalWrite(PIN_MOTOR_L_FWD, LOW);
  digitalWrite(PIN_MOTOR_L_BWD, LOW);
  
  Serial.begin(115200);

  attachInterrupt(digitalPinToInterrupt(Encoder_Der), IntEncDer, RISING);
  attachInterrupt(digitalPinToInterrupt(Encoder_Izq), IntEncIzq, RISING);
}

void loop() {

  if((millis() - previousMillis) >= interval)
  {
    RPM_Der = (60 * 1000 / ranuras_encoder) / (millis() - previousMillis) * Cont_Der;
    RPM_Izq = (60 * 1000 / ranuras_encoder) / (millis() - previousMillis) * Cont_Izq;

    VEL_Der = RPM_Der * PI * diametro_rueda * 60 / 1000000;
    VEL_Izq = RPM_Izq * PI * diametro_rueda * 60 / 1000000;

    previousMillis = millis();
    Cont_Der = 0;
    Cont_Izq = 0;
  }

  if (Serial.available() > 0){
    byteReceived();
  }

  Serial.print("RPM Motor Derecho: ");
  Serial.println(VEL_Der);
  Serial.print("RPM Motor Izquierdo: ");
  Serial.println(VEL_Izq);
}

