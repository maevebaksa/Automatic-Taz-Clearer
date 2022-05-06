//Maeve Baksa - May 5, 2022 - Automatic TAZ Clearer - This Was Not Written the Day Before the Deadline

#include <Arduino.h>
#include <AccelStepper.h>
#include <Servo.h>


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
#define stepsPerMMCalibrationLeft 0
#define stepsPerMMCalibrationRight 0

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

// In a pinch, the default values of 2560 for each motor can be used in order to test the system.
//For production it is HIGHLY reccomended that you calibrate the E-Steps for maximum accuracy.

#define stepsPerMMLeft 2560
#define stepsPerMMRight 2560

//the total distance you want the aparatus to move (in mm)

#define travelDistance 305



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

#define rightServoLimit 0
#define leftServoLimit 180




// * mechanical limits *
// these not need be changed -- before touching these as 
// troubleshooting, ensure that your stepper drivers are set to the correct
// vref. When in doubt -- go slow.

//the left stepper motor's max speed (in steps/sec)
#define leftStepperMaxSpeed 2000
//the left stepper motor's acceleration (in steps/sec^2)
#define leftStepperAccel 200
//the right stepper motor's max speed (in steps/sec)
#define rightStepperMaxSpeed 2000
//the right stepper motor's acceleration (in steps/sec^2)
#define rightStepperAccel 200

//* misc *
//ignore this

//this is the analogread trigger sensetivity at which point it should read a
//go command
#define triggerSensitivity 512

//define the loop checking variable, will make
//sure that the pin has been high for at least
//1 second.

int wasHigh = 0;


//* debugging *
//ignore this

//set this to one if you are using the arduino outside of an environment 
//in which it is connected to an octopi.
#define overrideTriggerPin 0

// * validate configuration *

//at least, somewhat
#if (stepsPerMMCalibrationLeft == 1 && stepsPerMMCalibrationRight == 1)
    #error Please only calibrate one motor at a time, it will safe your time -- trust me on this.
#endif

// * define steppers *
//define the two stepper motors -- classes :>
//the "DRIVER" option allows for a CNC shield with existing motor drivers instead
//of utilizing the arduino as the driver itself (better solution this way).
//The pins are defined above

AccelStepper leftStepper(AccelStepper::DRIVER,leftMotorStepPin,leftMotorDirPin);
AccelStepper rightStepper(AccelStepper::DRIVER,rightMotorStepPin,rightMotorDirPin);

// * define servos *
//make an instance of the servo class, attach the corresponding pin to it in setup
Servo rightServo;
Servo leftServo;

void setup() {

  //first the stepper motors must be initialized, as per the documetnation for accelstepper and this source
  // https://lastminuteengineers.com/a4988-stepper-motor-driver-arduino-tutorial/
  // ^^ the name of this source could not be more accurate ;>




  //set your limits as defined above -- this is insipired  from this code:
  // https://codebender.cc/sketch:356209#2%20stepper%20motors%20test%20using%20accelstepper%20library.ino

  rightStepper.setMaxSpeed(rightStepperMaxSpeed);
  rightStepper.setAcceleration(rightStepperAccel);

  leftStepper.setMaxSpeed(leftStepperMaxSpeed);
  leftStepper.setAcceleration(leftStepperAccel);


  //check if you want to *calibrate*

  //the calibration sequence will do as explained above, move the motor 100mm, and using the actual distance you
  // can calculate how far you need to adjust.
  // The movement code, is cited from the source above.
  //calibrate left
  if (stepsPerMMCalibrationLeft == 1 && stepsPerMMCalibrationRight == 0) {
    leftStepper.runToNewPosition(100*stepsPerMMLeft);
    delay(20000);
    leftStepper.runToNewPosition(0);
  }
  //calibrate right
  else if (stepsPerMMCalibrationRight == 1 && stepsPerMMCalibrationLeft == 0){
    rightStepper.runToNewPosition(100*stepsPerMMRight);
    delay(20000);
    rightStepper.runToNewPosition(0);

  }

  //attach pins to the servos
  rightServo.attach(rightServoPin);
  leftServo.attach(leftServoPin);

  //move the servos to the middle position:

  rightServo.write(90);
  leftServo.write(90);

}

void clear(){
  //this is the actual clearing script. It is pulled to it's own function as this way it is easier for customization by the end user.

  //the code for moving them together calls from this forum post, explaining the two types of motion:
  //https://forum.arduino.cc/t/running-2-stepper-with-accelstepper-together-and-seperate/387822/2

  //first move the arms to the end point

  leftStepper.moveTo(travelDistance*stepsPerMMLeft);
  rightStepper.moveTo(travelDistance*stepsPerMMRight);
  while (!leftStepper.run() || !rightStepper.run());

  //then bring in the servos to scoop
  rightServo.write(rightServoLimit);
  leftServo.write(leftServoLimit);

  //return the arms to origin
  leftStepper.moveTo(0);
  rightStepper.moveTo(0);

  //AT THIS POINT THE PARTS ARE OFF AND LIFE IS GOOD!!!!

  //return the servos back to the rest point
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