#include "includes.h"
//#include <string.h>

#define	BUFFER_SIZE	1024*20

//extern volatile uint16_t *DataBufferPtr;
uint8 DataBuffer[BUFFER_SIZE];
extern FIL	fil;
extern uint8_t buf_flag;
uint32_t bw;
uint32_t count;//字节统计
uint8_t FILE_OPENED = 0;//文件是否已经打开的标志
uint8_t STOP_FLAG = 0;//是否收到停止命令的标志

static void SPI2_GPIO_Config(void)
{
	
  GPIO_InitTypeDef GPIO_InitStructure;
	
	
  /* Enable SPI1 and GPIO clocks */
  /*!< SPI_FLASH_SPI_CS_GPIO, SPI_FLASH_SPI_MOSI_GPIO, 
       SPI_FLASH_SPI_MISO_GPIO, SPI_FLASH_SPI_DETECT_GPIO 
       and SPI_FLASH_SPI_SCK_GPIO Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
 
  
  /*!< Configure SPI_FLASH_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure SPI_FLASH_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure SPI_FLASH_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}

/*static void SPI2_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	
	NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}*/

static void DMA_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

static void SPI2_Mode_Config(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	  /*!< SPI_FLASH_SPI Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	 /* SPI2 configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//2017.3.14
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;//lancedemo 2017.5.22
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;//2017.5.15 lancedemo 从机频率需小于主机
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

	//SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);//2017.2.22
	
  SPI_Cmd(SPI2, DISABLE);
}



static void SPI2_DMA_Config(void)
{
	DMA_InitTypeDef	DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	DMA_DeInit(DMA1_Channel4);
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//2017.5.9 lancedemo
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)(&SPI2->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)DataBuffer;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);
	
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC | DMA_IT_HT, ENABLE);
	//DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	
	DMA_Cmd(DMA1_Channel4, ENABLE);//2017.5.7
}
void SPI2_Configuration(void)
{
	SPI2_GPIO_Config();
	SPI2_Mode_Config();
	DMA_NVIC_Config();
	//SPI2_NVIC_Config();//2017.2.21
	SPI2_DMA_Config();
}

/**
  * @brief 字符串转数字 将字符串形式IP配置信息转换为数字型
	* @param  src: 源字符串地址
	* @param	des: 目的数组地址
	* @retval	none
  */
void IPTrans(char *src, uint8 *des)
{
	int i = 0;
	char *tmp = strtok(src, ".");
	while(tmp != NULL)
	{
		des[i++] = atoi(tmp);
		tmp = strtok(NULL, ".");
	}
}

int8 ProcessCmd(uint8 counter, char *cmd)
{
			cmd[counter-2] = '\0';
			//printf("Command recieved: %s\r\n",cmd);
			if(!strcmp(cmd,"save"))
			{
				printf("Saving data...\r\n");
				//DMA_Cmd(DMA1_Channel4, ENABLE);//2017.2.23
				SPI_Cmd(SPI2, ENABLE);
			}
			else if(strstr(cmd, "server"))
			{
				
				char *temp, *server_ip, *server_port;
				DMA_Cmd(DMA1_Channel4, DISABLE);//接收到命令后停止SPI2的数据传输
				//printf("Configuring server...\r\n");
				temp = strtok(cmd, "/");
				temp = strtok(NULL, "/");
				server_ip = temp;
				temp = strtok(NULL, "/");
				server_port = temp;
				IPTrans(server_ip, remote_ip);
				remote_port = atoi(server_port);
				write_server_config_to_eeprom(remote_ip, remote_port);
				printf("Server Info: %d.%d.%d.%d:%d\r\n", remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], remote_port);
				
			}
			else if(!strcmp(cmd, "send"))
				return RES_SEND_DATA;
			else if(!strcmp(cmd,"stop"))
			{
				SPI_Cmd(SPI2, DISABLE);
				STOP_FLAG = 1;
				f_close(&fil);
				FILE_OPENED = 0;
				//disconnect(SOCK_TCPS);
				printf("Data transmission abort!\r\n");
			}
			//查询文件大小
			else if(!strcmp(cmd,"fsize"))
				return RES_REQ_SIZE;
			else if(strstr(cmd, "netcfg"))
			{
				int i = 0;
				char *ori_info = strtok(cmd, "/");//得到的原始字符串形式配置信息
				char *pro_info[5];//存放处理后的配置信息
				
				uint8 ip[4];//整型的网络信息
				uint8 sub[4];
				uint8 gw[4];
				
				ori_info = strtok(NULL, "/");//丢弃第一个子字符串
				while(ori_info != NULL)
				{
					pro_info[i++] = ori_info;
					ori_info = strtok(NULL, "/");
				}
				//printf("IP: %s\r\nSUB: %s\r\nGW: %s\r\n",pro_info[0],pro_info[1],pro_info[2]);
				IPTrans(pro_info[0], ip);
				IPTrans(pro_info[1], sub);
				IPTrans(pro_info[2], gw);
				IPTrans(pro_info[3], remote_ip);
				remote_port = atoi(pro_info[4]);
				/*将IP配置信息写入W5500相应寄存器并保存到EEPROM*/	
				reset_w5500_ip(ip, gw, sub, remote_ip, remote_port);
				getSIPR (ip);			
				printf("W5500 IP Address: %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
				getSUBR(sub);
				printf("W5500 Netmask: %d.%d.%d.%d\r\n", sub[0],sub[1],sub[2],sub[3]);
				getGAR(gw);
				printf("W5500 gw: %d.%d.%d.%d\r\n", gw[0],gw[1],gw[2],gw[3]);
				printf("Server Info: %d.%d.%d.%d:%d", remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], remote_port);
				
			}
			else 
			{
				printf("Invalid command!\r\n");
				return RES_ERROR;
			}
			
			return RES_OK;
		
}


void SendData()
{
	FRESULT res;
	UINT br;
	uint8 databuf[512];
	uint8 timeout_count = 0;
	//uint8 state;
	if(getSn_SR(SOCK_TCPS) != SOCK_ESTABLISHED)
	{
		printf("Connecting TCP server...\r\n");
		socket(SOCK_TCPS, Sn_MR_TCP, local_port, Sn_MR_ND);
		connect(SOCK_TCPS, remote_ip, remote_port);
	}
	while(getSn_SR(SOCK_TCPS) != SOCK_ESTABLISHED)
	
	{
		
		if(timeout_count == 10)
		{
			printf("Timeout!\r\n");
			return;
		}
		delay_s(1);
		timeout_count++;
		
	}
	
	printf("Server connected, sending data...\r\n");
	res = f_open(&fil, "0:test.jpg", FA_OPEN_EXISTING | FA_READ);
	if(res == FR_OK)
	{ 
		while(getSn_SR(SOCK_TCPS) == SOCK_ESTABLISHED && !STOP_FLAG)                           
		{
			res = f_read(&fil, databuf, 512, &br);
			send(SOCK_TCPS, databuf, br);
			if(br < 512)
			{
				f_close(&fil);
				printf("Data file transmission complete!\r\n");
				return;
			}
			f_sync(&fil);
		}
		if(getSn_SR(SOCK_TCPS) != SOCK_ESTABLISHED)		//判断停止原因
			printf("Disconnected from TCP server!\r\n");
		STOP_FLAG = 0;
		close(SOCK_TCPS);
		return;
	}
	else
	{
		printf("Failed to open data file: ");
		switch(res)
		{
			case FR_NOT_READY : 
				printf("No SD card insert!\r\n");
				break;
			case FR_NO_FILE : 
				printf("File not exist!\r\n");
				break;
			case FR_NO_PATH : 
				printf("Invalid path! \r\n");
				break;
			case FR_INVALID_NAME :	
				printf("Invalid file name!\r\n"); 
				break;
			default : 
				break;
		}
		//delay_ms(100);
	}
}

void SaveData()
{
	uint16 offset;
	FRESULT res = FR_OK;
	uint8 *wptr;
		
	LED1_OFF;
	
	offset = buf_flag ? BUFFER_SIZE/2 : 0;//通过偏移量选择缓冲区域
	
	//DMA_ClearITPendingBit(DMA1_IT_HT4 | DMA1_IT_TC4);
	
	if(!FILE_OPENED)
	{
		res = f_open(&fil, "datatest.dat", FA_OPEN_EXISTING | FA_WRITE);
		FILE_OPENED = 1;
	}
	/*
	if(res == FR_OK)
	{
		uint8 *wptr = DataBuffer + offset;//目标写入指针
		uint8 i = BUFFER_SIZE/1024;//需要写几个512字节
		
		while(i--)
		{
			f_write(&fil, wptr, 512, &bw);
			wptr += 512;
			count+=bw;
		}
	}
	*/
	wptr = DataBuffer + offset;

	f_write(&fil, wptr, BUFFER_SIZE/2, &bw);
	f_sync(&fil);
	printf("%d bytes stored!\r\n", bw);
}



/************************i2c2命令发送相关***********************/
/*
void I2C2_GPIO_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;//PB10 PB11 : SCL SDA
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}



void I2C2_Mode_Config()
{
	I2C_InitTypeDef I2C_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_ClockSpeed = 200000;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable; 
	
	I2C_Init(I2C2, &I2C_InitStructure);
}

void I2C2_Configuration()
{
	I2C2_GPIO_Config();
	I2C2_Mode_Config();
	I2C2->TRISE = 37;
	I2C_Cmd(I2C2, ENABLE);
}

void I2CSendCmd(int cmd)
{
	
}

#define I2C2_Slave_Address 0x04

void I2CTest()
{
	I2C2_Configuration();
	printf("starting I2C test...");
	I2C_GenerateSTART(I2C2, ENABLE);
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C2, I2C2_Slave_Address, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	I2C_SendData(I2C2, 0x61);
	I2C_GenerateSTOP(I2C2, ENABLE);
}

*/








