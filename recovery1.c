#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// 函数声明
void polynomial_fit(double *x, double *y, int n, int degree, double *coeffs);
double calculate_mse(double *x, double *y, int n, double *coeffs, int degree);
double evaluate_polynomial(double *coeffs, int degree, double x);

// 定义常量
#define MAX_DEGREE 10
#define ERROR_THRESHOLD 1e-2

int main() {
    // 示例数据
    int n = 10;
    double x[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};  // 输入x坐标
    double y[] = {2.3, 2.8, 3.6, 4.5, 5.1, 5.9, 7.3, 8.1, 9.0, 10.2};  // 输入y坐标

    // 自适应多项式拟合
    double coeffs[MAX_DEGREE + 1];
    int best_degree = 1;
    for (int degree = 1; degree <= MAX_DEGREE; degree++) {
        polynomial_fit(x, y, n, degree, coeffs);
        double mse = calculate_mse(x, y, n, coeffs, degree);
        
        // 判断是否满足误差阈值
        if (mse < ERROR_THRESHOLD) {
            best_degree = degree;
            break;
        }
    }

    printf("Best polynomial degree: %d\n", best_degree);

    // 恢复数据（去趋势）
    double detrended_data[n];
    for (int i = 0; i < n; i++) {
        double trend = evaluate_polynomial(coeffs, best_degree, x[i]);
        detrended_data[i] = y[i] - trend;
        printf("Original: %.2f, Trend: %.2f, Detrended: %.2f\n", y[i], trend, detrended_data[i]);
    }

    return 0;
}

// 最小二乘法多项式拟合
void polynomial_fit(double *x, double *y, int n, int degree, double *coeffs) {
    double X[2 * degree + 1];
    double B[degree + 1][degree + 2];
    for (int i = 0; i < 2 * degree + 1; i++) {
        X[i] = 0;
        for (int j = 0; j < n; j++) {
            X[i] += pow(x[j], i);
        }
    }

    for (int i = 0; i <= degree; i++) {
        for (int j = 0; j <= degree; j++) {
            B[i][j] = X[i + j];
        }
    }

    double Y[degree + 1];
    for (int i = 0; i <= degree; i++) {
        Y[i] = 0;
        for (int j = 0; j < n; j++) {
            Y[i] += pow(x[j], i) * y[j];
        }
        B[i][degree + 1] = Y[i];
    }

    // 高斯消元法解线性方程组
    for (int i = 0; i <= degree; i++) {
        for (int k = i + 1; k <= degree; k++) {
            if (B[i][i] < B[k][i]) {
                for (int j = 0; j <= degree + 1; j++) {
                    double temp = B[i][j];
                    B[i][j] = B[k][j];
                    B[k][j] = temp;
                }
            }
        }
    }

    for (int i = 0; i <= degree; i++) {
        for (int k = i + 1; k <= degree; k++) {
            double t = B[k][i] / B[i][i];
            for (int j = 0; j <= degree + 1; j++) {
                B[k][j] -= t * B[i][j];
            }
        }
    }

    for (int i = degree; i >= 0; i--) {
        coeffs[i] = B[i][degree + 1];
        for (int j = 0; j < degree; j++) {
            if (j != i) {
                coeffs[i] -= B[i][j] * coeffs[j];
            }
        }
        coeffs[i] /= B[i][i];
    }
}

// 计算均方误差
double calculate_mse(double *x, double *y, int n, double *coeffs, int degree) {
    double mse = 0;
    for (int i = 0; i < n; i++) {
        double y_fit = evaluate_polynomial(coeffs, degree, x[i]);
        mse += pow(y[i] - y_fit, 2);
    }
    return mse / n;
}

// 多项式求值
double evaluate_polynomial(double *coeffs, int degree, double x) {
    double result = 0;
    for (int i = 0; i <= degree; i++) {
        result += coeffs[i] * pow(x, i);
    }
    return result;
}
