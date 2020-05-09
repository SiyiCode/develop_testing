#ifndef __SOFT_I2C_H__
#define	__SOFT_I2C_H__

#include "stm32f1xx_hal.h"
#include <inttypes.h>


#define MPU_INT_RCC_CLK_ENABLE           __HAL_RCC_GPIOB_CLK_ENABLE
#define MPU_INT_GPIO_PIN                 GPIO_PIN_12
#define MPU_INT_GPIO                     GPIOB
#define MPU_INT_EXTI_IRQHandler          EXTI15_10_IRQHandler
#define MPU_INT_EXTI_IRQn                EXTI15_10_IRQn


#define Soft_I2C_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE()
#define Soft_I2C_SDA 		                   GPIO_PIN_7
#define Soft_I2C_SCL 		                   GPIO_PIN_6
#define Soft_I2C_PORT 		                 GPIOB
//
#define Soft_I2C_SCL_0 		                 HAL_GPIO_WritePin(Soft_I2C_PORT,Soft_I2C_SCL,GPIO_PIN_RESET)    // 输出低电平
#define Soft_I2C_SCL_1 		                 HAL_GPIO_WritePin(Soft_I2C_PORT,Soft_I2C_SCL,GPIO_PIN_SET)      // 输出高电平
#define Soft_I2C_SDA_0 		                 HAL_GPIO_WritePin(Soft_I2C_PORT,Soft_I2C_SDA,GPIO_PIN_RESET)    // 输出低电平
#define Soft_I2C_SDA_1   	                 HAL_GPIO_WritePin(Soft_I2C_PORT,Soft_I2C_SDA,GPIO_PIN_SET)      // 输出高电平


#define MPU_DEBUG_ON                        0

#define MPU_INFO(fmt,arg...)           printf("<<-MPU-INFO->> "fmt"\n",##arg)
#define MPU_DEBUG(fmt,arg...)          do{\
                                          if(MPU_DEBUG_ON)\
                                          printf("<<-MPU-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                          }while(0)



void mdelay(uint32_t tick);		
int get_tick_count(unsigned long *count);
#define MPL_LOGI(fmt,arg...)           printf("<<-MPU-INFO->> "fmt"\n",##arg)
#define MPL_LOGE(fmt,arg...)           printf("<<-MPU-INFO->> "fmt"\n",##arg)
#define __no_operation()	__ASM("nop")



void I2C_Bus_Init(void);
void Set_I2C_Retry(unsigned short ml_sec);
unsigned short Get_I2C_Retry(void);
int Sensors_I2C_ReadRegister(unsigned char Address, unsigned char RegisterAddr, 
                                          unsigned short RegisterLen, unsigned char *RegisterValue);

int Sensors_I2C_WriteRegister(unsigned char Address, unsigned char RegisterAddr, 
                                           unsigned short RegisterLen, const unsigned char *RegisterValue);


void EXTI15_10_IRQHandler(void);
	

#endif

/***************END OF FILE****/
