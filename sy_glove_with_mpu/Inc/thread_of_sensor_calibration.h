#ifndef __THREAD_OF_SENSOR_CALIBRATION_H__
#define __THREAD_OF_SENSOR_CALIBRATION_H__


#include <stdlib.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"


#include "adc.h"

#include "internal_flash.h"

#include "communicate_to_host_tasks.h"

extern osThreadId tid_thread_of_sensor_calibration;

int8_t init_sensor_calibration_tasks(void);


void send_host_connected_cmd_to_host(void);
void send_calibration_result_cmd_to_host(void);
void send_calibration_result_param_cmd_to_host(void);
void send_software_version_cmd_to_host(void);
void send_raw_advalue_cmd_to_host(void);
void send_full_function_cmd_to_host(void);
void send_angle_data_notify_to_host(void);
void send_during_calibration_notify_to_host(void);


#endif
/****************************************END OF FILE*************************************/
