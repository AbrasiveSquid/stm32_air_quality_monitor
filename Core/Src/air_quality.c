#include "air_quality.h"
#include "sps30.h"

HAL_StatusTypeDef sample_pm_data(UART_HandleTypeDef uart, float data[], int data_len)
{
    if (data_len < 10) // data array not long enough to save collected data
    {
        return HAL_ERROR;
    }

    // start the measurements
    HAL_StatusTypeDef return_code;
    if ((return_code = start_measurement(uart)) != HAL_OK)
    {
      return return_code;
    }
  
    // wait 30 seconds to stablize
    const int STABILIZATION_TIME_IN_MS = 30000;
    HAL_Delay(STABILIZATION_TIME_IN_MS);

    // setup variables needed for 30 sampling runs
    const int PM_DATA_LEN = 10;
    float curr_pm_data[10] = {0}; // store current_readngs
    float pm_data_sum[10] = {0}; // sum of all the pm data measurements
    int successful_sample_runs = 0;
    const int TOTAL_SAMPLE_RUNS = 30;
    
    // run 30 sampling runs with 1 second between samples
    for (int i = 0; i < TOTAL_SAMPLE_RUNS; i++)
    {
        HAL_Delay(1000);
        if ((return_code = poll_measurements(uart)) != HAL_OK)
        { 
            // cont_measurements = false; // if error code flip flag so nothing else runs
            continue; // end current run and try again
        }
    
      if ((return_code = get_pm_data(uart, curr_pm_data, PM_DATA_LEN)) == HAL_OK)
      {
        for (int i = 0; i <PM_DATA_LEN; i++)
        {
            pm_data_sum[i] += curr_pm_data[i];
        }
        successful_sample_runs++;

      }
    }

    // stop the measurement, if it fails return to error code
    HAL_StatusTypeDef stop_status;
    if ((stop_status = stop_measurement(uart)) != HAL_OK) 
    {
      return stop_status;
    }

    // check there was at least one successful run
    if (successful_sample_runs == 0)
    {
      return HAL_ERROR;
    }

    // calculate average
    for (int i = 0; i < PM_DATA_LEN; i++)
    {
        data[i] = (pm_data_sum[i] / successful_sample_runs);
    }
  

    return HAL_OK;


}
