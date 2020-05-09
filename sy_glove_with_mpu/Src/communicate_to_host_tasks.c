/************************************************************
  * Copyright @shanghai.siyi
  * FileName: communicate_to_host_task.c
  * CreatDate: 2020.3.9
  * Description:
  * Version:		1
  * Subversion:	1
  * Revision:		1
  * change log:
	* 2020.3.9:first edit
  ***********************************************************/
	
	
#include "communicate_to_host_tasks.h"

#include "usart.h"

osThreadId host_communicate_uart_tx_handle;
osThreadId host_communicate_uart_rx_handle;

osMailQId host_communicate_mail_handle;

static void host_communicate_uart_tx(void const *argument);
static void host_communicate_uart_rx(void const *argument);

static void host_connected_cmd_handle(const void *argument);
static void calibration_start_cmd_handle(const void *argument);
static void calibration_result_cmd_handle(const void *argument);
static void calibration_result_param_cmd_handle(const void *argument);
static void software_version_cmd_handle(const void *argument);
static void raw_advalue_cmd_handle(const void *argument);


static uint8_t uart_data_receive[UART_RECEIVE_BUFFER_SIZE];

extern UART_HandleTypeDef huart1;
#define communicate_uart	huart1

extern void send_host_connected_cmd_to_host(void);
extern void send_calibration_result_cmd_to_host(void);
extern void send_calibration_result_param_cmd_to_host(void);
extern void send_software_version_cmd_to_host(void);
extern void send_raw_advalue_cmd_to_host(void);
extern void send_full_function_cmd_to_host(void);
extern void send_angle_data_notify_to_host(void);
extern void send_during_calibration_notify_to_host(void);

host_communicate_uart_typedef	host_communicate_uart =
{
	.status = uart_status_reset,
	.frame_rx_flag = 0,
	.max_buffer_size = UART_RECEIVE_BUFFER_SIZE,
};

glove_device_typedef	glove_device = 
{
	.glove_connected = 0,
	.software_version = 1,
	.calibration_started = 0,
	.current_work_mode = disconnected_mode,
	.last_work_mode = disconnected_mode,
	.host_connected_cmd_txf = send_host_connected_cmd_to_host,
	.calibration_result_cmd_txf = send_calibration_result_cmd_to_host,
	.calibration_result_param_cmd_txf = send_calibration_result_param_cmd_to_host,
	.software_version_cmd_txf = send_software_version_cmd_to_host,
	.raw_advalue_cmd_txf = send_raw_advalue_cmd_to_host,
	.full_function_cmd_txf = send_full_function_cmd_to_host,
	.angle_data_notify_txf = send_angle_data_notify_to_host,
	.during_calibration_notify_txf = send_during_calibration_notify_to_host,
};

int8_t init_communicate_to_host_tasks(void)
{
	osThreadDef(uart_tx_task, host_communicate_uart_tx,
			osPriorityNormal, 0, 128);
	
	osThreadDef(uart_rx_task, host_communicate_uart_rx,
			osPriorityNormal, 0, 128);
	
	host_communicate_uart_tx_handle = osThreadCreate(osThread(uart_tx_task), NULL);
	if(!host_communicate_uart_tx_handle)
	{
		return -1;
	}
	
	host_communicate_uart_rx_handle = osThreadCreate((osThread(uart_rx_task)), NULL);
	if(!host_communicate_uart_rx_handle)
	{
		return -1;
	}
	
	osMailQDef(host_communicate, MAX_NUMBER_OF_MSG_IN_MAILBOX, uint8_t[MAX_SIZE_OF_MSG_IN_MAILBOX]);
	host_communicate_mail_handle = osMailCreate(osMailQ(host_communicate), NULL);
	if(!host_communicate_mail_handle)
	{
		return -1;
	}
	
	host_communicate_uart_datagram_receive();
	
	return 0;
}


static void host_communicate_uart_tx(void const *argument)
{
	for(;;)
	{
		osEvent evt = osMailGet(host_communicate_mail_handle, osWaitForever);
		if(evt.status == osEventMail && evt.value.p != NULL)
		{
			data_head_typedef *head_temp = evt.value.p;
			uint8_t len_temp = head_temp->data_len;
			if(len_temp <= MAX_NUMBER_OF_MSG_IN_MAILBOX)
			{
				host_communicate_uart_datagram_transmit(head_temp, len_temp);
			}
		}
		host_communicate_uart_datagram_free(evt.value.p);
	}
}

