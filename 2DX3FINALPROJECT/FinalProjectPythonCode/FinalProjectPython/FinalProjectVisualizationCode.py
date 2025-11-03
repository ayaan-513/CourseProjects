import serial
import numpy as np
import open3d as o3d
import math

# Open serial connection (modify COM port if needed)
s = serial.Serial('COM6', 115200, timeout=10)
print("Opening: " + s.name)

# Reset buffers to clear any leftover data
s.reset_output_buffer()
s.reset_input_buffer()

# Variables for storing data
angle = 0
xpoints = []
distances = []
totalscans = 3
totaldata = totalscans*32

# Collect 3 sets of data points
for set_num in range(3):
    print(f"Collecting data set {set_num + 1}...")
    input("Press Enter to start communication...")
    s.write('s'.encode())  # Signal MCU to start transmission

    # Read 35 lines, ignore the first 3 
    temp_xpoints = []
    temp_distances = []
    
    for i in range(35):
        x = s.readline()
        try:
            decoded_line = x.decode().strip()
            print(f"Raw Data Received: {decoded_line}")  # Debugging
            
            intermediatee = decoded_line.split(",")
            if len(intermediatee) != 2:
                raise ValueError("Incorrect data format!")
            
            # Convert to float and store
            temp_xpoints.append(float(intermediatee[0]))
            temp_distances.append(float(intermediatee[1]))
        except (ValueError, IndexError) as e:
            print(f"Error parsing line: {decoded_line} - {e}")
    
    # Ensure we have 32 valid data points
    if len(temp_xpoints) < 32 or len(temp_distances) < 32:
        print("Error: Not enough valid data points received!")
        s.close()
        exit()
    
    xpoints.extend(temp_xpoints[:32])
    distances.extend(temp_distances[:32])

# Save data to file
with open("3DScanData.xyz", "w") as f:
    for i in range(totaldata):  # Now we have 3 sets of 32 points
        angle = (i % 32) * 11.25
        x = xpoints[i]
        y = distances[i] * math.cos(math.radians(angle))
        z = distances[i] * math.sin(math.radians(angle))
        print(f"Point {i}: x={x}, y={y}, z={z}")  # Debugging
        f.write(f"{x} {y} {z}\n")

# Close the serial connection
print("Closing: " + s.name)
s.close()

# Read and visualize the point cloud
try:
    print("Read in the 3D Scan data (pcd)")
    pcd = o3d.io.read_point_cloud("3DScanData.xyz", format="xyz")
    print("The 3D Scan PCD array:")
    print(np.asarray(pcd.points))
    
    print("Visualizing the 3D Scan PCD...")
    o3d.visualization.draw_geometries([pcd])
except Exception as e:
    print(f"Error loading point cloud: {e}")

# Define connectivity for lines
yz_slice_vertex = list(range(totaldata))
lines = []

# Connect each yz slice (closing the loop)
for set_num in range(totalscans):
    start_index = set_num * 32
    for i in range(start_index, start_index + 31):
        lines.append([yz_slice_vertex[i], yz_slice_vertex[i + 1]])
    lines.append([yz_slice_vertex[start_index + 31], yz_slice_vertex[start_index]])

# Connect corresponding points between slices for proper 3D structure
for set_num in range(totalscans - 1):  # Connecting set 1 -> set 2 and set 2 -> set 3
    start_index = set_num * 32
    next_index = (set_num + 1) * 32
    for i in range(32):
        lines.append([yz_slice_vertex[start_index + i], yz_slice_vertex[next_index + i]])

# Create and visualize LineSet
try:
    line_set = o3d.geometry.LineSet(
        points=o3d.utility.Vector3dVector(np.asarray(pcd.points)),
        lines=o3d.utility.Vector2iVector(lines)
    )
    print("Visualizing point cloud with lines...")
    o3d.visualization.draw_geometries([line_set])
except Exception as e:
    print(f"Error displaying lines: {e}")

    
