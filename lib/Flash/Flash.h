// AuthorLink: https://blog.csdn.net/lwb450921/article/details/125022862
#ifndef _FLASH_H_
#define _FLASH_H_
#include "stm32f1xx_hal_flash_ex.h"
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t


#define STM32_FLASH_SIZE 	64
    #if     STM32_FLASH_SIZE < 256
    #define STM_SECTOR_SIZE     1024
    #else
    #define STM_SECTOR_SIZE	    2048
#endif

#define STM32_FLASH_BASE            0x08000000
#define FLASH_USER_START_ADDR   ( STM32_FLASH_BASE + STM_SECTOR_SIZE * 63 )
#define FLASH_USER_END_ADDR     ( STM32_FLASH_BASE + STM_SECTOR_SIZE * 64 )


void flashErase(void);
void flashWrite(u32 *pBuffer);
void flashRead(u32 *pBuffer);


#endif
