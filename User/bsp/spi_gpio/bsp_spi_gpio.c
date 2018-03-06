#include "bsp_spi_gpio.h"
#include "bsp_usart1.h"

void SPI_GPIO_Init()
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_SPI_PORT, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = SPI_CS_PIN | SPI_CLK_PIN | SPI_MOSI_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(SPI_CS_PORT, &GPIO_InitStructure);
	
	SPI_CS_HIGH;
}

void SPI_Send_Byte(uint8 data)
{
	uint8 i;
	
	SPI_CLK_LOW;
	
	for(i = 0; i < 8; ++i)
	{
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

void SPI_Send_Cmd(uint32 cmd)
{
	uint8 i;
	
	uint8 *temp = ((uint8 *) &cmd) + 3;
	
	SPI_CS_LOW;
	for(i = 0; i < 4; ++i)
	{
		
		SPI_Send_Byte(*temp);
		--temp;

	}
	
	SPI_CS_HIGH;
}


void SPI_Test()
{
	printf("\r\nstarting spi_gpio test...\r\n");
	SPI_GPIO_Init();
	SPI_Send_Cmd(0x61626364);
	SPI_CS_LOW;
	SPI_Send_Byte('\n');
	SPI_CS_HIGH;
}


