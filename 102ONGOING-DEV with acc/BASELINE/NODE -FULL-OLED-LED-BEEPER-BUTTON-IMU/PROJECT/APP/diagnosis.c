/**
 * @file diagnosis.c
 * @brief This script is to perform wavelet transform on the generated signal.
 * @version 1.0
 * @date 2024-08-12
 * @copyright Copyright (c) 2024
 */

/**
 * @name INCLUDES
 *
 */
#include "diagnosis.h"
#include "recovery.h"
#include "mpu6050.h"
#include "app_control.h"
#include "setup.h"

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h> 

/**
 * @name MACROS
 *
 */
#define NOISE_TOLERANCE 0.1f // noise tolerance
#define WINDOW_SIZE 10
#define GYRO_THRESHOLD 200 
/**
 * @name VARIABLES
 *
 */

/**
 * @name FUNCTIONS
 *
 */

// Function to calculate metrics
SensorMetrics calculate_metrics(SensorData sensor_data) {
    SensorMetrics metrics = {0};

    // Calculate battery voltage metrics
    
    metrics.max_voltage = sensor_data.battery_voltage[0];
    metrics.min_voltage = sensor_data.battery_voltage[0];
    metrics.battery_size = sizeof(sensor_data.battery_voltage) / sizeof(sensor_data.battery_voltage[0]);
    float total_voltage = 0.0;
    
    for (int i = 0; i < metrics.battery_size; i++) {
        if (sensor_data.battery_voltage[i] > metrics.max_voltage)
            metrics.max_voltage = sensor_data.battery_voltage[i];
        if (sensor_data.battery_voltage[i] < metrics.min_voltage)
            metrics.min_voltage = sensor_data.battery_voltage[i];
        total_voltage += sensor_data.battery_voltage[i];
    }
    metrics.avg_voltage = total_voltage / metrics.battery_size;

    // Calculate temperature metrics
    metrics.max_temp = sensor_data.temperature[0];
    metrics.min_temp = sensor_data.temperature[0];
    metrics.temperature_size = sizeof(sensor_data.temperature) / sizeof(sensor_data.temperature[0]);
    float total_temp = 0.0;


    for (int i = 0; i < metrics.temperature_size; i++) {
        if (sensor_data.temperature[i] > metrics.max_temp)
            metrics.max_temp = sensor_data.temperature[i];
        if (sensor_data.temperature[i] < metrics.min_temp)
            metrics.min_temp = sensor_data.temperature[i];
        total_temp += sensor_data.temperature[i];
    }
    metrics.avg_temp = total_temp / metrics.temperature_size;

    // Calculate acceleration metrics
    metrics.max_accel = sensor_data.acceleration[0];
    metrics.min_accel = sensor_data.acceleration[0];
    metrics.acceleration_size = sizeof(sensor_data.acceleration) / sizeof(sensor_data.acceleration[0]);
    float total_accel = 0.0;

    for (int i = 0; i < metrics.acceleration_size; i++) {
        if (sensor_data.acceleration[i] > metrics.max_accel)
            metrics.max_accel = sensor_data.acceleration[i];
        if (sensor_data.acceleration[i] < metrics.min_accel)
            metrics.min_accel = sensor_data.acceleration[i];
        total_accel += sensor_data.acceleration[i];
    }
    metrics.avg_accel = total_accel / metrics.acceleration_size;
		
		// Calculate gyroscope metrics
    metrics.gyroscope_size = sizeof(sensor_data.gyroscope) / sizeof(sensor_data.gyroscope[0]);

    // Calculate resolution and range
    metrics.range = current_accel_range;  // 6g gives a total range of 12g
    metrics.resolution = metrics.range / (1 << 12);  // Example for a 12-bit sensor
		metrics.resolution_noise = 0.7;

    return metrics;
}

char* check_drift(SensorData sensor_data, SensorMetrics metrics) {
    float voltage_diff;
    int drift_count = 0;  // Counter for voltage changes
    
    // Loop through the battery voltage readings
    for (int i = 1; i < metrics.battery_size; i++) {
        voltage_diff = sensor_data.battery_voltage[i] - sensor_data.battery_voltage[i - 1];
        
        // Check if the change is significant
        if (voltage_diff >= 0.1 || voltage_diff <= -0.1) {
            drift_count++;
        }
        
        // If there are two or more significant changes, return drift detected
        if (drift_count >= 2) {
            return "Drift detected due to multiple voltage variations.\nPlease check the battery!";
        }
    }
    
    // If there are fewer than two significant changes, return no drift detected
    return "No Drift detected";
}

