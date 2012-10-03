#ifndef DISPLAY_H
#define DISPLAY_H

#define DISP_CLOCK 0
#define DISP_LAT   1
#define DISP_ACC   2
#define DISP_ROT   3
#define DISP_DEB   4

extern int displayMode;

extern char debugDisplay0[17];
extern char debugDisplay1[17];

void incDisplayMode(void);
void decDisplayMode(void);
void clearDisplay(void);
void display(void);
void displayLightOn(void);
void displayLightOff(void);

#endif
