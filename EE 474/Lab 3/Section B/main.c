#include <tm4c123gh6pm.h>
#include <stdint.h>

#define RED 0x01
#define BLUE 0x02
#define VIOLET 0x03
#define GREEN 0x04
#define YELLOW 0x05
#define LIGHT_BLUE 0x06
#define WHITE 0x07
#define LOCK_ENABLE 0x4C4F434B

void PortF_Init(void);
void Timer0_Init(void);
void Timer0_Handler(void);
void PLL_Init(void);
void Clock_Set_4MHz(void);
void Clock_Set_80MHz(void);
void ADC_Init(void);
void ADC0_Handler(void);
void UART1_Init_4MHz(void);
void UART1_Init_80MHz(void);
void UART1_printChar(char c);
void printString(char * string);
char int_To_Char(int i);
void Temp_LED_Ctrl(void);

volatile int temp;

int main(void) {
  PLL_Init();
  UART1_Init_80MHz();
  PortF_Init();
  Timer0_Init();
  ADC_Init();
  while (1) {
    volatile unsigned long sw = (GPIO_PORTF_DATA_R & 0x11);
    if (sw == 0x01) {
      Clock_Set_4MHz();
      UART1_Init_4MHz();
    } else if (sw == 0x10) {
      Clock_Set_80MHz();
      UART1_Init_80MHz();
    }
  }
  return 0;
}

void PortF_Init(void) {
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

void Timer0_Handler(void) {
  TIMER0_ICR_R |= (1<<0);
}

void Timer0_Init(void) {
  SYSCTL_RCGCTIMER_R |= 0x01; // enable timer 0 clock
  TIMER0_CTL_R &= ~(1<<0); // disable timer maybe 0x01
  TIMER0_CFG_R |= 0x00000000; // set 32-bit config
  TIMER0_TAMR_R |= (0x2<<0); // set TAMR to periodic timer mode
  TIMER0_CTL_R |= (1<<5); // enable timer trigger for ADC
  TIMER0_TAMR_R &= ~(1<<4); // timer countdown
  TIMER0_TAILR_R = 0x04C4B400; // start counter at 80,000,000
  NVIC_EN0_R |= (1<<19); // enable timer handler
  TIMER0_IMR_R |= (1<<0); // enable timer inturrupt mask might need to move
  TIMER0_CTL_R |= (1<<0); // enable timer
}

void PLL_Init(void) {
  SYSCTL_RCC2_R |= (1<<31); // use RCC2
  SYSCTL_RCC2_R |= (1<<11); // bypass PLL
  SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x7C0) + 0x540; // select crystal and oscillator
  SYSCTL_RCC2_R &= ~0x70; // config main oscillator
  SYSCTL_RCC2_R &= ~(1<<13); // activate PLL with PWRDN
  SYSCTL_RCC2_R |= (1<<30); // set DIV400
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) + (0x2<<23); // selecting divisor for 80MHz
  while ((SYSCTL_RIS_R & 0x40) == 0) {}; // wait for PLL lock
  SYSCTL_RCC2_R &= ~(1<<11); // clear bypass
}

void ADC_Init(void) {
  SYSCTL_RCGCADC_R |= (1<<0); // enable ADC0
  volatile unsigned long delay = SYSCTL_RCGCADC_R; // wait for clock
  ADC0_ACTSS_R &= ~(1<<3); // disable sequencer 3
  ADC0_EMUX_R = (0x5<<12); // tiggered by timer
  ADC0_SSCTL3_R |= (1<<3); // set input to temperature sensor once per sequence
  ADC0_SSCTL3_R |= (1<<1); // set END0 to avoid unpredictable behaviors
  ADC0_SSCTL3_R |= (1<<2); // enable interrupt
  ADC0_IM_R |= (1<<3); // set sequencer 3 interrupt mask  (0x7<<1)
  ADC0_ISC_R |= (1<<3); // clear previous flags
  NVIC_PRI4_R |= (0x3<<13); // set interrupt priority
  NVIC_EN0_R |= (1<<17); // enable ADC0 interrupt handler
  ADC0_ACTSS_R |= (1<<3); // enable sequencer 3
  
}

void ADC0_Handler(void) {
  temp = (int) (147.5 - (247.5 * ADC0_SSFIFO3_R) / 4096.0);
  Temp_LED_Ctrl();
  printString("Temperature: ");
  UART1_printChar(int_To_Char(temp/10));
  UART1_printChar(int_To_Char(temp%10));
  printString("C\n\r");
  ADC0_ISC_R |= (1<<3);
}