char* check_trend(SensorData sensor_data, SensorMetrics metrics) {
    // Calculate the average of acceleration using a window size of 10 points
    int avg_index = metrics.acceleration_size / WINDOW_SIZE;
    float avg_accel[avg_index];
    
    // Calculate the average acceleration for each window
    for (int i = 0; i < avg_index; i++) {
        float sum = 0.0;
        for (int j = 0; j < WINDOW_SIZE; j++) {
            sum += sensor_data.acceleration[i * WINDOW_SIZE + j];
        }
        avg_accel[i] = sum / WINDOW_SIZE;
    }

    // Calculate percentage of temperature variation
    float temp_diff_percentage = ((metrics.max_temp - metrics.min_temp) / metrics.min_temp) * 100;

    // Initialize variables for tracking trends
    int is_increasing = 0;  // Track if we have 10 consecutive increasing windows
    int is_decreasing = 0;  // Track if we have 10 consecutive decreasing windows

    int increasing_count = 0;  // Counter for consecutive increasing windows
    int decreasing_count = 0;  // Counter for consecutive decreasing windows

    // Check for trends across the average acceleration windows
    for (int i = 1; i < avg_index; i++) {
        if (avg_accel[i] > avg_accel[i - 1]) {
            increasing_count++;  // Increment if current window is increasing
            decreasing_count = 0; // Reset decreasing counter
        } else if (avg_accel[i] < avg_accel[i - 1]) {
            decreasing_count++;  // Increment if current window is decreasing
            increasing_count = 0; // Reset increasing counter
        } else {
            // If it's neither increasing nor decreasing, reset both counters
            increasing_count = 0;
            decreasing_count = 0;
        }

        // Check if we have reached 10 consecutive increasing windows
        if (increasing_count >= 10) {
            is_increasing = 1;
            break; // Exit early since we've detected the trend
        }

        // Check if we have reached 10 consecutive decreasing windows
        if (decreasing_count >= 10) {
            is_decreasing = 1;
            break; // Exit early since we've detected the trend
        }
    }

    // Handle trend detection and recovery based on conditions
    if (is_increasing || is_decreasing) {
        // If temperature variation is above 30%
        if (temp_diff_percentage > 30) {
            trend_recovery(sensor_data.acceleration, 5, 10);  // Apply trend recovery to acceleration data
            return "Trend detected due to temperature variation, recovery applied";
        }

        // If sensor is active
        if (sensor_data.is_active) {
            trend_recovery(sensor_data.acceleration, 5, 10);  // Apply trend recovery to acceleration data
            return "Trend detected due to sensor activation, recovery applied";
        }
    }

    // If no trend detected
    return "No trend detected";
}


//char* check_trend(SensorData sensor_data, SensorMetrics metrics) {
//    // Calculate percentage of change
//    float temp_diff_percentage = ((metrics.max_temp - metrics.min_temp) / metrics.min_temp) * 100;

//    // If the percentage is over 30%
//    if (temp_diff_percentage > 30)
//        return "Trend due to temperature variation";

//    // Check activate state of sensor
//    if (metrics.max_accel - metrics.min_accel >= 5)
//        return "Trend due to sensor activation";

//    return "No Trend detected";
//}

// Function to get the actual threshold in terms of g's
float get_threshold(AccelRange range) {
    switch (range) {
        case ACCEL_RANGE_2G: return 2.0;
        case ACCEL_RANGE_4G: return 4.0;
        case ACCEL_RANGE_8G: return 8.0;
        case ACCEL_RANGE_16G: return 16.0;
        default: return 2.0; // Default to 2g in case of an error
    }
}

