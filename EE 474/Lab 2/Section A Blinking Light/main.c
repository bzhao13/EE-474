#include <tm4c123gh6pm.h>
#include <stdint.h>

#define SYSCTL_RCGC2_GPIOF 0x00000020
#define RED 0x01
#define BLUE 0x02
#define VIOLET 0x03
#define GREEN 0x04
#define YELLOW 0x05
#define LIGHT_BLUE 0x06
#define WHITE 0x07
#define LOCK_ENABLE 0x4C4F434B

void LED_Init(void);
void Timer_Init(void);

int main(void) {
  
  // array for all led colors
  int array[7] = {RED, BLUE, VIOLET, GREEN, YELLOW, LIGHT_BLUE, WHITE};
  int i = 0;
  int j = 0;
  
  LED_Init();
  Timer_Init();

  while (1) {
    if ((TIMER0_RIS_R & 0x01) == 1) {
      TIMER0_ICR_R |= (1<<0);
      GPIO_PORTF_DATA_R = (array[i]<<1);
      i++;
      if (i > 6) {
        i = 0;
      }
    }
  }
  
  return 0;
}

void LED_Init(void) {
  volatile unsigned long delay;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // enable port F

  delay = SYSCTL_RCGC2_R; // wait for clock setup
  GPIO_PORTF_LOCK_R = LOCK_ENABLE; // unlock PF0
  GPIO_PORTF_CR_R |= 0xFF; // 0b1111.1111 enable write to PUR
  GPIO_PORTF_AFSEL_R = 0x00; // disable alternate functions of pins
  GPIO_PORTF_DIR_R = 0x0E; // 0b01110 PF4 and PF0 inputs, PF1, PF2, PF3 outputs
  GPIO_PORTF_PUR_R = 0x11; // enable PUR on PF4 and PF0
  GPIO_PORTF_DEN_R = 0x1F; // enable pins PF4 to PF0
  GPIO_PORTF_DATA_R = 0; // all LEDs off
}

void Timer_Init(void) {
  SYSCTL_RCGCTIMER_R |= 0x01; // enable timer 0
  TIMER0_CTL_R |= (0<<0); // disable timer maybe 0x01
  TIMER0_CFG_R |= 0x00000000; // set 32-bit config
  TIMER0_TAMR_R |= (0x2<<0); // set TAMR to periodic timer mode
  TIMER0_TAMR_R |= (0<<4); // timer countdown
  TIMER0_TAILR_R = 0x00F42400; // start counter at 16,000,000
  TIMER0_CTL_R |= (1<<0); // enable timer
}