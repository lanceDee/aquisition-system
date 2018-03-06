/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   测试led
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 iSO STM32 开发板 
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */ 
	
#include "includes.h"

/*************lancedemo**************/
//#define	BUFFER_BLOCK_NUM		1			
//#define BUFFER_BLOCK_SIZE		1024
	
//OS_MEM	DataBuffer;

//uint8_t DataBufferStorage[BUFFER_BLOCK_NUM][BUFFER_BLOCK_SIZE];//数据缓冲区
//volatile uint8_t *DataBufferPtr;//缓冲区指针


char cmd[50];//存放命令的数组
uint8_t counter = 0;//串口计数
uint8_t buf_flag = 0;//缓冲区存取位置标志 0前半 1后半



/*************fatfs***************/
FATFS				fs;
FIL					fil;
//FRESULT			res;



/*************end of lancedemo*****************/

static  OS_TCB	 AppTaskStartTCB;		     //定义任务控制块
static	CPU_STK	 AppTaskStartStk[APP_TASK_START_STK_SIZE];	   //定义任务堆栈

static	OS_TCB   AppTaskGetCmdTCB;
static  CPU_STK  AppTaskGetCmdStk[APP_TASK_GET_CMD_STK_SIZE];

  OS_TCB   AppTaskSendDataTCB;
static  CPU_STK  AppTaskSendDataStk[APP_TASK_SEND_DATA_STK_SIZE];

static	OS_TCB   AppTaskSaveDataTCB;
static  CPU_STK  AppTaskSaveDataStk[APP_TASK_SAVE_DATA_STK_SIZE];

static	OS_TCB	 AppTaskGetUsageTCB;
static	CPU_STK	 AppTaskGetUsageStk[APP_TASK_GET_USAGE_STK_SIZE];

static  void   	 AppTaskStart			(void *p_arg);
static  void  	 AppTaskCreate		(void				);
static  void   	 AppTaskGetCmd		(void *p_arg);
static  void   	 AppTaskSendData	(void *p_arg);
static  void   	 AppTaskSaveData	(void *p_arg);
static	void		 AppTaskGetUsage	(void *p_arg);



/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
	OS_ERR err;	
	
	//CPU_IntDis();//BSP_IntDisAll();                           /* Disable all interrupts.  
	/* 初始化"uC/OS-III"内核 */  
	OSInit(&err);	
	
	/*创建任务*/
	OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,              // 任务控制块指针          
               (CPU_CHAR   *)"App Task Start",		          // 任务名称
               (OS_TASK_PTR )AppTaskStart, 	                  // 任务代码指针
               (void       *)0,			                      // 传递给任务的参数parg
               (OS_PRIO     )APP_TASK_START_PRIO,			  // 任务优先级
               (CPU_STK    *)&AppTaskStartStk[0],	          // 任务堆栈基地址
               (CPU_STK_SIZE)APP_TASK_START_STK_SIZE/10,	  // 堆栈剩余警戒线
               (CPU_STK_SIZE)APP_TASK_START_STK_SIZE,		  // 堆栈大小
               (OS_MSG_QTY  )5u,			                      // 可接收的最大消息队列数
               (OS_TICK     )0u,			                      // 时间片轮转时间
               (void       *)0,			                      // 任务控制块扩展信息
               (OS_OPT      )(OS_OPT_TASK_STK_CHK | 
                              OS_OPT_TASK_STK_CLR),	      // 任务选项
               (OS_ERR     *)&err);		                    // 返回值
							 
	 /*创建存储分区用作数据缓冲	lancedemo*/
//	 OSMemCreate((OS_MEM		 *)&DataBuffer,
//							 (CPU_CHAR	 *)"Data Buffer",
//							 (void			 *)&DataBufferStorage,
//							 (OS_MEM_QTY	)BUFFER_BLOCK_NUM,
//							 (OS_MEM_SIZE )BUFFER_BLOCK_SIZE,
//							 (OS_ERR		 *)&err);
	
							 
  /* 启动多任务系统，控制权交给uC/OS-III */
  OSStart(&err);                                       
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static void  AppTaskStart(void *p_arg)
{
    OS_ERR      err;
   (void)p_arg;
	
	/* 板级初始化 */	
		systick_init(72);										/*初始化Systick工作时钟*/
    BSP_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();
		BSP_Tick_Init();
		
		OS_CPU_SysTickInit(BSP_CPU_ClkFreq()/OSCfg_TickRate_Hz);
    Mem_Init();                                                 /* Initialize Memory Management Module                  */
		
#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
    
	
   // APP_TRACE_INFO(("Creating Application Tasks...\r\n"));
    AppTaskCreate();                                            /* Create Application Tasks                             */
		
		
	 /******************模拟SPI测试*********************/
	 //SPI_Test();
	 printf("Starting uCOS...\r\n");
    
	/*Delete task*/
	OSTaskDel(&AppTaskStartTCB,&err);	

}


