#include "lab1_header.h"
#include <stdint.h>

#define SYSCTL_RCGC2_GPIOF 0x00000020
#define RED 0x02
#define GREEN 0x08
#define BLUE 0x04
#define RED_GREEN 0x0A
#define RED_BLUE 0x06
#define GREEN_BLUE 0x0C
#define RED_GREEN_BLUE 0x0E
#define LOCK_ENABLE 0x4C4F434B

int main(void) {
  SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;
  
  int array[7] = {RED, GREEN, BLUE, RED_GREEN, RED_BLUE, GREEN_BLUE, RED_GREEN_BLUE};
  int i = 0;
  int j = 0;
  
  while (1) {
    for (j = 0; j < 7; j++) {
      GPIO_PORTF_DIR_R = array[j];
      GPIO_PORTF_DEN_R = array[j];
      GPIO_PORTF_DATA_R = array[j];
      for (i = 0; i < 1000000; i++) {}
      GPIO_PORTF_DATA_R = 0;
      for (i = 0; i < 1000000; i++) {}
    }
  }
  return 0;
}