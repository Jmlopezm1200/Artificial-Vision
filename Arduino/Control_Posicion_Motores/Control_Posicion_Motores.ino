// A class to compute the control signal
class SimplePID{
  private:
    float kp, kd, ki, umax; // Parameters
    float eprev, eintegral; // Storage

  public:
  // Constructor
  SimplePID() : kp(0), kd(0), ki(0), umax(255), eprev(0.0), eintegral(0.0){}

  // A function to set the parameters
  void setParams(float kpIn, float kdIn, float kiIn, float umaxIn){
    kp = kpIn; kd = kdIn; ki = kiIn; umax = umaxIn;
  }

  // A function to compute the control signal
  void evalu(int value, int target, float deltaT, int &pwr, int &dir){
    // error
    int e = target - value;
  
    // derivative
    float dedt = (e-eprev)/(deltaT);
  
    // integral
    eintegral = eintegral + e*deltaT;
  
    // control signal
    float u = kp*e + kd*dedt + ki*eintegral;
  
    // motor power
    pwr = (int) fabs(u);
    if( pwr > umax ){
      pwr = umax;
    }
  
    // motor direction
    dir = 1;
    if(u<0){
      dir = -1;
    }
  
    // store previous error
    eprev = e;
  }
  
};

// How many motors
#define NMOTORS 2
// Pins  Derecha - Izquierda
const int enca[] = {15,4};
const int pwm[] = {27,14};
const int in1[] = {19,5};
const int in2[] = {18,17};

// Globals
long prevT = 0;
volatile int posi[] = {0,0};

// PID class instances
SimplePID pid[NMOTORS];

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


void IRAM_ATTR readEncoderDer(){
  int a=digitalRead(in1[0]);
  if(a == 1){
    posi[0]++;
  }
  else if(a == 0){
    posi[0]--;
  }
}

void IRAM_ATTR readEncoderIzq(){
  int b=digitalRead(in1[1]);
  if(b == 1){
    posi[1]++;
  }
  else if(b == 0){
    posi[1]--;
  }
}

void setup() {
  Serial.begin(9600);

  for(int k = 0; k < NMOTORS; k++){
    pinMode(enca[k],INPUT);
    pinMode(pwm[k],OUTPUT);
    pinMode(in1[k],OUTPUT);
    pinMode(in2[k],OUTPUT);
  }

  pid[0].setParams(2.85,0.4,0.07,255);
  pid[1].setParams(3.6,1.5,0.101,255);

  attachInterrupt(digitalPinToInterrupt(enca[0]),readEncoderDer,RISING);
  attachInterrupt(digitalPinToInterrupt(enca[1]),readEncoderIzq,RISING);
  
  Serial.println("target pos");
}

void loop() {

  // time difference
  long currT = micros();
  float deltaT = ((float) (currT - prevT))/( 1.0e6 );
  prevT = currT;

  // set target position
  int target[NMOTORS];
  target[0] = 250;
  target[1] = -250;

  // Read the position
  int pos[NMOTORS];
  noInterrupts(); // disable interrupts temporarily while reading
  for(int k = 0; k < NMOTORS; k++){
      pos[k] = posi[k];
    }
  interrupts(); // turn interrupts back on
  
  // loop through the motors
  for(int k = 0; k < NMOTORS; k++){
    int pwr, dir;
    // evaluate the control signal
    pid[k].evalu(pos[k],target[k],deltaT,pwr,dir);
    // signal the motor
    setMotor(dir,pwr,pwm[k],in1[k],in2[k]);
  }

  for(int k = 0; k < NMOTORS; k++){
    Serial.print(target[k]);
    Serial.print(" ");
    Serial.print(pos[k]);
    Serial.print(" ");
  }
  Serial.println();
}

