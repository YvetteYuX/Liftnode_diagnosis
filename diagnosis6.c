#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Define sensor data struct
typedef struct {
    float *battery_voltage;
    float *temperature;
    float *acceleration;
    char previous_data[10];
    // int size;  // Size of data arrays
    int sampling_frequency;     // Sampling frequency of the sensor data
    int duration;               // Duration of the data collection in seconds
    int acceleration_size;
    int battery_size;
    int temperature_size;
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
    int battery_size;
    int temperature_size;
    int acceleration_size;
} SensorMetrics;

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

// char* check_outlier(SensorData sensor_data, SensorMetrics metrics) {
//     if (sensor_data.battery_voltage[sensor_data.size - 1] < 3.2)
//         return "Outlier due to sudden low battery voltage";

//     return "No Outlier detected";
// }



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

    // // Check for missing data points (represented as zeros or unusual constant values)
    // for (int i = 0; i < sensor_data.size; i++) {
    //     if (sensor_data.acceleration[i] == 0 || sensor_data.acceleration[i] == -999) {
    //         // Assume zero or -999 represents a missing or uncollected point
    //         missing_count++;
    //     }
    // }

    // // Diagnose based on the number of missing points
    // if (missing_count >= threshold_missing_data) {
    //     return "Missing data detected due to interference or signal loss";
    // }

    // return "No missing data detected";
}


// char* check_missing(SensorData sensor_data, SensorMetrics metrics) {
//     if (sensor_data.previous_data[0] != 'c')
//         return "Missing data due to sensor saturation";

//     if (sensor_data.battery_voltage[sensor_data.size - 1] < 3.3)
//         return "Missing data due to low battery voltage";

//     if (sensor_data.temperature[sensor_data.size - 1] > 50 || sensor_data.temperature[sensor_data.size - 1] < -10)
//         return "Missing data due to extreme temperature";

//     return "Missing data due to hardware/software issue";
// }

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



int main() {
    // Sample sensor data
    SensorData sensor_data;
    // sensor_data.size = 5; // Example size
    sensor_data.sampling_frequency = 1;  // Example frequency: 1 Hz (1 sample per second)
    sensor_data.duration = 5;  // Example duration: 10 seconds
    

    // Input sensor data
    sensor_data.battery_voltage = (float[]) {3.75, 3.50, 3.50, 3.85, 3.50};
    sensor_data.temperature = (float[]) {22.5, 23.0, 21.5, 24.0, 22.0};
    sensor_data.acceleration = (float[]) {0.0, 1.0, -1.0, 0.5};


// int main() {
  
//     // Sample sensor data
//     SensorData sensor_data;
//     sensor_data.size = 100; // Example size
//     sensor_data.battery_voltage = (float *)malloc(sensor_data.size * sizeof(float));
//     sensor_data.temperature = (float *)malloc(sensor_data.size * sizeof(float));
//     sensor_data.acceleration = (float *)malloc(sensor_data.size * sizeof(float));

//     // Initialize sensor data
//     for (int i = 0; i < sensor_data.size; i++) {
//         sensor_data.battery_voltage[i] = (float)(3.5 + (rand() % 100) / 100.0);
//         sensor_data.temperature[i] = (float)(20 + (rand() % 100) / 10.0);
//         sensor_data.acceleration[i] = (float)(-6 + (rand() % 120) / 10.0);
//     }

//     // 输出传感器数据
//     printf("Sensor Data:\n");
//     printf("Index\tBattery Voltage (V)\tTemperature (°C)\tAcceleration (g)\n");
//     for (int i = 0; i < sensor_data.size; i++) {
//         printf("%d\t%.2f\t\t\t%.1f\t\t\t%.1f\n", i, sensor_data.battery_voltage[i], sensor_data.temperature[i], sensor_data.acceleration[i]);
//     }

    // Calculate metrics once
    SensorMetrics metrics = calculate_metrics(sensor_data);
    // printf("battery_size: %.2f \n", metrics.acceleration_size);



    // Select the diagnostic type
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

    // Run the diagnostic function
    diagnose(sensor_data, metrics, choice);

    // // Free allocated memory
    // free(sensor_data.battery_voltage);
    // free(sensor_data.temperature);
    // free(sensor_data.acceleration);

    return 0;
}

