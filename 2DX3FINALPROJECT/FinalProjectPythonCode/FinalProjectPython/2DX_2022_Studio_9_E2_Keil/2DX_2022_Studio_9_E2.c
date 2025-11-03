/* Half-Duplex Serial Communication Example for 2DX4


		Written by Tom Doyle
		Last Updated: March 28, 2020
*/

#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "PLL.h"
#include "SysTick.h"
#include "uart.h"
#include "onboardLEDs.h"
#include "stdio.h"

int main(void) {
	char i;
	char TxChar;
	int input = 0;
	int data_array [3];
	PLL_Init();	
	SysTick_Init();
	onboardLEDs_Init();
	UART_Init();

	// always wait for the transmission code from pc. if 's' recieved then send data	
	while(1){		
		//wait for the right transmition initiation code
		while(1){
			input = UART_InChar();
			if (input == 's')
				break;
		}
		
		// simulating the transmission of 10 measurements
		for(i = 0; i < 10; i++) {
			
			//use systick register value as random number for data
			data_array[0] = 5592;
			data_array[1] = 56429;
			data_array[2] = 43;
			
			//write the data into string format
			// for your ToF sensor, you will have to modify this to meet your project specification
			sprintf(printf_buffer,"%d,  %d, %d, %d\r\n"
														,i,data_array[0],data_array[1], data_array[2]);
			//send string to uart
			UART_printf(printf_buffer);
			FlashLED3(1);
			SysTick_Wait10ms(50);
		}
	}
}



