#include "epd.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdint.h>



HAL_StatusTypeDef epd_send_command(SPI_HandleTypeDef *spi, const uint8_t cmd_byte)
{
    // select the display
    HAL_GPIO_WritePin(EPD_CS_GPIO_Port, EPD_CS_Pin, GPIO_PIN_RESET);

    // select command
    HAL_GPIO_WritePin(EPD_DC_GPIO_Port, EPD_DC_Pin, GPIO_PIN_RESET);

    uint8_t cmd[] = {cmd_byte};
    const uint8_t CMD_LEN = 1;
    const uint32_t TIMEOUT_IN_MS = 1000;
    HAL_StatusTypeDef tx_status = HAL_SPI_Transmit(spi, cmd, CMD_LEN, TIMEOUT_IN_MS);

    // deselect display
    HAL_GPIO_WritePin(EPD_CS_GPIO_Port, EPD_CS_Pin, GPIO_PIN_SET);


    return tx_status;

}

HAL_StatusTypeDef epd_send_data(SPI_HandleTypeDef *spi, 
    const uint8_t data_buffer[], const uint16_t size)
{
    if (size == 0)
    {
        return HAL_ERROR;
    }

    // select the display
    HAL_GPIO_WritePin(EPD_CS_GPIO_Port, EPD_CS_Pin, GPIO_PIN_RESET);

    // select data
    HAL_GPIO_WritePin(EPD_DC_GPIO_Port, EPD_DC_Pin, GPIO_PIN_SET);

    const uint32_t TIMEOUT_IN_MS = 3000;
    HAL_StatusTypeDef tx_status = HAL_SPI_Transmit(spi, data_buffer, size, TIMEOUT_IN_MS);

    // deselect display
    HAL_GPIO_WritePin(EPD_CS_GPIO_Port, EPD_CS_Pin, GPIO_PIN_SET);


    return tx_status;
}

HAL_StatusTypeDef epd_busy_wait(void)
{
    GPIO_PinState busy_pin;
    const uint32_t TIMEOUT_MAX = 10000U;
    const uint32_t WAIT_TIME_IN_MS = 10;

    uint32_t busy_start = HAL_GetTick();
    while((uint32_t)(HAL_GetTick() - busy_start) < TIMEOUT_MAX) {
        busy_pin = HAL_GPIO_ReadPin(EPD_BUSY_GPIO_Port, EPD_BUSY_Pin);
        if (!(busy_pin))
        {
            HAL_Delay(WAIT_TIME_IN_MS);
            return HAL_OK;
        }
        HAL_Delay(WAIT_TIME_IN_MS);
    }
    HAL_Delay(WAIT_TIME_IN_MS);
    return HAL_TIMEOUT;
}

