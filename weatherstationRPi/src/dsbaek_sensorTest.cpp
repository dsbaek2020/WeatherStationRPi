#include<stdio.h>
#include<fcntl.h>
#include<unistd.h> //for sleep function
#include<termios.h>
#include<string.h>
#include<stdlib.h>


////#include<wiringPi.h>
#include "GPIO.h"
using namespace exploringRPi;
using namespace std;


#define  LED_GPIO    17

const char readResultCmd[4]      = {0x68,0x01,0x04,0x93};
const char stopAutoSendCmd[4]    = {0x68,0x01,0x20,0x77};
const char enableAutoSendCmd[4] = {0x68,0x01,0x40,0x57};



typedef struct response{
	   char head;
	   char len;
	   char cmd;
	   char dataField[4];
	   char cs;
}t_response;


typedef union rxSensorResponse{
	   char rxByte[8];
	   t_response resp;

}t_rxSensorResponse;

int main(int argc, char *argv[]){
   int client, count=0;
   unsigned char c;
   char rxBuff[10];

   char *command = (char*) malloc(255*sizeof(char));

   char controlVal = 0;
   t_rxSensorResponse rxSensorResponse;
   short PM10 = 0;
   short PM2dot5 =0;
   char timeOutCount =0;
   int timeTag =0;

   ////wiringPiSetupGpio();                    // initialize wiringPi
   ////pinMode(LED_GPIO, OUTPUT);              // the LED is an output
   GPIO outGPIO(LED_GPIO);
   outGPIO.setDirection(OUTPUT);


   if ((client = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY ))<0){
      perror("UART: Failed to open the file.\n");
      return -1;
   }

   struct termios options;
   tcgetattr(client, &options);
   options.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
   options.c_iflag = IGNPAR | ICRNL;

   // disable local echo
   options.c_lflag &= ~ECHO;
   options.c_lflag &= ~ICANON; //정규모드 비활성화, 정규모드일 경우에는 문자열 한 줄 단위로 수신함


   tcflush(client, TCIFLUSH);
   //fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);  // make reads non-blocking
   tcsetattr(client, TCSANOW, &options);
   /*if (message(client, "\n\rRPi Serial Server running")<0){
      perror("UART: Failed to start server.\n");
      return -1;
   }*/

   //자동 전송모드 비활성화
   if (write(client, stopAutoSendCmd, sizeof(stopAutoSendCmd))<0){
	  perror("Error: stopAutoSendCmd Failed to write to the sensor\n");
	  return -1;
   }

   //Clear Receive Buffer
   printf("clear rx buffer ... ");
   do{
	   //nop //clear rx buff
	   printf(".");
	   usleep(20000); //rx time (1 byte)
   }while(read(client,&c,1)>0);
   printf("\n");

   while(controlVal!=1){
	   //Send SensorReadCommand from RPi to Sensor with UART(ttyUSB0).
	   if(write(client, readResultCmd, sizeof(readResultCmd))<0){
		  perror("Error: Failed to write to the sensor\n");
		  return -1;
	   }

 /* //단순한 수신 알고리즘 ---- (최종적으로는 사용하면 안됨)
	   if(read(client,&rxSensorResponse.rxByte,8)==8){
 		   PM10 = rxSensorResponse.resp.dataField[2]*256 + rxSensorResponse.resp.dataField[3];
		  	   printf("PM10 = %d \n", PM10);
	   }
*/

     //센서 데이터 수신 알고리즘 작성 부분 ------------
	   char packetStatus = 0;
	   char timeOut =0;
	   char rByte;

	   usleep(4000); //홀드 오프 시간 (데이터가 올때 까지 작은 기다림)
	   //헤더 검출 알고리즘
	   do{

		   if(read(client,&rByte,1)>0){
			   if(packetStatus == 0 and rByte==0x40){
				   rxSensorResponse.resp.head = rByte;
				   packetStatus = 1;
				   timeOut=0;
			   }else if(packetStatus == 1 and rByte ==0x05){
				   rxSensorResponse.resp.len = rByte;
				   packetStatus =2;
			   }else{
				   packetStatus = 0;
				   timeOut++;
			   }
		   }
		   else{
			   timeOut++;
		   }

	   }while(packetStatus!=2 and timeOut<10);


		if(packetStatus ==2){
			if(read(client,&rxSensorResponse.rxByte[2],6)==6){
				PM2dot5 = rxSensorResponse.resp.dataField[0]*256 + rxSensorResponse.resp.dataField[1];
				PM10    = rxSensorResponse.resp.dataField[2]*256 + rxSensorResponse.resp.dataField[3];
				printf("%d: PM10 = %d, PM2.5 = %d , totalTimeOut = %d \n",timeTag, PM10, PM2dot5, timeOutCount);
				timeTag++;
			}
		}else{
			timeOutCount++;
			printf("%d: PM10 = ??, PM2.5 = ?? , totalTimeOut = %d \n",  timeTag, timeOutCount);
		}


     // ----------------------------------------

	  usleep(100000); //100ms  //센서 데이터 받는 주기를 0.1초마다 수행하기 위해서 딜레이
	  //sleep(1);
   }

   close(client);
   return 0;


}
