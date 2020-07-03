#include <SoftwareSerial.h>
#include <Servo.h>
Servo base, claw, elbow, wrist;
SoftwareSerial Bluetooth(13, 8); //Assigns the HC-06 bluetooth module to its connected Arduino RX, TX pins
int basePPos = 90, clawPPos = 70, elbowPPos = 70, wristPPos = 90; //PPos is current servo position
int basePos = 90, clawPos = 70, elbowPos = 70, wristPos = 90; //Pos will be used as future servo position
double speed = 1.0;
String phoneOutput = "", outputN = "", outputX = "";
int enA = 11, in1 = 12, in2 = 7; //DC motor 1 connections
int enB = 3, in3 = 4, in4 = 2; //DC motor 2 connections
int m1speed = 0, m2speed = 0; //Current DC motor speeds
int m1speedf = 0, m2speedf = 0; //Future DC motor speeds

void setup() {
  base.attach(6); //Assigns each servo to its corresponding pin on the Arduino
  claw.attach(9);
  elbow.attach(10);
  wrist.attach(5);
  Bluetooth.begin(9600); //Sets bluetooth baud rate to 9600 bps (same as Android tablet)
  Bluetooth.setTimeout(1); //Sets bluetooth timeout time to 1 ms; Arduino will check for bluetooth data for 1ms before interpreting it
  delay(20);
  base.write(basePPos); //Sets each servo's position to default
  claw.write(clawPPos);
  elbow.write(elbowPPos);
  wrist.write(wristPPos);
  pinMode(enA, OUTPUT); //Sets motor control pins to outputs
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  digitalWrite(in1, LOW); //Turns off both motors
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  Serial.begin(9600);
}

//Repeatedly checks for bluetooth signals and moves the servos and DC motors based on the signals
void loop() {
  if (Bluetooth.available()) { //Detects if data is being sent over bluetooth
    phoneOutput = Bluetooth.readString(); //Reads incoming bluetooth data as a String and assigns it to phoneOutput
    while (phoneOutput.indexOf(';') != -1) { //If the data contains a semicolon
      if (phoneOutput.charAt(0) <= 122 && phoneOutput.charAt(0) >= 97) //If a new command is sent after a command was cut off
        outputX = ""; //Clears the storage to prevent the cut off command from interfering with the new one
      outputN = outputX + phoneOutput.substring(0, phoneOutput.indexOf(';')); //Separates first command from list of commands. If a command was cut in two, joins the 2 parts.
      phoneOutput = phoneOutput.substring(phoneOutput.indexOf(';') + 1, phoneOutput.length()); //Removes current command from list of commands
      switch (char n = outputN.charAt(0)) {
        case ('b'):
          basePos = outputN.substring(1, outputN.length()).toInt(); //Sets the future base position to the position sent over bluetooth
          changePos(basePPos, basePos, base); //Moves base from current position to future position
          basePPos = base.read(); //Updates the current base position
          break;
        case ('c'): //Same as above, but for the claw servo
          clawPos = outputN.substring(1, outputN.length()).toInt();
          changePos(clawPPos, clawPos, claw);
          clawPPos = claw.read();
          break;
        case ('e'): //Same as above, but for the elbow servo
          elbowPos = outputN.substring(1, outputN.length()).toInt();
          changePos(elbowPPos, elbowPos, elbow);
          elbowPPos = elbow.read();
          break;
        case ('w'): //Same as above, but for wrist servo
          wristPos = outputN.substring(1, outputN.length()).toInt();
          changePos(wristPPos, wristPos, wrist);
          wristPPos = wrist.read();
          break;
        case ('s'): //Changes speed to a value between 0.1 and 4.0
          speed = 0.01 * outputN.substring(1, outputN.length()).toInt();
          break;
        case ('l'): //Changes speed and/or direction of left DC motor
          if (outputN.charAt(1) == '-') { //If the command is to go backwards
            m1speedf = -outputN.substring(2, outputN.length()).toInt(); //toInt method only works on positives
            m1speedf -= 130; //Shifts to a higher speed to prevent stalling
          }
          else {
            m1speedf = outputN.substring(1, outputN.length()).toInt();
            if (m1speedf != 0)
              m1speedf += 150; //Shifts to a higher speed to prevent stalling
          }
          changeSpeed(m1speedf, m1speed, enB, in4, in3); //Changes left motor speed
          m1speed = m1speedf; //Updates current left motor speed
          break;
        case ('r'): //Same as above, but for right DC motor
          if (outputN.charAt(1) == '-') {
            m2speedf = -outputN.substring(2, outputN.length()).toInt();
            m2speedf -= 200; //Shifts speed more, because right motor is slower than left motor
          }
          else {
            m2speedf = outputN.substring(1, outputN.length()).toInt();
            if (m2speedf != 0)
              m2speedf += 200; //Shifts speed more, because right motor is slower than left motor
          }
          changeSpeed(m2speedf, m2speed, enA, in1, in2);
          m2speed = m2speedf;
          break;
      }
    }
    outputX = phoneOutput; //Stores first part of a truncated command
  }
}

//Moves servo position to the position sent by the phone application over bluetooth, one degree at a time
void changePos(int PPos, int Pos, Servo servo) {
  if (PPos > Pos) {
    for (int i = PPos; i >= Pos; i--) {
      servo.write(i);
      delay((int) (20 / speed));
    }
  }
  else if (PPos < Pos) {
    for (int i = PPos; i <= Pos; i++) {
      servo.write(i);
      delay((int) (20 / speed));
    }
  }
}

// Changes DC motor speed/direction to the speed/direction sent by the phone application
void changeSpeed(int speedf, int speedp, int en, int in1, int in2) {
  if (speedp >= 0 && speedf >= 0) {
    digitalWrite(in1, HIGH); //forwards
    digitalWrite(in2, LOW);
    incrementSpeed(speedf, speedp, en);
  }
  else if (speedp <= 0 && speedf <= 0) {
    digitalWrite(in1, LOW); //backwards
    digitalWrite(in2, HIGH);
    incrementSpeed(-speedf, -speedp, en);
  }
  else if (speedp <= 0 && speedf >= 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    incrementSpeed(0, -speedp, en);
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    incrementSpeed(speedf, 0, en);
  }
  else if (speedp >= 0 && speedf <= 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    incrementSpeed(0, speedp, en);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    incrementSpeed(-speedf, 0, en);
  }
}

//Changes current speed to future speed incrementally
void incrementSpeed (int speedf, int speedp, int en) {
  if (speedf > speedp) {
    for (int i = speedp; i <= speedf; i++) {
      analogWrite(en, i);
      delay(2);
    }
  }
  else {
    for (int i = speedp; i >= speedf; i--) {
      analogWrite(en, i);
      delay(2);
    }
  }
}
