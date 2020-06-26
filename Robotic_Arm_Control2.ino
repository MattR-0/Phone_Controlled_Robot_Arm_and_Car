#include <SoftwareSerial.h>
#include <Servo.h>
Servo base;
Servo claw;
Servo elbow;
Servo wrist;
SoftwareSerial Bluetooth(12, 11); //Assigns the HC-06 bluetooth module to its connected Arduino RX, TX pins
int baseDef = 90, clawDef = 70, elbowDef = 70, wristDef = 90; //Sets default servo positions
int basePPos = 90, clawPPos = 70, elbowPPos = 70, wristPPos = 90; //PPos is current servo position
int basePos = 90, clawPos = 70, elbowPos = 70, wristPos = 90; //Pos will be used as future servo position
double speed = 1.0;
String phoneOutput = "";

void setup() {
  base.attach(9); //Assigns each servo to its corresponding pin on the Arduino
  claw.attach(6);
  elbow.attach(3);
  wrist.attach(5);
  Bluetooth.begin(9600); //Sets bluetooth baud rate to 9600 bps (same as Android tablet)
  Bluetooth.setTimeout(1); //Sets bluetooth timeout time to 1 ms; Arduino will check for bluetooth data for 1ms before interpreting it
  delay(20);
  base.write(baseDef); //Sets each servo's position to default
  claw.write(clawDef);
  elbow.write(elbowDef);
  wrist.write(wristDef);
}

//Repeatedly checks for bluetooth signals and moves the servos based on the signals
void loop() {
  if (Bluetooth.available()) { //Detects if data is being sent over bluetooth
    phoneOutput = Bluetooth.readString(); //Reads incoming bluetooth data as a String and assigns it to phoneOutput
    String outputN, outputX;
    while (phoneOutput.indexOf(';') != -1) {
      outputN = outputX + phoneOutput.substring(0,phoneOutput.indexOf(';'));
      phoneOutput = phoneOutput.substring(phoneOutput.indexOf(';')+1,phoneOutput.length());
      switch (char n = outputN.charAt(0)) {
        case ('b'):
          basePos = outputN.substring(1,outputN.length()).toInt(); //Sets the future base position to the position sent over bluetooth
          changePos(basePPos,basePos,base); //Moves base from current position to future position
          basePPos = base.read(); //Updates the current base position
          break;
        case ('c'): //Same as above, but for the claw servo
          clawPos = outputN.substring(1,outputN.length()).toInt();
          changePos(clawPPos,clawPos,claw);
          clawPPos = claw.read();
          break;
        case ('e'): //Same as above, but for the elbow servo
          elbowPos = outputN.substring(1,outputN.length()).toInt();
          changePos(elbowPPos,elbowPos,elbow);
          elbowPPos = elbow.read();
          break;
        case ('w'): //Same as above, but for wrist servo
          wristPos = outputN.substring(1,outputN.length()).toInt();
          changePos(wristPPos,wristPos,wrist);
          wristPPos = wrist.read();
          break;
        case ('s'):
          speed = 0.01*outputN.substring(1,outputN.length()).toInt(); //Sets speed to a value between .01 and 2.0
          break;
      }
    }
    outputX = phoneOutput;
  }
}

//Moves servo position to the position sent by the phone application over bluetooth, one degree at a time
void changePos(int PPos, int Pos, Servo servo) {
  if (PPos > Pos) {
    for (int i = PPos; i >= Pos; i--) {
      servo.write(i);
      delay((int) (30/speed));
    }
  }
  else if (PPos < Pos) {
    for (int i = PPos; i <= Pos; i++) {
      servo.write(i);
      delay((int) (30/speed));
    }
  }
}