char* check_minor(SensorData sensor_data, SensorMetrics metrics) { 
    int minor_count = 0;

    // Calculate the percentage of samples within the resolution range
    for (int i = 1; i < metrics.acceleration_size; i++) {
        if (fabs(sensor_data.acceleration[i] - sensor_data.acceleration[i - 1]) <= metrics.resolution_noise) {
            minor_count++;
        }
    }

    // Check if 80% of samples are within the resolution range
    if ((minor_count / (float)(metrics.acceleration_size - 1)) >= 0.8) {
        // Decrease range to improve resolution if possible
        if (current_accel_range == ACCEL_RANGE_4G) {
            adjust_accel_range(&hi2c2, ACCEL_RANGE_2G);
            printf("Reduced range to 2G for improved resolution. New range: 0x%02X, g value = %.1f\n", 
                   current_accel_range, get_threshold(current_accel_range));
        } else if (current_accel_range == ACCEL_RANGE_8G) {
            adjust_accel_range(&hi2c2, ACCEL_RANGE_4G);
            printf("Reduced range to 4G for improved resolution. New range: 0x%02X, g value = %.1f\n", 
                   current_accel_range, get_threshold(current_accel_range));
        } else if (current_accel_range == ACCEL_RANGE_16G) {
            adjust_accel_range(&hi2c2, ACCEL_RANGE_8G);
            printf("Reduced range to 8G for improved resolution. New range: 0x%02X, g value = %.1f\n", 
                   current_accel_range, get_threshold(current_accel_range));
        } else {
            printf("Already at minimum range: 0x%02X\n", current_accel_range);
            return "Minor issue detected due to insufficient sensor resolution, but range is already at minimum";
        }
        return "Minor issue detected due to insufficient sensor resolution";
    }
    return "No minor issues detected";
}



char* check_square(SensorData sensor_data, SensorMetrics metrics) {
    // Print the current range in both hex and g units
    printf("Before: Hex Range Code = 0x%02X, g value = %.1f\n", current_accel_range, get_threshold(current_accel_range));
    
    float high_threshold = get_threshold(current_accel_range);
    printf("High threshold: %f\n", high_threshold);
    float low_threshold = -high_threshold;
    float tolerance = 0.5;  // Allowable offset for near saturation
    int near_max_count = 0;
    int near_min_count = 0;

    for (int i = 0; i < metrics.acceleration_size; i++) {
        if (sensor_data.acceleration[i] >= high_threshold - tolerance &&
            sensor_data.acceleration[i] <= high_threshold + tolerance) {
            near_max_count++;
            near_min_count = 0;
        } else if (sensor_data.acceleration[i] >= low_threshold - tolerance &&
                   sensor_data.acceleration[i] <= low_threshold + tolerance) {
            near_min_count++;
            near_max_count = 0;
        } else {
            near_max_count = 0;
            near_min_count = 0;
        }

        if (near_max_count >= 3 || near_min_count >= 3) {
            printf("Square issue detected due to sensor saturation\n");

            // Adjust range if possible
            if (current_accel_range == ACCEL_RANGE_2G) {
                adjust_accel_range(&hi2c2, ACCEL_RANGE_4G);
            } else if (current_accel_range == ACCEL_RANGE_4G) {
                adjust_accel_range(&hi2c2, ACCEL_RANGE_8G);
            } else if (current_accel_range == ACCEL_RANGE_8G) {
                adjust_accel_range(&hi2c2, ACCEL_RANGE_16G);
            } else {
                printf("Maximum range reached or invalid range: Hex Range Code = 0x%02X\n", current_accel_range);
                return "Square issue detected due to sensor saturation, but range is already at maximum";
            }

            printf("After adjustment, Hex Range Code = 0x%02X, g value = %.1f\n", current_accel_range, get_threshold(current_accel_range));
            return "Increase acceleration range by one step";
        }
    }

    return "No Square issues detected";
}

