/************************************************************
  * Copyright @shanghai.siyi
  * FileName: algorithm.h
  * CreatDate: 2020.3.6
  * Description:
  * Version:		1
  * Subversion:	1
  * Revision:		1
  * change log:
	* 2020.3.6:first edit
  ***********************************************************/
	
#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__

#include <math.h>
#include <string.h>

#include "cmsis_os.h"

#if !defined(__NO_INIT)
 //lint -esym(9071, __NO_INIT) "defined macro is reserved to the compiler"
 #if   defined (__CC_ARM)                                           /* ARM Compiler 4/5 */
  #define __NO_INIT __attribute__ ((section (".bss.noinit"), zero_init))
 #elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)    /* ARM Compiler 6 */
  #define __NO_INIT __attribute__ ((section (".bss.noinit")))
 #elif defined (__GNUC__)                                           /* GNU Compiler */
  #define __NO_INIT __attribute__ ((section (".bss.noinit")))
 #else
  #warning "No compiler specific solution for __NO_INIT. __NO_INIT is ignored."
  #define __NO_INIT
 #endif
#endif

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

//	<o>MPU 3-Axis Algorithm Buffer Depth
//	<i>Data Buffer Depth, Bigger and Motion will Slower
//	<i>Default: 18
#define MPU_AXIS_BUF_DEPTH 25

//	<o>MPU Algorithm Moving-Average Step Length
//     <3=>3 <4=>4 <5=>5 <6=>6 <7=>7 <8=>8 <9=>9 <10=>10 
//	<i>Algorithm Moving Average Step Length 
#define MPU_MA_STEP_LEN	5


//------------- <<< end of configuration section >>> ---------------------------


typedef struct MPU_Data_Buf_tag
{
	float Raw_Pitch_DataBuf[MPU_AXIS_BUF_DEPTH];
	float Raw_Roll_DataBuf[MPU_AXIS_BUF_DEPTH];
	float Raw_Yaw_DataBuf[MPU_AXIS_BUF_DEPTH];
	uint16_t Data_Size:7;
	uint16_t Data_Count:7;
	uint16_t Data_FullFlag:2;
}MPU_Data_Buf_TypeDef;

#endif
	
	
