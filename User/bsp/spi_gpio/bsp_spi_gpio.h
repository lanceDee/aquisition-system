//gpio模拟spi串口

#include <stm32f10x_gpio.h>
#include <Types.h>

//端口定义
#define 	RCC_SPI_PORT		RCC_APB2Periph_GPIOC

#define		SPI_CLK_PORT		GPIOC //PC0 PC2 PC3
#define 	SPI_CLK_PIN			GPIO_Pin_0

#define 	SPI_MOSI_PORT		GPIOC
#define 	SPI_MOSI_PIN		GPIO_Pin_2

#define 	SPI_CS_PORT		 	GPIOC
#define 	SPI_CS_PIN			GPIO_Pin_3

#define		SPI_CLK_HIGH		GPIO_SetBits(SPI_CLK_PORT, SPI_CLK_PIN)
#define 	SPI_CLK_LOW			GPIO_ResetBits(SPI_CLK_PORT, SPI_CLK_PIN)

#define		SPI_MOSI_HIGH		GPIO_SetBits(SPI_MOSI_PORT, SPI_MOSI_PIN)
#define 	SPI_MOSI_LOW		GPIO_ResetBits(SPI_MOSI_PORT, SPI_MOSI_PIN)

#define		SPI_CS_HIGH			GPIO_SetBits(SPI_CS_PORT, SPI_CS_PIN)
#define 	SPI_CS_LOW			GPIO_ResetBits(SPI_CS_PORT, SPI_CS_PIN)

void SPI_Send_Byte(uint8 data);
void SPI_Send_Cmd(uint32 cmd);//发送32位命令
void SPI_GPIO_Init(void);
void SPI_Test(void);
