#include "thread_of_sensor_calibration.h"

extern ADC_HandleTypeDef hadc1;
uint16_t ADC_ConvertedValue[5];
static uint16_t adc_convert_result_millivolt[5];


struct sensor_calibration_result_t
{
	uint16_t diff[5];
	uint16_t max[5];
	uint16_t min[5];
}sensor_calibration_result,sensor_calibration_defualt = 
{
	{70,70,70,70,70},
	{200,200,200,200,200},
	{30,30,30,30,30},
};

/*****************************************************************************************/
osThreadId sensor_adc_calibration_task_handle;
osThreadId sensor_process_calibration_task_handle;

osTimerId process_task_tmr_handle;
osTimerId adc_task_tmr_handle;


static void sensor_adc_calibration_task(void const *argument);
static void sensor_process_calibration_task(void const *argument);

static void glove_calibration_process(void);
static enum_calibration_result glove_calibration_check(void);


int8_t init_sensor_calibration_tasks(void)
{
	osThreadDef(sensor_adc_calibration, sensor_adc_calibration_task,
			osPriorityNormal, 0, 128);
  sensor_adc_calibration_task_handle = osThreadCreate(osThread(sensor_adc_calibration), NULL);
  if(!sensor_adc_calibration_task_handle)
	{
		return(-1);
	}
	
	osThreadDef(sensor_process_calibration, sensor_process_calibration_task,
			osPriorityNormal, 0, 128);
  sensor_process_calibration_task_handle = osThreadCreate(osThread(sensor_process_calibration), NULL);
  if(!sensor_process_calibration_task_handle)
	{
		return(-1);
	}
  
  return(0);
}

/*****************************************************************************************/

uint8_t is_calibration,is_glove_calibration,return_resualt = 0;
uint32_t return_mark;
static void sensor_adc_calibration_task(void const *argument)
{
	TimerCallbackParameter_t ADC_Timer_Info = {osThreadGetId(), SIG_USER_0};
	osTimerDef(adc_task_tmr, osTimerCallback);
	adc_task_tmr_handle = osTimerCreate(osTimer(adc_task_tmr), osTimerPeriodic, &ADC_Timer_Info);
	osTimerStart(adc_task_tmr_handle, 50);
  for(;;)
	{
		Internal_Flash_Read_Bytes(FLASH_USER_START_ADDR, (uint16_t *)&sensor_calibration_result,
														sizeof(sensor_calibration_result));
		
		uint8_t no_init_bytes = 0;
		for(uint8_t i = 0; i < sizeof(sensor_calibration_result); i ++)
		{
			if(* ((uint8_t *)(&sensor_calibration_result + i)) == 0xFF)
			{
				no_init_bytes ++;
			}
		}
		if(no_init_bytes > (sizeof(sensor_calibration_result) >> 2))
		{
			memcpy(&sensor_calibration_result, &sensor_calibration_defualt, sizeof(sensor_calibration_result));
		}
		
		memcpy(&glove_device.calibration_param, &sensor_calibration_result, sizeof(sensor_calibration_result));

		
		osDelay(100);
		
		for(;;){
					
			osSignalWait(SIG_USER_0, osWaitForever);
			
			memcpy(ADC_ConvertedValue, ADC_Original_Value, sizeof(ADC_Original_Value));
			for(uint8_t i = 0;i < 5;i ++)
			{
				adc_convert_result_millivolt[i] = ((ADC_ConvertedValue[i]&0xFFF) * 3300) >> 12;
				glove_device.adc_rawdata[i] = adc_convert_result_millivolt[i];
				
				if(glove_device.current_work_mode != during_calibration_mode)
				{
					/*if not in calibration mode, limit max & min value*/
					if(adc_convert_result_millivolt[i] < glove_device.calibration_param.sensor_calibration_min[i])
					{
						adc_convert_result_millivolt[i] = glove_device.calibration_param.sensor_calibration_min[i];
					}

					if(adc_convert_result_millivolt[i] > glove_device.calibration_param.sensor_calibration_max[i])
					{
						adc_convert_result_millivolt[i] = glove_device.calibration_param.sensor_calibration_max[i];
					}
					
					if(adc_convert_result_millivolt[i] < (0.35 * sensor_calibration_result.diff[i] + sensor_calibration_result.min[i]))
					{
						adc_convert_result_millivolt[i] = ((adc_convert_result_millivolt[i] - sensor_calibration_result.min[i])*90) / (0.35 * sensor_calibration_result.diff[i]);
					}
					else if(adc_convert_result_millivolt[i] >= (0.35 * sensor_calibration_result.diff[i]  + sensor_calibration_result.min[i]))
					{
						adc_convert_result_millivolt[i] = (((adc_convert_result_millivolt[i] - sensor_calibration_result.min[i] - 0.35 * sensor_calibration_result.diff[i])*90)/(0.65*sensor_calibration_result.diff[i])) + 90;
					}
					/*flip the angle value*/
//					adc_convert_result_millivolt[i] = 180 - adc_convert_result_millivolt[i];
					glove_device.glove_angle[i] = 180 - adc_convert_result_millivolt[i];
				}
			}
			
			
		}
  }
}