static void epd_hardware_reset(void)
{
    // this hardware reset sequenece adapted from waveshare code on their website for stm32

    // sets reset pin to high
    HAL_GPIO_WritePin(EPD_RST_GPIO_Port, EPD_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(20); // waits 20 ms for display to settle
    // sets the RST pin to low
    HAL_GPIO_WritePin(EPD_RST_GPIO_Port, EPD_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(2); // waits 2 ms for pin to be set
    // set pin back to high since it is active low
    HAL_GPIO_WritePin(EPD_RST_GPIO_Port, EPD_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(20); // waits 20 ms for display to settle
}

HAL_StatusTypeDef epd_software_reset(SPI_HandleTypeDef *spi)
{
    HAL_StatusTypeDef busy_status;

    busy_status = epd_busy_wait();
    if (busy_status != HAL_OK)
    {
        return busy_status;
    }

    uint8_t reset_cmd = 0x12;
    HAL_StatusTypeDef cmd_status = epd_send_command(spi, reset_cmd);
    if (cmd_status != HAL_OK)
    {
        return cmd_status;
    }

    busy_status = epd_busy_wait();
    if (busy_status != HAL_OK)
    {
        return busy_status;
    }

    return busy_status;    
}

HAL_StatusTypeDef epd_full_reset(SPI_HandleTypeDef *spi)
{
    // do a hardware then software reset
    epd_hardware_reset();

    HAL_StatusTypeDef cmd_status = epd_software_reset(spi);
    if (cmd_status != HAL_OK)
    {
        return cmd_status;
    }
    return HAL_OK;
}

HAL_StatusTypeDef epd_init(SPI_HandleTypeDef *spi)
{
    HAL_StatusTypeDef tx_status;
    // do a device reset
    if ((tx_status = epd_full_reset(spi)) != HAL_OK)
    {
        return tx_status;
    }

    // configure screen for 250 rows
    const uint8_t driver_output_control_byte = 0x01;
    const uint16_t set_rows_len = 3;
    const uint8_t set_rows[] = {0xf9, 0x0, 0x0};

    if ((tx_status = epd_send_command(spi, driver_output_control_byte)) != HAL_OK)
    {
        return tx_status;
    }
    if ((tx_status = epd_send_data(spi, set_rows, set_rows_len)) != HAL_OK)
    {
        return tx_status;
    }

    // configure RAM address direction
    const uint8_t data_entry_cmd = 0x11;
    const uint16_t ram_direction_len = 1;
    const uint8_t ram_direction[] = {0x03};

    if ((tx_status = epd_send_command(spi, data_entry_cmd)) != HAL_OK)
    {
        return tx_status;
    }
    if ((tx_status = epd_send_data(spi, 
        ram_direction, ram_direction_len)) != HAL_OK)
    {
        return tx_status;
    }

    // set window size
    const uint16_t x_and_y_start = 0;
    if ((tx_status = epd_set_window(spi, 
        x_and_y_start, x_and_y_start, 
        EPD_WIDTH - 1, EPD_HEIGHT -1)) != HAL_OK)
    {
        return tx_status;
    }

    // set cursor initial position
    if ((tx_status = epd_set_cursor(spi, x_and_y_start, x_and_y_start)) != HAL_OK)
    {
        return tx_status;
    }


    // configure display border
    const uint8_t display_border_cmd = 0x3c;
    const uint16_t display_border_len = 1;
    const uint8_t display_border_data[] = {0x05};
    if ((tx_status = epd_send_command(spi, display_border_cmd)) != HAL_OK)
    {
        return tx_status;
    }
    if ((tx_status = epd_send_data(spi, 
        display_border_data, display_border_len)) != HAL_OK)
    {
        return tx_status;
    }


    // configure display update behaviour
    const uint8_t display_border_update_cmd = 0x21;
    const uint16_t display_border_update_len = 2;
    const uint8_t display_border_update_data[] = {0x0, 0x80};
    if ((tx_status = epd_send_command(spi, display_border_update_cmd)) != HAL_OK)
    {
        return tx_status;
    }
    if ((tx_status = epd_send_data(spi, 
        display_border_update_data, display_border_update_len)) != HAL_OK)
    {
        return tx_status;
    }


    // configure temperature-sensor
    const uint8_t read_temp_sensor_cmd = 0x18;
    const uint16_t read_temp_sensor_len = 1;
    const uint8_t read_temp_sensor_data[] = {0x80};
    if ((tx_status = epd_send_command(spi, read_temp_sensor_cmd)) != HAL_OK)
    {
        return tx_status;
    }
    if ((tx_status = epd_send_data(spi, 
        read_temp_sensor_data, read_temp_sensor_len)) != HAL_OK)
    {
        return tx_status;
    }


    return epd_busy_wait(); // do a final wait until not busy
}

HAL_StatusTypeDef epd_set_window(
    SPI_HandleTypeDef *spi,
    uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    HAL_StatusTypeDef tx_status;
    
    const uint8_t set_ram_x_start_end_pos = 0x44;
    if ((tx_status = epd_send_command(spi, set_ram_x_start_end_pos)) != HAL_OK) 
    {
        return tx_status;
    }

    uint8_t x_start_byte = (x_start >> 3) & 0xff;
    uint8_t x_end_byte = (x_end >> 3) & 0xff; 
    const uint16_t x_positions_len = 2;
    const uint8_t x_postions[] = {x_start_byte, x_end_byte};
    if ((tx_status = epd_send_data(spi, x_postions, x_positions_len)) != HAL_OK )
    {
        return tx_status;
    }


     const uint8_t set_ram_y_start_end_pos = 0x45;
    if ((tx_status = epd_send_command(spi, set_ram_y_start_end_pos)) != HAL_OK) 
    {
        return tx_status;
    }

    uint8_t y_start_byte_1 = y_start  & 0xff;
    uint8_t y_start_byte_2 = (y_start >> 8)  & 0xff;

    uint8_t y_end_byte_1 = y_end  & 0xff;
    uint8_t y_end_byte_2 = (y_end >> 8)  & 0xff;
    const uint16_t y_positions_len = 4;
    const uint8_t y_postions[] = {
        y_start_byte_1, y_start_byte_2,  
        y_end_byte_1, y_end_byte_2};
    if ((tx_status = epd_send_data(spi, y_postions, y_positions_len)) != HAL_OK )
    {
        return tx_status;
    }
    return HAL_OK;
}

HAL_StatusTypeDef epd_set_cursor(SPI_HandleTypeDef *spi,uint16_t x_start, uint16_t y_start)
{
    HAL_StatusTypeDef tx_status;

    // set column position
    const uint8_t x_ram_address_counter_cmd = 0x4e;
    if ((tx_status = epd_send_command(spi, x_ram_address_counter_cmd)) != HAL_OK)
    {
        return tx_status;
    }
    uint16_t x_position = x_start & 0xff;
    const uint16_t x_position_buf_len = 1;
    const uint8_t x_position_buf[] = {x_position};
    if ((tx_status = epd_send_data(spi, x_position_buf, x_position_buf_len)) != HAL_OK)
    {
        return tx_status;
    }

    // set row position
    const uint8_t y_ram_address_counter_cmd = 0x4f;
    if ((tx_status = epd_send_command(spi, y_ram_address_counter_cmd)) != HAL_OK)
    {
        return tx_status;
    }
    uint16_t y_position_byte_1 = y_start & 0xff;
    uint16_t y_position_byte_2 = (y_start >> 8) & 0xff;

    const uint16_t y_position_buf_len = 2;
    const uint8_t y_position_buf[] = {y_position_byte_1, y_position_byte_2};
    if ((tx_status = epd_send_data(spi, y_position_buf, y_position_buf_len)) != HAL_OK)
    {
        return tx_status;
    }

    return HAL_OK;
}
