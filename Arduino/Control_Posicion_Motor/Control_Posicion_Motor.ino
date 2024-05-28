// Motor Izquierdo
// #define ENCA 4 
// #define PWM 14
// #define IN1 17
// #define IN2 5

// Motor Derecho 
#define ENCA 15
#define PWM 27
#define IN1 19
#define IN2 18

volatile int posi = 0; 
long prevT = 0;
float eprev = 0;
float eintegral = 0;

void setMotor(int dir, int pwmVal, int pwm, int in1, int in2){
  analogWrite(pwm,pwmVal);
  if(dir == 1){
    digitalWrite(in1,HIGH);
    digitalWrite(in2,LOW);
  }
  else if(dir == -1){
    digitalWrite(in1,LOW);
    digitalWrite(in2,HIGH);
  }
  else{
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);
  }  
}

void IRAM_ATTR readEncoder(){
  int a=digitalRead(IN1);
  if(a == 0){
    posi++;
  }
  else if(a == 1){
    posi--;
  }
}

void setup() {
  pinMode(ENCA,INPUT);
  pinMode(PWM,OUTPUT);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);

  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(ENCA),readEncoder,RISING);
  

  Serial.println("target pos");
}

void loop() {

  // set target position
  int target = 250;
  //int target = 250*sin(prevT/1e6);

  // PID constants izquierda
  // float kp = 2.7;
  // float kd = 0.7;
  // float ki = 0.07;

  // PID constants Derecha
  float kp = 2.85;
  float kd = 0.4;
  float ki = 0.07;


  // time difference
  long currT = micros();
  float deltaT = ((float) (currT - prevT))/( 1.0e6 );
  prevT = currT;

  // Read the position
  int pos = 0; 
  noInterrupts(); // disable interrupts temporarily while reading
  pos = posi;
  interrupts(); // turn interrupts back on
  
  // error
  int e = pos - target;

  // derivative
  float dedt = (e-eprev)/(deltaT);

  // integral
  eintegral = eintegral + e*deltaT;

  // control signal
  float u = kp*e + kd*dedt + ki*eintegral;

  // motor power
  float pwr = fabs(u);
  if( pwr > 255 ){
    pwr = 255;
  }

  // motor direction
  int dir = 1;
  if(u<0){
    dir = -1;
  }

  // signal the motor
  setMotor(dir,pwr,PWM,IN1,IN2);

  // store previous error
  eprev = e;

  Serial.print(target);
  Serial.print(" ");
  Serial.print(pos);
  Serial.println();
}



