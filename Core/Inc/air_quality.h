#ifndef AIR_QUALITY_H
#define AIR_QUALITY_H
#include "sps30.h"


/*
    Starts a 60 second sampling cycle, includes 30 second startup time
    and 30 seconds of measurements averaged
    
    Parameters:
    huart: UART_HandleTypeDef
        determines what ports to transmit over

    data: float array
        an array of floats to store the averaged pm data
    
    data_len: int
        the size of the data array, must be at least 10

    Returns:
        HAL_StatusTypeDef
*/
HAL_StatusTypeDef sample_pm_data(UART_HandleTypeDef uart, float data[], int data_len);

#endif
