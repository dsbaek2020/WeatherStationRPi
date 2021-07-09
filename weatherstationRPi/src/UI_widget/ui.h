//
//  ui.h
//  bakery_porgram
//
//  Created by 최서영 on 2020/11/09.
//

#ifndef ui_h
#define ui_h



#ifdef __cplusplus // preprocessor <--- When c++ code call c-function.
extern "C"{
#endif


#include <unistd.h> /*for sleep(), usleep()  */

#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))
#define clear() printf("\033[H\033[J")


typedef struct{
    int x;
    int y;
    char shape[3][80];
}UI_BUTTON;

typedef struct{
    int x;
    int y;
    char text[80];
    char shape[3][80];
}UI_INPUT_TEXT;

typedef struct{
    int x;
    int y;
    char shape[14][150];
}UI_OUTLINE;

void showButton(UI_BUTTON *p);
void showOutLine(UI_OUTLINE *p);
void showInputBox(UI_INPUT_TEXT *p);

#ifdef __cplusplus
}
#endif

#endif /* ui_h */
