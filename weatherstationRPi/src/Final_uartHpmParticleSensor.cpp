//Author : ChoiSY
//2021.8.27(금)




#include<stdio.h>
#include<fcntl.h>
#include<unistd.h> //for sleep function
#include<termios.h>
#include<string.h>
#include<stdlib.h>


////#include<wiringPi.h>
//#include "GPIO.h"
//using namespace exploringRPi;
//using namespace std;


//#define  LED_GPIO    17


//센서와 통신하기 위한 명령 및 응답 데이터 자료구조 정의
//명령
const char readResultCmd[4]      = {0x68,0x01,0x04,0x93};
const char stopAutoSendCmd[4]    = {0x68,0x01,0x20,0x77};
const char enableAutoSendCmd[4]  = {0x68,0x01,0x40,0x57};
const char stopParticleMes[4]    = {0x68,0x01,0x02,0x95};

//응답(구조체)
typedef struct response{
	   char head;  //0
	   char len;   //1
	   char cmd;   //2
	   char dataField[4];  //3 4 5 6
	   char cs;            // 7
}t_response;

//응답(구조체 공용체)
typedef union rxSensorResponse{
	   char rxByte[8];
	   t_response resp;
}t_rxSensorResponse;


enum responseRxStatus{
	Ready,
	Head,
	Length,
	CMD
};



int main(int argc, char *argv[]){
   int client, count=0;
   unsigned char c;
   char rxBuff[10];

   char *command = (char*) malloc(255*sizeof(char));

   char controlVal = 0;
   t_rxSensorResponse rxSensorResponse;
   short PM10 = 0;
   short PM25 = 0;


   ////wiringPiSetupGpio();                    // initialize wiringPi
   ////pinMode(LED_GPIO, OUTPUT);              // the LED is an output
   //GPIO outGPIO(LED_GPIO);
   //outGPIO.setDirection(OUTPUT);


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

   //열번만 센서값을 읽어와서 전시해 본다.
   int k = 0;
   while(1){
      //Send SensorReadCommand from RPi to Sensor with UART(ttyUSB0).
      if(write(client, readResultCmd, sizeof(readResultCmd))<0){
        perror("Error: Failed to write to the sensor\n");
        return -1;
      }


     //센서 데이터 수신 알고리즘 작성 부분 (센서 전원이 중간에 다시켜지는 상황까지 고려한 알고리즘)------------
       //여기에 코딩하세요

      char readData;
      enum responseRxStatus rxStatus;
      rxStatus = Ready;
      char errorCount =0;

      do{
    	  if(read(client,&readData,1)>0){

    		  if(rxStatus == Ready && readData == 0x40){
    			  rxStatus = Head;
    			  rxSensorResponse.resp.head = readData;

    		  }
    		  else if(rxStatus == Head && readData == 0x05){
    			  rxStatus = Length;
    			  rxSensorResponse.resp.len = readData;
    		  }
    		  else if(rxStatus == Length && readData == 0x04){
    		      rxStatus = CMD;
    		      rxSensorResponse.resp.cmd = readData;
    		  }
    		  else{
    			  rxStatus = Ready;
    			  rxSensorResponse.rxByte[0] = 0;
    			  rxSensorResponse.rxByte[1] = 0;
    			  rxSensorResponse.rxByte[2] = 0;
    			  errorCount++;
    		  }

    	  }
    	  else{
    		  errorCount++;

    		  //에러를 카운트 한다. 만약 에러가 100개 넘으면 break 해서 나온다.
    		  if(errorCount == 10){
    			  break;
    		  }
    	  }

      }while(rxStatus != CMD);

      if(rxStatus == CMD){
		if(read(client,&rxSensorResponse.resp.dataField,5)==5){
			PM10 = rxSensorResponse.resp.dataField[2]*256 + rxSensorResponse.resp.dataField[3];
			PM25 = rxSensorResponse.resp.dataField[0]*256 + rxSensorResponse.resp.dataField[1];
			printf("PM10 = %d, PM25 = %d \n", PM10, PM25);

			/*or(int i=0; i<8; i++){
				printf("[%x] ", rxSensorResponse.rxByte[i]);
			}*/
			//printf("\n");
		}
      }


     // -------------------------------------------------------------------------------------


	  usleep(100000); //100ms  //센서 데이터 받는 주기를 0.1초마다 수행하기 위해서 딜레이
	  //sleep(1);
      k++;
   }

    //센서를 정지 시킨다.
    if(write(client, stopParticleMes,4)<0){
    perror("Error : Failed to write to the sensor\n");
    return -1;
    }

    //정지가 되었는지 확인한다. NOTE ***** (디버깅 필요함, 데이터가 0x40, 0x05가 계속 수신됨)  << NOTE *****
    char response[2];
    if(read(client,&response,2)==2){

      printf("[%x], [%x] ", response[0], response[1]);

      if((response[0]==0xa5) && (response[1]==0xa5)){
        printf("closed!!!!!! \n ");
      }else{
        printf("Bad closed \n");
      }
    }

   close(client);
   return 0;


}


