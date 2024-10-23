/**
 * @file app_control.h
 * @brief This script is to control the application related codes.
 * @version 1.0
 * @date 2024-07-22
 * @copyright Copyright (c) 2024
 */

#ifndef APP_CONTROL_H
#define APP_CONTROL_H

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
#define BATTERY_VOLTAGE_SIZE 100
#define TEMPERATURE_SIZE 5
#define ACCELERATION_SIZE 5

/**
 * @name VARIABLES
 * 
 */
extern int a;

/**
 * @name FUNCTIONS
 * 
 */

#endif /* APP_CONTROL_H */