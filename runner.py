import argparse
import numpy as np
import subprocess
import random

parser = argparse.ArgumentParser(description='Generate simulation.txt with specified attributes and run plife simulation.')

# Required arguments
parser.add_argument('-s', '--size', type=int, required=True,
                    help='Size of the matrix (number of particle types)')
parser.add_argument('-n', '--number', type=int, required=True,
                    help='Number of particles per type')

# Optional attributes
parser.add_argument('-a', '--assortativity', choices=['high', 'low'],
                    help='Assortativity of the matrix (high or low)')
parser.add_argument('-d', '--degree', type=int,
                    help='Degree distribution (number of significant connections per particle type)')
parser.add_argument('-c', '--clustering', type=float,
                    help='Clustering coefficient (between 0 and 1)')
parser.add_argument('-k', '--skew-symmetry', choices=['high', 'medium', 'low'],
                    help='Skew symmetry level (high, medium, low)')
parser.add_argument('-r', '--reciprocity', type=float,
                    help='Reciprocity level (0 to 1)')
parser.add_argument('-p', '--sparsity', type=float,
                    help='Proportion of possible edges to include (0 to 1)')

args = parser.parse_args()

size = args.size
number = args.number

# Generate quantities line
quantities = ' '.join([str(number)] * size)

# Generate base random matrix with values between -1 and 1
matrix = np.random.uniform(-1, 1, (size, size))

# Ensure diagonal elements represent self-interaction appropriatly
np.fill_diagonal(matrix, np.random.uniform(-1, 1, size))

# Apply transformations based on provided flags
# Transformations are applied in the following order:
# 1. Sparsity
# 2. Degree distribution and Clustering
# 3. Assortativity
# 4. Skew Symmetry
# 5. Reciprocity

# 1. Sparsity
if args.sparsity is not None:
    sparsity = args.sparsity
    if not (0 <= sparsity <= 1):
        raise ValueError("Sparsity must be between 0 and 1.")
    # Create a mask for edges to keep
    mask = np.random.rand(size, size) < sparsity
    # Ensure diagonal elements are always included
    np.fill_diagonal(mask, True)
    # Apply mask to the matrix
    matrix = matrix * mask

# 2. Degree Distribution and Clustering
if args.degree is not None:
    degree = args.degree
    if degree < 1 or degree > size:
        raise ValueError(f"Degree must be between 1 and the size of the matrix ({size}).")
    # Initialize connections
    connections = {i: set() for i in range(size)}
    for i in range(size):
        # Exclude self in initial choice
        possible_indices = list(set(range(size)) - {i})
        # Randomly select neighbors
        neighbors = set(np.random.choice(possible_indices, min(degree - 1, len(possible_indices)), replace=False))
        neighbors.add(i)  # Include self-interaction
        connections[i] = neighbors

    # Introduce clustering
    if args.clustering is not None:
        clustering = args.clustering
        if not (0 <= clustering <= 1):
            raise ValueError("Clustering coefficient must be between 0 and 1.")
        for i in range(size):
            neighbors = connections[i]
            neighbor_list = list(neighbors - {i})
            for j in neighbor_list:
                for k in neighbor_list:
                    if j != k and k not in connections[j]:
                        if random.random() < clustering:
                            connections[j].add(k)
                            connections[k].add(j)
    else:
        clustering = 0  # Default clustering coefficient is 0

    # Adjust matrix to keep only specified connections
    new_matrix = np.zeros((size, size))
    for i in range(size):
        for j in connections[i]:
            new_matrix[i][j] = matrix[i][j]
    matrix = new_matrix

# 3. Assortativity
if args.assortativity is not None:
    if args.assortativity == 'high':
        # Increase diagonal values (self-attraction)
        for i in range(size):
            matrix[i][i] = np.random.uniform(0.2, 0.5)
        # Decrease off-diagonal values (repulsion)
        for i in range(size):
            for j in range(size):
                if i != j and matrix[i][j] != 0:
                    matrix[i][j] = np.random.uniform(-0.5, -0.1)
    elif args.assortativity == 'low':
        # Decrease diagonal values (self-repulsion)
        for i in range(size):
            matrix[i][i] = np.random.uniform(-0.5, -0.1)
        # Increase off-diagonal values (attraction)
        for i in range(size):
            for j in range(size):
                if i != j and matrix[i][j] != 0:
                    matrix[i][j] = np.random.uniform(0.2, 0.5)

# 4. Skew Symmetry
if args.skew_symmetry is not None:
    skew_symmetry = args.skew_symmetry
    for i in range(size):
        for j in range(i+1, size):
            if matrix[i][j] != 0 or matrix[j][i] != 0:
                if skew_symmetry == 'high':
                    # Enforce a_ji = -a_ij + noise
                    noise = np.random.normal(0, 0.05)
                    matrix[j][i] = -matrix[i][j] + noise
                elif skew_symmetry == 'medium':
                    if random.random() < 0.5:
                        noise = np.random.normal(0, 0.05)
                        matrix[j][i] = -matrix[i][j] + noise
                    else:
                        # Keep original values
                        pass
                elif skew_symmetry == 'low':
                    # Assign independently (do nothing)
                    pass

# 5. Reciprocity
if args.reciprocity is not None:
    reciprocity = args.reciprocity
    if not (0 <= reciprocity <= 1):
        raise ValueError("Reciprocity level must be between 0 and 1.")
    for i in range(size):
        for j in range(i+1, size):
            if matrix[i][j] != 0 or matrix[j][i] != 0:
                if random.random() < reciprocity:
                    # Enforce reciprocity
                    avg_value = (matrix[i][j] + matrix[j][i]) / 2
                    matrix[i][j] = avg_value
                    matrix[j][i] = avg_value

# Write to build/simulation.txt
with open('build/simulation.txt', 'w') as f:
    f.write('#Epochs\n')
    f.write('100000\n')
    f.write('#Quantities\n')
    f.write(quantities + '\n')
    f.write('#Attraction\n')
    for row in matrix:
        row_str = ' '.join([str(round(x, 3)) for x in row])
        f.write(row_str + '\n')
    f.write('#Viscosity\n')
    f.write('0.4\n')
    f.write('#Radius\n')
    f.write('50\n')
    f.write('#RepulsionStrength\n')
    f.write('4.0\n')

# Print the matrix with improved readability
print('Generated Attraction Matrix:')
for row in matrix:
    row_str = ' '.join([f"{x:>8.3f}" for x in row])
    print(row_str)

# Run the plife simulation
subprocess.run(['./build/plife', 'build/simulation.txt'])