char* check_bias(SensorData sensor_data, SensorMetrics metrics) {
    float derivative_threshold = 0.5;  // Define a threshold for the derivative
    int max_derivative_index = 0;
    float max_derivative = 0.0;

    // Apply a moving window average to smooth the acceleration data
    float smoothed_accel[metrics.acceleration_size / WINDOW_SIZE];
    int avg_index = 0;

    for (int i = 0; i <= metrics.acceleration_size - WINDOW_SIZE; i += WINDOW_SIZE) {
        float sum = 0.0;
        for (int j = 0; j < WINDOW_SIZE; j++) {
            sum += sensor_data.acceleration[i + j];
        }
        smoothed_accel[avg_index++] = sum / WINDOW_SIZE;
    }

    // Find the maximum numerical derivative of the smoothed data
    for (int i = 1; i < avg_index; i++) {
        float derivative = fabs(smoothed_accel[i] - smoothed_accel[i - 1]);
        if (derivative > max_derivative) {
            max_derivative = derivative;
            max_derivative_index = i;
        }
    }

    // Check if the maximum derivative exceeds the threshold
    if (max_derivative > derivative_threshold) {
        // Check the battery voltage at the point of the maximum derivative
        if (sensor_data.battery_voltage[max_derivative_index * WINDOW_SIZE] == 3.4) {
            return "Bias detected due to low battery voltage.\nPlease charge the battery!";
        }
    }

    // for (int i = 0; i < metrics.gyroscope_size; i++) {
    //     if (fabs(sensor_data.gyroscope[i]) > GYRO_THRESHOLD) {
    //         // Bias detected due to sudden rotation, now apply data recovery
    //         int bias_index = max_derivative_index * WINDOW_SIZE;

    //         // Split the data into two segments: before and after the bias
    //         float* before_bias = &sensor_data.acceleration[0];
    //         int before_size = bias_index;
    //         float* after_bias = &sensor_data.acceleration[bias_index];
    //         int after_size = metrics.acceleration_size - bias_index;

    //         // Apply trend recovery separately on both segments
    //         trend_recovery(before_bias, before_size, WINDOW_SIZE);
    //         trend_recovery(after_bias, after_size, WINDOW_SIZE);

    //         return "Bias detected due to sudden rotation and recovered!";
    //     }
    // }
        // Check for bias due to sudden rotation using gyroscope data
    for (int i = 0; i < metrics.gyroscope_size; i++) {
        if (fabs(sensor_data.gyroscope[i]) > GYRO_THRESHOLD) {
            int bias_index = max_derivative_index * WINDOW_SIZE;
            correct_bias(sensor_data.acceleration, metrics.acceleration_size, smoothed_accel, bias_index, max_derivative_index, WINDOW_SIZE);
            return "Bias detected due to sudden rotation and recovery applied";
        }
    }

    return "No bias detected";
}

//char* check_and_recover_bias(SensorData sensor_data, SensorMetrics metrics) {
//    float derivative_threshold = 0.5;  // Define a threshold for the derivative
//    int max_derivative_index = 0;
//    float max_derivative = 0.0;

//    // Apply a moving window average to smooth the acceleration data
//    float smoothed_accel[metrics.acceleration_size / WINDOW_SIZE];
//    int avg_index = 0;

//    for (int i = 0; i <= metrics.acceleration_size - WINDOW_SIZE; i += WINDOW_SIZE) {
//        float sum = 0.0;
//        for (int j = 0; j < WINDOW_SIZE; j++) {
//            sum += sensor_data.acceleration[i + j];
//        }
//        smoothed_accel[avg_index++] = sum / WINDOW_SIZE;
//    }

//    // Find the maximum numerical derivative of the smoothed data
//    for (int i = 1; i < avg_index; i++) {
//        float derivative = fabs(smoothed_accel[i] - smoothed_accel[i - 1]);
//        if (derivative > max_derivative) {
//            max_derivative = derivative;
//            max_derivative_index = i;
//        }
//    }

//    // Check if the maximum derivative exceeds the threshold
//    if (max_derivative > derivative_threshold) {
//        // Check the battery voltage at the point of the maximum derivative
//        if (sensor_data.battery_voltage[max_derivative_index * WINDOW_SIZE] == 3.4) {
//            return "Bias detected due to low battery voltage.\nPlease charge the battery!";
//        }
//    }

//    for (int i = 0; i < metrics.gyroscope_size; i++) {
//        if (fabs(sensor_data.gyroscope[i]) > GYRO_THRESHOLD) {
//            // Bias detected due to sudden rotation, now apply data recovery
//            int bias_index = max_derivative_index * WINDOW_SIZE;

//            // Split the data into two segments: before and after the bias
//            float* before_bias = &sensor_data.acceleration[0];
//            int before_size = bias_index;
//            float* after_bias = &sensor_data.acceleration[bias_index];
//            int after_size = metrics.acceleration_size - bias_index;

//            // Apply trend recovery separately on both segments
//            trend_recovery(before_bias, before_size, WINDOW_SIZE);
//            trend_recovery(after_bias, after_size, WINDOW_SIZE);

//            return "Bias detected due to sudden rotation and recovered!";
//        }
//    }
//    //     // Check for bias due to sudden rotation using gyroscope data
//    // for (int i = 0; i < metrics.gyroscope_size; i++) {
//    //     if (fabs(sensor_data.gyroscope[i]) > GYRO_THRESHOLD) {
//    //         return "Bias detected due to sudden rotation";
//    //     }
//    // }

