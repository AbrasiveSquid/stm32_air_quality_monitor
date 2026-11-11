/* Header file for the driver for the waveshare 2.13inch e-Paper HAT v4 */

#ifndef EPD_H
#define EPD_H
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_spi.h"
#include <stdint.h>
#include <stdbool.h>

#define EPD_WIDTH 122
#define EPD_HEIGHT 250


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
void epd_hardware_reset(void);


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
    Does an initialization for the epaper device

    Parameters:
        spi: SPI_HandleTypeDef *
            pointer to SPI_HandleTypeDef, the SPI interface the epaper is on

    Returns:
        HAL_OK if display initialized successfully
        HAL_TIMEOUT if display timedout
        HAL_TIMEOUT for all other errors
*/
HAL_StatusTypeDef epd_init(SPI_HandleTypeDef *spi);

/*
    Sets the RAM window for writing to the epaper display

    Parameters:
        spi: SPI_HandleTypeDef *
            pointer to SPI_HandleTypeDef, the SPI interface the epaper is on
        
        x_start: uint16_t
            the initial column
        
        y_start: uint16_t
            the initial row
        
        x_end: uint16_t
            the last column

        y_end: uint16_t
            the final row
    
    Returns:
        HAL_OK if command sent successfully
        HAL_TIMEOUT if display timedout
        HAL_BUSY if display is busy
        HAL_ERROR for all other errors
*/
HAL_StatusTypeDef epd_set_window(
    SPI_HandleTypeDef *spi,
    uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);


/*
    Sets the cursor position for writing to epaper display

    Parameters:
        spi: SPI_HandleTypeDef *
            pointer to SPI_HandleTypeDef, the SPI interface the epaper is on
        
        x_start: uint16_t
            the column ps for the cuorsor
        
        y_start: uint16_t
            the row ps for the cuorsor
    
    Returns:
        HAL_OK if command sent successfully
        HAL_TIMEOUT if display timedout
        HAL_BUSY if display is busy
        HAL_ERROR for all other errors
*/
HAL_StatusTypeDef epd_set_cursor(SPI_HandleTypeDef *spi,uint16_t x_start, uint16_t y_start);

/*
    Updates the framebuffer to all white pixels to prepare to clear screen

*/
void epd_clear_screen(void);

/* 
    Refreshes the epaper display by updating pixels to match what is in ram

    Parameters:
        spi: SPI_HandleTypeDef *
            pointer to SPI_HandleTypeDef, the SPI interface the epaper is on

    Returns:
        HAL_OK if command sent successfully
        HAL_TIMEOUT if display timedout
        HAL_BUSY if display is busy
        HAL_ERROR for all other errors
*/
HAL_StatusTypeDef epd_display_refresh(SPI_HandleTypeDef *spi);

/*
    Sends the frame_buffer to the epaper display and updates it to display what is currently
    in the frame buffer

    Parameters:
        spi: SPI_HandleTypeDef *
            pointer to SPI_HandleTypeDef, the SPI interface the epaper is on
    
*/
HAL_StatusTypeDef epd_update_display(SPI_HandleTypeDef *spi);

/* 
    Draws a one pixel to the epaper display

    Parameters:
        x_coord: uint8_t
            the x coordinate of the pixel (column)
        
        y_coord: uint8_t
            the y coordinate of the pixel (row)

        colour: uint8_t
            the colour of the pixel 0 for black, 1 for white

    Returns:
        HAL_OK if command sent successfully
        HAL_TIMEOUT if display timedout
        HAL_BUSY if display is busy
        HAL_ERROR for all other errors
*/
HAL_StatusTypeDef epd_draw_pixel(uint8_t x_coord, uint8_t y_coord, uint8_t colour);

/*
    Verifies if a width and a height are within the bounds of the epaper display
    
    Parameters:
        width: const uint8_t
            the number of columns within the epaper display, defined in EPD_WIDTH

        height: const uint8_t
            the number of rows within the epaper display, defined in EPD_HEIGHT


    Returns:
        true if within bounds, otherwise false
*/
bool verify_coordinates_within_bounds(const uint8_t width, const uint8_t height);





// HAL_StatusTypeDef epd_draw_line(SPI_HandleTypeDef *spi);


#endif

