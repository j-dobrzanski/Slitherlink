import matplotlib.pyplot as plt
import numpy as np

# Parameters
NUM_POLYGONS = 100      # how many polygons to draw in the grid
POLY_RADIUS = 30        # radius from center to corner
POLY_SIDES = 6          # sides of each polygon (6 = hexagon)

# Axial directions for hex grid neighbors
hex_directions = [
    (1, 0), (1, -1), (0, -1),
    (-1, 0), (-1, 1), (0, 1)
]

def polygon_corner(center, radius, sides, i):
    angle_deg = 360 / sides * i - 30  # -30 to make hexes pointy-topped
    angle_rad = np.radians(angle_deg)
    return (
        center[0] + radius * np.cos(angle_rad),
        center[1] + radius * np.sin(angle_rad)
    )

def draw_polygon(ax, center, radius, sides):
    corners = [polygon_corner(center, radius, sides, i) for i in range(sides)]
    corners.append(corners[0])  # close the polygon
    xs, ys = zip(*corners)
    ax.plot(xs, ys, color='black')

def axial_to_pixel(q, r, size):
    x = size * np.sqrt(3) * (q + r / 2)
    y = size * 3/2 * r
    return (x, y)

# BFS/spiral-style growth
visited = set()
queue = [(0, 0)]
visited.add((0, 0))

fig, ax = plt.subplots()
count = 0

while queue and count < NUM_POLYGONS:
    q, r = queue.pop(0)
    center = axial_to_pixel(q, r, POLY_RADIUS)
    draw_polygon(ax, center, POLY_RADIUS, POLY_SIDES)
    count += 1

    for dq, dr in hex_directions:
        neighbor = (q + dq, r + dr)
        if neighbor not in visited:
            visited.add(neighbor)
            queue.append(neighbor)

ax.set_aspect('equal')
plt.axis('off')
plt.savefig('hex_grid.png', bbox_inches='tight', pad_inches=0)
