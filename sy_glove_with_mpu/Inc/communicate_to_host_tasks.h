/************************************************************
  * Copyright @shanghai.siyi
  * FileName: communicate_to_host_task.h
  * CreatDate: 2020.3.9
  * Description:
  * Version:		1
  * Subversion:	1
  * Revision:		1
  * change log:
	* 2020.3.9:first edit
  ***********************************************************/
	
#ifndef __COMMUNICATE_TO_HOST_TASKS_H__
#define	__COMMUNICATE_TO_HOST_TASKS_H__

#include <string.h>

#include "cmsis_os.h"
#include "usart.h"



//-------- <<< Use Configuration Wizard in Context Menu >>> ---------------------
//	<o>Max Size of Message in Mailbox
//	<i>must be power of 2,default: 128
#define MAX_SIZE_OF_MSG_IN_MAILBOX	32

//	<o>Max Number of Message in Mailbox
//	<i>default: 25
#define MAX_NUMBER_OF_MSG_IN_MAILBOX	25

//	<o>OS Signal for UART Transmit End
//	<i>default: 0x01
#define	SIGNAL_OF_TX_END	1

//	<o>OS Signal for UART Receive End
//	<i>default: 0x02
#define SIGNAL_OF_RX_END	2

//	<o>OS Signal for SIG_SERVER_TERMINATED
//	<i>default: 0x04
#define SIG_SERVER_TERMINATED         4

//	<o>OS Signal SIG_CLIENT_TERMINATE
//	<i>default: 0x08
#define SIG_CLIENT_TERMINATE          8

//	<o>OS Signal for UART Receive End
//	<i>default: 0x10
#define SIG_USER_TIMER               16

//	<o>OS Signal for UART Receive End
//	<i>default: 0x20
#define SIG_USER_0                   32

//	<o>OS Signal for UART Receive End
//	<i>default: 0x40
#define SIG_USER_1                   64

//	<o>OS Signal SIG_USER_2
//	<i>default: 0x80
#define SIG_USER_2                  128

//	<o>OS Signal SIG_USER_3
//	<i>default: 0x100
#define SIG_USER_3                  256

//	<o>UART Receive Data Buffer Size
//	<i>default: 128
#define UART_RECEIVE_BUFFER_SIZE	128

//-------------  Configuration Wizard in Context END  ---------------------

typedef struct
{
	osThreadId CurrentThread;
	uint32_t Signal;
}TimerCallbackParameter_t;

typedef struct tmrTimerControl
{
	const char				*pcTimerName;		/*<< Text name.  This is not used by the kernel, it is included simply to make debugging easier. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
	ListItem_t				xTimerListItem;		/*<< Standard linked list item as used by all kernel features for event management. */
	TickType_t				xTimerPeriodInTicks;/*<< How quickly and often the timer expires. */
	UBaseType_t				uxAutoReload;		/*<< Set to pdTRUE if the timer should be automatically restarted once expired.  Set to pdFALSE if the timer is, in effect, a one-shot timer. */
	void 					*pvTimerID;			/*<< An ID to identify the timer.  This allows the timer to be identified when the same callback is used for multiple timers. */
	TimerCallbackFunction_t	pxCallbackFunction;	/*<< The function that will be called when the timer expires. */
	#if( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t			uxTimerNumber;		/*<< An ID assigned by trace tools such as FreeRTOS+Trace */
	#endif

	#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
		uint8_t 			ucStaticallyAllocated; /*<< Set to pdTRUE if the timer was created statically so no attempt is made to free the memory again if the timer is later deleted. */
	#endif
} xTIMER;

typedef enum
{
	uart_status_reset = 0,
	uart_status_ready,
	uart_status_tx_busy,
	uart_status_rx_busy,
	uart_status_error,
}enum_host_communicate_uart_status;

typedef struct
{
	enum_host_communicate_uart_status status;
	uint8_t frame_rx_flag:1;
	uint8_t frame_processed:1;
	uint8_t frame_rx_size;
	uint8_t max_buffer_size;
	uint8_t data_buffer[UART_RECEIVE_BUFFER_SIZE];
}host_communicate_uart_typedef;


