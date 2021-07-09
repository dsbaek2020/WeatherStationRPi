/*
 * client.cpp
 *
 *  Created on: 2021. 7. 7.
 *      Author: jelly
 */

#include <iostream>
#include "network/SocketClient.h"
using namespace std;
using namespace exploringRPi;

int main(int argc, char *argv[]){
   if(argc!=2){
      cout << "Incorrect usage: " << endl;
      cout << "   client server_name" << endl;
      return 2;
   }
   cout << "Starting RPi Client Example" << endl;
   SocketClient sc(argv[1], 54321);
   sc.connectToServer();

   while(1){
	   string message("Hello from the Client");
	   cout << "Sending [" << message << "]" << endl;
	   sc.send(message);
   }

   cout << "End of RPi Client Example" << endl;
}


