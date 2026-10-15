/* source code file for the driver for the sensirion sps30 PM sensor for an stm32 nucelo*/
#include "sps30.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include <stdint.h>
#include <string.h>

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

int stop_measurement(UART_HandleTypeDef huart)
{
    // byte sequence that starts measurements using IEEE floats
    uint8_t start_measurement_float_cmd[] = {0x7e, 0x00, 0x01, 0x00, 0xfe, 0x7e};
    HAL_StatusTypeDef tx_status;
    int cmd_len = 6;
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
    const uint8_t EMPTY_RESPONSE_FRAME[] = {0x7e, 0x00, 0x01, 0x00, 0x00, 0xfe, 0x7e};

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
                return 0; // return error code
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
        return 0; // if not stop byte, error 
    }

    return i; // success

}


int get_pm_data(UART_HandleTypeDef uart, float data[], int data_len)
{
    int raw_buff_len = 262;
    uint8_t raw_buff[raw_buff_len];
    int err; // used for error codes from function return values

    int bytes_received = get_response_frame(uart, raw_buff, raw_buff_len);
    if (!bytes_received)
    {
        return 1;
    }

    // raw_buff starts and ends with 0x7e start/stop byte as verified in get_response_frame

    // unpack the bytes
    int unstuffed_bytes_buff_len = 262;
    uint8_t unstuffed_bytes_buff[unstuffed_bytes_buff_len];
    UnstuffedReturn unstuff_return = unstuff_bytes(raw_buff, unstuffed_bytes_buff, unstuffed_bytes_buff_len );
    
    // check for error
    if (unstuff_return.err_code)
    {
        return 1;
    }

    // verify checksum
    int checksum = verify_checksum(unstuffed_bytes_buff, unstuff_return.array_len);
    if (!checksum) {
        return 1; // checksum is invalid
    }

    // check that the number of bytes in data is correct
    const int LENGTH_BYTE = 4; // position of length byte
    const int LENGTH_BYTE_CORRECT_SIZE = 0x28; // correct size must be 40 bytes for pm data
    if (unstuffed_bytes_buff[LENGTH_BYTE] !=  LENGTH_BYTE_CORRECT_SIZE) 
    {
        return 1;
    }

    // flip endianness
    const int CHUNK_SIZE = 4;
    const int ARRAY_SIZE = 40;
    const int HEADER_OFFSET = 5;

    if (data_len != (ARRAY_SIZE/4))
    {
        return 1; // the received data will not fit inside data array
    }
    uint8_t data_flip_endianess[ARRAY_SIZE];
    err =  flip_endianess(unstuffed_bytes_buff + HEADER_OFFSET, unstuff_return.array_len,
        data_flip_endianess, ARRAY_SIZE, CHUNK_SIZE);

    if (err)
    {
        return 1;
    }

    // copy byte for byte into data, which will present data as floats
    memcpy(data, data_flip_endianess, ARRAY_SIZE);

    return 0; // success
}

UnstuffedReturn unstuff_bytes(uint8_t frame[], uint8_t unstuffed_frame[], int frame_len)
{
    const uint8_t START_STOP_BYTE = 0x7e; 
    const uint8_t STUFFED_BYTE_MARKER = 0x7d; // if this byte appears, is a stuffed byte 
    UnstuffedReturn unstuffed_return = {0}; // initialize err_code and arr_len to 0
    uint8_t *curr_byte = frame; // set curr_byte to start of frame
    
    // check curr_byte is a start of frame
    if (*curr_byte != START_STOP_BYTE)
    {
        unstuffed_return.err_code = 1;
        return unstuffed_return;
    }

    // copy start_byte into unstuffed_frame before loop
    unstuffed_frame[0] = *curr_byte;

    // increment curr_byte to next byte in frame
    curr_byte++;
    int i = 1;
    while (*curr_byte != START_STOP_BYTE && i < frame_len)
    {

        // check for stuffed byte
        if (*curr_byte == STUFFED_BYTE_MARKER)
        {
            // if stuffed byte, unstuff it
            if (*(curr_byte+1) == 0x5e)
            {
            unstuffed_frame[i] = 0x7e;
            }
            else if (*(curr_byte+1) == 0x5d)
            {
            unstuffed_frame[i] = 0x7d;
            }
            else if (*(curr_byte+1) == 0x31)
            {
            unstuffed_frame[i] = 0x11;
            } 
            else if (*(curr_byte+1) == 0x33)
            {
            unstuffed_frame[i] = 0x13;
            }
            else 
            {
                // error as no other possible stuffed bytes
                unstuffed_return.err_code = 1;
                return unstuffed_return;
            }

            // increment by 2 to skip stuffed byte
            curr_byte += 2;
            i++;
            continue;
        }

        // otherwise copy byte over and increment i and curr_byte
        unstuffed_frame[i++] = *curr_byte++;
    }

    // check that final byte is end of frame and copy it
    if (*curr_byte == START_STOP_BYTE)
    {
        unstuffed_frame[i++] = *curr_byte; // increment i
    }
    else 
    {
        // last byte not the stop byte, error
        unstuffed_return.err_code = 1;
        return unstuffed_return;
    }

    // set array len to index
    unstuffed_return.array_len = i;

    return unstuffed_return;
}


int verify_checksum(uint8_t frame[], int frame_len)
{
    if (frame_len < 3)
    {
        return 0; // frame_len too short, will be outside bounds 
    }

    const uint8_t START_STOP_BYTE = 0x7e; 

    // frame_len -2 because checksum is second last byte
    const uint8_t CHECKSUM_BYTE = frame[frame_len - 2];

    if (!(frame[0] == START_STOP_BYTE && frame[frame_len-1] == START_STOP_BYTE))
    {
        return 0; // invalid frame returns false
    }

    int byte_sum = 0;

    // start at byte 1 to skip start byte, end at i -2 to skip stop byte and check sumbyte
    for (int i = 1; i < frame_len -2; ++i)
    {
        byte_sum += frame[i];
    }

    // get the lsb of byte sum by casting to 8 bit value, then invert it
    uint8_t lsb_byte_sum = (uint8_t) byte_sum;
    uint8_t invert_lsb = ~lsb_byte_sum;
    
    if (invert_lsb != CHECKSUM_BYTE) {
        return 0; // returns false
    }

    return 1; // returns true, checksum valid
}

int flip_endianess(uint8_t frame[], int frame_len, 
    uint8_t endian_flip_frame[], int flip_frame_len, int chunk_size)
{
    if (flip_frame_len % chunk_size != 0)
    {
        // must be evenly divisible
        return 1;
    }

    uint8_t *curr_byte;
    for (int i = 0; i * chunk_size < flip_frame_len; ++i)
    {
        // get address of curr byte
        curr_byte = &frame[i*chunk_size];

        // flip bytes from curr_byte up to curr_byte + chunksize
        for (int j = 3, k = 0; j >= 0; j--, k++)
        {
            endian_flip_frame[j+(i*chunk_size)] = *(curr_byte+k);
        }
    }

    return 0;

}
