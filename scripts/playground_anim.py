import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.collections import LineCollection
from matplotlib.patches import Circle
from scipy.interpolate import CubicSpline


# Function to smooth the path
def smooth_path(x, y, n=300, bc_type='not-a-knot'):
    t = np.arange(len(x))
    cs_x = CubicSpline(t, x, bc_type=bc_type)
    cs_y = CubicSpline(t, y, bc_type=bc_type)
    x_new = cs_x(np.linspace(t.min(), t.max(), n))
    y_new = cs_y(np.linspace(t.min(), t.max(), n))
    return x_new, y_new

# Load the data
data = pd.read_csv('data/boid_paths_2.csv')

# Define the moving rectangle's start and end bounds
bounds_x_0 = np.array([200, 900])
bounds_y_0 = np.array([200, 200])
bounds_x_1 = np.array([900, 900])
bounds_y_1 = np.array([200, 600])
bounds_x_2 = np.array([900, 200])
bounds_y_2 = np.array([600, 600])
bounds_x_3 = np.array([200, 200])
bounds_y_3 = np.array([600, 200])

bounds_x_0 = np.array([200, 800])
bounds_y_0 = np.array([200, 200])
bounds_x_1 = np.array([800, 800])
bounds_y_1 = np.array([200, 800])
bounds_x_2 = np.array([800, 200])
bounds_y_2 = np.array([800, 800])
bounds_x_3 = np.array([200, 200])
bounds_y_3 = np.array([800, 200])

# Number of frames for the animation
num_frames = 160

# Interpolate bounds for each frame
x_bounds = np.concatenate([
    np.linspace(bounds_x_0[0], bounds_x_0[1], num_frames),
    np.linspace(bounds_x_1[0], bounds_x_1[1], num_frames),
    np.linspace(bounds_x_2[0], bounds_x_2[1], num_frames),
    np.linspace(bounds_x_3[0], bounds_x_3[1], num_frames)
])
y_bounds = np.concatenate([
    np.linspace(bounds_y_0[0], bounds_y_0[1], num_frames),
    np.linspace(bounds_y_1[0], bounds_y_1[1], num_frames),
    np.linspace(bounds_y_2[0], bounds_y_2[1], num_frames),
    np.linspace(bounds_y_3[0], bounds_y_3[1], num_frames)
])

x_corners = [200, 800, 800, 200, 200]
y_corners = [200, 200, 800, 800, 200]

x_bounds, y_bounds = smooth_path(x_corners, y_corners, n=num_frames,bc_type='periodic')

# Set up the base figure
fig, ax = plt.subplots(figsize=(8, 8))
ax.set_facecolor('black')
ax.set_xlim(0, 1000)
ax.set_ylim(0, 1000)
ax.set_aspect('equal', adjustable='box')
plt.axis('off')

# Initialize a rectangle which will be updated during the animation
# circle_radius = 150
# circle = Circle((500, 500), circle_radius, fc='white', ec='white', lw=2, zorder=1000)
# ax.add_patch(circle)

# Animation update function
def update(frame):
    # Clear the axes to remove previous lines
    ax.clear()
    circle_radius = 150
    circle = Circle((500, 500), circle_radius, fc='black', ec='white', lw=1, zorder=1000)
    ax.add_patch(circle)
    
    # Set the background color of the plot to black again
    ax.set_facecolor('black')
    
    # Reset the plot limits and aspect after clearing
    # todo: parameterize
    ax.set_xlim(0, 1000)
    ax.set_ylim(0, 1000)
    ax.set_xlim(0, 1000)
    ax.set_ylim(0, 1000)
    ax.set_aspect('equal', adjustable='box')
    fig.patch.set_facecolor('black')
    ax.set_facecolor('black')
    
    # Remove axis labels for a cleaner look
    plt.axis('off')

    # Get the new bounds for this frame
    # ax.add_patch(new_rect)

    # Add boids with endpoints within the new bounds
    for i in range(1, 4000, 2):
        x = data.iloc[:, i]
        y = data.iloc[:, i + 1]
        # if new_x_bounds[0] <= x.iloc[-1] <= new_x_bounds[1] and new_y_bounds[0] <= y.iloc[-1] <= new_y_bounds[1]:
        # if new_x_bounds[0] <= x.iloc[-1] <= new_x_bounds[0] + 200 and new_y_bounds[0] <= y.iloc[-1] <= new_y_bounds[0] + 200:
        if (x_bounds[frame] - x.iloc[-1]) ** 2 + (y_bounds[frame] - y.iloc[-1]) ** 2 <= circle_radius ** 2:
            x_smooth, y_smooth = smooth_path(x, y)
            points = np.array([x_smooth, y_smooth]).T.reshape(-1, 1, 2)
            segments = np.concatenate([points[:-1], points[1:]], axis=1)
            lc = LineCollection(segments, cmap='gray', norm=plt.Normalize(0, 1), linewidth=1, zorder=5)
            lc.set_array(np.linspace(0, 1, len(x_smooth)))
            ax.add_collection(lc)

    # Return the rectangle and the collections in the axes
    circle.center = (x_bounds[frame], y_bounds[frame])
    
    return [circle] + ax.collections

# Create the animation
anim = FuncAnimation(fig, update, frames=num_frames, blit=True)

# Save the animation
anim.save('boid_paths_animation_03_s.gif', fps=30, writer='pillow', savefig_kwargs={'facecolor':'black'})
#anim.save('boid_paths_animation.mp4', fps=30, extra_args=['-vcodec', 'libx264'])
# anim.save('boid_paths_animation.gif', fps=30, writer='pillow')
plt.close()



