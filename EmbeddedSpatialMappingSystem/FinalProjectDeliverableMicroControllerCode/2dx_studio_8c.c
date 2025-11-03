#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "PLL.h"
#include "SysTick.h"
#include "uart.h"
#include "onboardLEDs.h"
#include "VL53L1X_api.h"
#include <math.h>



#define I2C_MCS_ACK             0x00000008  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x00000008  // Acknowledge Data
#define I2C_MCS_ADRACK          0x00000004  // Acknowledge Address
#define I2C_MCS_STOP            0x00000004  // Generate STOP
#define I2C_MCS_START           0x00000002  // Generate START
#define I2C_MCS_ERROR           0x00000002  // Error
#define I2C_MCS_RUN             0x00000001  // I2C Master Enable
#define I2C_MCS_BUSY            0x00000001  // I2C Busy
#define I2C_MCR_MFE             0x00000010  // I2C Master Function Enable

#define MAXRETRIES              5           // number of receive attempts before giving up
#define M_PI										3.14159265358979323846
void I2C_Init(void){
  SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;           													// activate I2C0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;          												// activate port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};																		// ready?

    GPIO_PORTB_AFSEL_R |= 0x0C;           																	// 3) enable alt funct on PB2,3       0b00001100
    GPIO_PORTB_ODR_R |= 0x08;             																	// 4) enable open drain on PB3 only

    GPIO_PORTB_DEN_R |= 0x0C;             																	// 5) enable digital I/O on PB2,3
//    GPIO_PORTB_AMSEL_R &= ~0x0C;          																// 7) disable analog functionality on PB2,3

                                                                            // 6) configure PB2,3 as I2C
//  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00003300;
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00002200;    //TED
    I2C0_MCR_R = I2C_MCR_MFE;                      													// 9) master function enable
    I2C0_MTPR_R = 0b0000000000000101000000000111011;                       	// 8) configure for 100 kbps clock (added 8 clocks of glitch suppression ~50ns)
//    I2C0_MTPR_R = 0x3B;                                        						// 8) configure for 100 kbps clock
        
}

//The VL53L1X needs to be reset using XSHUT.  We will use PG0
void PortG_Init(void){
    //Use PortG0
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R6;                // activate clock for Port N
    while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R6) == 0){};    // allow time for clock to stabilize
    GPIO_PORTG_DIR_R &= 0x00;                                        // make PG0 in (HiZ)
  GPIO_PORTG_AFSEL_R &= ~0x01;                                     // disable alt funct on PG0
  GPIO_PORTG_DEN_R |= 0x01;                                        // enable digital I/O on PG0
                                                                                                    // configure PG0 as GPIO
  //GPIO_PORTN_PCTL_R = (GPIO_PORTN_PCTL_R&0xFFFFFF00)+0x00000000;
  GPIO_PORTG_AMSEL_R &= ~0x01;                                     // disable analog functionality on PN0

    return;
}

void PortH_Init(void){
	//Use PortH pins (PH0-PH3) for output
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7;		// activate clock for Port H
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R7) == 0){};	// allow time for clock to stabilize
	GPIO_PORTH_DIR_R |= 0x0F;        			// configure Port H pins (PH0-PH3) as output
  GPIO_PORTH_AFSEL_R &= ~0x0F;     				// disable alt funct on Port H pins (PH0-PH3)
  GPIO_PORTH_DEN_R |= 0x0F;        				// enable digital I/O on Port H pins (PH0-PH3)
																									// configure Port H as GPIO
  GPIO_PORTH_AMSEL_R &= ~0x0F;     				// disable analog functionality on Port H	pins (PH0-PH3)	
	return;
}

void PortM_Init(void){
	//Use PortM pins (PM0-PM1) for input
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11;		// activate clock for Port M
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R11) == 0){};	// allow time for clock to stabilize
	GPIO_PORTM_DIR_R &= ~0x07;        			// configure Port M pins (PM0-PM1) as input
  GPIO_PORTM_AFSEL_R &= ~0x07;     				// disable alt funct on Port M pins (PM0-PM1)
  GPIO_PORTM_DEN_R |= 0x07;        				// enable digital I/O on Port M pins (PM0-PM1)
																									// configure Port M as GPIO
  GPIO_PORTM_AMSEL_R &= ~0x07;     				// disable analog functionality on Port M	pins (PM0-PM1)	
	GPIO_PORTM_PUR_R |= 0x07;
	return;
}

