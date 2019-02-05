#include "lab1_header.h"
#include <stdint.h>

void Switch_Init(void);
unsigned long Start_Stop(void);
unsigned long Ped_Cross(void);
void Green_On(void);
void Yellow_On(void);
void Red_On(void);
void delay(int a);

/*
  states
  0 = go
  1 = warn
  2 = stop
  3 = reset
*/
int state = 3;

/*
  0 = green
  1 = red
  2 = none
*/
int last_on = 2;

// Value for if the start/stop button is held down
int hold = 1;
  
int main(void) {
  Switch_Init();
    
  while (1) {
    if (state == 0) {
      if (0x04 != Start_Stop()) {
        hold = 0;
      }
      
      Green_On();
      last_on = 0;
      
      delay(1000000);
      if (0x08 == Ped_Cross()) {
        state = 1;
      } else if ((hold == 0) && (0x04 == Start_Stop())){
        state = 3;
        hold = 1;
      } else {
        state = 2;
      }
      
    } else if (state == 1) {
      Yellow_On();
      delay(1000000);
      state = 2;
    } else if (state == 2) {
      if (0x04 != Start_Stop()) {
        hold = 0;
      }
      
      Red_On();
      last_on = 1;
      delay(1000000);
      if (0x08 == Ped_Cross()) {
        state = 2;
      } else if ((hold == 0) && (0x04 == Start_Stop())){
        state = 3;
        hold = 1;
      } else {
        state = 0;
      }
      
    } else if (state == 3) {
      if (0x04 != Start_Stop()) {
        hold = 0;
      }
      
      // pauses light that was last on
      if (last_on == 0) {
        Green_On();
      } else if (last_on == 1) {
        Red_On();
      }
      
      delay(1000000);
      if ((hold == 0) && (0x04 == Start_Stop())) {
        state = 0;
        hold = 1;
      } else {
        state = 3;
      }
      
    }
  }
  return 0;
}

// Initialize GPIO port A and necessary pins
void Switch_Init(void) {
  volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x01; // enable GPIO port A
  delay = SYSCTL_RCGC2_R; // wait for clock start
    
  GPIO_PORTA_AMSEL_R &= ~0xEC; // 0b1110.1100 disable anlog on PA7, PA6, PA5, PA3, PA2
  GPIO_PORTA_DEN_R |= 0xEC; // 0b1110.1100 enable digital on PA7, PA6, PA5, PA3, PA2
  
  GPIO_PORTA_DIR_R |= 0xE0; // 0b1110.0000 set PA7, PA6, PA5 as output, PA3, PA2 as input
  GPIO_PORTA_PCTL_R &= ~0xFFF0FF00; // PCTL GPIO on PA7, PA6, PA5, PA3, PA2
  GPIO_PORTA_AFSEL_R &= ~0x0C; // 0b0000.1100 regular port function on PA7, PA6, PA5, PA3, PA2
  
  GPIO_PORTA_DATA_R = 0; // reset data
}

// detect when start button is active
unsigned long Start_Stop(void) {
  return (GPIO_PORTA_DATA_R & 0x04); // return 0x04 if active
}

// detect when pedestrian crossing is active
unsigned long Ped_Cross(void) {
  return (GPIO_PORTA_DATA_R & 0x08); // return 0x08 if active
}

void Green_On(void) {
  GPIO_PORTA_DATA_R = 0x20;
}

void Yellow_On(void) {
  GPIO_PORTA_DATA_R = 0x40;
}

void Red_On(void) {
  GPIO_PORTA_DATA_R = 0x80;
}

void delay(int a) {
  int i = 0;
  for(i = 0; i < a; i++) {}
}