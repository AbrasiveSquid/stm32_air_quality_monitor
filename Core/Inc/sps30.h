/* Header file for the driver for the sensirion sps30 PM sensor for an stm32 nucelo*/

#ifndef SPS30_H
#define SPS30_h

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct 
{
    int err_code;
    int array_len;
} UnstuffedReturn;

/*
    start measuring data EEE754 float values

    Parameters:
        huart: UART_HandleTypeDef
            determines what ports to transmit over

    Returns:
        int:
            HAL_OK if success
            HAL_ERROR if any errors
*/
HAL_StatusTypeDef start_measurement(UART_HandleTypeDef huart);

/*
    stop measuring data

    Parameters:
        huart: UART_HandleTypeDef
            determines what ports to transmit over

    Returns:
        int:
            HAL_OK if success
            HAL_ERROR if any errors
*/
HAL_StatusTypeDef stop_measurement(UART_HandleTypeDef huart);

/*
    Check for new measurements, need to send this command before receiving

    Parameters:
        huart: UART_HandleTypeDef
            determines what ports to transmit over

    Returns:
        int:
            HAL_OK if success
            HAL_ERROR if any errors
*/
HAL_StatusTypeDef poll_measurements(UART_HandleTypeDef huart);


/*
    Receives a response from the sps30 and puts each byte into the frame
    
    Parameters:
        huart: UART_HandleTypeDef
            determines what ports to transmit over

        frame: uint8_t array
            the array to put the received bytes into
        
        frame_len: int
            the number of bytes that can be received

    Returns:
        int:
           > 0, the number of bytes received into the frame
           0, HAL_TIMEOUT, no valid frame received
           -1 HAL_BUSY
           -2 HAL_ERROR

*/
int get_response_frame(UART_HandleTypeDef huart, uint8_t frame[], int frame_len);

/*
    Interprets err_code and returns the proper HAL_StatusTypeDef

    err_code            HAL_StatusTypeDef
    0                   HAL_TIMEOUT
    -1                  HAL_BUSY
    -2                  HAL_ERROR

    Parameters:
        err_code: int
             a 0 or negative integer value

    Returns:
        HAL_StatusTypeDef
*/
HAL_StatusTypeDef handle_error_codes(int err_code);



/*
    Handles getting the measured data from the sensor, and converting the raw bytes
    into floats, and putting them into pm_data
    
    In position 0-9 it puts this data:
    concentrations in ug/m3:
        0 = PM1.0 
        1 = PM2.5 
        2 = PM4.0 
        3 = PM10 

    number counts in #/cm3:
        4 = PM0.5  
        5 = PM1.0  
        6 = PM2.5  
        7 = PM4.0  
        8 = PM10 

    particle size in um:
        9 - typical particle size


    Parameters:
        huart: UART_HandleTypeDef
            determines what ports to transmit over

        data: float array
            an array of floats to store the pm data
        
        data_len: int
            the length of the array

    Returns:
        int:
            0 if success
            1 for HAL_TIMEOUT
            2 for HAL_BUSY
            3 for HAL_ERROR
*/
int get_pm_data(UART_HandleTypeDef uart, float data[], int data_len);

/*
    Unstuffs the bytes in frame with the exception of the start and stop if the following
    bytes appear in the data, they are converted into type bytes.
    0x7e -> 0x7d, 0x5e
    0x7d -> 0x7d, 0x5d
    0x11 -> 0x7d, 0x31
    0x13 -> 0x7d, 0x33

    So if 0x7d is found in frame, the next byte is checked and then those bytes are replaced with
    the bytes they were supposed to be (left hand side above)

    Parameters:
        frame: uint8_t array
            an array of bytes to be checked for unstuffing

        unstuffed_frame: uint8_t array
            a frame where unstuffed bytes will be copied to

        frame_len: int
            length of frame and unstuffed_frame

    Returns:
        UnstuffedReturn: a struct
            contains two values: 
                int err_code: 
                    if there is any error, it is added here, 0 if no error, otherwise 1

                array_len: int
                    the number of bytes copied into unstuffed_frame

    Postcondition:
        unstuffed_frame will be a copy of frame, with the exception of any sequence of 0x7d and 
        another byte, those two bytes will be replced by a single byte as described above
    */
UnstuffedReturn unstuff_bytes(uint8_t frame[], uint8_t unstuffed_frame[], int frame_len);

/*
    Verifies the data in frame is valid by verifying the checksum
    To verify checksum, all bytes are summed between start and stop byte (exclusive).
    Then take the least significant byte of the sum and invert the byte.
    
    Parameters:
        frame: uint8_t array
            array of unstuffed bytes
            
        frame_len: int
            the number of bytes in frame
    
    Returns:
        int:
            1 for checksum is valid    
            0 for checksum is invalid
    
    Preconditions:
        frame bytes must be unstuffed
        There must be a byte 0x7e in the 0th position and the frame_len - 1 position
*/
int verify_checksum(uint8_t frame[], int frame_len);

/*
    Flips the endianness of frame and copies the flipped bytes in chunks of chunksize
    For example if chunksize is 4, it goes 4 bytes at a time and reverse the order.
    The new order is then copied into endian_flip_frame
    
    Parameters:
        frame: uint8_t array
            the frame to have the bytes flipped
        
        frame_len: int
            number of bytes in frame

        endian_flip_frame: uint8_t array
            array to have the copied bytes to

        flip_frame_len: int
            number of bytes that can fit into endian_flip_frame

        chunk_size: int
            the number of bytes to flip order, for example if 2, then goes 2 bytes at a time
            and reverses the order of those 2

    Returns:
        int:
            0 if success
            1 if any errors

    Precondition:
        flip_frame_len must be divisible by chunk_size

    Postcondition:
        endian_flip_frame will be a copy of frame up to flip_frame_len, but the order of bytes
        will be reversed in grouping of chunk size

    Examples:
        chunk_size = 4
        frame = [0x00, 0x11, 0x12, 0x13, 0x15, 0x16, 0x17, 0x18]
        after processing:
        endian_flip_frame = [0x13, 0x12, 0x11, 0x00, 0x18, 0x17, 0x16, 0x15]
*/
int flip_endianess(uint8_t frame[], int frame_len, 
    uint8_t endian_flip_frame[], int flip_frame_len, int chunk_size);

#endif
