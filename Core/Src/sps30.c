/* source code file for the driver for the sensirion sps30 PM sensor for an stm32 nucelo*/
#include "sps30.h"

int start_measurement(UART_HandleTypeDef huart)
{
    // byte sequence that starts measurements using IEEE floats
    uint8_t start_measurement_float_cmd[] = {0x7e, 0x00, 0x00, 0x02, 0x01, 0x03, 0xf9, 0x7e};

    HAL_StatusTypeDef tx_status;
    
    while ((tx_status = HAL_UART_Transmit(&huart, start_measurement_float_cmd, 8, 2000)) != HAL_OK)
    {
        if (tx_status == HAL_TIMEOUT)
        {
            HAL_Delay(1000); // wait one second and try again
            continue;
        }
        // just return an error if can't start up
        return 1;
    }

    // successful started measurements
    return 0;
}
