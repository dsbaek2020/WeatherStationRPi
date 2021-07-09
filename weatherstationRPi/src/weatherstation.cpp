//============================================================================
// Name        : weatherstation.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <iostream>
#include "SocketServer.h"
#include "ui.h"
#include "getch.h"
#include "led.h"

#include <pthread.h>
#include <string.h>

using namespace std;
using namespace exploringRPi;

#define LED_PWM_DELAY  20000 // 20 milliseconds


typedef struct Point {
    int x;
    int y;
} Point;




UI_BUTTON ReadSensor={
    30,20,
	{{"╭―――――――――――――――――╮"},
	 {"│ 1. 센서값 읽기  │"},
	 {"╰―――――――――――――――――╯"}}
};

UI_BUTTON ledControl={
	30,23,
		{{"╭―――――――――――――――――╮"},
		 {"│ 2. LED밝기      │"},
		 {"╰―――――――――――――――――╯"}}
	};

UI_BUTTON sendMessage={
	30,26,
		{{"╭―――――――――――――――――╮"},
		 {"│ 3. 문자 보내기  │"},
		 {"╰―――――――――――――――――╯"}}
	};


//----MultiThread type scanf  ------------------
void InputTextBox(UI_INPUT_TEXT *textBox, char string[]){

	int i;
	char keyValue;

	i=0;
	do{
		keyValue= getch();
		if(keyValue !='\n'){
			string[i] =keyValue;
			//goto cursor to input area
			gotoxy((textBox->x)+1+i,(textBox->y)+2);
			printf("%c",string[i]);
			i++;
		}
	}while(keyValue !='\n');  //LF (character : \n, Unicode : U+000A, hex : 0x0a): This is simply the '\n'
	string[i] ='\0'; //널문자 추가

}

int GetMenuNumber(void){

    char keyValue;
    int select;
    //do{
    keyValue= getch();
    //printf("key value1: %c\n",keyValue);


    if(keyValue==0x1b){
        keyValue= getch();
        //printf("key value2: %x\n",keyValue);


        if(keyValue==0x5b){

            keyValue= getch();
            //printf("your selection3: %x, ", keyValue);
            switch(keyValue){
                case 0x41: printf("UP WAS PRESSED\n"); break;
                case 0x42: printf("DOWN WAS PRESSED\n"); break;
                case 0x44: printf("LEFT WAS PRESSED\n"); break;
                case 0x43: printf("RIGHT WAS PRESSED\n"); break;

                    /*case 72: printf("UP WAS PRESSED\n"); break;
                     case 80: printf("DOWN WAS PRESSED\n"); break;
                     case 75: printf("LEFT WAS PRESSED\n"); break;
                     case 77: printf("RIGHT WAS PRESSED\n"); break;*/
            }
        }
    }
    else{
        //printf("key : %c \n", (char)keyValue);
        select = keyValue - '0';
    }

    return select;
}




void DisplayMainPanel(void){

	gotoxy(1,1);

	printf(
	"Clinet IP: 192.168.0.6 \n"
	"Rx Msg: Start_AirSensor \n"
	"LED Brightness: 70%  \n"

	);

	printf(" ____  _____ ____  _   _  \n");
	printf("|    || ___ |  _ || | | | \n");
	printf("| | | | ____| | | | |_| | \n");
	printf("|_|_|_|_____)_| |_|____/ \n");

	showButton(&ReadSensor);
	showButton(&ledControl);
	showButton(&sendMessage);
	//showButton(&ReadSensor);

	//rec = ((SocketServer *)socket)->receive(1024);
	//cout << "Received from the client [" << rec << "]" << endl;

}



void *DisplayMainView(void *socket)
{
	while(1)
	{
		DisplayMainPanel();
		usleep(100000);
	}
}



void ControlLED(void){

	cout << "Starting the makeLEDs program" << endl;
	LED led1(4), led2(17);          // create two LED objects

	char Brightness[20];
	char BrightnessInt;
	int ledOnTime;
	int ledOffTime;

	char string[20];

	UI_OUTLINE outLine={
		1,20,
		{  { "╭―――――――――――――――――――――――――╮"},
			{"│                         │"},
			{"│                         │"},
			{"│                         │"},
			{"│                         │"},
			{"│                         │"},
			{"│                         │"},
			{"│                         │"},
			{"│                         │"},
			{"│                         │"},
			{"│                         │"},
			{"│                         │"},
			{"│                         │"},
			{"╰―――――――――――――――――――――――――╯"}}
	};

	UI_INPUT_TEXT InputTextMenu={
		3,22,
		"LED 밝기%를 입력하십시오: ",
		{{"+-----------------+"},
		 {"|                 |"},
		 {"+-----------------+"}}
	};


	showOutLine(&outLine);

    while(1){
    	showInputBox(&InputTextMenu);

    	//----for MultiThread--------------------------
    	InputTextBox(&InputTextMenu, Brightness);
    	BrightnessInt = atoi(Brightness);
    	//----------------------------------
    	//  여기서 LED control 한다.
		ledOnTime =int(LED_PWM_DELAY*((float)BrightnessInt/100.0));
		ledOffTime = LED_PWM_DELAY-ledOnTime;

		for(int i=0; i<500; i++) //5초간 LED 켜지기
		{
			led1.turnOn();
			usleep(ledOnTime);

			led1.turnOff();
			usleep(ledOffTime);
		}

    	//----------------------------------

		strcpy(InputTextMenu.text,"마치시겠습니까? (yes/no)");
		showInputBox(&InputTextMenu);
		//----for MultiThread--------------------------
		InputTextBox(&InputTextMenu, string);
		//scanf("%s", &string[0]);

		if(  strcmp(string, "yes") == 0){
			printf("테스트를 종료합니다. \n");
			sleep(1);
			clear();
			//result = 1;
			break;
		}
    }
	//LED의 상태를 출력하고 gpio 객체를 닫기 한다.
	led1.displayState();            // display final GPIO4 state
	led2.displayState();            // display final GPIO17 state
}

//#define APP_MODE

int main(int argc, char *argv[]){


	int userCommand =0;
	SocketServer server(54321);
#ifdef  APP_MODE
   cout << "Listening for a connection..." << endl;
   server.listen();
#endif


   pthread_t thread_t;

   if(pthread_create(&thread_t, NULL, DisplayMainView, (void*)&server) < 0)
   {
	   perror("thread create error:");
	   exit(0);
   }

   while(1){
	   userCommand = GetMenuNumber();

		if(userCommand == 1){ // 센서값 읽기 함수 호춯

		}
		else if(userCommand == 2){ //LED 함수 호출

			ControlLED();
		}
		else{
			//nop
		}

   }



   cout << "End of WeatherStation " << endl;
   return 0;
}