void PortN_Init(void){
	//Use PortN pins for output
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;		// activate clock for Port N
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R12) == 0){};	// allow time for clock to stabilize
	GPIO_PORTN_DIR_R |= 0x07;        			// configure Port N pins (PN0-PN1) as output
  GPIO_PORTN_AFSEL_R &= ~0x07;     				// disable alt funct on Port N pins (PN0-PN1)
  GPIO_PORTN_DEN_R |= 0x07;        				// enable digital I/O on Port N pins (PN0-PN1)
																									// configure Port N as GPIO
  GPIO_PORTN_AMSEL_R &= ~0x07;     				// disable analog functionality on Port N	pins (PN0-PN1)	
	return;
}

void PortF_Init(void){
	//Use PortF pins (PF0 and PF4) for ouput
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;		// activate clock for Port F
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5) == 0){};	// allow time for clock to stabilize
	GPIO_PORTF_DIR_R |= 0x11;        			// configure Port F pins (PF0 and PF4) as output
  GPIO_PORTF_AFSEL_R &= ~0x11;     				// disable alt funct on Port F pins (PF0 and PF4)
  GPIO_PORTF_DEN_R |= 0x11;        				// enable digital I/O on Port F pins (PF0 and PF4)
																									// configure Port F as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x11;     				// disable analog functionality on Port F	pins (PF0 and PF4)	
	return;
}

void PortJ_Init(void){
	//Use PortJ pins (PJ0-PJ1) for output
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R8;		// activate clock for Port J
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R8) == 0){};	// allow time for clock to stabilize
	GPIO_PORTJ_DIR_R &= ~0x03;        			// configure Port J pins (PJ0-PJ1) as input
  GPIO_PORTJ_AFSEL_R &= ~0x03;     				// disable alt funct on Port J pins (PJ0-PJ1)
  GPIO_PORTJ_DEN_R |= 0x03;        				// enable digital I/O on Port J pins (PJ0-PJ1)
																									// configure Port J as GPIO
  GPIO_PORTJ_AMSEL_R &= ~0x03;     				// disable analog functionality on Port J	pins (PJ0-PJ1)	
	GPIO_PORTJ_PUR_R |= 0x03;
	return;
}

//XSHUT     This pin is an active-low shutdown input; 
//					the board pulls it up to VDD to enable the sensor by default. 
//					Driving this pin low puts the sensor into hardware standby. This input is not level-shifted.
void VL53L1X_XSHUT(void){
    GPIO_PORTG_DIR_R |= 0x01;                                        // make PG0 out
    GPIO_PORTG_DATA_R &= 0b11111110;                                 //PG0 = 0
    FlashAllLEDs();
    SysTick_Wait10ms(10);
    GPIO_PORTG_DIR_R &= ~0x01;                                            // make PG0 input (HiZ)
    
}

// Enable interrupts
void EnableInt(void)
{    __asm("    cpsie   i\n");
}

// Disable interrupts
void DisableInt(void)
{    __asm("    cpsid   i\n");
}

// Low power wait
void WaitForInt(void)
{    __asm("    wfi\n");
}

// Global variable visible in Watch window of debugger
// Increments at least once per button press
volatile unsigned long FallingEdges = 0;

