//Author : KimYL, ChoiSY
//2021.9.3(금)


#include<stdio.h>
#include<fcntl.h>
#include<unistd.h> //for sleep function
#include<termios.h>
#include<string.h>
#include<stdlib.h>
#include "GPIO.h"
#include "ArduiPi_OLED_lib.h"
#include "ArduiPi_OLED.h"
#include "Adafruit_GFX.h"




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


typedef enum responseRxStatus{
	Ready,
	Head,
	Length,
	CMD
}t_rxStatus;


//Define Function Prototype -------------
static int initOLED(ArduiPi_OLED *pOledHandle);
static int initUART(void);
static int initParticleSensor(int client);
static t_rxStatus readPacket(int client,t_rxSensorResponse *pSensorData);
//---------------------------------------

int main(int argc, char *argv[]){
   ArduiPi_OLED display;
   //GPIO switch(17);
   t_rxSensorResponse rxSensorResponse;
   int client, count=0;
   short PM10 = 0;
   short PM25 = 0;
   unsigned char c;
   char rxBuff[10];
      
   if(initOLED(&display) == -1){
      return -1;
   }
   client=initUART();
   
   if(initParticleSensor(client) == -1){
      return -1;
   }

   while(1){
      //Send SensorReadCommand from RPi to Sensor with UART(ttyUSB0).
      if(write(client, readResultCmd, sizeof(readResultCmd))<0){
        perror("Error: Failed to write to the sensor\n");
        return -1;
      }

      if(readPacket(client, &rxSensorResponse) == CMD){
		if(read(client,&rxSensorResponse.resp.dataField,5)==5){
			PM10 = rxSensorResponse.resp.dataField[2]*256 + rxSensorResponse.resp.dataField[3];
			PM25 = rxSensorResponse.resp.dataField[0]*256 + rxSensorResponse.resp.dataField[1];
			printf("PM10 = %d, PM25 = %d \n", PM10, PM25);
		}
      }
      usleep(100000); //100ms  //센서 데이터 받는 주기를 0.1초마다 수행하기 위해서 딜레이
   }

   close(client);
   return 0;
}


int initOLED(ArduiPi_OLED *pOledHandle){
    
    if(!pOledHandle->init(OLED_I2C_RESET, OLED_ADAFRUIT_I2C_128x64)){
        perror("Failed to set up the display\n");
        return -1;
    }
    printf("Setting up the I2C Display output\n");
    pOledHandle->begin();
    pOledHandle->clearDisplay();
    pOledHandle->setTextSize(1);
    pOledHandle->setTextColor(WHITE);
    pOledHandle->setCursor(27,5);
    pOledHandle->printf("helllo");
    pOledHandle->display();
    pOledHandle->close();
    printf("End of the I2C Display program\n");
    
    return 0;
}

int initUART(void){
   int client;
   
      //----uart init (linux ap: open , termios)-----------------------------------------
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
   //---end uart init-----------------------------------------------------------------
   
   return client;
   
}
int initParticleSensor(int client){
//---초기화--HPMA particle sensor -------------------------------------------
   //자동 전송모드 비활성화
   char c;
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
   
   return 0;
   
   //------------------------------------------------------------------------

}

t_rxStatus readPacket(int client, t_rxSensorResponse *pSensorData){
   char readData;
   t_rxStatus rxStatus;
   t_rxSensorResponse rxSensorResponse;
   
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
   
   memcpy(pSensorData, &rxSensorResponse, sizeof(rxSensorResponse));
   
   return rxStatus;

   /*if(rxStatus == CMD){
      return 0;
   }else{
      return -1;
   }*/
}




