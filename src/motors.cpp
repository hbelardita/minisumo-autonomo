#include "motors.h"
#include "config.h"

// Inicialización de los pines de motor como salidas
void motorsInit() {
    pinMode(PIN_STBY, OUTPUT);
    pinMode(PIN_AIN1, OUTPUT);
    pinMode(PIN_AIN2, OUTPUT);
    pinMode(PIN_PWMA, OUTPUT);
    pinMode(PIN_BIN1, OUTPUT);
    pinMode(PIN_BIN2, OUTPUT);
    pinMode(PIN_PWMB, OUTPUT);
    digitalWrite(PIN_STBY, LOW);

}

// Función auxiliar interna para configurar cada motor
static void setMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int speed) {
    speed = constrain(speed, -255, 255);
    if (speed > 0) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        analogWrite(pwm, speed);
    } else if (speed < 0) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        analogWrite(pwm, -speed);
    } else {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        analogWrite(pwm, 0);
    }
}

// Establece la velocidad de ambos motores
void motorsSetSpeed(int rightSpeed, int leftSpeed) {
    digitalWrite(PIN_STBY, HIGH); // Activa el driver
      int speedA = -rightSpeed;   // Motor A (Derecho)
  int speedB = -leftSpeed; // Motor B (Izquierdo)
    // setMotor(PIN_AIN1, PIN_AIN2, PIN_PWMA, leftSpeed);
    // setMotor(PIN_BIN1, PIN_BIN2, PIN_PWMB, rightSpeed);

  if (speedA > 0)
  {
    digitalWrite(PIN_BIN1, HIGH);
    digitalWrite(PIN_BIN2, LOW);
  }
  else
  {
    digitalWrite(PIN_BIN1, LOW);
    digitalWrite(PIN_BIN2, HIGH);
    speedA = -speedA;
  }

  if (speedB > 0)
  {
    digitalWrite(PIN_AIN1, HIGH);
    digitalWrite(PIN_AIN2, LOW);
  }
  else
  {
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, HIGH);
    speedB = -speedB;
  }

  analogWrite(PIN_PWMA, constrain(speedA, 0, 255));
  analogWrite(PIN_PWMB, constrain(speedB, 0, 255));
  }

  void motorsBrake() {
    setMotor(PIN_AIN1, PIN_AIN2, PIN_PWMA, 0);
    setMotor(PIN_BIN1, PIN_BIN2, PIN_PWMB, 0);
}

// Desactiva el pin STBY para dejar los motores sueltos
void motorsStandby() {
    digitalWrite(PIN_STBY, LOW);
}
