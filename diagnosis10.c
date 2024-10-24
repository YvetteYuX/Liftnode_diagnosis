#define WINDOW_SIZE 10
#define GYRO_THRESHOLD 200 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h> 

// define sensor data struct
typedef struct {
    float battery_voltage[5];
    float temperature[5];
    float acceleration[5];
    float gyroscope[5];
    char previous_data[10];
    bool is_active;
} SensorData;

// Define calculated metrics struct
typedef struct {
    float max_voltage;
    float min_voltage;
    float avg_voltage;
    float max_temp;
    float min_temp;
    float avg_temp;
    float max_accel;
    float min_accel;
    float avg_accel;
    float resolution;
    float range; // Range total
    float slope;
    int battery_size;
    int temperature_size;
    int acceleration_size;
    int gyroscope_size;
} SensorMetrics;

// // Function prototypes
// void moving_average_filter(float* data, int size, int window_size, float* filtered_data);
// int find_bias_point(float* data, int size, int window_size);
// void trend_recovery(float* data, int size, int window_size);
// void process_segment(float* data, int size);


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
    metrics.range = 12.0;  // ±6g gives a total range of 12g
    metrics.resolution = metrics.range / (1 << 12);  // Example for a 12-bit sensor

    // calculate slope of acceleration

    float total_difference = 0.0;
    int valid_pairs = 0;

    // Calculate differences between successive pairs of acceleration values
    for (int i = 0; i < metrics.acceleration_size - 1; i += 2) {
        // Calculate difference for the pair
        float difference = sensor_data.acceleration[i + 1] - sensor_data.acceleration[i];
        total_difference += difference;
        valid_pairs++;
    }

    // Calculate the average slope
    if (valid_pairs > 0) {
        metrics.slope = total_difference / valid_pairs;
    } else {
        metrics.slope = 0.0; // Default to 0 if no pairs
    }

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
            return "Drift detected due to multiple voltage variations.\nPlease charge the battery!";
        }
    }
    
    // If there are fewer than two significant changes, return no drift detected
    return "No Drift detected";
}


// char* check_drift(SensorData sensor_data, SensorMetrics metrics) {
//     float voltage_diff;
//     for (int i = 1; i < metrics.battery_size; i++) {
//         voltage_diff = sensor_data.battery_voltage[i] - sensor_data.battery_voltage[i - 1];
//         if (voltage_diff >= 0.1 || voltage_diff <= -0.1) {
//             return "Drift detected due to voltage variation.\nPlease charge the battery!";
//         }
//     }
//     return "No Drift detected";
// }


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
    metrics.acceleration_size = 5;
    float high_threshold = 6.0;   // Maximum sensor range
    float low_threshold = -6.0;   // Minimum sensor range
    float tolerance = 0.5;        // Allowable offset for near saturation
    int near_max_count = 0;       // Counter for consecutive near-max values
    int near_min_count = 0;       // Counter for consecutive near-min values

    // Loop through the acceleration data
    for (int i = 0; i < metrics.acceleration_size; i++) {
        // Check if the value is near the maximum threshold
        if (sensor_data.acceleration[i] >= high_threshold - tolerance &&
            sensor_data.acceleration[i] <= high_threshold + tolerance) {
            near_max_count++;  // Increment near max counter
            near_min_count = 0; // Reset min counter if max is detected
            if (near_max_count >= 3) {
                return "Square issue detected due to sensor saturation at maximum";
            }
        }
        // Check if the value is near the minimum threshold
        else if (sensor_data.acceleration[i] >= low_threshold - tolerance &&
                 sensor_data.acceleration[i] <= low_threshold + tolerance) {
            near_min_count++;  // Increment near min counter
            near_max_count = 0; // Reset max counter if min is detected
            if (near_min_count >= 3) {
                return "Square issue detected due to sensor saturation at minimum";
            }
        }
        // Reset counters if neither max nor min threshold is hit
        else {
            near_max_count = 0;
            near_min_count = 0;
        }
    }

    return "No Square issues detected";   
}
   

// Function for outlier recovery in the acceleration data
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
    int expected_data_points = 1 * 5;

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
}

// Function to correct bias in acceleration data
void correct_bias(float* acceleration, int acceleration_size, float* smoothed_accel, int bias_index, int max_derivative_index, int window_size) {
      // This calculates the bias index

    float mean_before_bias = 0.0;
    int count_before = 0;

    // Step 1: Calculate mean of data before the bias
    for (int i = 0; i < bias_index; i++) {
        mean_before_bias += acceleration[i];
        count_before++;
    }
    if (count_before > 0) {
        mean_before_bias /= count_before;
    }

    // Step 2: Bias correction by shifting post-bias data to align with pre-bias mean
    for (int i = bias_index; i < acceleration_size; i++) {
        acceleration[i] -= (smoothed_accel[max_derivative_index] - mean_before_bias);
    }
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


// char* check_bias(SensorData sensor_data, SensorMetrics metrics) {
//     float noise_tolerance = 0.1;
//     for (int i = 0; i < metrics.acceleration_size; i++) {
//         if (fabs(sensor_data.acceleration[i] - metrics.avg_accel) > noise_tolerance * 5)
//             return "Bias detected due to sensor offset";
//     }
//     return "No Bias detected";
// }

// char* check_sudden_rotation(SensorData sensor_data, SensorMetrics metrics) {
//     for (int i = 1; i < metrics.acceleration_size - 1; i++) {
//         if (fabs(sensor_data.acceleration[i + 1] - sensor_data.acceleration[i]) > 2.0)
//             return "Sudden rotation detected";
//     }
//     return "No Sudden rotation detected";
// }

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



int main() {

    SensorData sensor_data = {
        {3.5, 3.4, 3.3, 3.2, 3.1},
        {20, 21, 22, 23, 20}, 
        // {20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0,
        //  30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0},
        // {5.8, 6.0, 5.9, 6.1, 6.0, 6.2, 6.0, 5.9, 5.8, 6.1,
        //  6.2, 6.1, 5.9, 5.7, 5.8, 5.9, 6.0, 6.1, 5.9, 5.8,
        //  6.0, 6.1, 5.9, 6.0},
        // {3.5, 3.4, 3.5, 3.4, 3.5},  // battery_voltage
        // {21, 21, 22, 23, 23},       // temperature
        {-6.0, -6.0, 6.0, 6.0, 6.0},  // acceleration
        {200, 200, 200, 200, 200},  // gyroscope
        "variable",                  // previous_data
        true
    };
    
    SensorMetrics metrics = calculate_metrics(sensor_data);
    printf("%d\n", metrics.acceleration_size);

    int choice;
    printf("Please select the fault type:\n");
    printf("0: Normal\n");
    printf("1: Missing\n");
    printf("2: Minor\n");
    printf("3: Outlier\n");
    printf("4: Square\n");
    printf("5: Trend\n");
    printf("6: Drift\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);


    diagnose(sensor_data, metrics, choice);

    return 0;
}
