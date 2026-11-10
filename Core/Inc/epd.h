/* Header file for the driver for the waveshare 2.13inch e-Paper HAT v4 */

#ifndef EPD_H
#define EPD_H
#include "stm32f4xx_hal.h"


/*
    Sends a command to the epaper display

    Parameters:
        spi: SPI_HandleTypeDef *
            pointer to SPI_HandleTypeDef, the peripherial to be sent a command

        cmd_byte: uint8_t
            single byte that is the command to be send to the display

    Returns:
        HAL_OK if command sent successfully
        HAL_TIMEOUT if display timeouted
        HAL_BUSY if display is busy
        HAL_ERROR for all other errors
*/  
HAL_StatusTypeDef epd_send_command(SPI_HandleTypeDef *spi, const uint8_t cmd_byte);


/*
    Sends data to the epaper display

    Parameters:
        spi: SPI_HandleTypeDef *
            pointer to SPI_HandleTypeDef, the peripherial to be sent a command

        data: uint8_t []
            array of single bytes that is the data to be sent to the display

        size: uint16_t
            the amount of bytes in data
        
    Returns:
        HAL_OK if command sent successfully
        HAL_TIMEOUT if display timeouted
        HAL_BUSY if display is busy
        HAL_ERROR for all other errors
*/  
HAL_StatusTypeDef epd_send_data(SPI_HandleTypeDef *spi, 
        const uint8_t data_buffer[], const uint16_t size);


#endif