typedef enum
{
	cmd_id_min = 0,
	host_connected_cmd,
	calibration_start_cmd,
	calibration_result_cmd,
	calibration_result_param_cmd,
	software_version_cmd,
//	machinary_id_cmd,
//	serial_calibration_noti_cmd,	
	raw_advalue_cmd,
	full_function_cmd,
	cmd_id_max = 0x0f,
	notify_id_min = 0x10,
	angle_data_notify,
	during_calibration_notify,
//	imu_msg_cmd,
//	imu_debug_cmd,
//	imu_motion_distance_cmd = 0x14,
	notify_id_max,
}enum_data_cmdtype;

typedef struct
{
	enum_data_cmdtype cmd;
	uint8_t data_len;
}data_head_typedef;

typedef struct
{
	uint16_t difference_value[5];
	uint16_t sensor_calibration_max[5];
	uint16_t sensor_calibration_min[5];
}sensor_calibration_result_typedef;


typedef struct
{
	data_head_typedef head;
	uint8_t state;
}data_frame_ack_typedef;

typedef struct
{
	data_head_typedef head;
	uint8_t state;
}host_connected_cmd_typedef;

typedef struct
{
	data_head_typedef head;
	uint8_t state;
}calibration_start_cmd_typedef;

typedef struct
{
	data_head_typedef head;
	uint8_t result;
}calibration_result_cmd_typedef;

typedef struct
{
	data_head_typedef head;
	sensor_calibration_result_typedef	calibration_param;
}calibration_result_param_cmd_typedef;

typedef struct
{
	data_head_typedef head;
	uint16_t version[3];
}software_version_cmd_typedef;

typedef struct
{
	data_head_typedef head;
	uint16_t adc_rawdata[5];
}raw_advalue_cmd_typedef;

typedef struct
{
	data_head_typedef head;
	int8_t motion_displacement[2];
	uint8_t angle_data[5];
}full_function_cmd_typedef;

typedef struct
{
	data_head_typedef head;
	uint8_t angle_data[5];
}angle_data_notify_typedef;

typedef struct
{
	data_head_typedef head;
	uint8_t state;
}during_calibration_notify_typedef;

typedef enum
{
	disconnected_mode = 0,
	ready_mode,
	full_function_mode,
	during_calibration_mode,
	finish_calibration_mode,
}enum_device_workmode;

typedef enum
{
	calibration_success_result = 0,
	circuit_shorted_result,
	circuit_open_result,
	width_invalid_result,
}enum_calibration_result;


typedef struct
{
	uint8_t glove_connected:1;
	uint8_t calibration_started:1;
	enum_calibration_result calibration_finish_result:6;
	uint8_t software_version;
	uint8_t	glove_angle[5];
	int8_t motion_displacement[2];
	enum_device_workmode current_work_mode;
	enum_device_workmode last_work_mode;
	uint16_t adc_rawdata[5];
	uint32_t timetick;
	sensor_calibration_result_typedef calibration_param;
	float euler_angle[3];
	void (*host_connected_cmd_txf)(void);
	void (*calibration_result_cmd_txf)(void);
	void (*calibration_result_param_cmd_txf)(void);
	void (*software_version_cmd_txf)(void);
	void (*raw_advalue_cmd_txf)(void);
	void (*full_function_cmd_txf)(void);
	void (*angle_data_notify_txf)(void);
	void (*during_calibration_notify_txf)(void);
}glove_device_typedef;


int8_t init_communicate_to_host_tasks(void);

int8_t host_communicate_uart_datagram_transmit(void *msg, const size_t len);
int8_t host_communicate_uart_datagram_receive(void);
void *host_communicate_uart_datagram_alloc(size_t len);
int8_t host_communicate_uart_datagram_put(void *msg);
int8_t host_communicate_uart_datagram_free(void *msg);

void UART_IDLE_IRQHandler(UART_HandleTypeDef *huart);

void set_work_mode(enum_device_workmode work_mode);
void resume_work_mode(enum_device_workmode work_mode);

void osTimerCallback(const void *argument);

#define HOST_COMMUNICATE_DATAGRAM_INIT(name, id)          \
	memset(&name, 0, sizeof name);                \
	name.head.cmd       = id;                    \
	name.head.data_len	 = sizeof(id##_typedef);


extern host_communicate_uart_typedef	host_communicate_uart;
extern glove_device_typedef	glove_device;

extern osThreadId MPL_TaskHandle;
extern osThreadId sensor_adc_calibration_task_handle;
extern osThreadId sensor_process_calibration_task_handle;

extern osTimerId mpl_task_tmr_handle;
extern osTimerId process_task_tmr_handle;
extern osTimerId adc_task_tmr_handle;


#endif

