/*
 * i2c_addrScaner.h
 *
 *  Created on: Jan 6, 2025
 *      Author: admin
 */
#include "main.h"

#ifndef INC_I2C_ADDRSCANER_H_
#define INC_I2C_ADDRSCANER_H_

#define I2C_REQUEST_WRITE 0x00 //I2C scanner



//++++ I2C scanner
//uint8_t dataStartBlock1[] = "Block #1\n";

uint8_t dataCl[]="\r";
uint8_t dataStartBlock1[] = "Block #1\n";

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

uint8_t CDC_tx_buff[64];
uint8_t CDC_rx_buff[8];
uint8_t CDC_rx_flag = 0;
uint32_t I2C_speed[4] = { 100000, 200000, 300000, 400000 };

uint8_t countLoop = 0;

int isSent =1;


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
uint8_t I2C_Check(uint16_t addr);
uint8_t I2C_Roll_Speed(uint16_t addr);

#endif /* INC_I2C_ADDRSCANER_H_ */
