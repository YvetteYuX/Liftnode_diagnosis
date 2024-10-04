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
#include <stdio.h>

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
 
 

void test(void)
{
   printf("Hello, worldaaaaa!!!!!\n\r");
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define NOISE_TOLERANCE 0.1f // noise tolerance

// Function to calculate metrics
SensorMetrics calculate_metrics(SensorData sensor_data) {
    SensorMetrics metrics = {0};

    // Calculate battery voltage metrics
    
    metrics.max_voltage = sensor_data.battery_voltage[0];
    metrics.min_voltage = sensor_data.battery_voltage[0];
    metrics.battery_size = sizeof(sensor_data.battery_voltage)/sizeof(sensor_data.battery_voltage[0]);
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

    // Calculate resolution and range
    metrics.range = 12.0;  // ±6g gives a total range of 12g
    metrics.resolution = metrics.range / (1 << 12);  // Example for a 12-bit sensor

    return metrics;
}

char* check_drift(SensorData sensor_data, SensorMetrics metrics) {
    float voltage_diff;
    for (int i = 1; i < metrics.battery_size; i++) {
        voltage_diff = sensor_data.battery_voltage[i] - sensor_data.battery_voltage[i - 1];
        if (voltage_diff > 0.1 || voltage_diff < -0.1) {
            return "Drift detected due to voltage variation.\nPlease charge the battery!";
        }
    }
    return "No Drift detected";
}

char* check_trend(SensorData sensor_data, SensorMetrics metrics) {
    // Calculate percentage of change
    float temp_diff_percentage = ((metrics.max_temp - metrics.min_temp) / metrics.min_temp) * 100;

    // If the percentage is over 30%
    if (temp_diff_percentage > 30)
        return "Trend due to temperature variation";

    // Check activate state of sensor
    if (metrics.max_accel - metrics.min_accel >= 5)
        return "Trend due to sensor activation";

    return "No Trend detected";
}

char* check_minor(SensorData sensor_data, SensorMetrics metrics) {
    int minor_count = 0;

    // Calculate the percentage of samples within the resolution range
    for (int i = 1; i < metrics.acceleration_size; i++) {
        if (fabs(sensor_data.acceleration[i] - sensor_data.acceleration[i - 1]) <= metrics.resolution) {
            minor_count++;
        }
    }

    // Check if 80% of samples are within the resolution range
    if ((minor_count / (float)(metrics.acceleration_size - 1)) >= 0.8) {
        return "Minor issue detected due to insufficient sensor resolution";
    }
    return "No minor issues detected";
}

char* check_square(SensorData sensor_data, SensorMetrics metrics) {
    float high_threshold = 6.0;   // Maximum sensor range
    float low_threshold = -6.0;   // Minimum sensor range
    float tolerance = 0.5;        // Allowable offset for near saturation
    int near_max_count = 0;       // Counter for consecutive near-max values
    int near_min_count = 0;       // Counter for consecutive near-min values

    // Loop through the acceleration data
    for (int i = 0; i < metrics.acceleration_size; i++) {
        // Check if the value is near the maximum threshold
        if (sensor_data.acceleration[i] >= high_threshold - tolerance && sensor_data.acceleration[i] <= high_threshold + tolerance) {
            near_max_count++;  // Increment near max counter
            near_min_count = 0; // Reset min counter if max is detected
        }
        // Check if the value is near the minimum threshold
        else if (sensor_data.acceleration[i] >= low_threshold - tolerance && sensor_data.acceleration[i] <= low_threshold + tolerance) {
            near_min_count++;  // Increment near min counter
            near_max_count = 0; // Reset max counter if min is detected
        }
        // Reset counters if neither max nor min threshold is hit
        else {
            near_max_count = 0;
            near_min_count = 0;
        }

        // Check if near saturation values appear 3 times consecutively
        if (near_max_count >= 3) {
            return "Square issue detected due to sensor saturation at maximum";
        }
        if (near_min_count >= 3) {
            return "Square issue detected due to sensor saturation at minimum";
        }
    }

    return "No Square issues detected";
}

char* check_bias(SensorData sensor_data, SensorMetrics metrics) {
    float noise_tolerance = 0.1;
    for (int i = 0; i < metrics.acceleration_size; i++) {
        if (fabs(sensor_data.acceleration[i] - metrics.avg_accel) > noise_tolerance * 5)
            return "Bias detected due to sensor offset";
    }
    return "No Bias detected";
}

char* check_sudden_rotation(SensorData sensor_data, SensorMetrics metrics) {
    for (int i = 1; i < metrics.acceleration_size - 1; i++) {
        if (fabs(sensor_data.acceleration[i + 1] - sensor_data.acceleration[i]) > 2.0)
            return "Sudden rotation detected";
    }
    return "No Sudden rotation detected";
}

char* check_outlier(SensorData sensor_data, SensorMetrics metrics) {
    // Check for outliers due to extreme low temperature
    if (sensor_data.temperature[metrics.temperature_size - 1] <= -20.0) {
        return "Outlier due to extreme low temperature";
    }

    // Check for outliers due to sudden drop or spike in battery voltage
    float current_voltage = sensor_data.battery_voltage[metrics.battery_size - 1];
    float previous_voltage = sensor_data.battery_voltage[metrics.battery_size - 2]; // assuming size > 1

    // Check if there's a significant drop or spike in voltage (threshold set to 0.5V as an example)
    if (fabs(current_voltage - previous_voltage) > 0.3) {
        return "Outlier due to loose electrical contact (sudden voltage change).\nPlease fix loose contact!";
    }

    // // Check for consistently low battery voltage
    // if (current_voltage < 3.2) {
    //     return "Outlier due to consistently low battery voltage";
    // }

    return "No Outlier detected";
}



char* check_missing(SensorData sensor_data, SensorMetrics metrics) {
    // int missing_count = 0;
    // int threshold_missing_data = 5;  // Example threshold for missing data points

    // Calculate the expected number of data points
    int expected_data_points = sensor_data.sampling_frequency * sensor_data.duration;

    // Check if the actual data size is less than expected
    if (metrics.acceleration_size < expected_data_points) {
        return "Missing data detected due to insufficient sample points";
    }
    return "No missing data detected";
	}

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

