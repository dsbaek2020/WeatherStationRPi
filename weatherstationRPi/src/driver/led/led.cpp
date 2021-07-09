/*
 * led.cpp
 *
 *  Created on: 2021. 7. 9.
 *      Author: jelly
 */

#include "led.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
using namespace std;

namespace exploringRPi {


LED::LED(int gpioNumber){  // constructor implementation
   this->gpioNumber = gpioNumber;
   gpioPath = string(GPIO "gpio") + to_string(gpioNumber) + string("/");
   writeSysfs(string(GPIO), "export", to_string(gpioNumber));
   usleep(100000);         // ensure GPIO is exported
   writeSysfs(gpioPath, "direction", "out");
}

// This implementation function is "hidden" outside of the class
void LED::writeSysfs(string path, string filename, string value){
   ofstream fs;
   fs.open((path+filename).c_str());
   fs << value;
   fs.close();
}

void LED::turnOn(){
   writeSysfs(gpioPath, "value", "1");
}

void LED::turnOff(){
   writeSysfs(gpioPath, "value", "0");
}

void LED::displayState(){
   ifstream fs;
   fs.open((gpioPath + "value").c_str());
   string line;
   cout << "The current LED state is ";
   while(getline(fs,line)) cout << line << endl;
   fs.close();
}

LED::~LED(){  // The destructor unexports the sysfs GPIO entries
   cout << "Destroying the LED with GPIO number " << gpioNumber << endl;
   writeSysfs(string(GPIO), "unexport", to_string(gpioNumber));
}

} /* namespace exploringRPi */

/* example */
/*
int main(int argc, char* argv[]){  // the main function start point
   cout << "Starting the makeLEDs program" << endl;
   LED led1(4), led2(17);          // create two LED objects
   cout << "Flashing the LEDs for 5 seconds" << endl;
   for(int i=0; i<50; i++){        // LEDs will alternate
      led1.turnOn();               // turn GPIO4 on
      led2.turnOff();              // turn GPIO17 off
      usleep(FLASH_DELAY);         // sleep for 50ms
      led1.turnOff();              // turn GPIO4 off
      led2.turnOn();               // turn GPIO17 on
      usleep(FLASH_DELAY);         // sleep for 50ms
   }
   led1.displayState();            // display final GPIO4 state
   led2.displayState();            // display final GPIO17 state
   cout << "Finished the makeLEDs program" << endl;
   return 0;
}
*/