void PortJ_Interrupt_Init(void){
	
		FallingEdges = 0;             		// Initialize counter

		GPIO_PORTJ_IS_R = 0x0;      						// (Step 1) PJ1 is Edge-sensitive 
		GPIO_PORTJ_IBE_R = 0x0;    						//     			PJ1 is not triggered by both edges 
		GPIO_PORTJ_IEV_R = 0x0;   						//     			PJ1 is falling edge event 
		GPIO_PORTJ_ICR_R = 0x2;      					// 					Clear interrupt flag by setting proper bit in ICR register
		GPIO_PORTJ_IM_R = 0x2;      					// 					Arm interrupt on PJ1 by setting proper bit in IM register
    
		NVIC_EN1_R = 0x00080000;            					// (Step 2) Enable interrupt 51 in NVIC (which is in Register EN1)
	
		NVIC_PRI12_R = 0xA0000000; 									// (Step 4) Set interrupt priority to 5

		EnableInt();																	// (Step 3) Enable Global Interrupt. lets go!
}

	
float angle = 0;
uint16_t x = 0;
uint8_t byteData, sensorState=0, myByteArray[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , i=0;
uint16_t wordData;
uint16_t Distance;
uint16_t SignalRate;
uint16_t AmbientRate;
uint16_t SpadNum; 
uint8_t RangeStatus;
uint8_t dataReady;
uint16_t dev = 0x29;			
int status=0;
int index = 0;
uint16_t distances[32] = {0};
uint16_t x_coord[32] = {0};







void TOFspin()
{
	uint32_t delay = 11;			
	for(int i=0; i< 512; i++)
	{
		if ((i%16) == 0)
		
		{
			
			
			while (dataReady == 0)
			{
				status = VL53L1X_CheckForDataReady(dev, &dataReady);
        VL53L1_WaitMs(dev, 5);
			}
		dataReady = 0;
			
	  do{
			status = VL53L1X_GetRangeStatus(dev, &RangeStatus);
			status = VL53L1X_GetDistance(dev, &Distance);
		}while(RangeStatus != 0);

		FlashLED1(1);
	
		distances[index] = Distance;
		x_coord[index] = x;
	
	  status = VL53L1X_ClearInterrupt(dev); 
		index++;
		}
		GPIO_PORTH_DATA_R = 0b00000011;
		SysTick_Wait1ms(delay);			
		GPIO_PORTH_DATA_R = 0b00000110;			
		SysTick_Wait1ms(delay);
		GPIO_PORTH_DATA_R = 0b00001100;			
		SysTick_Wait1ms(delay);
		GPIO_PORTH_DATA_R = 0b00001001;			
		SysTick_Wait1ms(delay);
	}
	
	GPIO_PORTF_DATA_R ^= 0x10;
	for(int i=0; i< 512; i++){					
		GPIO_PORTH_DATA_R = 0b00001001;
		SysTick_Wait1ms(delay);			
		GPIO_PORTH_DATA_R = 0b00001100;			
		SysTick_Wait1ms(delay);
		GPIO_PORTH_DATA_R = 0b00000110;			
		SysTick_Wait1ms(delay);
		GPIO_PORTH_DATA_R = 0b00000011;			
		SysTick_Wait1ms(delay);
	}
	GPIO_PORTF_DATA_R ^= 0x10;
	
	x+= 100;
	VL53L1X_StopRanging(dev);
}

void datatransmission()
{
	int input = 0;
	
	while(1)
		{
			input = UART_InChar();
			if (input == 's')
			{
				break;
			}
		}
	for(int i = 0; i < 32; i++) 
	{		
		// print the resulted readings to UART
		FlashLED2(1);
		sprintf(printf_buffer,"%u,%u\r\n",x_coord[i],distances[i]);
		UART_printf(printf_buffer);
	  SysTick_Wait10ms(50);
  }
}

void GPIOJ_IRQHandler(void){
  FallingEdges = FallingEdges + 1;	// Increase the global counter variable ;Observe in Debug Watch Window
																// 0.1s delay
																// Flash the LED D2 once
	angle = 0;
	index = 0;
// 1 Wait for device ToF booted
while(sensorState==0){
status = VL53L1X_BootState(dev, &sensorState);
SysTick_Wait10ms(10);
}

status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/

/* 2 Initialize the sensor with the default setting  */
status = VL53L1X_SensorInit(dev);
Status_Check("SensorInit", status);


/* 3 Optional functions to be used to change the main ranging parameters according the application requirements to get the best ranging performances */
//  status = VL53L1X_SetDistanceMode(dev, 2); /* 1=short, 2=long */
//  status = VL53L1X_SetTimingBudgetInMs(dev, 100); /* in ms possible values [20, 50, 100, 200, 500] */
//  status = VL53L1X_SetInterMeasurementInMs(dev, 200); /* in ms, IM must be > = TB */

status = VL53L1X_StartRanging(dev);   // 4 This function has to be called to enable the ranging
TOFspin();
datatransmission();
GPIO_PORTJ_ICR_R = 0x02;     					// Acknowledge flag by setting proper bit in ICR register
}
			



int main(void){
	PLL_Init();						// Default Set System Clock to 120MHz
	SysTick_Init();						// Initialize SysTick configuration
	PortH_Init();						// Initialize Port H
	PortM_Init();	
	PortN_Init();	
	PortF_Init();	
	PortJ_Init();
	PortJ_Interrupt_Init();
	
	
onboardLEDs_Init();
I2C_Init();
UART_Init();
while (1)
	{
	WaitForInt();
	}
	return 0;
/*	
	while(1)
	{
		SysTick_Wait10ms(1);
		GPIO_PORTN_DATA_R ^= 0b100;
	}
	*/
}



