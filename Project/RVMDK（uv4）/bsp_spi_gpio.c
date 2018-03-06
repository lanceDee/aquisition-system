#include "bsp_spi_gpio.h"


void SPI_GPIO_Init(){
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_SPI_PORT, ENABLE);
	
	GPIO_InitStructure.Mode = GPIO_MODE_Out_PP;
	GPIO_InitStructure.Pin = SPI_CS_PIN | SPI_CLK_PIN | SPI_MOSI_PIN;
	GPIO_InitStructure.Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(SPI_CS_PORT, &GPIO_InitSturcture);
	
	SPI_CS_HIGH;
}

void SPI_Send_Byte(uint8 data){
	
	SPI_CLK_LOW;
	
	for(uint8 i = 0; i < 8; ++i){
		if(data & 0x80)
			SPI_MOSI_HIGH;
		else
			SPI_MOSI_LOW;
		data <<= 1;
		__NOP;
		__NOP;
		__NOP;
		SPI_CLK_HIGH;
		__NOP;
		__NOP;
		__NOP;
		SPI_CLK_LOW;
	}
}

void SPI_Send_Cmd(uint32 cmd){
	uint8 temp;
	
	SPI_CS_LOW;
	for(uint8 i = 0; i < 4; ++i){
		temp = cmd & 0xff;
		SPI_Send_Byte(temp);
		cmd >>= 8;
	}
	
	SPI_CS_HIGH;
}