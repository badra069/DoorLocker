/******************************************************************************
 *
 * Module: uart , i2c , eeprom , dc motors
 *
 * File Name: application.h
 *
 * Description: app src file
 *
 * Author: Ahmed Mohamed
 *
 *******************************************************************************/
/*********************************************************************************
 * 									INCLUDES									 *
 *********************************************************************************/
#include"../MCAL/uart.h"
#include"../ECUAL/external_eeprom.h"
#include"../ECUAL/dc_motor.h"
#include"../MCAL/timer1.h"
/***************************************************************************************
 * 									GLOBAL VARIABLES									*
 ***************************************************************************************/
#define NEW_PASSWORD		0x65
#define PASSWORD_ADDRESS 	0x0010
#define MC2_READY			0xFC
/* to inform MC1 that MC2 ready to receive */
#define MC1_READY			0xAB
/* to inform MC2 that MC1 ready to receive */
/*****************************************************************************
 *  so the protocol goes as follows
 * 1. if MC1 finished some code and want to transmit byte or string to MC2
 * 	it must check at MC2_READY flag , once it's received it can transmit the data to MC2
 * 2. if MC2 finished some code and want to transmit byte or string to MC2
 * 	it must check at MC1_READY flag , once it's received it can transmit the data to MC1
 *****************************************************************************/
uint8 password[20];
uint16 i;
uint8 DELAY_DONE;
/*********************************************************************************
 * 									APPLICATION									 *
 *********************************************************************************/

/****************************DESCRIPTION*********************************
 * the user interface Mcu that takes the inputs from the keypad
 * and displays the status to the user(interact with the user)
 *
 * 1. at the start it checks for a password exist in eeprom or not
 * 2. if the password exist it will show the options list
 * 3. if not it will ask for a new password
 * 4. if option change password -> ask for new password , confirm , and 3 trials
 * 5. then return to options list
 * 6. if option open door -> enter the pass , 3 trials
 * 7. if password match -> rotate the motor clockwise 15 seconds
 * 		then rotate it unti-cw for 15 seconds indicating the closing again
 * 		during this step the lcd displaying th status of the door -> opening or closing
 * 8. any check for password has 3 trials if exceeded it will activate the buzzer for 60 sec
 ************************************************************************/

void Mc1_init(void)
{
	/**************************************************
	 * [name] : UART_ConfigType
	 * [Type] : Structure
	 * [Function] : UART Module Dynamic configuration
	 * [Members] :
	 * 			Parity_enable enable or disable
	 * 			Parity_type odd disable or even
	 * 			stop_bit 1 or 2
	 * 			character_size 5,6,7,8bits char
	 * 			speed x or U2x
	 * 			type  Sync or Async
	 ***************************************************/

	UART_ConfigType UART_configStruct = {	UART_PARITY_BIT_DISABLE ,
			UART_PARITY_DISABLE ,
			UART_1_STOP_BIT ,
			UART_8_BIT ,
			UART_2X ,
			UART_ASYNCHRONOUS_OPERATION ,};

	UART_init(&UART_configStruct);
}
#if FALSE
void countDown(void)
{
	/* printing the current remaining seconds to count in the lcd */
	g_t1tick--;
	PORTB ^= (1 << 7);
	if(!g_t1tick)
	{
		DELAY_DONE = TRUE;
	}

}
void TIMER1_delay_init(void)
{

	/******************************************************
	 * [name] : TIMER1_configType
	 * [Type] : Structure
	 * [Function] : TIMER1 Module Dynamic configuration
	 * [Members] :
	 * 			mode TIMER1_NORMAL or TIMER1_CTC (16bit only so it's not a conig for me)
	 * 			output_mode TIMER1_NORMAL_OUTPUT or TIMER1_TOGGLE_OUTPUT etc..
	 * 			compare_interrupt enable or disable
	 * 			overflow_interrupt enable or disable
	 * 			compare_value 0 -> 65535
	 * 			initial_value 0 -> 65535
	 ***************************************************/



	TIMER1_configType TIMER1_configStruct = { 	TIMER1_CTC ,
			TIMER1_F_CPU_1024 ,
			ENABLE ,
			DISABLE ,
			7812 ,
			0	};
	TIMER1_init(&TIMER1_configStruct);
	TIMER1_setCallBackCompareMode(countDown);
	TIMER1_stop();
}
void TIMER1_delay(uint8 seconds)
{
	DELAY_DONE = FALSE;
	g_t1tick =seconds;
	//LCD_clearScreen();
	//LCD_displayString("hey!");
	TIMER1_start(TIMER1_F_CPU_1024);
	while(!DELAY_DONE){}
}
#endif
/***************************************************************************************
 * 									MAIN  FUNCTION										*
 ***************************************************************************************/

/*****************************************
 *
  	//for MC1 receive and MC2 transmit
 	UART_sendByte(MC2_READY);
	while(UART_receiveByte() != MC1_READY){}

	//for MC1 transmit and MC2 receive
	while(UART_receiveByte() != MC1_READY){}
	send(MC2_READY);
 *****************************************/

int main(void)/*MCU2*/
{
	/*initializaiton code*/
	//TIMER1_delay_init();
	EEPROM_init();
	DCMOTOR_init();
	Mc1_init();
	SET_BIT(DDRD , 3);
	CLEAR_BIT(PORTD , 3);
#if FALSE
	/* set the manufacturer password (DEFAULT)*/
	uint8 default_password[20] = "444444";
	for(uint16 i = 0 ; i < 19 ; ++i)
	{
		EEPROM_writeByte(PASSWORD_ADDRESS + i , default_password[i]);
		_delay_ms(10);
	}
	_delay_ms(300);
#endif

	/* getting the stored password */
	uint16 i = 0;
	for (i = 0; i < 19; ++i) {
		EEPROM_readByte(PASSWORD_ADDRESS + i , password + i);
		_delay_ms(10);
		if(password[i] == '\0')
		{
			password[i] = '#';
			break;
		}
	}
	for(uint8 j = i + 1 ; j <20 ; j++)
	{
		password[j] = '\0';
	}
	//for MC1 receive and MC2 transmit
	while(UART_receiveByte() != MC1_READY){}
	UART_sendString(password);

	//	UART_sendByte(MC2_READY);

	//		UART_sendByte(MC2_READY);
	//_delay_ms(500);
	while(TRUE)
	{
		/* Application code*/
		PORTD ^=(1<<3);
		while(UART_receiveByte() != MC1_READY){}
		if(UART_receiveByte() == NEW_PASSWORD)
		{
			/* set the user password */
			UART_sendByte(MC2_READY);
			UART_receiveString(password);
			int i = 0;
			while(password[i] != '\0')
			{
				i++;
			}
			if(i < 20)
			{
				password[i] = '#';
			}
			else
			{
				password[19] = '#';
			}
			for(uint16 i = 0 ; i < 19 ; ++i)
			{
				EEPROM_writeByte(PASSWORD_ADDRESS + i , password[i]);
				_delay_ms(10);
			}
			_delay_ms(100);
			SET_BIT(PORTD , 3);
			_delay_ms(3000);
			CLEAR_BIT(PORTD ,  3);
			UART_sendByte(MC2_READY);
		}
		else
		{
			//UART_sendByte(MC2_READY);
			DCMOTOR_move();
			DCMOTOR_setSpeed(/*MAX*/);
			_delay_ms(5000);
			DCMOTOR_toggleMove();
			_delay_ms(5000);
			DCMOTOR_stop();
			UART_sendByte(MC2_READY);
		}
	}
}
