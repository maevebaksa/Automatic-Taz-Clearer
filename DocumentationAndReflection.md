# Notes and Reflections Through Writing the Code:

There were a few issues that I could not figure out while writing the code.

The first one was the seeming incompatibility with the AccelStepper Library and then the Servo Library. This however, upon further realization could have been caused by a dead servo. Either way, the AccelStepper has some very weird limitation and I believe that this could be caused by Arduino as well. Throughout trying to make the AccelStepper library handle the larger amounts of pulses required to make the aparatus go the total 305mm length, the aparatus would only go a certain part of the way. A note in the compiler implied this was due to an overflow error, which could be possible. By my estimations, I believe that it could be possible that for some reason or another the runToNewPosition method within the AccelStepper class can only take a 16bit integer capping the maximum that the  aparatus could go to at 65356 pulses. This was not sufficient for my task.

I atttempted to rewrite the code in a way of manually setting the pulses - and a similar issue could be noted when running the while loop to pulse the digital output pin in which eventually the code would stop running. I believe that this could be caused due to some sort of microprocessing limitation. I am still unable to explain the reasoning behind why this doesn't work as intended. My solution to this was to limit the while loops amount of required pulses to only 1 mm, and then repeatedly  calling the function essentially turning the moveStepper function into a moveStepper1MM function.

The proper utilization of the function is as such:

```
 for (int i = 0; i < mmNeeded; i++) { 
   moveStepper(int motor, int direction, 1);  
}
```

I still do not entirely understand the nature of this limitation and I believe that as such, utilizing a similar block of code as what is seen above could allow for a functional use of the "AccelStepper" library as the "multistepper.h" would allow for much simpler control and more individualized execution of code allowing for different steps / mm values between the two differnet motors and providing a more accurate calibration. In the current state it is most reccomended to find the average steps/mm for the two motors and utilize that value. In theory -- the difference should not be large enough to warrant concern provided thourough assembly and high quality components.

For any support around this project, please feel free to open a GitHub issue or reach out:
maeve@maevebaksa.com
