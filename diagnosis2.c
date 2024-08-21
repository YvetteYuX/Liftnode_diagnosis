#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// define sensor data struct
typedef struct {
    float battery_voltage[5];
    float temperature[5];
    float acceleration[5];
    char previous_data[10];
} SensorData;

// define diagnosis function
char* check_drift(SensorData sensor_data) {
    float max_voltage = sensor_data.battery_voltage[0];
    float min_voltage = sensor_data.battery_voltage[0];
    for (int i = 1; i < 5; i++) {
        if (sensor_data.battery_voltage[i] > max_voltage)
            max_voltage = sensor_data.battery_voltage[i];
        if (sensor_data.battery_voltage[i] < min_voltage)
            min_voltage = sensor_data.battery_voltage[i];
    }
    if (max_voltage - min_voltage > 0.5) 
        return "Drift due to battery voltage variation";
    return "No Drift detected";
}

char* check_trend(SensorData sensor_data) {
    float max_temp = sensor_data.temperature[0];
    float min_temp = sensor_data.temperature[0];
    for (int i = 1; i < 5; i++) {
        if (sensor_data.temperature[i] > max_temp)
            max_temp = sensor_data.temperature[i];
        if (sensor_data.temperature[i] < min_temp)
            min_temp = sensor_data.temperature[i];
    }
    if (max_temp - min_temp > 10)
        return "Trend due to temperature variation";

    if (sensor_data.acceleration[0] - sensor_data.acceleration[4] >= 5)
        return "Trend due to sensor activation";

    return "No Trend detected";
}

char* check_minor(SensorData sensor_data) {
    float noise_tolerance = 0.1;
    for (int i = 0; i < 4; i++) {
        if (fabs(sensor_data.acceleration[i+1] - sensor_data.acceleration[i]) > noise_tolerance) 
            return "No Minor issues detected";
    }
    return "Minor issue detected due to low sensor resolution";
}

char* check_square(SensorData sensor_data) {
    float noise_tolerance = 0.1;
    float avg_acceleration = 0.0;
    for (int i = 0; i < 5; i++) {
        avg_acceleration += sensor_data.acceleration[i];
    }
    avg_acceleration /= 5;

    for (int i = 1; i < 5; i++) {
        if (fabs(sensor_data.acceleration[i] - sensor_data.acceleration[i-1]) > noise_tolerance || 
            fabs(sensor_data.acceleration[i] - avg_acceleration) > noise_tolerance) {
            return "No Square issues detected";
        }
    }

    if (avg_acceleration >= 6.0)
        return "Square issue detected due to sensor saturation";
    
    return "No Square issues detected";
}

char* check_bias(SensorData sensor_data) {
    float noise_tolerance = 0.1;
    float avg_acceleration = 0.0;
    for (int i = 0; i < 5; i++) {
        avg_acceleration += sensor_data.acceleration[i];
    }
    avg_acceleration /= 5;

    for (int i = 1; i < 5; i++) {
        if (fabs(sensor_data.acceleration[i] - avg_acceleration) > noise_tolerance * 5) 
            return "Bias detected due to sensor offset";
    }
    return "No Bias detected";
}

char* check_sudden_rotation(SensorData sensor_data) {
    for (int i = 1; i < 4; i++) {
        if (fabs(sensor_data.acceleration[i+1] - sensor_data.acceleration[i]) > 2.0)
            return "Sudden rotation detected";
    }
    return "No Sudden rotation detected";
}

char* check_outlier(SensorData sensor_data) {
    if (sensor_data.battery_voltage[4] < 3.2)
        return "Outlier due to sudden low battery voltage";

    return "No Outlier detected";
}

char* check_missing(SensorData sensor_data) {
    if (sensor_data.previous_data[0] != 'c')
        return "Missing data due to sensor saturation";

    if (sensor_data.battery_voltage[4] < 3.3)
        return "Missing data due to low battery voltage";

    if (sensor_data.temperature[4] > 50 || sensor_data.temperature[4] < -10)
        return "Missing data due to extreme temperature";

    return "Missing data due to hardware/software issue";
}

// Select the data type
void diagnose(SensorData sensor_data, int choice) {
    switch (choice) {
        case 0:
            printf("Normal: No diagnostics needed.\n");
            break;
        case 1:
            printf("Missing: %s\n", check_missing(sensor_data));
            break;
        case 2:
            printf("Minor: %s\n", check_minor(sensor_data));
            break;
        case 3:
            printf("Outlier: %s\n", check_outlier(sensor_data));  
            break;
        case 4:
            printf("Square: %s\n", check_square(sensor_data));
            break;
        case 5:
            printf("Trend: %s\n", check_trend(sensor_data));
            break;
        case 6:
            printf("Drift: %s\n", check_drift(sensor_data));
            break;
        default:
            printf("Invalid choice.\n");
            break;
    }
}

int main() {
    SensorData sensor_data = {
        {3.5, 3.4, 3.3, 3.3, 3.2},  // battery_voltage
        {20, 21, 22, 23, 24},       // temperature
        {5.8, 6.0, 6.0, 6.1, 5.9},  // acceleration
        "variable"                  // previous_data
    };

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

    diagnose(sensor_data, choice);

    return 0;
}
