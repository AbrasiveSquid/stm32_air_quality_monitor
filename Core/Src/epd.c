#include "epd.h"
#include "main.h"



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