void Clock_Set_4MHz(void) {
  TIMER0_CTL_R &= ~(1<<0); // disable timer
  TIMER0_TAILR_R = 0x003D0900; // start counter at 4,000,000
  SYSCTL_RCC2_R |= (1<<31); // use RCC2
  SYSCTL_RCC2_R |= (1<<11); // bypass PLL
  SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x7C0) + 0x540; // select crystal and oscillator
  SYSCTL_RCC2_R &= ~0x70; // config main oscillator
  SYSCTL_RCC2_R &= ~(1<<13); // activate PLL with PWRDN
  SYSCTL_RCC2_R |= (1<<30);
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) + (0x32<<23); // selecting divisor for 4MHz
  while ((SYSCTL_RIS_R & 0x40) == 0) {}; // wait for PLL lock
  SYSCTL_RCC2_R &= ~(1<<11);
  TIMER0_CTL_R |= (1<<0); // enable timer
}

void Clock_Set_80MHz(void) {
  TIMER0_CTL_R &= ~(1<<0); // disable timer
  TIMER0_TAILR_R = 0x04C4B400; // start counter at 80,000,000
  SYSCTL_RCC2_R |= (1<<31); // use RCC2
  SYSCTL_RCC2_R |= (1<<11); // bypass PLL
  SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x7C0) + 0x540; // select crystal and oscillator
  SYSCTL_RCC2_R &= ~0x70; // config main oscillator
  SYSCTL_RCC2_R &= ~(1<<13); // activate PLL with PWRDN
  SYSCTL_RCC2_R |= (1<<30); // set DIV400
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) + (0x2<<23); // selecting divisor for 80MHz
  while ((SYSCTL_RIS_R & 0x40) == 0) {}; // wait for PLL lock
  SYSCTL_RCC2_R &= ~(1<<11); // clear bypass
  TIMER0_CTL_R |= (1<<0); // enable timer
}

void UART1_Init_4MHz(void) {
  SYSCTL_RCGCUART_R |= (1<<1); // provide clock to UART1
  SYSCTL_RCGC2_R |= (1<<1); // provide clock to GPIOB
  GPIO_PORTB_AFSEL_R = (1<<1) | (1<<0); // enable alternate functions on pins PB0 and PB1
  GPIO_PORTB_PCTL_R = (1<<0) | (1<<4); // configure PMCn fields for pins
  GPIO_PORTB_DEN_R = (1<<0) | (1<<1); // configure inputs and outputs
  UART1_CTL_R &= ~(1<<0); // disable UART1
  UART1_IBRD_R = 2; // integer portion
  UART1_FBRD_R = 11; // decimal portion
  UART1_LCRH_R = (0x3<<5) | (1<<4); // configure signal parameters 8-bits, no parity, 1-bit stop
  UART1_CC_R = 0x0; // send system clock to UART
  UART1_CTL_R = (1<<0) | (1<<8) | (1<<9); // enable UART0, Tx, and Rx
}

void UART1_Init_80MHz(void) {
  SYSCTL_RCGCUART_R |= (1<<1); // provide clock to UART1
  SYSCTL_RCGC2_R |= (1<<1); // provide clock to GPIOB
  GPIO_PORTB_AFSEL_R = (1<<1) | (1<<0); // enable alternate functions on pins PB0 and PB1
  GPIO_PORTB_PCTL_R = (1<<0) | (1<<4); // configure PMCn fields for pins
  GPIO_PORTB_DEN_R = (1<<0) | (1<<1); // configure inputs and outputs
  UART1_CTL_R &= ~(1<<0); // disable UART1
  UART1_IBRD_R = 43; // integer portion
  UART1_FBRD_R = 26; // decimal portion
  UART1_LCRH_R = (0x3<<5) | (1<<4); // configure signal parameters 8-bits, no parity, 1-bit stop
  UART1_CC_R = 0x0; // send system clock to UART
  UART1_CTL_R = (1<<0) | (1<<8) | (1<<9); // enable UART0, Tx, and Rx
}

void UART1_printChar(char c) {
  while ((UART1_FR_R & (1<<5)) != 0) {}; // wait for previous transmission to complete
  UART1_DR_R = c;
}

void printString(char * string) {
  while (*string) {
    UART1_printChar(*(string++));
  }
}

char int_To_Char(int i) {
    return (char) (i + 48);
}

void Temp_LED_Ctrl(void) {
  if ((temp >= 0) & (temp <= 17)) {
    GPIO_PORTF_DATA_R = (RED<<1);
  } else if ((temp > 17) & (temp <= 19)) {
    GPIO_PORTF_DATA_R = (BLUE<<1);
  } else if ((temp > 19) & (temp <= 21)) {
    GPIO_PORTF_DATA_R = (VIOLET<<1);
  } else if ((temp > 21) & (temp <= 23)) {
    GPIO_PORTF_DATA_R = (GREEN<<1);
  } else if ((temp > 23) & (temp <= 25)) {
    GPIO_PORTF_DATA_R = (YELLOW<<1);
  } else if ((temp > 25) & (temp <= 27)) {
    GPIO_PORTF_DATA_R = (LIGHT_BLUE<<1);
  } else if ((temp > 27) & (temp <= 40)) {
    GPIO_PORTF_DATA_R = (WHITE<<1);
  }
}