static void host_communicate_uart_rx(void const *argument)
{
	for(;;)
	{
		osSignalWait(SIGNAL_OF_RX_END, osWaitForever);
		if(host_communicate_uart.frame_rx_flag && !host_communicate_uart.frame_processed)
		{
			data_head_typedef *head_temp = (data_head_typedef *)host_communicate_uart.data_buffer;
			enum_data_cmdtype cmd_temp = head_temp->cmd;
			
			switch	(cmd_temp)
			{
				case	host_connected_cmd	:
					host_connected_cmd_handle(host_communicate_uart.data_buffer);
					
					break;
				case	calibration_start_cmd	:
					calibration_start_cmd_handle(host_communicate_uart.data_buffer);
					
					break;
				
				case calibration_result_cmd:
					calibration_result_cmd_handle(host_communicate_uart.data_buffer);
					break;
				
				case calibration_result_param_cmd:
					calibration_result_param_cmd_handle(host_communicate_uart.data_buffer);
					break;
				
				case software_version_cmd:
					software_version_cmd_handle(host_communicate_uart.data_buffer);
					break;
				
				case raw_advalue_cmd:
					raw_advalue_cmd_handle(host_communicate_uart.data_buffer);
					break;

				
				default:
					break;

			}
			memset(host_communicate_uart.data_buffer, 0, sizeof(host_communicate_uart.data_buffer));
		}
	}
}

int8_t host_communicate_uart_datagram_transmit(void *msg, const size_t len)
{

	host_communicate_uart.status = uart_status_tx_busy;
	
	HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&communicate_uart, (uint8_t *)msg, len);
	
	if(status != HAL_OK)
	{
		return -1;
	}
	return 0;
}

int8_t host_communicate_uart_datagram_receive(void)
{
	return (HAL_UART_Receive_DMA(&communicate_uart, uart_data_receive, UART_RECEIVE_BUFFER_SIZE));
}

void *host_communicate_uart_datagram_alloc(size_t len)
{
	void *ret = NULL;
	
	if(len <= MAX_NUMBER_OF_MSG_IN_MAILBOX)
	{
		ret = osMailAlloc(host_communicate_mail_handle, 0);
	}
	
	return ret;
}

int8_t host_communicate_uart_datagram_put(void *msg)
{
	return (osMailPut(host_communicate_mail_handle, msg));
}

int8_t host_communicate_uart_datagram_free(void *msg)
{
	return (osMailFree(host_communicate_mail_handle, msg));
}


void UART_IDLE_IRQHandler(UART_HandleTypeDef *huart)
{
	if(huart == &communicate_uart)
	{
		if(__HAL_UART_GET_FLAG(&communicate_uart, UART_FLAG_IDLE) != RESET)
		{
			HAL_UART_DMAStop(&communicate_uart);
			host_communicate_uart.frame_rx_flag = 1;
			host_communicate_uart.frame_processed = 0;
			host_communicate_uart.frame_rx_size = UART_RECEIVE_BUFFER_SIZE - communicate_uart.hdmarx->Instance->CNDTR;
			memcpy(host_communicate_uart.data_buffer, uart_data_receive, host_communicate_uart.frame_rx_size);
			osSignalSet(host_communicate_uart_rx_handle, SIGNAL_OF_RX_END);
			host_communicate_uart_datagram_receive();
		}
	}
}

void set_work_mode(enum_device_workmode work_mode)
{
	glove_device.last_work_mode = glove_device.current_work_mode;
	glove_device.current_work_mode = work_mode;
}

void resume_work_mode(enum_device_workmode work_mode)
{
	glove_device.current_work_mode = glove_device.last_work_mode;
}


void osTimerCallback(const void *argument)
{
	const xTIMER *Timer = argument;
	const TimerCallbackParameter_t *Parameter = Timer->pvTimerID;
	osSignalSet(Parameter->CurrentThread, Parameter->Signal);
}

static void host_connected_cmd_handle(const void *argument)
{
	host_connected_cmd_typedef *connected = (host_connected_cmd_typedef *)argument;
	if(connected->state == 1)
	{
		set_work_mode(ready_mode);
	}
	else if(connected->state == 2)
	{
		set_work_mode(full_function_mode);
	}
	else
	{
		set_work_mode(disconnected_mode);
	}
}

static void calibration_start_cmd_handle(const void *argument)
{
	calibration_start_cmd_typedef *start = (calibration_start_cmd_typedef *)argument;
	if(start->state)
	{
		set_work_mode(during_calibration_mode);
	}
}

static void calibration_result_cmd_handle(const void *argument)
{
	calibration_result_cmd_typedef *result = (calibration_result_cmd_typedef *)argument;
	if(result->result == 0)
	{
		set_work_mode(finish_calibration_mode);
	}
	else if(result->result == 1)
	{
		set_work_mode(ready_mode);
	}
}

static void calibration_result_param_cmd_handle(const void *argument)
{
	glove_device.calibration_result_param_cmd_txf();
}

static void software_version_cmd_handle(const void *argument)
{
	glove_device.software_version_cmd_txf();
}

static void raw_advalue_cmd_handle(const void *argument)
{
	glove_device.raw_advalue_cmd_txf();
}




