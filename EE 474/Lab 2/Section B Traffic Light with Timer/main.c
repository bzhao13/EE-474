#include <tm4c123gh6pm.h>
#include <stdint.h>

void Switch_Init(void);
void Traffic_State_Ctrl(void);
void Light_Timer(void);
void Light_Timer_Init(void);
void Button_Timer(void);
void Button_Timer_Init(void);
void Restart_Button_Timer(void);
void Restart_Light_Timer(void);
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

volatile int light_counter = 0;
volatile int button_counter = 0;

int main(void) {
  Switch_Init();

  Light_Timer_Init();
  Button_Timer_Init();

  while (1) {
    Traffic_State_Ctrl();

    Light_Timer();
    Button_Timer();
  }
  return 0;
}

void Traffic_State_Ctrl(void) {
  if (state == 0) {
    Green_On();
    if ((button_counter == 2) & (Ped_Cross() == 0x08)) {
      state = 1;
      Restart_Light_Timer();
    } else if (light_counter == 5) {
      state = 2;
      Restart_Light_Timer();
    }
  } else if (state == 1) {
    Yellow_On();
    if (light_counter == 5) {
      state = 2;
      Restart_Light_Timer();
    }
  } else if (state == 2) {
    Red_On();
    if (light_counter == 5) {
      state = 0;
      Restart_Light_Timer();
    }
  } else if (state == 3) {
    if ((button_counter == 2) & (Start_Stop() == 0x04)) {
      state = 2;
      Restart_Light_Timer();
    }
  }
}

void Light_Timer(void) {
  if ((TIMER0_RIS_R & 0x01) == 1) {
      TIMER0_ICR_R |= (1<<0);
      light_counter++;
    }
}

void Button_Timer(void) {
  if ((Ped_Cross() == 0x08) | (Start_Stop() == 0x04)) {
    TIMER1_CTL_R |= (1<<0); // enable timer
  } else {
    button_counter = 0;
    TIMER1_CTL_R |= (0<<0); // disable timer
    TIMER1_TAILR_R = 0x00F42400; // start counter at 16,000,000
  }

  if ((TIMER1_RIS_R & 0x01) == 1) {
      TIMER1_ICR_R |= (1<<0);
      button_counter++;
    }
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

void Light_Timer_Init(void) {
  SYSCTL_RCGCTIMER_R |= 0x01; // enable timer 0
  TIMER0_CTL_R |= (0<<0); // disable timer
  TIMER0_CFG_R |= 0x00000000; // set 32-bit config
  TIMER0_TAMR_R |= (0x2<<0); // set TAMR to periodic timer mode
  TIMER0_TAMR_R |= (0<<4); // timer countdown
}

void Restart_Light_Timer(void) {
  light_counter = 0;
  TIMER0_CTL_R |= (0<<0); // disable timer
  TIMER0_TAILR_R = 0x00F42400; // start counter at 16,000,000
  TIMER0_CTL_R |= (1<<0); // enable timer
}

void Button_Timer_Init(void) {
  SYSCTL_RCGCTIMER_R |= 0x02; // enable timer 1
  TIMER1_CTL_R |= (0<<0); // disable timer
  TIMER1_CFG_R |= 0x00000000; // set 32-bit config
  TIMER1_TAMR_R |= (0x2<<0); // set TAMR to periodic timer mode
  TIMER1_TAMR_R |= (0<<4); // timer countdown

}

void Restart_Button_Timer(void) {
  TIMER1_CTL_R |= (0<<0); // disable timer
  TIMER1_TAILR_R = 0x00F42400; // start counter at 16,000,000
  TIMER1_CTL_R |= (1<<0); // enable timer
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