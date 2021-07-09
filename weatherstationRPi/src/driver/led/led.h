/** Simple Sysfs LED control program - written by Derek Molloy
*    simple OOP  struture for the Exploring Raspberry Pi
*
*    This program can be used to easliy control multiple LEDS using a class.
*    This program uses C++11 for the to_string() function and therefore must
*    be compiled with the -std=c++11 flag.
*/
#ifndef LED_H_
#define LED_H_

#include<iostream>
#include<fstream>
#include<string>
#include<unistd.h>         // for the microsecond sleep function

using namespace std; // for string 자료형을 쓰기 위해서 네임스페이스 선언

#define GPIO         "/sys/class/gpio/"


namespace exploringRPi {

class LED{
   private:                // the following is part of the implementation
      string gpioPath;     // private states
      int    gpioNumber;
      void writeSysfs(string path, string filename, string value);
   public:                 // part of the public interface
      LED(int gpioNumber); // the constructor -- create the object
      virtual void turnOn();
      virtual void turnOff();
      virtual void displayState();
      virtual ~LED();      // the destructor -- called automatically
};

} /* namespace exploringRPi */

#endif /* LED_H_ */

