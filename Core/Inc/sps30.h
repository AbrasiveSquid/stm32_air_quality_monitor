/* Header file for the driver for the sensirion sps30 PM sensor for an stm32 nucelo*/

#ifndef SPS30_H
#define SPS30_h

#include "stm32f4xx_hal.h"


/*
    start measuring data EEE754 float values

    Parameters:
        huart: UART_HandleTypeDef
            determines what ports to transmit over

    Returns:
        int:
            0 if success
            1 if any errors
*/
int start_measurement(UART_HandleTypeDef huart);

/*
    Check for new measurements, need to send this command before receiving

    Parameters:
        huart: UART_HandleTypeDef
            determines what ports to transmit over

    Returns:
        int:
            0 if success
            1 if any errors
*/
int poll_measurements(UART_HandleTypeDef huart);


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
            0 if success
            1 if any errors
*/
int get_response_frame(UART_HandleTypeDef huart, uint8_t frame[], int frame_len);



#endif
