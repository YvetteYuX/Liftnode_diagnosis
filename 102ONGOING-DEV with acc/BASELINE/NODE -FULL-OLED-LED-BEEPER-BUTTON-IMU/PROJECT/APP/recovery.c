/**
 * @file recovery.c
 * @brief This script is to recover the trend, outlier and bias.
 * @version 1.0
 * @date 2024-10-18
 * @copyright Copyright (c) 2024
 */

/**
 * @name INCLUDES
 *
 */
#include "app_control.h"


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
 * @brief Sudden damage detection function: I - Data process, II - Edge computation: WT + ICA + NN
 *
 */
 

// Function to calculate the moving average
void moving_average_filter(float* data, int size, int window_size, float* filtered_data) {
    for (int i = 0; i < size; i++) {
        int start = (i - window_size / 2 < 0) ? 0 : i - window_size / 2;
        int end = (i + window_size / 2 >= size) ? size - 1 : i + window_size / 2;
        
        float sum = 0.0;
        int count = 0;
        for (int j = start; j <= end; j++) {
            sum += data[j];
            count++;
        }
        filtered_data[i] = sum / count; // calculate the average
    }
}

// Function for trend recovery using moving average window filter
void trend_recovery(float* data, int size, int window_size) {
    float filtered_data[size];
    float original_mean = 0.0;

    // Calculate the moving average
    moving_average_filter(data, size, window_size, filtered_data);

    // Calculate the mean of the original data
    for (int i = 0; i < size; i++) {
        original_mean += data[i];
    }
    original_mean /= size; // overall mean

    // Perform trend recovery
    for (int i = 0; i < size; i++) {
        data[i] = data[i] - filtered_data[i] + original_mean; // trend recovery formula
    }
}

void outlier_recovery(float* data, int size) {
    // Step 1: Calculate the mean and standard deviation of the acceleration data
    float sum = 0.0, sum_sq = 0.0, mean = 0.0, std_dev = 0.0;
    
    for (int i = 0; i < size; i++) {
        sum += data[i];
        sum_sq += data[i] * data[i];
    }
    mean = sum / size;
    std_dev = sqrt((sum_sq / size) - (mean * mean));

    // Step 2: Set threshold (four times the standard deviation)
    float upper_bound = 4 * std_dev;

    // Step 3: Detect the segment with spike fault
    for (int i = 0; i < size; i++) {
        if (fabs(data[i] - mean) > upper_bound) {
            // Outlier detected, perform outlier recovery on this segment
            printf("Outlier detected at index %d, performing recovery...\n", i);
            
            // Step 4: Replace values larger than three standard deviations + mean
            for (int j = i; j < size && fabs(data[j] - mean) > (3 * std_dev); j++) {
                if (fabs(data[j] - mean) > (3 * std_dev)) {
                    data[j] = mean; // replace with mean
                }
            }
        }
    }
}


