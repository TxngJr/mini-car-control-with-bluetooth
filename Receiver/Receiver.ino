#include <HardwareSerial.h>
#include <ESP32Servo.h>

#define FRONT_LEFT_FORWARD_PIN 22
#define FRONT_LEFT_BACKWARD_PIN 23

#define FRONT_RIGHT_FORWARD_PIN 2
#define FRONT_RIGHT_BACKWARD_PIN 15

#define BACK_LEFT_FORWARD_PIN 12
#define BACK_LEFT_BACKWARD_PIN 14

#define BACK_RIGHT_FORWARD_PIN 33
#define BACK_RIGHT_BACKWARD_PIN 32

#define SERVO_X_PIN 21
#define SERVO_Y_PIN 18

Servo servoX;
Servo servoY;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  pinMode(FRONT_LEFT_FORWARD_PIN, OUTPUT);
  pinMode(FRONT_LEFT_BACKWARD_PIN, OUTPUT);

  pinMode(FRONT_RIGHT_FORWARD_PIN, OUTPUT);
  pinMode(FRONT_RIGHT_BACKWARD_PIN, OUTPUT);

  pinMode(BACK_LEFT_FORWARD_PIN, OUTPUT);
  pinMode(BACK_LEFT_BACKWARD_PIN, OUTPUT);

  pinMode(BACK_RIGHT_FORWARD_PIN, OUTPUT);
  pinMode(BACK_RIGHT_BACKWARD_PIN, OUTPUT);

  pinMode(BACK_RIGHT_BACKWARD_PIN, OUTPUT);
  pinMode(BACK_RIGHT_BACKWARD_PIN, OUTPUT);

  servoX.attach(SERVO_X_PIN);
  servoY.attach(SERVO_Y_PIN);
}
void loop() {
  static byte controlMoveStatus = 0;
  static byte speedMoveStatus = 0;
  static byte spectatorViewX = 90;
  static byte spectatorViewY = 90;
  if (Serial2.available()) {
    String receivedData = Serial2.readStringUntil('&');
    Serial.println(receivedData);

    int pos1 = receivedData.indexOf(',');
    int pos2 = receivedData.indexOf(',', pos1 + 1);
    int pos3 = receivedData.indexOf(',', pos2 + 1);

    controlMoveStatus = receivedData.substring(0, pos1).toInt();
    speedMoveStatus = receivedData.substring(pos1 + 1, pos2).toInt();
    spectatorViewX = receivedData.substring(pos2 + 1, pos3).toInt();
    spectatorViewY = receivedData.substring(pos3 + 1).toInt();

    // Serial.println(controlMoveStatus);
    // Serial.println(speedMoveStatus);
    // Serial.println(spectatorViewX);
    // Serial.println(spectatorViewY);
  }
  servoX.write(spectatorViewX);
  servoY.write(spectatorViewY);
  controlMoveFunction(controlMoveStatus, speedMoveStatus);
  delay(30);
}

