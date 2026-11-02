/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "debug_cli.h"

#include "vehicle_state_mgr.h"
#include "body_state_mgr.h"
#include "command_mgr.h"
#include "can_codec.h"
#include "can_matrix.h"
/* Khai báo các hàm từ platform/can_if/ */
extern ServiceResult_t CanIf_InitAndStart(void);
extern void            CanScheduler_Step10ms(void);
extern void UartPort_StartReceive(void);

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
  osMessageQueueId_t canRxQueueHandle;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for CanRxTask */
osThreadId_t CanRxTaskHandle;
const osThreadAttr_t CanRxTask_attributes = {
  .name = "CanRxTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 512 * 4
};
/* Definitions for VehicleTask */
osThreadId_t VehicleTaskHandle;
const osThreadAttr_t VehicleTask_attributes = {
  .name = "VehicleTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 512 * 4
};
/* Definitions for CanTxTask */
osThreadId_t CanTxTaskHandle;
const osThreadAttr_t CanTxTask_attributes = {
  .name = "CanTxTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 512 * 4
};
/* Definitions for DiagTask */
osThreadId_t DiagTaskHandle;
const osThreadAttr_t DiagTask_attributes = {
  .name = "DiagTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1028 * 4
};
/* Definitions for HealthTask */
osThreadId_t HealthTaskHandle;
const osThreadAttr_t HealthTask_attributes = {
  .name = "HealthTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for NvmTask */
osThreadId_t NvmTaskHandle;
const osThreadAttr_t NvmTask_attributes = {
  .name = "NvmTask",
  .priority = (osPriority_t) osPriorityBelowNormal,
  .stack_size = 512 * 4
};
/* Definitions for CliTask */
osThreadId_t CliTaskHandle;
const osThreadAttr_t CliTask_attributes = {
  .name = "CliTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartCanRxTask(void *argument);
void StartVehicleTask(void *argument);
void StartCanTxTask(void *argument);
void StartDiagTask(void *argument);
void StartHealthTask(void *argument);
void StartNvmTask(void *argument);
void StartCliTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
}
/* USER CODE END 5 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	/* Tạo hàng đợi nhận CAN (16 phần tử, mỗi phần tử 16 bytes) */
	canRxQueueHandle = osMessageQueueNew(16U, 16U, NULL);
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of CanRxTask */
  CanRxTaskHandle = osThreadNew(StartCanRxTask, NULL, &CanRxTask_attributes);

  /* creation of VehicleTask */
  VehicleTaskHandle = osThreadNew(StartVehicleTask, NULL, &VehicleTask_attributes);

  /* creation of CanTxTask */
  CanTxTaskHandle = osThreadNew(StartCanTxTask, NULL, &CanTxTask_attributes);

  /* creation of DiagTask */
  DiagTaskHandle = osThreadNew(StartDiagTask, NULL, &DiagTask_attributes);

  /* creation of HealthTask */
  HealthTaskHandle = osThreadNew(StartHealthTask, NULL, &HealthTask_attributes);

  /* creation of NvmTask */
  NvmTaskHandle = osThreadNew(StartNvmTask, NULL, &NvmTask_attributes);

  /* creation of CliTask */
  CliTaskHandle = osThreadNew(StartCliTask, NULL, &CliTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  Cli_Init();
  UartPort_StartReceive(); /* Bật ngắt UART1 lắng nghe bàn phím */

  VehicleStateMgr_Init();
  BodyStateMgr_Init();
  CommandMgr_Init();
  (void)CanIf_InitAndStart();
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartCanRxTask */
/**
* @brief Function implementing the CanRxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCanRxTask */
void StartCanRxTask(void *argument)
{
  /* USER CODE BEGIN StartCanRxTask */
  /* Infinite loop */
	typedef struct {
	      uint32_t can_id;
	      uint8_t  dlc;
	      uint8_t  data[8];
	  } RawCanFrame_t;

	  RawCanFrame_t rx_frame;
	  for(;;)
	  {
	    /* Ngủ chờ sự kiện từ canRxQueueHandle (timeout = osWaitForever) */
	    if (osMessageQueueGet(canRxQueueHandle, &rx_frame, NULL, osWaitForever) == osOK)
	    {
	      if (rx_frame.can_id == CAN_ID_REMOTE_COMMAND) /* 0x400 */
	      {
	        RemoteCommandReq_t cmd_req;
	        CanCodec_DecodeRemoteCommand(rx_frame.data, rx_frame.dlc, &cmd_req);
	        CommandMgr_ProcessRemoteCommand(&cmd_req);
	      }
	    }
	  }
  /* USER CODE END StartCanRxTask */
}

/* USER CODE BEGIN Header_StartVehicleTask */
/**
* @brief Function implementing the VehicleTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartVehicleTask */
void StartVehicleTask(void *argument)
{
  /* USER CODE BEGIN StartVehicleTask */
  /* Infinite loop */
	uint32_t wake_tick = osKernelGetTickCount();
	  for(;;)
	  {
	    VehicleStateMgr_Step10ms();
	    BodyStateMgr_Step10ms();
	    wake_tick += 10U;
	    osDelayUntil(wake_tick);
	  }
  /* USER CODE END StartVehicleTask */
}

/* USER CODE BEGIN Header_StartCanTxTask */
/**
* @brief Function implementing the CanTxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCanTxTask */
void StartCanTxTask(void *argument)
{
  /* USER CODE BEGIN StartCanTxTask */
  /* Infinite loop */
	uint32_t wake_tick = osKernelGetTickCount();
	  for(;;)
	  {
	    CanScheduler_Step10ms();
	    wake_tick += 10U;
	    osDelayUntil(wake_tick);
	  }
  /* USER CODE END StartCanTxTask */
}

/* USER CODE BEGIN Header_StartDiagTask */
/**
* @brief Function implementing the DiagTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDiagTask */
void StartDiagTask(void *argument)
{
  /* USER CODE BEGIN StartDiagTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDiagTask */
}

/* USER CODE BEGIN Header_StartHealthTask */
/**
* @brief Function implementing the HealthTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartHealthTask */
void StartHealthTask(void *argument)
{
  /* USER CODE BEGIN StartHealthTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartHealthTask */
}

/* USER CODE BEGIN Header_StartNvmTask */
/**
* @brief Function implementing the NvmTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartNvmTask */
void StartNvmTask(void *argument)
{
  /* USER CODE BEGIN StartNvmTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartNvmTask */
}

/* USER CODE BEGIN Header_StartCliTask */
/**
* @brief Function implementing the CliTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCliTask */
__weak void StartCliTask(void *argument)
{
  /* USER CODE BEGIN StartCliTask */
  /* Infinite loop */
	uint32_t wake_tick = osKernelGetTickCount();
	  for(;;)
	  {
	    /* Gọi hàm xử lý dòng lệnh của tầng Application */
	    Cli_Process();

	    /* Chạy chu kỳ 50ms (Vì UART gõ phím rất chậm, không cần chạy liên tục) */
	    wake_tick += 50U;
	    osDelayUntil(wake_tick);
	  }
  /* USER CODE END StartCliTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

