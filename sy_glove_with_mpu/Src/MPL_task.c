/************************************************************
  * Copyright @shanghai.siyi
  * FileName: MPL_task.c
  * CreatDate: 2020.3.9
  * Description:
  * Version:		1
  * Subversion:	1
  * Revision:		1
  * change log:
	* 2020.3.9:first edit
  ***********************************************************/
	
#include "MPL_task.h"

#include "mpu6050_app.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "algorithm.h"


double Pitch,Roll,Yaw;
double pitch_old, roll_old, yaw_old;

extern struct hal_s hal;

MPU_Data_Buf_TypeDef mpu_data;

float mpu_pitch_ma[MPU_AXIS_BUF_DEPTH - MPU_MA_STEP_LEN];
float mpu_roll_ma[MPU_AXIS_BUF_DEPTH - MPU_MA_STEP_LEN];
float mpu_yaw_ma[MPU_AXIS_BUF_DEPTH - MPU_MA_STEP_LEN];


osThreadId MPL_TaskHandle;
osTimerId mpl_task_tmr_handle;

static void Start_MPL_task(void const * argument);
static void	 Algorithm_Motion_Distance(float *data);


void init_MPL_task(void)
{
	osThreadDef(MPL_task, Start_MPL_task, osPriorityNormal, 0, 128);
  MPL_TaskHandle = osThreadCreate(osThread(MPL_task), NULL);
}



void Start_MPL_task(void const * argument)
{
	eMPL_start_cfg();
	
	TimerCallbackParameter_t MPL_Timer_Info = {osThreadGetId(), SIG_USER_TIMER};
	osTimerDef(mpl_task_tmr, osTimerCallback);
	mpl_task_tmr_handle = osTimerCreate(osTimer(mpl_task_tmr), osTimerPeriodic, &MPL_Timer_Info);
	osTimerStart(mpl_task_tmr_handle, 10);
	osThreadSuspend(osThreadGetId());
	
	for(;;)
	{
		osSignalWait(SIG_USER_TIMER, osWaitForever);
		unsigned long sensor_timestamp;

		if (hal.new_gyro)
		{
			short gyro[3], accel[3], sensors;
			unsigned char more;
			long quat[4];
			/* This function gets new data from the FIFO when the DMP is in
			 * use. The FIFO can contain any combination of gyro, accel,
			 * quaternion, and gesture data. The sensors parameter tells the
			 * caller which data fields were actually populated with new data.
			 * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
			 * the FIFO isn't being filled with accel data.
			 * The driver parses the gesture data to determine if a gesture
			 * event has occurred; on an event, the application will be notified
			 * via a callback (assuming that a callback function was properly
			 * registered). The more parameter is non-zero if there are
			 * leftover packets in the FIFO.
			 */
			dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors,
					&more);
			if (!more)
			{
				hal.new_gyro = 0;
			}
					
			/* Unlike gyro and accel, quaternions are written to the FIFO in
			 * the body frame, q30. The orientation is set by the scalar passed
			 * to dmp_set_orientation during initialization.
			 */
			if (sensors & INV_WXYZ_QUAT && hal.report & PRINT_QUAT)
			{
				float data[3];
				inv_get_sensor_type_euler(data, quat);
				Pitch = data[0];
				Roll = data[1];
				Yaw = data[2];
				
				pitch_old = Pitch;
				yaw_old = Yaw;
				memcpy(glove_device.euler_angle, data, sizeof(data));
				Algorithm_Motion_Distance(data);
//				log_e("%.4f %.4f %.4f\r\n",Pitch, Roll, Yaw);
			}
		}
	}
	
}

static void	 Algorithm_Motion_Distance(float *data)
{
	if(!mpu_data.Data_FullFlag)
	{
		mpu_data.Raw_Pitch_DataBuf[mpu_data.Data_Count] = data[0];
		mpu_data.Raw_Roll_DataBuf[mpu_data.Data_Count] = data[1];
		mpu_data.Raw_Yaw_DataBuf[mpu_data.Data_Count ++] = data[2];
		
		if(mpu_data.Data_Count > MPU_AXIS_BUF_DEPTH - 1)
		{
			mpu_data.Data_FullFlag = 1;
		}
	}
	else
	{
		for(uint8_t i = 0; i < MPU_AXIS_BUF_DEPTH - MPU_MA_STEP_LEN; i ++)
		{
			double p_ma = 0, r_ma = 0, y_ma = 0;
			for(uint8_t j = 0; j < MPU_MA_STEP_LEN - 1; j ++)
			{
				p_ma += mpu_data.Raw_Pitch_DataBuf[i + j];
				r_ma += mpu_data.Raw_Roll_DataBuf[i + j];
				y_ma += mpu_data.Raw_Yaw_DataBuf[i + j];
			}
			mpu_pitch_ma[i] = p_ma / (MPU_MA_STEP_LEN);
			mpu_roll_ma[i] = r_ma / (MPU_MA_STEP_LEN);
			mpu_yaw_ma[i] = y_ma / (MPU_MA_STEP_LEN);
			
		}
		
//		for(uint8_t i = 0; i < MPU_AXIS_BUF_DEPTH - 1; i ++)
//		{
//			mpu_data.Raw_Pitch_DataBuf[i] = mpu_data.Raw_Pitch_DataBuf[i + 1];
//			mpu_data.Raw_Roll_DataBuf[i] = mpu_data.Raw_Roll_DataBuf[i + 1];
//			mpu_data.Raw_Yaw_DataBuf[i] = mpu_data.Raw_Yaw_DataBuf[i + 1];
//		}
		
		memmove(mpu_data.Raw_Pitch_DataBuf, &(mpu_data.Raw_Pitch_DataBuf[1]), sizeof(float) * (MPU_AXIS_BUF_DEPTH - 1));
		memmove(mpu_data.Raw_Roll_DataBuf, &(mpu_data.Raw_Roll_DataBuf[1]), sizeof(float) * (MPU_AXIS_BUF_DEPTH - 1));
		memmove(mpu_data.Raw_Yaw_DataBuf, &(mpu_data.Raw_Yaw_DataBuf[1]), sizeof(float) * (MPU_AXIS_BUF_DEPTH - 1));
		
		mpu_data.Raw_Pitch_DataBuf[MPU_AXIS_BUF_DEPTH - 1] = data[0];
		mpu_data.Raw_Roll_DataBuf[MPU_AXIS_BUF_DEPTH - 1] = data[1];
		mpu_data.Raw_Yaw_DataBuf[MPU_AXIS_BUF_DEPTH - 1] = data[2];
		
		glove_device.motion_displacement[0] = (int8_t)(mpu_yaw_ma[0] - mpu_yaw_ma[MPU_AXIS_BUF_DEPTH - MPU_MA_STEP_LEN - 1]);
		glove_device.motion_displacement[1] = (int8_t)(mpu_roll_ma[0] - mpu_roll_ma[MPU_AXIS_BUF_DEPTH - MPU_MA_STEP_LEN - 1]);
		
		glove_device.full_function_cmd_txf();
		
	}
}
		