static void sensor_process_calibration_task(void const *argument)
{
	TimerCallbackParameter_t Process_Timer_Info = {osThreadGetId(), SIG_USER_1};
	osTimerDef(process_task_tmr, osTimerCallback);
	process_task_tmr_handle = osTimerCreate(osTimer(process_task_tmr), osTimerPeriodic, &Process_Timer_Info);
	osTimerStart(process_task_tmr_handle, 50);
	for(;;)
	{
		osSignalWait(SIG_USER_1, osWaitForever);
		switch (glove_device.current_work_mode)
		{
			case disconnected_mode:
				osThreadSuspend(MPL_TaskHandle);
				if(HAL_GetTick() - glove_device.timetick > 500)
				{
					glove_device.host_connected_cmd_txf();
					glove_device.timetick = HAL_GetTick();
				}
				
				break;
			case ready_mode:
				osThreadSuspend(MPL_TaskHandle);
				glove_device.angle_data_notify_txf();
				break;
				
			case full_function_mode:
				osThreadResume(MPL_TaskHandle);
				break;
				
			case during_calibration_mode:
				osThreadSuspend(MPL_TaskHandle);
				glove_device.during_calibration_notify_txf();
				glove_calibration_process();
				break;
				
			case finish_calibration_mode:
				osThreadSuspend(MPL_TaskHandle);
				glove_device.calibration_started = 0;
				glove_device.calibration_result_cmd_txf();
				break;
				
			default:
				osThreadSuspend(MPL_TaskHandle);
				break;
		}
		

	}
}

static struct sensor_calibration_result_t sensor_calibration_register;

static void glove_calibration_process(void)
{
	/*in every calibration process enter here once*/
	if(!glove_device.calibration_started)
	{
		glove_device.calibration_started = 1;
		memcpy(&sensor_calibration_register.max, glove_device.adc_rawdata, sizeof(sensor_calibration_register.max));
		memcpy(&sensor_calibration_register.min, glove_device.adc_rawdata, sizeof(sensor_calibration_register.min));
	}
	
	for(uint8_t i = 0;i < 5;i ++)
	{
		if(sensor_calibration_register.max[i] <= adc_convert_result_millivolt[i])
		{
			sensor_calibration_register.max[i] = adc_convert_result_millivolt[i];
		}

		if(sensor_calibration_register.min[i] >= adc_convert_result_millivolt[i])
		{
			sensor_calibration_register.min[i] = adc_convert_result_millivolt[i];
		}
		sensor_calibration_register.diff[i] = sensor_calibration_register.max[i] - sensor_calibration_register.min[i];
	}
}

static enum_calibration_result glove_calibration_check(void)
{
	uint8_t i;
	
	
	for(i = 0; i < 5; i ++)
	{
		if(sensor_calibration_register.max[i] > 3295)
		{
			return	circuit_shorted_result;
		}
		if(sensor_calibration_register.min[i] < 5)
		{
			return	circuit_open_result;
		}
		
	}
	
	if(sensor_calibration_register.max[0] - sensor_calibration_register.min[0] < 50)
	{
		return width_invalid_result;
	}

	for(i = 1; i < 5; i ++)
	{
		if(sensor_calibration_register.max[i] - sensor_calibration_register.min[i] < 70)
		{
			break;
		}
		if(i == 4)
		{
			for(i = 0; i < 5; i ++)
			{
				sensor_calibration_register.diff[i] = sensor_calibration_register.max[i] - sensor_calibration_register.min[i];
				sensor_calibration_register.max[i] -= sensor_calibration_register.diff[i] * 0.05;
				sensor_calibration_register.min[i] -= sensor_calibration_register.diff[i] * 0.05;
			}
			memcpy(&sensor_calibration_result,&sensor_calibration_register,sizeof(struct sensor_calibration_result_t));	
			return calibration_success_result;
		}
	}
	
	return calibration_success_result;
}

