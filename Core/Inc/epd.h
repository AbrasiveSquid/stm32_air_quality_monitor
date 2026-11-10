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
        HAL_TIMEOUT if display timedout
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
        HAL_TIMEOUT if display timedout
        HAL_BUSY if display is busy
        HAL_ERROR for all other errors
*/  
HAL_StatusTypeDef epd_send_data(SPI_HandleTypeDef *spi, 
        const uint8_t data_buffer[], const uint16_t size);

/*
    Completes a hardware reset of the epaper display
*/  
static void epd_hardware_reset(void);


/*
    Checks if epaper display is busy, and waits for it to be free

    Returns:
        HAL_OK if busy ended successfully
        HAL_TIMEOUT if display timedout
*/
HAL_StatusTypeDef epd_busy_wait(void);

/*
    Completes a software reset of the epaper device

    Parameters:
        spi: SPI_HandleTypeDef *
            pointer to SPI_HandleTypeDef, the SPI interface the epaper is on

    Returns:
        HAL_OK if software reset sent successfully
        HAL_TIMEOUT if display timedout
*/
HAL_StatusTypeDef epd_software_reset(SPI_HandleTypeDef *spi);

/*
    Completes a full reset of the epaper device

    Parameters:
        spi: SPI_HandleTypeDef *
            pointer to SPI_HandleTypeDef, the SPI interface the epaper is on

    Returns:
        HAL_OK if reset sent successfully
        HAL_TIMEOUT if display timedout
*/
HAL_StatusTypeDef epd_full_reset(SPI_HandleTypeDef *spi);

/*
    Completes a full reset of the epaper device

    Parameters:
        spi: SPI_HandleTypeDef *
            pointer to SPI_HandleTypeDef, the SPI interface the epaper is on

    Returns:
        HAL_OK if reset sent successfully
        HAL_TIMEOUT if display timedout
*/
HAL_StatusTypeDef epd_init(SPI_HandleTypeDef *spi);


#endif