//    return "No bias detected";
//}
// Function to check for outliers
char* check_outlier(SensorData sensor_data, SensorMetrics metrics) {
    // Check for outliers due to extreme low temperature
    if (sensor_data.temperature[metrics.temperature_size - 1] <= -20.0) {
        trend_recovery(sensor_data.acceleration, metrics.acceleration_size, WINDOW_SIZE);  // Apply trend recovery
        outlier_recovery(sensor_data.acceleration, metrics.acceleration_size);  // Apply outlier recovery
        return "Outlier due to extreme low temperature, recovery applied.";
    }

    // Check for outliers due to sudden drop or spike in battery voltage
    float current_voltage = sensor_data.battery_voltage[metrics.battery_size - 1];
    float previous_voltage = sensor_data.battery_voltage[metrics.battery_size - 2]; // assuming size > 1

    // Check if there's a significant drop or spike in voltage (threshold set to 0.5V as an example)
    if (fabs(current_voltage - previous_voltage) > 0.3) {
        return "Outlier due to loose electrical contact (sudden voltage change).\nPlease fix loose contact!";
    }

    return "No Outlier detected";
}


char* check_missing(SensorData sensor_data, SensorMetrics metrics) {
    // Calculate the expected number of data points
//    int expected_data_points = sensor_data.sampling_frequency * sensor_data.duration;
	  int expected_data_points = sensing_rate * sensing_duration;

    // Check if no data was collected (hard failure)
    if (metrics.acceleration_size == 0) {
        return "Hard failure detected due to no data collection";
    }

    // Check if all collected data points are the same (hard failure)
    bool all_same_value = true;
    for (int i = 1; i < metrics.acceleration_size; i++) {
        if (sensor_data.acceleration[i] != sensor_data.acceleration[0]) {
            all_same_value = false;
            break;
        }
    }
    if (all_same_value) {
        return "Hard failure detected due to all data points having the same value";
    }

    // Check if the actual data size is less than expected (missing data)
    if (metrics.acceleration_size > 0 && metrics.acceleration_size < expected_data_points) {
        return "Missing data detected due to insufficient sample points";
    }

    // Check if the actual data size equals the expected data points (no missing data)
    if (metrics.acceleration_size == expected_data_points) {
        return "No missing data detected";
    }
		return "Error: Unexpected condition";
}

//char* check_missing(SensorData sensor_data, SensorMetrics metrics) {
//    // int missing_count = 0;
//    // int threshold_missing_data = 5;  // Example threshold for missing data points

//    // Calculate the expected number of data points
//    int expected_data_points = sensor_data.sampling_frequency * sensor_data.duration;

//    // Check if the actual data size is less than expected
//    if (metrics.acceleration_size < expected_data_points) {
//        return "Missing data detected due to insufficient sample points";
//    }
//    return "No missing data detected";
//	}

// Select the data type
void diagnose(SensorData sensor_data, SensorMetrics metrics, int choice) {
    switch (choice) {
        case 0:
            printf("Normal: No diagnostics needed.\n");
            break;
        case 1:
            printf("Missing: %s\n", check_missing(sensor_data, metrics));
            break;
        case 2:
            printf("Minor: %s\n", check_minor(sensor_data, metrics));
            break;
        case 3:
            printf("Outlier: %s\n", check_outlier(sensor_data, metrics));  
            break;
        case 4:
            printf("Square: %s\n", check_square(sensor_data, metrics));
            break;
        case 5:
            printf("Trend: %s\n", check_trend(sensor_data, metrics));
            break;
        case 6:
            printf("Drift: %s\n", check_drift(sensor_data, metrics));
            break;
        default:
            printf("Invalid choice.\n");
            break;
    }
}

//// Function to calculate the maximum value in an array
//char* check_drift(SensorData sensor_data) {
//    float max_voltage = sensor_data.battery_voltage[0];
//    float min_voltage = sensor_data.battery_voltage[0];
//    for (int i = 1; i < 5; i++) {
//        if (sensor_data.battery_voltage[i] > max_voltage)
//            max_voltage = sensor_data.battery_voltage[i];
//        if (sensor_data.battery_voltage[i] < min_voltage)
//            min_voltage = sensor_data.battery_voltage[i];
//    }
//    if (max_voltage - min_voltage > 0.5) 
//        return "Drift due to battery voltage variation";
//    return "No Drift detected";
//}

