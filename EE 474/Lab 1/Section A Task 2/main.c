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
  GPIO_PORTF_LOCK_R = LOCK_ENABLE;
  GPIO_PORTF_CR_R = 0xFF;
  GPIO_PORTF_DIR_R = 0x0E;
  GPIO_PORTF_PUR_R = 0x11;
  GPIO_PORTF_DEN_R = 0x1F;
  
  GPIO_PORTF_DATA_R = 0;
  
  while (1) {
    if (GPIO_PORTF_DATA_R == 0x10) {
      GPIO_PORTF_DATA_R = GREEN;
    } else if (GPIO_PORTF_DATA_R == 0x01) {
      GPIO_PORTF_DATA_R = RED;
    } else {
      GPIO_PORTF_DATA_R = 0;
    }
  }
  
  return 0;
}