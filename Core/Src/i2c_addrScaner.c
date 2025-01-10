/*
 * i2c_addrScaner.c
 *
 *  Created on: Jan 6, 2025
 *      Author: admin
 */


#include "i2c_addrScaner.h"


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	isSent =1;
	//coutinterrupt++;

}

uint8_t I2C_Check(uint16_t addr)
{

    uint8_t timeOut = 10;
    LL_I2C_DisableBitPOS(I2C1); //The POS bit is used when the procedure for reception of 2 bytes
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);//Prepare the generation of a ACKnowledge or Non ACKnowledge condition after the address receive match code
                                                 //or next received byte.
    LL_I2C_GenerateStartCondition(I2C1);//Generate a START or RESTART condition.
    while (!LL_I2C_IsActiveFlag_SB(I2C1)) { //Indicate the status of Start Bit (master mode)
        if (LL_SYSTICK_IsActiveCounterFlag()) { //This function checks if the Systick counter flag is active or not.
        	                                    //It can be used in timeout function on application side
            if (timeOut-- == 0) {
                LL_I2C_GenerateStopCondition(I2C1); //Generate a STOP condition after the current byte transfer (master mode).
                return 0;
            }
        }
    }

    LL_I2C_TransmitData8(I2C1, (addr) << 1 | I2C_REQUEST_WRITE); //Write in Transmit Data Register.
                                                                 //I2Cx: I2C Instance.
                                                                 //Data: Value between Min_Data=0x0 and Max_Data=0xFF
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {   //Indicate the status of Address sent (master mode) or Address matched flag (slave mode).
        if (LL_SYSTICK_IsActiveCounterFlag()) { //This function checks if the Systick counter flag is active or not.
                                                //It can be used in timeout function on application side
            if (timeOut-- == 0) {
                LL_I2C_GenerateStopCondition(I2C1);//Generate a STOP condition after the current byte transfer (master mode).
                return 0;
            }
        }
    }
    LL_I2C_ClearFlag_ADDR(I2C1);        //Clear Address Matched flag.
                                        //Clearing this flag is done by a read access to the I2Cx_SR1 register followed by a read access to the
                                        //I2Cx_SR2 register
    LL_I2C_GenerateStopCondition(I2C1); //Generate a STOP condition after the current byte transfer (master mode).
    return 1;
}

uint8_t I2C_Roll_Speed(uint16_t addr)
{
	LL_RCC_ClocksTypeDef rcc_clocks; //
    uint8_t idx;
    uint32_t freq;

    LL_RCC_GetSystemClocksFreq(&rcc_clocks); //Return the frequencies of different on chip clocks;
                                             //System, AHB, APB1 and APB2 buses clocks.
                                             //Each time SYSCLK, HCLK, PCLK1 and/or PCLK2 clock changes,
                                             //this function must be called to update structure fields.
                                             //Otherwise, any configuration based on this function will be incorrect.
    sprintf(CDC_tx_buff, "Scanning address: 0X%x\n", addr);
         if(isSent == 1){
    	// ++++++ Block 1 +++++++
		  HAL_UART_Transmit_IT(&huart1, dataStartBlock1, 64);
		  isSent = 0;
	         while(!isSent){
	         }
		  HAL_UART_Transmit_IT(&huart1, dataCl, 1);
		  isSent = 0;
         }
         while(!isSent){
         }
		  HAL_UART_Transmit_IT(&huart1, CDC_tx_buff, 64);
		  isSent = 0;
	         while(!isSent){
	         }
		  HAL_UART_Transmit_IT(&huart1, dataCl, 1);

    for (idx = 0; idx < 4; idx++) {
        LL_mDelay(3);
        LL_I2C_Disable(I2C1);
        LL_I2C_ConfigSpeed(I2C1, rcc_clocks.PCLK1_Frequency, I2C_speed[idx], LL_I2C_DUTYCYCLE_2);
        LL_I2C_Enable(I2C1);


        if(addr == 127){
        countLoop++;
        addr = 0;

        }

        if (I2C_Check(addr)) {
            sprintf(CDC_tx_buff, "Devise`s adores is: 0X%x\n", addr);
            //CDC_Transmit_FS(CDC_tx_buff, 7);

            if (isSent == 1) {
        		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
                //CDC_Transmit_FS("SCAN...\n\r", 9);
        		HAL_UART_Transmit_IT(&huart1, CDC_tx_buff, 64);
        		isSent == 0;
   	            while(!isSent){
   	            }
        		HAL_UART_Transmit_IT(&huart1, dataCl, 8);
        		isSent == 0;
                }

            LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13); // LED on : I2C slave found
        }
        else {
            sprintf(CDC_tx_buff, "  ---  ");
            //CDC_Transmit_FS(CDC_tx_buff, 7);

            if (isSent == 1) {
        		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
                //CDC_Transmit_FS("SCAN...\n\r", 9);
        		HAL_Delay(100);
        		isSent == 0;
                }
        }
    }
    return 0;
}