/*
*********************************************************************************************************
*	函 数 名: AppTaskGetData
*	功能说明: 接收数据
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static	void	AppTaskGetCmd(void	*p_arg)
{
	OS_ERR	err;
	CPU_TS	ts;
	CPU_INT08S res;
	//CPU_SR_ALLOC();
	printf("GetCommandTask Created\r\n");
	
	while(1)
	{
		
		OSTaskSemPend(0,
									OS_OPT_PEND_BLOCKING,
									&ts,
									&err);//等待USART中断
		LED1_OFF;
		res = ProcessCmd(counter, cmd);
		counter = 0;
	
		switch(res)
		{
			case RES_SEND_DATA:
				OSTaskSemPost(&AppTaskSendDataTCB,
											OS_OPT_POST_NONE,
											&err);
				break;
			case RES_REQ_SIZE:
				OSTaskSemPost(&AppTaskGetUsageTCB,
											OS_OPT_POST_NONE,
											&err);
				break;
			case RES_OK:
				break;
			default:
				break;
		}
		
	}
}


/*
*********************************************************************************************************
*	函 数 名: AppTaskSendData
*	功能说明: 发送数据
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/

static	void	AppTaskSendData(void	*p_arg)
{
	OS_ERR	err;

//	CPU_SR_ALLOC();
	printf("SendDataTask Created\r\n");

	while(1)
	{
		OSTaskSemPend(0,
									OS_OPT_PEND_BLOCKING,
									0,
									&err);
		
		SendData();
		
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskSaveData
*	功能说明: 发送数据
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/

static	void	AppTaskSaveData(void	*p_arg)
{
	OS_ERR	err;

	FRESULT res;
	LED1_OFF;
	OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_DLY, &err);
	if(f_mount(0,&fs) == FR_OK)
			printf("Fatfs mount: OK!\r\n");
		else
			printf("Fatfs mount: Failed!\r\n");
	
	//DataBufferPtr = OSMemGet(&DataBuffer, &err);//获取内存块
		
	res = f_open(&fil, "0:datatest.dat", FA_CREATE_ALWAYS);
	f_close(&fil);
	if(res == FR_OK)
		printf("Succeed creating data file!\r\n");
	else if(res == FR_EXIST)
		printf("Data file already exist!\r\n");
	else
		printf("Failed to create data file! Error code：%x\r\n",res);
	
	
	while(1)
	{
		OSTaskSemPend(0,
									OS_OPT_PEND_BLOCKING,
									0,
									&err);//等待DMA中断
		SaveData();
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGetUsage
*	功能说明: 获取CPU使用率等信息
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/

static void AppTaskGetUsage(void *p_arg)
{
	OS_ERR	err;
	//CPU_SR_ALLOC
	while(1)
	{
		OSTaskSemPend(0,
									OS_OPT_PEND_BLOCKING,
									0,
									&err);
		LED1_OFF;
		f_open(&fil, "0:test.jpg", FA_OPEN_EXISTING);
		printf("filesize%ld\r\n", f_size(&fil));
		f_close(&fil);
		//OS_CRITICAL_ENTER();
		
	//	printf ( "CPU使用率：%d.%d%%\r\n", OSStatTaskCPUUsage / 100, OSStatTaskCPUUsage % 100); 
		
	//	printf ( "GetCmdTask: %2.2f%%\r\n", (float) AppTaskGetCmdTCB.CPUUsage/100);
		
	//	printf ( "SaveDataTask: %2.2f%%\r\n", (float) AppTaskSaveDataTCB.CPUUsage/100);
		
	//	printf ( "SendDataTask: %2.2f%%\r\n", (float) AppTaskSendDataTCB.CPUUsage/100);
		
	//	printf ( "GetUsageTask: %2.2f%%\r\n", (float) AppTaskGetUsageTCB.CPUUsage/100);
		
	//	printf ( "OSIdleTask: %2.2f%%\r\n", (float) OSIdleTaskTCB.CPUUsage/100);
		
		//OS_CRITICAL_EXIT();

	}
}

/*
*********************************************************************************************************
*	函 数 名: DMA1_Channel4_IRQHandler
*	功能说明: 处理DMA1中断 向存储任务发布任务信号量
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

void DMA1_Channel4_IRQHandler(void)
{
	
	OS_ERR	err;
	buf_flag = DMA_GetITStatus(DMA1_IT_TC4) == SET ? 1 : 0;
	DMA_ClearITPendingBit(DMA1_IT_HT4 | DMA1_IT_TC4);
	//__set_PRIMASK(1);//2017.2.23
	//DMA_ITConfig(DMA1_Channel4, DMA_IT_TC | DMA_IT_HT, DISABLE);//2017.2.27
	OSTaskSemPost(&AppTaskSaveDataTCB,
								OS_OPT_POST_NONE,
								&err);
}

/*
*********************************************************************************************************
*	函 数 名: USART1_IRQHandler
*	功能说明: 处理串口中断
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

void USART1_IRQHandler(void)
{
	OS_ERR	err;

	if( USART_GetFlagStatus( macUSARTx, USART_IT_RXNE) == SET)
	{
		cmd[counter++] = USART_ReceiveData( macUSARTx );
			
		if(cmd[counter-1] == '\n')
			OSTaskSemPost(&AppTaskGetCmdTCB,
										OS_OPT_POST_NONE,
										&err);
		if( USART_GetITStatus (macUSARTx, USART_IT_RXNE) != RESET)
			USART_ClearITPendingBit( macUSARTx, USART_IT_RXNE );	
	}
}
/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppTaskCreate(void)
{
	OS_ERR      err;
	
	/***********************************/
	OSTaskCreate((OS_TCB       *)&AppTaskGetCmdTCB,             
                 (CPU_CHAR     *)"App Task GET CMD",
                 (OS_TASK_PTR   )AppTaskGetCmd, 
                 (void         *)0,
                 (OS_PRIO       )APP_TASK_GET_CMD_PRIO,
                 (CPU_STK      *)&AppTaskGetCmdStk[0],
                 (CPU_STK_SIZE  )APP_TASK_GET_CMD_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_TASK_GET_CMD_STK_SIZE,
                 (OS_MSG_QTY    )1,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/***********************************/
	OSTaskCreate((OS_TCB       *)&AppTaskSendDataTCB,            
                 (CPU_CHAR     *)"App Task Send Data",
                 (OS_TASK_PTR   )AppTaskSendData, 
                 (void         *)0,
                 (OS_PRIO       )APP_TASK_SEND_DATA_PRIO,
                 (CPU_STK      *)&AppTaskSendDataStk[0],
                 (CPU_STK_SIZE  )APP_TASK_SEND_DATA_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_TASK_SEND_DATA_STK_SIZE,
                 (OS_MSG_QTY    )2,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/***********************************/
	OSTaskCreate((OS_TCB       *)&AppTaskSaveDataTCB,             
                 (CPU_CHAR     *)"App Task Save Data",
                 (OS_TASK_PTR   )AppTaskSaveData, 
                 (void         *)0,
                 (OS_PRIO       )APP_TASK_SAVE_DATA_PRIO,
                 (CPU_STK      *)&AppTaskSaveDataStk[0],
                 (CPU_STK_SIZE  )APP_TASK_SAVE_DATA_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_TASK_SAVE_DATA_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);		

	OSTaskCreate((OS_TCB       *)&AppTaskGetUsageTCB,             
                 (CPU_CHAR     *)"App Task Get Usage",
                 (OS_TASK_PTR   )AppTaskGetUsage, 
                 (void         *)0,
                 (OS_PRIO       )APP_TASK_GET_USAGE_PRIO,
                 (CPU_STK      *)&AppTaskGetUsageStk[0],
                 (CPU_STK_SIZE  )APP_TASK_GET_USAGE_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_TASK_GET_USAGE_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
}
/*********************************************END OF FILE**********************/
