import matplotlib.pyplot as plt

# Define the start and end points of the line
start = (150.00, 110.00)
end = (170.00, 130.00)

# Define perpendicular vector (example values)
# Replace these with your actual perpendicular vector values
perp_x = 0.7
perp_y = 0.7

# Calculate points for thickness
p1_left = (start[0], start[1] + perp_y)
p1_right = (end[0], start[1] + perp_y)
p2_left = (start[0], end[1])
p2_right = (end[0], end[1])

# Create a plot for the line
fig, ax = plt.subplots(figsize=(10, 6))

# Draw the line
ax.plot([start[0], end[0]], [start[1], end[1]], 'k-', lw=2, label='Line')

# Plot the perpendicular points
ax.plot([p1_left[0], p1_right[0]], [p1_left[1], p1_right[1]], 'ro', label='Start Points')
ax.plot([p2_left[0], p2_right[0]], [p2_left[1], p2_right[1]], 'bo', label='End Points')

# Annotate the points on the main plot
def annotate_point(ax, point, label):
    ax.text(point[0] + 2, point[1] + 2, f'{label}\n({point[0]:.2f}, {point[1]:.2f})',
            fontsize=10, ha='left', va='bottom', color='black', bbox=dict(facecolor='white', alpha=0.7, edgecolor='none'))

# Annotate the points
annotate_point(ax, start, 'Start')
annotate_point(ax, end, 'End')
annotate_point(ax, p1_left, 'p1_left')
annotate_point(ax, p1_right, 'p1_right')
annotate_point(ax, p2_left, 'p2_left')
annotate_point(ax, p2_right, 'p2_right')

# Set plot limits
ax.set_xlim(140, 180)
ax.set_ylim(100, 140)

# Add labels and title
ax.set_xlabel('X Coordinate')
ax.set_ylabel('Y Coordinate')
ax.set_title('Line with Perpendicular Points')
ax.legend()
ax.grid(True)

# Create a separate figure for detailed annotations
fig2, ax2 = plt.subplots(figsize=(4, 6))
ax2.axis('off')  # Turn off the axis

# Define the detailed annotation text
annotation_text = (
    f"Start Point: {start}\n"
    f"End Point: {end}\n\n"
    f"p1_left: {p1_left}\n"
    f"p1_right: {p1_right}\n"
    f"p2_left: {p2_left}\n"
    f"p2_right: {p2_right}"
)

# Add text to the separate figure
ax2.text(0.5, 0.5, annotation_text, ha='center', va='center', fontsize=12,
         bbox=dict(facecolor='white', alpha=0.8, edgecolor='black'))

# Save both figures to files
fig.savefig('line_with_thickness.png')
fig2.savefig('annotations.png')

# If you can use a GUI-based environment later, you can uncomment these lines
# plt.show()
# plt.show()
