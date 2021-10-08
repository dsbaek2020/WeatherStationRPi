#include <iostream>
#include <unistd.h>
#include "GPIO.h"
using namespace exploringRPi;
using namespace std;


class keypad{
private  :
    char ID;
    char key [4][4];
    GPIO *pled;
    GPIO *pRow[4];
    GPIO *pCol[4];
    //char *pkeyMap[4][4];
    char keyLayout[4][4];

public :
  keypad(char ledPortNum, char colNum[4], char rowNum[4], char keyLayout[4][4]);
  char getKey(void);
  void testGPIO(int colNum);
  void setColsLow(void);
  int getGpioNum(int a){ return pCol[a]->getValue();}

};

keypad::keypad(char ledPortNum, char colNum[4], char rowNum[4],char keyLayout[4][4]){
    pCol[0] = new GPIO(colNum[0]);
    pCol[1] = new GPIO(colNum[1]);
    pCol[2] = new GPIO(colNum[2]);
    pCol[3] = new GPIO(colNum[3]);
    pCol[0]->setDirection(OUTPUT);
    pCol[1]->setDirection(OUTPUT);
    pCol[2]->setDirection(OUTPUT);
    pCol[3]->setDirection(OUTPUT);

    for(int i=0; i<4; i++){
      pRow[i] = new GPIO(rowNum[i]);
      pRow[i]->setDirection(INPUT);

    }

    //memcpy(this->keyLayout,keyLayout, 16);

for(int row = 0; row<4; row++){

   for(int col=0; col<4; col++){
     this->keyLayout[row][col] = keyLayout[row][col];
   }

}
}


void keypad :: testGPIO(int colNum){
  for(int i = 0;  i<100; i++){
    pCol[colNum]->setValue(HIGH);
    usleep(1000);
    pCol[colNum]->setValue(LOW);
    usleep(1000);
  }
}
char keypad :: getKey(void)
{
    setColsLow();
    for(int col = 0; col<4; col++){
          pCol[col]->setValue(LOW);
	  printf("col = %d", col);
          for(int row = 0; row<4; row++){
                if(pRow[row]->getValue() == 0){
                      key[row][col] = 1;
		     printf(" status= %c \n ",  keyLayout[row][col]);
                }else {
                      key [row][col] =0;
                }
          }//end  for(int i = 0; i<4; i++)
          pCol[col]->setValue(HIGH);
    }// end for(int col = 0; col<4; col++)
    return -1;
}

void keypad :: setColsLow(void)
{
  for(int col = 0; col<4; col++){
    pCol[col]->setValue(HIGH);
  }
}


int main() {
  char cPorts[4] = {3,4,5,6};
  char rPorts[4] = {9,10,11,12};
  char keyLayout[4][4] = {{'1','2','3','A'}, {'4','5','6','B'},{'7','8','9','C'},{'0','f','E','D'}};
  keypad keyPad(20, cPorts, rPorts, keyLayout);
  while(1){
 
//  keyPad.setColsLow();
//  keyPad.testGPIO(3);
   keyPad.getKey();
   usleep(1000);

  }

}



