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
void StartTask02(void *argument);
void StartTask03(void *argument);
void StartTask04(void *argument);
void StartTask05(void *argument);
void StartTask06(void *argument);
void StartTask07(void *argument);
void StartTask08(void *argument);

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
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of CanRxTask */
  CanRxTaskHandle = osThreadNew(StartTask02, NULL, &CanRxTask_attributes);

  /* creation of VehicleTask */
  VehicleTaskHandle = osThreadNew(StartTask03, NULL, &VehicleTask_attributes);

  /* creation of CanTxTask */
  CanTxTaskHandle = osThreadNew(StartTask04, NULL, &CanTxTask_attributes);

  /* creation of DiagTask */
  DiagTaskHandle = osThreadNew(StartTask05, NULL, &DiagTask_attributes);

  /* creation of HealthTask */
  HealthTaskHandle = osThreadNew(StartTask06, NULL, &HealthTask_attributes);

  /* creation of NvmTask */
  NvmTaskHandle = osThreadNew(StartTask07, NULL, &NvmTask_attributes);

  /* creation of CliTask */
  CliTaskHandle = osThreadNew(StartTask08, NULL, &CliTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
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

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the CanRxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the VehicleTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief Function implementing the CanTxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void *argument)
{
  /* USER CODE BEGIN StartTask04 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask04 */
}

/* USER CODE BEGIN Header_StartTask05 */
/**
* @brief Function implementing the DiagTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask05 */
void StartTask05(void *argument)
{
  /* USER CODE BEGIN StartTask05 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask05 */
}

/* USER CODE BEGIN Header_StartTask06 */
/**
* @brief Function implementing the HealthTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask06 */
void StartTask06(void *argument)
{
  /* USER CODE BEGIN StartTask06 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask06 */
}

/* USER CODE BEGIN Header_StartTask07 */
/**
* @brief Function implementing the NvmTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask07 */
void StartTask07(void *argument)
{
  /* USER CODE BEGIN StartTask07 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask07 */
}

/* USER CODE BEGIN Header_StartTask08 */
/**
* @brief Function implementing the CliTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask08 */
void StartTask08(void *argument)
{
  /* USER CODE BEGIN StartTask08 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask08 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

