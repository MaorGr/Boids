import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
from matplotlib.colors import LinearSegmentedColormap

from scipy.interpolate import CubicSpline

def smooth_path(x, y):
    # Generate a sequence of indices
    t = np.arange(len(x))
    # Fit a cubic spline to the path
    cs_x = CubicSpline(t, x)
    cs_y = CubicSpline(t, y)
    # Interpolate it to a new set of points for a smooth curve
    t_new = np.linspace(t.min(), t.max(), 300)
    x_new = cs_x(t_new)
    y_new = cs_y(t_new)
    return x_new, y_new

def plot_boid_paths(csv_file):
    # Load the data
    data = pd.read_csv(csv_file)

    black_white_cmap = LinearSegmentedColormap.from_list('black_white', ['black', 'white', 'red'])



    # Define the rectangle bounds for filtering
    x_bounds = [200, 400]
    y_bounds = [200, 400]

    # Create a new plot with equal aspect ratio
    plt.figure(figsize=(11, 8))

    # Iterate over all 2000 boids to plot only those with ends within the specified rectangle
    for i in range(1, 4000, 2):  # Skip the first column (timepoint), then go in pairs (x, y)
        x = data.iloc[:, i]
        y = data.iloc[:, i + 1]
        
        # Check if the last point (x, y) is within the bounds of the rectangle
        if x_bounds[0] <= x.iloc[-1] <= x_bounds[1] and y_bounds[0] <= y.iloc[-1] <= y_bounds[1]:
             # Smooth the path
            x_smooth, y_smooth = smooth_path(x, y)

            # Create a set of line segments so that we can color them individually
            points = np.array([x_smooth, y_smooth]).T.reshape(-1, 1, 2)
            segments = np.concatenate([points[:-1], points[1:]], axis=1)

            # Create a LineCollection from the segments
            lc = LineCollection(segments, cmap=black_white_cmap, norm=plt.Normalize(0, 1), linewidth=1)
            lc.set_array(np.linspace(0, 1, len(x_smooth)))
            plt.gca().add_collection(lc)

    # Set the aspect of the plot to be equal
    plt.gca().set_aspect('equal', adjustable='box')

    # Set limits to fully view the rectangle
    plt.xlim(0, 1100)
    plt.ylim(0, 800)

    # Hide axis labels for a cleaner look
    plt.axis('off')

    # Save the plot to a file
    # plt.savefig('boid_paths_filtered.png')
    plt.savefig('boid_paths_filtered_smooth_black_background_3.png', facecolor='black', transparent=True)
    
    plt.close()

# Run the function with the CSV file name
plot_boid_paths('data/boid_paths.csv')
