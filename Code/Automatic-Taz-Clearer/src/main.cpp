//Maeve Baksa - May 5, 2022 - Automatic TAZ Clearer - This Was Not Written the Day Before the Deadline

#include <Arduino.h>
#include <Servo.h>
#include <string.h>


// ** SETUP AND CALIBRATION WHOHOO ** 
// there is a lot of this, all needs to be configured for your specific setup:

// * hardware details * 
//(if using the same setup as the original plan, the hardware details can be skipped)
// Servos are powered by 5 volt power. This is important as the internal Arduino 5V converter will be needed 
// as if the servos draw too much power they can burn out the Arduino. It is easy to reuse an old
// USB cable and brick for this task. You have to connect the yellow wire to the corresponding pin. 
// Using the linked to CNC shield, there are two connections for digital pins -- 12 and 13 directly above the main
// power input connection. 

// Left Servo Pin:
#define leftServoPin 12

// Right Servo Pin:
#define rightServoPin 13

// Stepper motors draw substancially more power, as such they are needed to be powered with an additonal 12 - 36V power 
// supply (24V reccomended). Their STEP, DIR, SLP and RST pins are required to be connected. IN THE EVENT THE MOTOR IS MOVING
// IN THE INCORRECT DIRECTION, you MUST rotate the stepper motor connection on the side that is connecting to the CNC shield.
// Basic principals of a stepper motor --  they must be connected to a driver -- in this case an A4988. We must also
// then control them using PWM pulses. These are sent to the STEP Pin. The more frequently these pulses are sent, 
// the faster that the motor spins. The DIR pin determines the direction of the motor, with the RST and SLP
// pins determining if the motor should be powered (this is used to conserve power). 

// Utilize the next lines of code to define the pins. X axis denoting left motor, Y axis denoting right motor.

#define leftMotorStepPin 2
#define leftMotorDirPin 5

#define rightMotorStepPin 3
#define rightMotorDirPin 6


// Lastly, you need to set the sensor pin for your connection to 
// Octoprint, this is typically set to the "resume" pin or A2

#define triggerPin 2


// * movement setup *

// enable this (set to 1) in order to calibrate the motor.
#define stepsPerMMCalibration 0

// Utilize the following instructions for calibrating how far the motor has to turn to move 1mm.

// Set one of the above to true (1). Then follow these notes:
// the motor will move 100mm forward. If it is going the wrong way, rotate the motor wire 
// and try again. 
// Once the motor has stopped, measure the distance that it has traveled. This must be done 
// very precisely. The more accuracy, the better. The motor pauses for 20 seconds.
// Follow the following calculation to calculate the new steps / mm. 

//                         ACTUAL DISTANCE TRAVELED MEASURED
// EXISTING STEPS / MM *    ------------------------------    =   NEW STEPS / MM
//                              TARGET DISTANCE (100MM)


// The above will have to be repeated 2 - 3 times until the distance travaled is correct.
// Remember to write in the values here between goes -- save them -- then upload them.

// In a pinch, the default value of 2560 can be used in order to test the system.
//For production it is HIGHLY reccomended that you calibrate the E-Steps for maximum accuracy.

int stepsPerMM = 400;

//the total distance you want the aparatus to move (in mm)

int travelDistance = 305;



// * servo travel distances *

// IMPORTANT !! During first installation of the wiper blades DO NOT attach them until
// the motor has moved to the initial point of 90*. When this happens, attach the blades so 
// that they point towards the same direction that the servo's mount point is on. SEE:

//      |  <BLADE
//      |
// -----|----
// |   ()   | <MOUNT POINT
// |        |
// |        |
// |        |
// ----------


// Since the two motors will be moving in opposing directions, one of these values
// should be closer to 0, whereas the other closer to 180. Attempt to make these as straight as possible.

int rightServoLimit = 0;
int leftServoLimit = 180;

// * physical limits*

//the delay between pulses -- if your motor sounds like it is grinding and your VREF has been calibrated
//try increasing this value to slow the top speed of the motor

int delayTime = 500;

//* misc *
//ignore this

//this is the analogread trigger sensetivity at which point it should read a
//go command
int triggerSensitivity = 512;

// * define servos *
//make an instance of the servo class, attach the corresponding pin to it in setup
Servo rightServo;
Servo leftServo;

//define the loop checking variable, will make
//sure that the pin has been high for at least
//1 second.

int wasHigh = 0;

//* debugging *
//ignore this

//set this to one if you are using the arduino outside of an environment 
//in which it is connected to an octopi.
#define overrideTriggerPin 1


