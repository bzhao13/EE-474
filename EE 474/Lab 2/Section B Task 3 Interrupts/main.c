#include <tm4c123gh6pm.h>
#include <stdint.h>

#define RED 0x02
#define GREEN 0x08
#define BLUE 0x04
#define LOCK_ENABLE 0x4C4F434B

void Switch_Init(void);
void PortF_Init(void);
void Timer_Init(void);
void Timer_Handler(void);
void PortF_Handler(void);

int main(void) {
  Timer_Init();
  PortF_Init();
  Switch_Init();

  while (1) {}
  return 0;
}
void Switch_Init(void) {
  GPIO_PORTF_IS_R &= ~0x11; // bit 4 and 0 edge sensitive
  GPIO_PORTF_IBE_R &= ~0x11; // trigger controlled by IEV
  GPIO_PORTF_IEV_R = ~0x11; // falling edge trigger
  GPIO_PORTF_ICR_R |= 0x11; // clear prior interrupts
  GPIO_PORTF_IM_R |= 0x11; // unmask interrupt
  NVIC_EN0_R |= (1<<30); // enable port F handler
  NVIC_PRI7_R = (3<<21); // set interrupt priority to 3
}

void PortF_Handler(void) {
  volatile unsigned long sw = (GPIO_PORTF_DATA_R & 0x11);

  if (sw == 0x01) {
    TIMER0_IMR_R = (0<<0); // disable timer interrupt mask
    GPIO_PORTF_DATA_R = RED;
  } else if (sw == 0x10) {
    TIMER0_IMR_R |= (1<<0); // enable timer inturrupt mask
    GPIO_PORTF_DATA_R = 0;
  }

  GPIO_PORTF_ICR_R |= 0x11; // clear flag
}

// Initialize GPIO port F and necessary pins
void PortF_Init(void) {
  volatile unsigned long delay;
  SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF; // enable port F

  delay = SYSCTL_RCGC2_R; // wait for clock setup
  GPIO_PORTF_LOCK_R = LOCK_ENABLE; // unlock PF0
  GPIO_PORTF_CR_R = 0xFF; // 0b1111.1111 enable write to PUR
  GPIO_PORTF_AFSEL_R = 0x00; // disable alternate functions of pins
  GPIO_PORTF_DIR_R = 0x0E; // 0b01110 PF4 and PF0 inputs, PF1, PF2, PF3 outputs
  GPIO_PORTF_PUR_R = 0x11; // enable PUR on PF4 and PF0
  GPIO_PORTF_DEN_R = 0x1F; // enable pins PF4 to PF0
  GPIO_PORTF_DATA_R = 0; // all LEDs off
}

void Timer_Handler(void) {
  TIMER0_ICR_R |= (1<<0);
  GPIO_PORTF_DATA_R ^= (1<<2);
}

void Timer_Init(void) {
  SYSCTL_RCGCTIMER_R |= 0x01; // enable timer 0
  TIMER0_CTL_R |= (0<<0); // disable timer maybe 0x01
  TIMER0_CFG_R |= 0x00000000; // set 32-bit config
  TIMER0_TAMR_R |= (0x2<<0); // set TAMR to periodic timer mode
  TIMER0_TAMR_R |= (0<<4); // timer countdown
  TIMER0_TAILR_R = 0x00F42400; // start counter at 16,000,000
  NVIC_EN0_R |= (1<<19); // enable timer handler
  TIMER0_CTL_R |= (1<<0); // enable timer
}