void send_host_connected_cmd_to_host(void)
{
	host_connected_cmd_typedef *connected = host_communicate_uart_datagram_alloc(sizeof(host_connected_cmd_typedef));
	if(connected)
	{
		HOST_COMMUNICATE_DATAGRAM_INIT((*connected), host_connected_cmd);
		connected->state = glove_device.glove_connected;
		host_communicate_uart_datagram_put(connected);
	}
}

void send_calibration_result_cmd_to_host(void)
{
	if((HAL_GetTick() - return_mark) > 10000)
	{
		return_mark = HAL_GetTick();
		return_resualt = glove_calibration_check();		
	}
	osDelay(10);//don't delete this sytax or calibration will failed
	calibration_result_cmd_typedef *result = host_communicate_uart_datagram_alloc(sizeof(calibration_result_cmd_typedef));
	
	if(result)
	{
		HOST_COMMUNICATE_DATAGRAM_INIT((*result), calibration_result_cmd);				
		result->result = glove_calibration_check();
		host_communicate_uart_datagram_put(result);
	}
	
	Internal_Flash_Write_Bytes(FLASH_USER_START_ADDR, (uint16_t *)&sensor_calibration_result,
											sizeof(sensor_calibration_result));
	memcpy(&glove_device.calibration_param, &sensor_calibration_result, sizeof(sensor_calibration_result));
}

void send_calibration_result_param_cmd_to_host(void)
{
	calibration_result_param_cmd_typedef *calibration_result = host_communicate_uart_datagram_alloc(sizeof (calibration_result_param_cmd_typedef));
	if(calibration_result)
	{
		HOST_COMMUNICATE_DATAGRAM_INIT((*calibration_result), calibration_result_param_cmd);
		memcpy(&calibration_result->calibration_param, &glove_device.calibration_param, sizeof(calibration_result->calibration_param));
		host_communicate_uart_datagram_put(calibration_result);
	}
}

void send_software_version_cmd_to_host(void)
{
	software_version_cmd_typedef *version = host_communicate_uart_datagram_alloc(sizeof(software_version_cmd_typedef));
	if(version)
	{
		HOST_COMMUNICATE_DATAGRAM_INIT((*version), software_version_cmd);
		version->version[0] = 0x0000;
		version->version[1] = (uint16_t)((*(__IO uint32_t *)(0X1FFFF7E8))&0x0000ffff);
		version->version[2] = (uint16_t)(((*(__IO uint32_t *)(0X1FFFF7E8))&0xffff0000)>>16);
		host_communicate_uart_datagram_put(version);
	}
}

void send_raw_advalue_cmd_to_host(void)
{
	raw_advalue_cmd_typedef *rawdata = host_communicate_uart_datagram_alloc(sizeof(raw_advalue_cmd_typedef));
	if(rawdata)
	{
		HOST_COMMUNICATE_DATAGRAM_INIT((*rawdata), raw_advalue_cmd);
		memcpy(rawdata->adc_rawdata, glove_device.adc_rawdata, sizeof(rawdata->adc_rawdata));
		host_communicate_uart_datagram_put(rawdata);
	}
}

void send_full_function_cmd_to_host(void)
{
	full_function_cmd_typedef *full_data = host_communicate_uart_datagram_alloc(sizeof(full_function_cmd_typedef));
	if(full_data)
	{
		HOST_COMMUNICATE_DATAGRAM_INIT((*full_data), full_function_cmd);
		memcpy(full_data->angle_data, glove_device.glove_angle, sizeof(full_data->angle_data));
		memcpy(full_data->motion_displacement, glove_device.motion_displacement, sizeof(full_data->motion_displacement));
		host_communicate_uart_datagram_put(full_data);
	}
}


void send_angle_data_notify_to_host(void)
{
	angle_data_notify_typedef *angle = host_communicate_uart_datagram_alloc(sizeof(angle_data_notify_typedef));
	if(angle)
	{
		HOST_COMMUNICATE_DATAGRAM_INIT((*angle), angle_data_notify);
		
		memcpy(angle->angle_data, glove_device.glove_angle, sizeof(glove_device.glove_angle));
		host_communicate_uart_datagram_put(angle);
	}
}

void send_during_calibration_notify_to_host(void)
{
	during_calibration_notify_typedef *noti = host_communicate_uart_datagram_alloc(sizeof(during_calibration_notify_typedef));			
	if(noti)
	{
		HOST_COMMUNICATE_DATAGRAM_INIT((*noti), during_calibration_notify);
		host_communicate_uart_datagram_put(noti);
	}
}


/****************************************END OF FILE*************************************/
