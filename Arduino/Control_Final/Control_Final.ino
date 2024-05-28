#include <WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Definiciones de pines usados para motores
#define PIN_MOTOR_R_FWD 19
#define PIN_MOTOR_R_BWD 18
#define PIN_MOTOR_L_FWD 5
#define PIN_MOTOR_L_BWD 17
// Definiciones de pines usados para Enable de motores (Velocidad PWM)
#define PIN_ENABLE_R 27
#define PIN_ENABLE_L 14
// Definiciones de pines usados para encoders
#define PIN_ENCODER_R 33
#define PIN_ENCODER_L 32
Adafruit_MPU6050 mpu;

enum Comandos { CMD_FORWARD = 'w', CMD_BACKWARD = 's', CMD_RIGHT = 'd', CMD_LEFT = 'a', CMD_STOP = 'q' }; // Enumeración de comandos recibidos

// Configuración de red
const char* ssid = "HUAWEI_p20";
const char* password = "12345678";

WiFiServer TCPServer(8266); // Servidor del ESP32
WiFiClient TCPClient; // Cliente TCP (PC)

// Variables para almacenar las sumatorias de las rotaciones
float sum_rotation_x = 0.0;
float sum_rotation_y = 0.0;
float sum_rotation_z = 0.0;

// Parámetros del control
const float Kp = 6; // Constante proporcional para la corrección
unsigned long startTime;

void byteReceived(byte byteReceived) {
    switch (byteReceived) {
        case CMD_FORWARD:
            Serial.println("Forward");
            digitalWrite(PIN_MOTOR_R_FWD, LOW);
            digitalWrite(PIN_MOTOR_R_BWD, HIGH);
            digitalWrite(PIN_MOTOR_L_FWD, HIGH);
            digitalWrite(PIN_MOTOR_L_BWD, LOW);
            break;
        case CMD_BACKWARD:
            Serial.println("Backward");
            digitalWrite(PIN_MOTOR_R_FWD, HIGH);
            digitalWrite(PIN_MOTOR_R_BWD, LOW);
            digitalWrite(PIN_MOTOR_L_FWD, LOW);
            digitalWrite(PIN_MOTOR_L_BWD, HIGH);
            break;
        case CMD_RIGHT:
            Serial.println("Right");
            digitalWrite(PIN_MOTOR_R_FWD, HIGH);
            digitalWrite(PIN_MOTOR_R_BWD, LOW);
            digitalWrite(PIN_MOTOR_L_FWD, HIGH);
            digitalWrite(PIN_MOTOR_L_BWD, LOW);
            break;
        case CMD_LEFT:
            Serial.println("Left");
            digitalWrite(PIN_MOTOR_R_FWD, LOW);
            digitalWrite(PIN_MOTOR_R_BWD, HIGH);
            digitalWrite(PIN_MOTOR_L_FWD, LOW);
            digitalWrite(PIN_MOTOR_L_BWD, HIGH);
            break;
        case CMD_STOP:
            Serial.println("Stop");
            digitalWrite(PIN_MOTOR_R_FWD, LOW);
            digitalWrite(PIN_MOTOR_R_BWD, LOW);
            digitalWrite(PIN_MOTOR_L_FWD, LOW);
            digitalWrite(PIN_MOTOR_L_BWD, LOW);
            break;
        default:
            break;
    }
}

void setup() {
    pinMode(PIN_MOTOR_R_FWD, OUTPUT);
    pinMode(PIN_MOTOR_R_BWD, OUTPUT);
    pinMode(PIN_MOTOR_L_FWD, OUTPUT);
    pinMode(PIN_MOTOR_L_BWD, OUTPUT);

    pinMode(PIN_ENABLE_R, OUTPUT);
    pinMode(PIN_ENABLE_L, OUTPUT);

    digitalWrite(PIN_MOTOR_R_FWD, LOW);
    digitalWrite(PIN_MOTOR_R_BWD, LOW);
    digitalWrite(PIN_MOTOR_L_FWD, LOW);
    digitalWrite(PIN_MOTOR_L_BWD, LOW);

    digitalWrite(PIN_ENABLE_R, LOW);
    digitalWrite(PIN_ENABLE_L, LOW);

    Serial.begin(115200);

    Serial.printf("Conectando a: %s\n", ssid);

    WiFi.begin(ssid, password);

    // Intentamos que se conecte a la red wifi
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Conectando...");
        delay(2000);
    }

    Serial.print("Conectado.  ");
    Serial.print(" Dirección IP del módulo: ");
    Serial.println(WiFi.localIP());

    TCPServer.begin();

    // Try to initialize!
    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        while (1) {
            delay(10);
        }
    }
    Serial.println("MPU6050 Found!");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    // Empezar a moverse hacia adelante
    digitalWrite(PIN_MOTOR_R_FWD, LOW);
    digitalWrite(PIN_MOTOR_R_BWD, HIGH);
    digitalWrite(PIN_MOTOR_L_FWD, HIGH);
    digitalWrite(PIN_MOTOR_L_BWD, LOW);

    // Guardar el tiempo de inicio
    startTime = millis();
}

void loop() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Si han pasado más de 2 segundos desde el inicio, comenzar a sumar las rotaciones
    if (millis() - startTime > 2000) {
        // Actualizar las sumatorias de las rotaciones
        sum_rotation_x += g.gyro.x;
        sum_rotation_y += g.gyro.y;
        sum_rotation_z += g.gyro.z;

        // Corrección de la rotación en Z
        float correction = Kp * sum_rotation_z;

        // Ajustar velocidades de los motores para corregir la rotación
        int speed_right = 160;  // Velocidad base del motor derecho
        int speed_left = 196;    // Velocidad base del motor izquierdo

        // Si la corrección es positiva, aumentar la velocidad del motor izquierdo
        if (correction > 0.06) {
            speed_left += abs(correction)+13; // Aumentar la velocidad del motor izquierdo
            speed_right -= abs(correction);
        }

        // Si la corrección es negativa, aumentar la velocidad del motor derecho
        else if(correction < -0.04){
            speed_right += abs(correction); // Aumentar la velocidad del motor derecho
            speed_left -= abs(correction);
        }

        // Limitar las velocidades a un rango válido (0-255)
        speed_right = constrain(speed_right, 0, 232);
        speed_left = constrain(speed_left, 0, 248);

        Serial.print("Speed Right: ");
        Serial.print(speed_right);
        Serial.print(", Speed Left: ");
        Serial.print(speed_left);

        analogWrite(PIN_ENABLE_R, speed_right);
        analogWrite(PIN_ENABLE_L, speed_left);
    }

    Serial.print(", Z: ");
    Serial.print(sum_rotation_z);

    Serial.println("");
    delay(1000);
}
