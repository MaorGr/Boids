import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.patches import Circle

# Function to create a circle that moves in a simple pattern
def create_circle_animation():
    # Set up the figure and axis
    fig, ax = plt.subplots()
    ax.set_xlim(0, 1100)
    ax.set_ylim(0, 800)

    # Create a circle
    circle_radius = 150
    circle = Circle((5, 5), circle_radius, fc='blue', ec='white', zorder=10)
    ax.add_patch(circle)

    # Function to update the position of the circle
    def update(frame):
        # Update the position of the circle
        circle.center = (frame * 10 % 1100, 400)
        return circle,

    # Create the animation
    anim = FuncAnimation(fig, update, frames=np.arange(0, 110, 1), blit=True)
    
    # Show the animation
    plt.show()

# Run the function to create the animation
create_circle_animation()
