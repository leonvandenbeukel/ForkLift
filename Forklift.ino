/*
 * Use an Arduino Mega 2560 because: 
 *  - Cannot use SoftwareSerial for the Bluetooth module because this will mess up the internal timers when using the Servo lib and causing servo jitter
 *  - At least 6 PWM pins are needed for the motors, the Servo lib disables PWM on pins 9 & 10  
 *    
 */

#include <Servo.h>

Servo servoSteering;        
Servo servoLift;    

// Warning: you need to calibrate these values with your specific servo:
int servoMaxLeft = 30;                                // Steering value when at the lowest position
int servoMaxRight = 145;                              // Steering value when at the highest position
int servoCenter = (servoMaxRight - servoMaxLeft)/2;   // Steering center position
int servoLiftCenter = 85;                             // Vertical lift center position
int servoLiftCenterMax = 155;                         // Vertical lift maximum position forward:  85 + 70
int servoLiftCenterMin = 35;                          // Vertical lift minimum position backward: 85 - 50

#define servoPin 22          // LET: GEBRUIK VAN SERVO DISABLED PWM OP 9 en 10!
#define servoLiftPin 24

String btBuffer;
 
#define Ain1 2
#define Ain2 3

#define Bin1 4
#define Bin2 5

#define Ain1_lift 6
#define Ain2_lift 7

void setup() {
  servoSteering.attach(servoPin);     
  servoLift.attach(servoLiftPin);

  Serial.begin(57600);
  Serial1.begin(9600);

  pinMode(Ain1, OUTPUT);  // A in1
  pinMode(Ain2, OUTPUT);  // A in2
  pinMode(Bin1, OUTPUT);  // B in1
  pinMode(Bin2, OUTPUT);  // B in2
  pinMode(Ain1_lift, OUTPUT);  // Lift motor
  pinMode(Ain2_lift, OUTPUT);  // Lift motor

  analogWrite(Ain1,0);
  analogWrite(Ain2,0);
  analogWrite(Bin1,0);
  analogWrite(Bin2,0);
  analogWrite(Ain1_lift, 0);
  analogWrite(Ain2_lift, 0);
  
  servoSteering.write(servoCenter); 
  servoLift.write(servoLiftCenterMin);
}

void loop() {
//return;
//  servoLift.write(servoLiftCenter - 50);
//  Serial.println(servoLift.read());
//  delay(2000);
//  servoLift.write(servoLiftCenter - 20);
//  delay(2000);
//  return;

  if (Serial1.available())
  {
    //char received = BTSerial.read();
    char received = Serial1.read();
    btBuffer += received; 
    if (received == '|')
    {
        Serial.println(btBuffer);
        processCommand();
        btBuffer = "";
    }
  }    
}

void processCommand() {
  char separator = ',';
  int percW = getValue(btBuffer, separator, 0).toInt();
  int percH = getValue(btBuffer, separator, 1).toInt();
  int liftPos = getValue(btBuffer, separator, 2).toInt();
  int liftVerticalPos = getValue(btBuffer, separator, 3).toInt();

  int valServo = map(percW, 100, 0, servoMaxLeft, servoMaxRight);   // Switch the values 100 and 0 to reverse direction
  int valLiftServo = map(liftVerticalPos, 100, 0, servoLiftCenterMin, servoLiftCenterMax); 
  int spdMotor = 0;

  Serial.print("W: ");
  Serial.print(percW);
  Serial.print(", H: ");  
  Serial.print(percH);
  Serial.print(", Servo: ");
  Serial.print(valServo);
  Serial.print(", Lift position: ");
  Serial.print(liftPos);  
  Serial.print(", Vertical raw: ");
  Serial.print(liftVerticalPos);
  Serial.print(", Vertical calculated: ");
  Serial.print(valLiftServo);

  // Forward/Backward
  if (percH < 50) {
    spdMotor = map(percH, 50, 0, 50, 255);
    Serial.print(", Motor Forward: ");
    Serial.print(spdMotor);
    motorForward(spdMotor);
  } else if (percH > 50) {
    spdMotor = map(percH, 50, 100, 50, 255);
    Serial.print(", Motor Backwards: ");
    Serial.print(spdMotor);
    motorBackward(spdMotor);
  } else if (percH == 50) {
    stopMoving();
  } 

  // Lift
  switch (liftPos) {
    case 1:
      analogWrite(Ain1_lift, 120);
      analogWrite(Ain2_lift, 0);
    break;
    case 0:
      analogWrite(Ain1_lift, 0);
      analogWrite(Ain2_lift, 0);
    break;
    case -1:
      analogWrite(Ain1_lift, 0);
      analogWrite(Ain2_lift, 60);
    break;
  }

  // Lift vertical position
  servoLift.write(valLiftServo);

  // Steering
  servoSteering.write(valServo);

  Serial.println();
  delay(60);  
}

// Switch the names of the functions motorBackward and motorForward to reverse direction
void motorBackward(int spd) {  
  analogWrite(Ain1,spd);  
  analogWrite(Ain2,0);
  analogWrite(Bin2,spd);
  analogWrite(Bin1,0);
}

// Switch the names of the functions motorBackward and motorForward to reverse direction
void motorForward(int spd) {
  analogWrite(Ain1,0);
  analogWrite(Ain2,spd);
  analogWrite(Bin2,0);
  analogWrite(Bin1,spd);
}

void stopMoving() {
  analogWrite(Ain1,0);
  analogWrite(Ain2,0);
  analogWrite(Bin2,0);
  analogWrite(Bin1,0);
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
