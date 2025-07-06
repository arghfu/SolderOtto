import matplotlib.pyplot as plt
import numpy as np
from numpy.polynomial import Polynomial


def poly_fit(data, degree=1):
    x, y = zip(*data)
    fitted = Polynomial.fit(x, y, degree)
    fitted = fitted.convert(domain=[min(x), max(x)])
    return fitted


def plot_data(temp_fitted, adc_fitted, temp_orig=None, adc_orig=None):
    x_temp, y_temp = temp_fitted.linspace(41)
    x_adc, y_adc = adc_fitted.linspace(41)

    marker_size = 4

    fig, ax1 = plt.subplots()

    color_temp = 'blue'
    color_adc = 'orange'

    ax1.plot(x_temp, y_temp, label='Temperature Polynomial Fit', linestyle='--', color=color_temp)
    if temp_orig is not None:
        x_temp_orig, y_temp_orig = zip(*temp_orig)
        ax1.plot(x_temp_orig, y_temp_orig, 'bo', label='Temperature Data', markersize=marker_size)
    ax1.set_xlabel('Set Point')
    ax1.set_ylabel('Temperature', color=color_temp)
    ax1.tick_params(axis='y', labelcolor=color_temp)
    ax1.spines['left'].set_color(color_temp)
    ax1.legend(loc='upper left')
    ax1.grid(True)

    ax2 = ax1.twinx()
    ax2.plot(x_adc, y_adc, label='ADC Polynomial Fit', linestyle='--', color=color_adc)
    if adc_orig is not None:
        x_adc_orig, y_adc_orig = zip(*adc_orig)
        ax2.plot(x_adc_orig, y_adc_orig, 'o', label='ADC Data', markersize=marker_size, color=color_adc)
    ax2.set_ylabel('ADC Value', color=color_adc)
    ax2.tick_params(axis='y', labelcolor=color_adc)
    ax2.spines['right'].set_color(color_adc)
    ax2.legend(loc='upper right')

    plt.title('Temperature vs Set Point with Polynomial Fit')
    plt.show()

    return x_temp, y_temp, x_adc, y_adc


def read_value():
    set_point_data = {}

    # Open the input file in read mode
    with open('t245_new.txt', 'r') as infile:
        # Iterate through each line in the input file
        for line in infile:
            # Check if the line starts with "INFO" but not "INFO  adjust setpoint to"
            if line.startswith('INFO') and not line.startswith('INFO  adjust setpoint to'):
                # Extract set_point and read_buffer values
                parts = line.split(',')
                set_point = int(parts[0].split(':')[1].strip())
                read_buffer = int(parts[1].split(':')[1].strip())

                # Store the read_buffer to the corresponding set_point if it's less than or equal to 3800
                if read_buffer <= 3800:
                    if set_point not in set_point_data:
                        set_point_data[set_point] = []
                    set_point_data[set_point].append(read_buffer)

    # Calculate the average read_buffer for each set_point
    set_points = []
    average_read_buffers = []

    values = []

    for set_point, read_buffers in set_point_data.items():
        average_read_buffer = sum(read_buffers) / len(read_buffers)
        set_points.append(set_point)
        average_read_buffers.append(average_read_buffer)
        # print(f"set point: {set_point}, average read: {average_read_buffer:.2f}")
        values.append((set_point, average_read_buffer))

    return values

def plot_yt_over_ya(adc, temp):

    plt.plot(adc, temp, marker='o', linestyle='-', color='green', label='temp over adc')
    plt.xlabel('ADC')
    plt.ylabel('T')
    plt.title('Plot of yt over ya')
    plt.legend()
    plt.grid(True)
    plt.show()


def plot_poly_fit(fitted_poly, label='Polynomial Fit', color='purple'):
    x, y = fitted_poly.linspace(4095)
    plt.plot(x, y, label=label, linestyle='-', color=color)
    plt.xlabel('x')
    plt.ylabel('y')
    plt.title('Polynomial Fit')
    plt.legend()
    plt.grid(True)
    plt.show()



def main():
    temp_values = [
        (0, 22),
        (5, 58),
        (10, 110),
        (15, 142),
        (20, 196),
        (25, 225),
        (30, 260),
        (35, 296),
        (40, 324),
    ]
    deg = 2

    adc_values = read_value()
    # Fit a first-order polynomial (linear regression) to set_point_temperatures
    temp_fitted = poly_fit(temp_values, deg)
    adc_fitted = poly_fit(adc_values, deg)

    xt, yt, xa, ya = plot_data(temp_fitted, adc_fitted, temp_values, adc_values)

    foo_fitted = poly_fit(zip(ya, yt), deg)


    plot_poly_fit(foo_fitted)
    value_at_zero = foo_fitted(0)
    print(f"The value of foo_fitted at x = 0 is: {value_at_zero}")
    print(f"{foo_fitted.convert().coef}")

if __name__ == "__main__":
    main()


