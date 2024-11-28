# PLife

A particle simulation written in C. Particle Life explores the concept of emergence, where complex structures arise from simple rules. Inspired by Tom Mohr's [video](https://www.youtube.com/watch?v=p4YirERTVF0&t=393s) on Particle Life simulations.

## C Environment

If you wish to modify the particle simulation code (`src/main.c`), you must set up your Conan environment or link dependencies manually. Conan is a package manager for C/C++ and is used in this project to resolve the SDL2 and ZLIB dependencies. Please refer to the [official documentation](https://docs.conan.io/2/index.html).

## Basic Usage (without runner)

```bash
cd build
./plife simulation.txt
```

Modify `simulation.txt` to configure simulation properties. Ensure that if you specify `n` quantities, the attraction matrix is `n x n`. We recommend a viscosity of `0.4` and a repulsion strength of `4.0` based on testing.

- `Epochs`: Number of simulation frames to render
- `Quantities`: Number of each particle type in simulation
- `Attraction`: Attraction matrix, in same order as quantities are defined
- `Viscosity`: Slows particles down as they move to prevent chaos
- `Radius`: How close particles need to be to interact
- `RepulsionStrength`: How much particles push back on eachother when getting too close (to minimize particle overlap)

Example `simulation.txt`:

```bash
#Epochs
10
#Quantities
50 50
#Attraction
0.5 -0.2
-0.3 0.8
#Viscosity
0.4
#Radius
50
#RepulsionStrength
4.0
```

## Using `runner.py`

The `runner.py` script automates the creation of `simulation.txt` with customizable attributes and runs the PLife simulation. Currently epochs, viscocity, radius, and repulsion strength cannot be modified with flags directly. If you wish to modify these, do so in the `runner.py` script directly. `runner.py` only supports creation of uniform particle quantities.

### Basic Usage

```bash
python3 runner.py -s SIZE -n NUMBER
```

- `-s SIZE`: Number of particle types (required).
- `-n NUMBER`: Number of particles per type (required).

Example:

```bash
python3 runner.py -s 5 -n 100
```

### Optional Flags

- `-a`, `--assortativity`: `high` or `low`.
- `-d`, `--degree`: Degree distribution (integer between 1 and size of matrix `-s`).
- `-c`, `--clustering`: Clustering coefficient (0 to 1).
- `-k`, `--skew-symmetry`: `high`, `medium`, or `low`.
- `-r`, `--reciprocity`: Reciprocity level (0 to 1).
- `-p`, `--sparsity`: Proportion of possible edges (0 to 1).

### Examples

#### High Assortativity

```bash
python3 runner.py -s 5 -n 100 -a high
```

#### Specifying Sparsity

```bash
python3 runner.py -s 5 -n 100 -p 0.2
```

#### Applying Skew Symmetry and Reciprocity

```bash
python3 runner.py -s 5 -n 100 -k high -r 0.7
```

### Combining Attributes

You can combine any number of attributes. However, some attributes will contradict each other, so note the following conditions.

- **Sparsity and Degree Distribution**: If both are specified, degree distribution takes precedence.
- **Reciprocity and Skew Symmetry**: Reciprocity is applied after skew symmetry and can override it.

## Notes

- Modify `build/simulation.txt` to manually adjust particles, the attraction matrix, or epochs.
- The `simulation.txt` file can be renamed; specify the correct filename when running the simulation. However, `runner.py` uses `build/simulation.txt`.