void controlMoveFunction(byte status, byte speed) {
  switch (status) {
    case 1:
      //forward
      analogWrite(FRONT_LEFT_FORWARD_PIN, speed);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, 0);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, speed);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, 0);

      analogWrite(BACK_LEFT_FORWARD_PIN, speed);
      analogWrite(BACK_LEFT_BACKWARD_PIN, 0);

      analogWrite(BACK_RIGHT_FORWARD_PIN, speed);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, 0);
      break;
    case 2:
      //moveLeft
      analogWrite(FRONT_LEFT_FORWARD_PIN, 0);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, speed);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, speed);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, 0);

      analogWrite(BACK_LEFT_FORWARD_PIN, speed);
      analogWrite(BACK_LEFT_BACKWARD_PIN, 0);

      analogWrite(BACK_RIGHT_FORWARD_PIN, 0);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, speed);
      break;
    case 3:
      //rotateLeft
      analogWrite(FRONT_LEFT_FORWARD_PIN, 0);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, speed);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, speed);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, 0);

      analogWrite(BACK_LEFT_FORWARD_PIN, 0);
      analogWrite(BACK_LEFT_BACKWARD_PIN, speed);

      analogWrite(BACK_RIGHT_FORWARD_PIN, speed);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, 0);
      break;
    case 4:
      //forwardLeftOblique
      analogWrite(FRONT_LEFT_FORWARD_PIN, 0);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, 0);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, speed);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, 0);

      analogWrite(BACK_LEFT_FORWARD_PIN, speed);
      analogWrite(BACK_LEFT_BACKWARD_PIN, 0);

      analogWrite(BACK_RIGHT_FORWARD_PIN, 0);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, 0);
      break;
    case 5:
      //moveRight
      analogWrite(FRONT_LEFT_FORWARD_PIN, speed);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, 0);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, 0);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, speed);

      analogWrite(BACK_LEFT_FORWARD_PIN, 0);
      analogWrite(BACK_LEFT_BACKWARD_PIN, speed);

      analogWrite(BACK_RIGHT_FORWARD_PIN, speed);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, 0);
      break;
    case 6:
      //rotateRight
      analogWrite(FRONT_LEFT_FORWARD_PIN, speed);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, 0);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, 0);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, speed);

      analogWrite(BACK_LEFT_FORWARD_PIN, speed);
      analogWrite(BACK_LEFT_BACKWARD_PIN, 0);

      analogWrite(BACK_RIGHT_FORWARD_PIN, 0);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, speed);
      break;
    case 7:
      //forwardRightOblique
      analogWrite(FRONT_LEFT_FORWARD_PIN, speed);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, 0);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, 0);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, 0);

      analogWrite(BACK_LEFT_FORWARD_PIN, 0);
      analogWrite(BACK_LEFT_BACKWARD_PIN, 0);

      analogWrite(BACK_RIGHT_FORWARD_PIN, speed);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, 0);
      break;
    case 8:
      //back
      analogWrite(FRONT_LEFT_FORWARD_PIN, 0);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, speed);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, 0);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, speed);

      analogWrite(BACK_LEFT_FORWARD_PIN, 0);
      analogWrite(BACK_LEFT_BACKWARD_PIN, speed);

      analogWrite(BACK_RIGHT_FORWARD_PIN, 0);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, speed);
      break;
    case 9:
      //backLeftOblique
      analogWrite(FRONT_LEFT_FORWARD_PIN, 0);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, speed);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, 0);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, 0);

      analogWrite(BACK_LEFT_FORWARD_PIN, 0);
      analogWrite(BACK_LEFT_BACKWARD_PIN, 0);

      analogWrite(BACK_RIGHT_FORWARD_PIN, 0);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, speed);
      break;
    case 10:
      //backReftOblique
      analogWrite(FRONT_LEFT_FORWARD_PIN, 0);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, 0);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, 0);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, speed);

      analogWrite(BACK_LEFT_FORWARD_PIN, 0);
      analogWrite(BACK_LEFT_BACKWARD_PIN, speed);

      analogWrite(BACK_RIGHT_FORWARD_PIN, 0);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, 0);
      break;
    default:
      //stop
      analogWrite(FRONT_LEFT_FORWARD_PIN, 0);
      analogWrite(FRONT_LEFT_BACKWARD_PIN, 0);

      analogWrite(FRONT_RIGHT_FORWARD_PIN, 0);
      analogWrite(FRONT_RIGHT_BACKWARD_PIN, 0);

      analogWrite(BACK_LEFT_FORWARD_PIN, 0);
      analogWrite(BACK_LEFT_BACKWARD_PIN, 0);

      analogWrite(BACK_RIGHT_FORWARD_PIN, 0);
      analogWrite(BACK_RIGHT_BACKWARD_PIN, 0);
      break;
  }
}