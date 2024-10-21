/**
 * @file recovery.h
 * @brief This script is to control the application related codes.
 * @version 1.0
 * @date 2024-10-18
 * @copyright Copyright (c) 2024
 */

#ifndef RECOVERY_H
#define RECOVERY_H

/**
 * @name INCLUDES
 * 
 */

#include "main.h"
//#include "bsp_init.h"   // Board support package header file & printf retargetting statement
#include "ff.h"
#include "diskio.h"     // Disk I/O header file
#include "fatfs.h"      // FATFS configuration header file (if using STM32CubeMX)
#include "ff_gen_drv.h" // Generic driver header file
#include "ffconf.h"
//#include "ifile.h"
#include "usart.h"
//#include "iusart.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "arm_math.h"
#include "diagnosis.h"


/**
 * @name MACROS
 * 
 */


/**
 * @name VARIABLES
 * 
 */


/**
 * @name FUNCTIONS
 * 
 */
 void moving_average_filter(float* data, int size, int window_size, float* filtered_data);
 void trend_recovery(float* data, int size, int window_size);
 void outlier_recovery(float* data, int size);
 void correct_bias(float* acceleration, int acceleration_size, float* smoothed_accel, int bias_index, int max_derivative_index, int window_size);
 int find_bias_point(float* data, int size, int window_size);
 void process_segment(float* data, int size);

#endif /* APP_CONTROL_H */






