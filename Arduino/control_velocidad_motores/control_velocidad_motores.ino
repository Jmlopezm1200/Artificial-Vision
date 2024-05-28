// Pins Derecha
#define ENCA1 15
#define PWM1 27
#define IN1_1 19
#define IN2_1 18

// Pins Izquierda
#define ENCA2 4
#define PWM2 14
#define IN1_2 5
#define IN2_2 17

// Constants for Motor 1
const float kp1 = 8;
const float ki1 = 16;

// Constants for Motor 2
const float kp2 = 10;
const float ki2 = 20;

// globals for Motor 1
long prevT1 = 0;
int posPrev1 = 0;
volatile int pos_i1 = 0;
volatile float velocity_i1 = 0;
volatile long prevT_i1 = 0;

// globals for Motor 2
long prevT2 = 0;
int posPrev2 = 0;
volatile int pos_i2 = 0;
volatile float velocity_i2 = 0;
volatile long prevT_i2 = 0;

float v1Filt = 0;
float v1Prev = 0;
float v2Filt = 0;
float v2Prev = 0;

float eintegral1 = 0;
float eintegral2 = 0;

void setMotor(int dir, int pwmVal, int pwm, int in1, int in2){
  analogWrite(pwm,pwmVal); // Motor speed
  if(dir == 1){ 
    // Turn one way
    digitalWrite(in1,HIGH);
    digitalWrite(in2,LOW);
  }
  else if(dir == -1){
    // Turn the other way
    digitalWrite(in1,LOW);
    digitalWrite(in2,HIGH);
  }
  else{
    // Or dont turn
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);    
  }
}

void IRAM_ATTR readEncoder1(){
  int b = digitalRead(IN1_1);
  int increment = 0;
  if(b == 1){
    // If B is high, increment forward
    increment = 1;
  }
  else if(b == 0){
    // Otherwise, increment backward
    increment = -1;
  }
  pos_i1 = pos_i1 + increment;
}

void IRAM_ATTR readEncoder2(){
  int b = digitalRead(IN1_2);
  int increment = 0;
  if(b == 1){
    // If B is high, increment forward
    increment = 1;
  }
  else if(b == 0){
    // Otherwise, increment backward
    increment = -1;
  }
  pos_i2 = pos_i2 + increment;
}

void setup() {
  Serial.begin(9600);

  pinMode(ENCA1,INPUT);
  pinMode(PWM1,OUTPUT);
  pinMode(IN1_1,OUTPUT);
  pinMode(IN2_1,OUTPUT);

  pinMode(ENCA2,INPUT);
  pinMode(PWM2,OUTPUT);
  pinMode(IN1_2,OUTPUT);
  pinMode(IN2_2,OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ENCA1),
                  readEncoder1,RISING);
  attachInterrupt(digitalPinToInterrupt(ENCA2),
                  readEncoder2,RISING);
}

void loop() {

  // read the position and velocity for motor 1
  int pos1 = 0;
  float velocity2_1 = 0;
  noInterrupts(); // disable interrupts temporarily while reading
  pos1 = pos_i1;
  velocity2_1 = velocity_i1;
  interrupts(); // turn interrupts back on

  // read the position and velocity for motor 2
  int pos2 = 0;
  float velocity2_2 = 0;
  noInterrupts(); // disable interrupts temporarily while reading
  pos2 = pos_i2;
  velocity2_2 = velocity_i2;
  interrupts(); // turn interrupts back on

  // Compute velocity with method 1 for motor 1
  long currT1 = micros();
  float deltaT1 = ((float) (currT1-prevT1))/1.0e6;
  float velocity1_1 = (pos1 - posPrev1)/deltaT1;
  posPrev1 = pos1;
  prevT1 = currT1;

  // Compute velocity with method 1 for motor 2
  long currT2 = micros();
  float deltaT2 = ((float) (currT2-prevT2))/1.0e6;
  float velocity1_2 = (pos2 - posPrev2)/deltaT2;
  posPrev2 = pos2;
  prevT2 = currT2;

  // Convert count/s to RPM for motor 1
  float v1_1 = velocity1_1/960.0*60.0;

  // Convert count/s to RPM for motor 2
  float v1_2 = velocity1_2/960.0*60.0;

  // Low-pass filter (25 Hz cutoff) for motor 1
  v1Filt = 0.854*v1Filt + 0.0728*v1_1 + 0.0728*v1Prev;
  v1Prev = v1_1;

  // Low-pass filter (25 Hz cutoff) for motor 2
  v2Filt = 0.854*v2Filt + 0.0728*v1_2 + 0.0728*v2Prev;
  v2Prev = v1_2;

  // Set a target for motor 1
  float vt1 = 7;

  // Set a target for motor 2
  float vt2 = -7;

  // Compute the control signal u for motor 1
  float e1 = vt1-v1Filt;
  eintegral1 = eintegral1 + e1*deltaT1;
  float u1 = kp1*e1 + ki1*eintegral1;

  // Compute the control signal u for motor 2
  float e2 = vt2-v2Filt;
  eintegral2 = eintegral2 + e2*deltaT2;
  float u2 = kp2*e2 + ki2*eintegral2;

  // Set the motor speed and direction for motor 1
  int dir1 = 1;
  if (u1<0){
    dir1 = -1;
  }
  int pwr1 = (int) fabs(u1);
  if(pwr1 > 255){
    pwr1 = 255;
  }
  setMotor(dir1,pwr1,PWM1,IN1_1,IN2_1);

  // Set the motor speed and direction for motor 2
  int dir2 = 1;
  if (u2<0){
    dir2 = -1;
  }
  int pwr2 = (int) fabs(u2);
  if(pwr2 > 255){
    pwr2 = 255;
  }
  setMotor(dir2,pwr2,PWM2,IN1_2,IN2_2);

  Serial.print(vt1);
  Serial.print(" ");
  Serial.print(v1Filt);
  Serial.print(" | ");
  Serial.print(vt2);
  Serial.print(" ");
  Serial.print(v2Filt);
  Serial.println();
  delay(1);
}