//this is the function in charge of moving the stepper motors.
//the main points are as follows
//motor has three options as an int, 1 for left, 2 for right, 3 for both
//direction has two options as an int, 1 for forward, 2 for backward
//mm is the distance traveled, which is multiplied with the steps per mm to get total steps
void moveStepper(int motor, int direction, int mm){

  //calculate steps to take:
  long int stepsNeeded = abs(mm * stepsPerMM);
  Serial.println(stepsNeeded);
  //source: 
  //https://www.makerguides.com/a4988-stepper-motor-driver-arduino-tutorial/
  //left motor
  if (motor == 1){
    //checks the direction that is desired, and set Dir accordingly
    if (direction == 1){
      digitalWrite(leftMotorDirPin, LOW);
    }
    else if (direction == 2){
      digitalWrite(leftMotorDirPin, HIGH);
    }
    //repeat this for the amount of pulses needed (mm*steps / mm = steps)
    for (int i = 0; i < stepsNeeded; i++) {
    // Pulsing the pin like with PWM will change the speed.
      digitalWrite(leftMotorStepPin, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(leftMotorDirPin, LOW);
      delayMicroseconds(delayTime);
  }
  }


  //right motor
  else if (motor == 2){
    //checks the direction that is desired, and set Dir accordingly
    if (direction == 1){
      digitalWrite(rightMotorDirPin, LOW);
    }
    else if (direction == 2){
      digitalWrite(rightMotorDirPin, HIGH);
    }
    for (int i = 0; i < stepsNeeded; i++) {
    // Pulsing the pin like with PWM will change the speed.
      digitalWrite(rightMotorStepPin, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(rightMotorStepPin, LOW);
      delayMicroseconds(delayTime);
  }
  }


  //both motors -- in this case my previous code
  //was able to treat two steps per mm values 
  //individually -- this version doesn't do that
  // but I don't want to rewrite everything
  // so it is assuming they are close
  //enough as that is better than them
  //drifting from different values
  else if (motor == 3){
    //checks the direction that is desired, and set Dir accordingly
    if (direction == 1){
      digitalWrite(rightMotorDirPin, LOW);
      digitalWrite(leftMotorDirPin, LOW);
    }
    else if (direction == 2){
      digitalWrite(rightMotorDirPin, HIGH);
      digitalWrite(leftMotorDirPin, HIGH);
    }

    for (int i = 0; i < stepsNeeded; i++) {
     // Pulsing the pin like with PWM will change the speed.
      digitalWrite(rightMotorStepPin, HIGH);
      digitalWrite(leftMotorStepPin,HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(rightMotorStepPin, LOW);
      digitalWrite(leftMotorStepPin,LOW);
      delayMicroseconds(delayTime);
  }
  }
}

//this is the actual clearing script. It is pulled to it's own function as this way it is easier for customization by the end user.
void clear(){
  //run the servos to the locations by utilizing the moveStepper function.

  Serial.println("Moving");
  int i = 0;
  while (i < travelDistance){
    moveStepper(3,1,1);
    i++;
  }
  Serial.println("Moved");

  //then bring in the servos to scoop
  Serial.println("Scooping");
  rightServo.write(rightServoLimit);
  leftServo.write(leftServoLimit);
  Serial.println("Scooped");

  //return the arms to origin repeating the above with 2 for the direction instead of
  //one to invert it


  Serial.println("Clearing");
  i=0;
  while (i < travelDistance){
    moveStepper(3,2,1);
    i++;
  }
  Serial.println("Cleared");


  //AT THIS POINT THE PARTS ARE OFF AND LIFE IS GOOD!!!!

  //return the servos back to the rest point
  Serial.println("Finishing");
  rightServo.write(90);
  leftServo.write(90);
  Serial.println("Finished");

}


void setup() {

  Serial.begin(9600);

  //pin modes (this took me far too long to realise this bug)
  pinMode(rightMotorDirPin,OUTPUT);
  pinMode(rightMotorStepPin,OUTPUT);
  pinMode(leftMotorStepPin,OUTPUT);
  pinMode(leftMotorDirPin,OUTPUT);
  pinMode(leftServoPin,OUTPUT);
  pinMode(rightServoPin,OUTPUT);

  //check if you want to *calibrate*

  //the calibration sequence will do as explained above, move the motor 100mm, and using the actual distance you
  // can calculate how far you need to adjust.
  // The movement code, is cited from the source above.
  //calibrate
  if (stepsPerMMCalibration == 1) {
    int i = 0;
    while (i < 100){
      moveStepper(3,1,1);
      i++;
    }
    delay(20000);
    i = 0;
    while (i < 100){
      moveStepper(3,2,1);
      i++;
    }
  }

  //attach pins to the servos
  rightServo.attach(rightServoPin);
  leftServo.attach(leftServoPin);

  //move the servos to the middle position:

  rightServo.write(90);
  leftServo.write(90);




}

void loop() {

  // the loop code, simply checks to see if the pin is active, and then as such start the script.

  int holdValue = analogRead(triggerPin);

  //check if it should increase wasHigh
  if (holdValue > triggerSensitivity){
    wasHigh = wasHigh + 1;
  }
  else if (holdValue < triggerSensitivity){
    wasHigh = 0;
  }

  //if the pin was high for at least 2 cycles OR override is set clear
  if (wasHigh >= 2 || overrideTriggerPin == 1){
    clear();
  }

  delay(1000);
  
  
}