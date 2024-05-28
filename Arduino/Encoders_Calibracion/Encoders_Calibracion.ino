#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <Wire.h>
#include "I2Cdev.h"     
#include "MPU6050.h" 
#include <Arduino.h>

#define WIFI_SSID "HUAWEI_p20"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyAw1xpf5PnmXKgMXKfqEH_2VAkNDpvv9ZU"
#define DATABASE_URL "https://diseno-138b6-default-rtdb.firebaseio.com/"

#define LED1_PIN 12
#define LED2_PIN 14
#define LDR_PIN 36



FirebaseData fbdo;
FirebaseAuth auth; 
FirebaseConfig config; 

MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

struct MyData {
  byte X;
  byte Y;
  byte Z;
};

MyData data;


// Dirección I2C del MPU6050

const float gravity = 9.81; // Aceleración debido a la gravedad en m/s^2
// Matriz de posiciones a seguir, con formato {x, y}
const int positions[4][2] = {{0, 0}, {0, 1}, {18, 1},{18,-1}};

//PWM1
const int motorPinPWM = 15;
//PWM2aaa
const int motorPinPWM2 = 2;
const int motorIN1 = 18;
const int motorIN2 = 19;

unsigned long sendDataPrevMillis = 0; 
bool signupOK = false; 
int ldrData = 0; 
int SensorPIR = 4;
String valor;
int pwmValue = 0;
boolean ledStatus = false;
int motor1 = 0; 
int motor2 = 0; 

float voltage = 0.0; 
WiFiServer TCPServer(8266); //Servidor del ESP32
WiFiClient TCPClient; //Cliente TCP (PC)

void setup() {

  pinMode(motorPinPWM, OUTPUT);
  pinMode(motorPinPWM2, OUTPUT);
  pinMode(motorIN1, OUTPUT);
  pinMode(motorIN2, OUTPUT);
  Serial.begin(115200); 
  pinMode(SensorPIR, INPUT);
  Wire.begin();
  mpu.initialize();
  //pinMode(LED_BUILTIN, OUTPUT);
  //Serial.println("MPU6050 inicializado");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  //Serial.print("Connecting to Wi-Fi");
  while(WiFi.status() != WL_CONNECTED){
  }
  config.api_key = API_KEY; 
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")){
    //Serial.println("signUp OK"); 
    signupOK = true; 
  }else{
    //Serial.printf("%s/n", config.signer.signupError.message.c_str());
  }
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  TCPServer.begin();

}

void byteReceived(byte byteReceived) {
    Serial.print("Byte received: ");
    Serial.println((int)byteReceived);
}

// Función para enviar comandos a los motores
void commandMotors(int motor1Speed, int motor2Speed) {
  Serial.print(motor1Speed);
  Serial.print(",");
  Serial.print(motor2Speed);
  Serial.println();
}

// Función para mover el robot hacia adelante
void moveForward(int speed, int speed2, int duration) {
  if (duration >= 8000) {
    // Ejecuta con incremento de speed2 por los primeros 8000 ms
    commandMotors(speed+2, speed2 );
    delay(8000);
    
    // Ejecuta con speed2 original por el resto del tiempo
    commandMotors(speed, speed2);
    delay(duration - 8000);
  } else {
    // Ejecuta normalmente si la duración es menor a 8000 ms
    commandMotors(speed, speed2);
    delay(duration);
  }
}

// Función para girar a la derecha
void turnRight(int speed, int duration) {
  commandMotors(speed, -speed);
  delay(duration);
}

// Función para girar a la izquierda
void turnLeft(int speed, int duration) {
  commandMotors(-speed, speed);
  delay(duration);
}

// Función para determinar y ejecutar el movimiento necesario entre dos puntos
void moveToNextPoint(int currentX, int currentY, int nextX, int nextY) {
  int deltaX = nextX - currentX;
  int deltaY = nextY - currentY;

  if (deltaX != 0) { // Cambio en x, implica un giro y avance
    if (deltaX > 0) {
      // Gira hacia la dirección deseada antes de avanzar
      turnRight(70, 1720);
    } else {
      turnLeft(70, 1720);
    }
    // Avanza el número absoluto de deltaX "unidades"
    moveForward(96,91, 2050 * abs(deltaX));
  }

  if (deltaY != 0) { // Cambio en y, solo avance
    moveForward(96,91, 2050 * abs(deltaY));
  }
}


void loop() {

    if (!TCPClient.connected()) {
        Serial.println("Trying to connect to a new client...");
        TCPClient = TCPServer.available();
        if (TCPClient) {
            Serial.println("Client connected!");
        }
    } else {
        if (TCPClient.available() > 0) {
            byteReceived(TCPClient.read());
        } else {
            Serial.println("No data available from client.");
        }
    }

  if(Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    data.X = map(ax, -17000, 17000, 0, 255 );
    data.Y = map(ay, -17000, 17000, 0, 255); 
    data.Z = map(az, -17000, 17000, 0, 255); 
    float accelX = (float)ax / 16384.0 * gravity;
    float accelY = (float)ay / 16384.0 * gravity;
    float accelZ = (float)az / 16384.0 * gravity;
    int inclinacion = (data.Y > 180) ? 1 : 0;


    ldrData = digitalRead(SensorPIR); 
    voltage = (float)analogReadMilliVolts(LDR_PIN)/1000;
    if (ldrData == HIGH) {
      valor = "Movimiento detectado!";
      analogWrite(motorPinPWM,255);
    } else {
      valor = "Sin movimiento.";
      analogWrite(motorPinPWM,0);
    }
    if (inclinacion == 1) {
      analogWrite(motorPinPWM2,255);
      // Motor gira en una dirección
      digitalWrite(motorIN1, HIGH);
      digitalWrite(motorIN2, LOW);
      delay(5000);
    } else {  
      analogWrite(motorPinPWM2,255);
      // Motor gira en la dirección opuesta
      digitalWrite(motorIN1, LOW);
      digitalWrite(motorIN2, HIGH);
    }
    if(Firebase.RTDB.setString(&fbdo, "PIR", valor)){
    } else{
    }
    if(Firebase.RTDB.setInt(&fbdo, "MPU/Aceleración en x: ", accelX)){
    } else{
    }
    if(Firebase.RTDB.setInt(&fbdo, "MPU/Aceleración en y: ", accelY)){
    } else{
    }
    if(Firebase.RTDB.setInt(&fbdo, "MPU/Aceleración en z", accelZ)){
    } else{
    }
    if(Firebase.RTDB.setInt(&fbdo, "MPU/Inclinacion: ", inclinacion)){
    } else{
    }
  }
  if(Firebase.RTDB.getInt(&fbdo, "Motors/motor1")){
    if(fbdo.dataType() == "int"){
      motor1 = fbdo.intData();
    }
  }else{
  }
  if(Firebase.RTDB.getInt(&fbdo, "Motors/motor2")){
    if(fbdo.dataType() == "int"){
      motor2 = fbdo.intData();
    }
  }else{
  }
}