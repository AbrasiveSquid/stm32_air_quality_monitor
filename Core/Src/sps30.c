/* source code file for the driver for the sensirion sps30 PM sensor for an stm32 nucelo*/
#include "sps30.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include <stdint.h>

int start_measurement(UART_HandleTypeDef huart)
{
    // byte sequence that starts measurements using IEEE floats
    uint8_t start_measurement_float_cmd[] = {0x7e, 0x00, 0x00, 0x02, 0x01, 0x03, 0xf9, 0x7e};
    HAL_StatusTypeDef tx_status;
    int cmd_len = 8;
    int time_out_in_ms = 2000;
    
    while ((tx_status = HAL_UART_Transmit(&huart, start_measurement_float_cmd, cmd_len, time_out_in_ms)) != HAL_OK)
    {
        if (tx_status == HAL_TIMEOUT)
        {
            HAL_Delay(1000); // wait one second and try again
            continue;
        }
        // just return an error if can't start up
        return 1;
    }

    // get response frame
    int len_response_frame = 7;
    uint8_t response_frame[len_response_frame];

    int err_code = get_response_frame(huart, response_frame, len_response_frame);
    const uint8_t EMPTY_RESPONSE_FRAME[] = {0x7e, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7e};

    if (err_code)
    {
        return 1;
    }

    // response frame must match empty response frame
    for (int i = 0; i < len_response_frame; i++)
    {
        if (response_frame[i] != EMPTY_RESPONSE_FRAME[i])
        {
            return 1;
        }
    }

    // successful started measurements
    return 0;
}


int poll_measurements(UART_HandleTypeDef huart)
{
    // byte sequence that starts the read sensor command
    uint8_t read_sensor_cmd[] = {0x7e, 0x00, 0x03, 0x00, 0xfc, 0x7e};
    int cmd_len = 6;
    int time_out_in_ms = 2000;
    HAL_StatusTypeDef tx_status;

    while ((tx_status = HAL_UART_Transmit(&huart, read_sensor_cmd, cmd_len, time_out_in_ms)) !=HAL_OK)
    {
         if (tx_status == HAL_TIMEOUT)
        {
            HAL_Delay(1000); // wait one second and try again
            continue;
        }
        // just return an error if can't start up
        return 1;
    }

    // can now receive new measurements
    return 0;
}

int get_response_frame(UART_HandleTypeDef huart, uint8_t frame[], int frame_len)
{
    const uint8_t START_STOP_BYTE = 0x7e;
    HAL_StatusTypeDef rx_status;
    int i = 0;
    int time_out_in_ms = 2000;
    int time_out_counter = 0;
    int start_frame = 0; // boolean flag to check that first byte is the START_STOP_BYTE




    while (i < frame_len)
    {
        // receive 1 byte at a time
        rx_status = HAL_UART_Receive(&huart, frame + i, 1, time_out_in_ms);
        
        // if timeout, wait 1 second and try again
        if (rx_status == HAL_TIMEOUT)
        {
            if(time_out_counter == 5) {
                return 1; // return error code
            }
            HAL_Delay(1000); // wait 1 second
            time_out_counter++;
            continue;
        }

        // check if byte is START byte
        if (!start_frame && frame[i] == START_STOP_BYTE) 
        {
            start_frame = 1; // receive frame to start byte so flag is now true
        }
        else if (frame[i] == START_STOP_BYTE)
        {
            // received last byte
            break;
        }

        i++;
    }

    // check that last byte received was stop byte
    if (frame[i] != START_STOP_BYTE)
    {
        return 1; // if not stop byte, error
    }

    return 0; // success

}
