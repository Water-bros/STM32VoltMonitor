// AuthorLink: https://blog.csdn.net/lwb450921/article/details/125022862
#include "Flash.h"
#include "stm32f1xx_hal_flash_ex.h"

static FLASH_EraseInitTypeDef EraseInitStruct;
u32 PAGEError = 0;

void flashErase(void)
{
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
    EraseInitStruct.NbPages = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / STM_SECTOR_SIZE;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        HAL_FLASH_Lock();
    }
}

void flashWrite(u32 *pBuffer)
{
    u32 Address = FLASH_USER_START_ADDR;
    HAL_FLASH_Unlock();
    flashErase();
    for (u16 i = 0; (Address < FLASH_USER_END_ADDR) && (i < 2); i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, pBuffer[i]) == HAL_OK)
        {
            Address = Address + 4;
        }
    }

    HAL_FLASH_Lock();
}

void flashRead(u32 *pBuffer)
{
    u32 Address = FLASH_USER_START_ADDR;

    for (u16 i = 0; (Address < FLASH_USER_END_ADDR) && (i < 2); i++)
    {
        pBuffer[i++] = *(__IO u32 *)Address;
        Address = Address + 4;
    }
}
