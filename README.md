# PLife

A particle life simulation written in C. Particle Life explores the concept of emergence, where complex structures arise from simple rules.

## Setting up Conan

If you wish to modify the particle simulation code itself (`src/main.c`), you must set up your Conan environment or link dependencies manually. Conan is a package manager for C/C++ and is used in this project to resolve the SDL2 and ZLIB dependencies. Please refer to the [official documentation](https://docs.conan.io/2/index.html).

## Usage

```bash
cd build
./plife simulation.txt
```

## Using `runner.py`

The `runner.py` script automates the creation of `simulation.txt` with customizable attributes and runs the PLife simulation.

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
- `-d`, `--degree`: Degree distribution (integer).
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

### Notes on Combining Attributes

- **Sparsity and Degree Distribution**: If both are specified, degree distribution takes precedence.
- **Reciprocity and Skew Symmetry**: Reciprocity is applied after skew symmetry and can override it.

## Notes

- Modify `build/simulation.txt` to manually adjust particles, the attraction matrix, or epochs.
- The `simulation.txt` file can be renamed; specify the correct filename when running the simulation. However, `runner.py` uses `build/simulation.txt`.