//char* check_trend(SensorData sensor_data) {
//    float max_temp = sensor_data.temperature[0];
//    float min_temp = sensor_data.temperature[0];
//    for (int i = 1; i < 5; i++) {
//        if (sensor_data.temperature[i] > max_temp)
//            max_temp = sensor_data.temperature[i];
//        if (sensor_data.temperature[i] < min_temp)
//            min_temp = sensor_data.temperature[i];
//    }
//    if (max_temp - min_temp > 10)
//        return "Trend due to temperature variation";

//    if (sensor_data.acceleration[0] - sensor_data.acceleration[4] >= 5)
//        return "Trend due to sensor activation";

//    return "No Trend detected";
//}

//char* check_minor(SensorData sensor_data) {
//    float noise_tolerance = 0.1;
//    for (int i = 0; i < 4; i++) {
//        if (fabs(sensor_data.acceleration[i+1] - sensor_data.acceleration[i]) > noise_tolerance) 
//            return "No Minor issues detected";
//    }
//    return "Minor issue detected due to low sensor resolution";
//}

//char* check_square(SensorData sensor_data) {
//    float noise_tolerance = 0.1;
//    float avg_acceleration = 0.0;
//    for (int i = 0; i < 5; i++) {
//        avg_acceleration += sensor_data.acceleration[i];
//    }
//    avg_acceleration /= 5;

//    for (int i = 1; i < 5; i++) {
//        if (fabs(sensor_data.acceleration[i] - sensor_data.acceleration[i-1]) > noise_tolerance || 
//            fabs(sensor_data.acceleration[i] - avg_acceleration) > noise_tolerance) {
//            return "No Square issues detected";
//        }
//    }

//    if (avg_acceleration >= 6.0)
//        return "Square issue detected due to sensor saturation";
//    
//    return "No Square issues detected";
//}

//char* check_bias(SensorData sensor_data) {
//    float noise_tolerance = 0.1;
//    float avg_acceleration = 0.0;
//    for (int i = 0; i < 5; i++) {
//        avg_acceleration += sensor_data.acceleration[i];
//    }
//    avg_acceleration /= 5;

//    for (int i = 1; i < 5; i++) {
//        if (fabs(sensor_data.acceleration[i] - avg_acceleration) > noise_tolerance * 5) 
//            return "Bias detected due to sensor offset";
//    }
//    return "No Bias detected";
//}

//char* check_sudden_rotation(SensorData sensor_data) {
//    for (int i = 1; i < 4; i++) {
//        if (fabs(sensor_data.acceleration[i+1] - sensor_data.acceleration[i]) > 2.0)
//            return "Sudden rotation detected";
//    }
//    return "No Sudden rotation detected";
//}

//char* check_outlier(SensorData sensor_data) {
//    if (sensor_data.battery_voltage[4] < 3.2)
//        return "Outlier due to sudden low battery voltage";

//    return "No Outlier detected";
//}

//char* check_missing(SensorData sensor_data) {
//    if (sensor_data.previous_data[0] != 'c')
//        return "Missing data due to sensor saturation";

//    if (sensor_data.battery_voltage[4] < 3.3)
//        return "Missing data due to low battery voltage";

//    if (sensor_data.temperature[4] > 50 || sensor_data.temperature[4] < -10)
//        return "Missing data due to extreme temperature";

//    return "Missing data due to hardware/software issue";
//}

//void diagnose(SensorData sensor_data, int choice) {
//    switch (choice) {
//        case 0:
//            printf("Normal: No diagnostics needed.\n");
//            break;
//        case 1:
//            printf("Missing: %s\n", check_missing(sensor_data));
//            break;
//        case 2:
//            printf("Minor: %s\n", check_minor(sensor_data));
//            break;
//        case 3:
//            printf("Outlier: %s\n", check_outlier(sensor_data));  
//            break;
//        case 4:
//            printf("Square: %s\n", check_square(sensor_data));
//            break;
//        case 5:
//            printf("Trend: %s\n", check_trend(sensor_data));
//            break;
//        case 6:
//            printf("Drift: %s\n", check_drift(sensor_data));
//            break;
//        default:
//            printf("Invalid choice.\n");
//            break;
//    }
//}

