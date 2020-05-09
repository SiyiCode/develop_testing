/************************************************************
  * Copyright @shanghai.siyi
  * FileName: MPL_task.h
  * CreatDate: 2020.3.9
  * Description:
  * Version:		1
  * Subversion:	1
  * Revision:		1
  * change log:
	* 2020.3.9:first edit
  ***********************************************************/
#ifndef __MPL_TASK_H__
#define __MPL_TASK_H__

#include <math.h>

#include "cmsis_os.h"

#include "communicate_to_host_tasks.h"


void init_MPL_task(void);

void sort_array_extremum(float *array, uint8_t *index, uint8_t size);

extern osThreadId MPL_TaskHandle;

#endif
