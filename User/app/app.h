#ifndef	_APP_H_
#define	_APP_H_
#include <Types.h>
/**
ProcessCmd（）处理后的返回值定义
*/
#define RES_ERROR		-1
#define RES_OK			0
#define RES_SEND_DATA		1
#define RES_REQ_SIZE	2//获取文件大小


/**
命令定义
*/
#define CMD_SEND_DATA	0x01
#define CMD_SAVE_DATA 0x02
#define CMD_STOP			0x03



/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void 	 SPI2_Configuration(void);
int8	 ProcessCmd(uint8 counter, char *cmd);
void	 GetCmd(char *chpt);
void	 SendData(void);
void	 SaveData(void);
void	 I2CTest(void);
#endif	//_APP_H_
