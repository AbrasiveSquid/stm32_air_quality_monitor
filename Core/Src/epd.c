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
            break;
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


