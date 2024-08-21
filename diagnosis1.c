#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> 

#define NOISE_TOLERANCE 0.1f  // noise tolerance

typedef struct {
    float* battery_voltage;
    int battery_voltage_len;
    float* temperature;
    int temperature_len;
    float* acceleration;
    int acceleration_len;
    char* previous_data;
} SensorData;

// Function to calculate the maximum value in an array
float max_value(float* array, int len) {
    float max = array[0];
    for (int i = 1; i < len; i++) {
        if (array[i] > max) {
            max = array[i];
        }
    }
    return max;
}

// Function to calculate the minimum value in an array
float min_value(float* array, int len) {
    float min = array[0];
    for (int i = 1; i < len; i++) {
        if (array[i] < min) {
            min = array[i];
        }
    }
    return min;
}

// Function to calculate the mean value of an array
float mean_value(float* array, int len) {
    float sum = 0.0f;
    for (int i = 0; i < len; i++) {
        sum += array[i];
    }
    return sum / len;
}

void check_drift(SensorData* data, char* result) {
    if (max_value(data->battery_voltage, data->battery_voltage_len) - min_value(data->battery_voltage, data->battery_voltage_len) > 0.5f) {
        sprintf(result, "Drift due to battery voltage variation");
    } else {
        sprintf(result, "No Drift detected");
    }
}

void check_trend(SensorData* data, char* result) {
    if (max_value(data->temperature, data->temperature_len) - min_value(data->temperature, data->temperature_len) > 10.0f) {
        sprintf(result, "Trend due to temperature variation");
    } else if (data->acceleration_len > 2000 && (data->acceleration[0] - data->acceleration[2000] >= 5.0f)) {  // Check that the array is large enough
        sprintf(result, "Trend due to sensor activation");
    } else {
        sprintf(result, "No Trend detected");
    }
}

void check_minor(SensorData* data, char* result) {
    float max_diff = 0.0f;
    for (int i = 0; i < data->acceleration_len - 1; i++) {
        float diff = fabsf(data->acceleration[i + 1] - data->acceleration[i]);
        if (diff > max_diff) {
            max_diff = diff;
        }
    }
    if (max_diff < NOISE_TOLERANCE) {
        sprintf(result, "Minor issue detected due to low sensor resolution");
    } else {
        sprintf(result, "No Minor issues detected");
    }
}

void check_square(SensorData* data, char* result) {
    float avg_acceleration = mean_value(data->acceleration, data->acceleration_len);
    int flat_zone_detected = 1;
    for (int i = 1; i < data->acceleration_len; i++) {
        if (fabsf(data->acceleration[i] - data->acceleration[i - 1]) > NOISE_TOLERANCE || fabsf(data->acceleration[i] - avg_acceleration) > NOISE_TOLERANCE) {
            flat_zone_detected = 0;
            break;
        }
    }
    if (flat_zone_detected && avg_acceleration >= 6.0f) {
        sprintf(result, "Square issue detected due to sensor saturation");
    } else {
        sprintf(result, "No Square issues detected");
    }
}

void check_bias(SensorData* data, char* result) {
    float avg_acceleration = mean_value(data->acceleration, data->acceleration_len);
    for (int i = 1; i < data->acceleration_len; i++) {
        if (fabsf(data->acceleration[i] - avg_acceleration) > NOISE_TOLERANCE * 5.0f) {  // 超过噪声范围
            sprintf(result, "Bias detected due to sensor offset");
            return;
        }
    }
    sprintf(result, "No Bias detected");
}

void check_sudden_rotation(SensorData* data, char* result) {
    for (int i = 1; i < data->acceleration_len - 1; i++) {
        if (fabsf(data->acceleration[i + 1] - data->acceleration[i]) > 2.0f) {  // 突然变化超过噪声范围
            sprintf(result, "Sudden rotation detected");
            return;
        }
    }
    sprintf(result, "No Sudden rotation detected");
}

void check_spike(SensorData* data, char* result) {
    float battery_voltage = data->battery_voltage[data->battery_voltage_len - 1];
    if (battery_voltage < 3.2f) {
        sprintf(result, "Spike due to sudden low battery voltage");
        return;
    }
    int accelerometer_outlier = 0;  // 这里假设accelerometer_outlier标志位
    if (accelerometer_outlier) {
        sprintf(result, "Spike due to structural response or sensor saturation");
    } else {
        sprintf(result, "Spike detected due to sensor attack");
    }
}

void check_missing(SensorData* data, char* result) {
    if (strcmp(data->previous_data, "constant") != 0) {
        sprintf(result, "Missing data due to sensor saturation");
        return;
    }
    float battery_voltage = data->battery_voltage[data->battery_voltage_len - 1];
    if (battery_voltage < 3.3f) {
        sprintf(result, "Missing data due to low battery voltage");
        return;
    }
    float temperature = data->temperature[data->temperature_len - 1];
    if (temperature > 50.0f || temperature < -10.0f) {
        sprintf(result, "Missing data due to extreme temperature");
        return;
    }
    sprintf(result, "Missing data due to hardware/software issue");
}

void diagnose(SensorData* data) {
    char result[256];

    printf("Starting Diagnostics...\n");

    check_drift(data, result);
    printf("Drift: %s\n", result);

    check_trend(data, result);
    printf("Trend: %s\n", result);

    check_minor(data, result);
    printf("Minor: %s\n", result);

    check_square(data, result);
    printf("Square: %s\n", result);

    check_bias(data, result);
    printf("Bias: %s\n", result);

    check_sudden_rotation(data, result);
    printf("Sudden Rotation: %s\n", result);

    check_spike(data, result);
    printf("Spike: %s\n", result);

    check_missing(data, result);
    printf("Missing: %s\n", result);
}

int main() {
    SensorData sensor_data = {
        .battery_voltage = (float[]){3.5f, 3.4f, 3.3f, 3.3f, 3.2f},
        .battery_voltage_len = 5,
        .temperature = (float[]){20.0f, 21.0f, 22.0f, 23.0f, 24.0f},
        .temperature_len = 5,
        .acceleration = (float[]){5.8f, 6.0f, 6.0f, 6.1f, 5.9f},
        .acceleration_len = 5,
        .previous_data = "variable"
    };

    diagnose(&sensor_data);

    return 0;
}

