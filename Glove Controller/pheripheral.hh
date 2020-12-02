#ifndef PERIOHERALS_HH
#define PERIOHERALS_HH

//define all the pin numbers
#define thumb A0
#define index A1
#define middle A2
#define ring A3
#define pinky A4

#define ModeSW 12
#define BNO08X_RESET 4

#define rgbRed 11
#define rgbGreen 10
#define rgbBlue 9

int getBattery(void);
int LEDWrite(int R, int G);
int FadeIn(int Pin);
int FadeOut(int Pin);

#endif
