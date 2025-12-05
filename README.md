# Brute Force Password Cracker - Parallel Computing Assignment

A comprehensive implementation of parallel brute-force password cracking using MD5 hash comparison. This project demonstrates four different computational approaches: Serial, OpenMP, MPI, and CUDA.



## 🎯 Project Overview

This project implements a brute-force password cracker that:
- Searches through all possible lowercase letter combinations (a-z)
- Uses MD5 hash comparison for password matching
- Compares four parallelization approaches: Serial, OpenMP, MPI, and CUDA
- Measures execution time and speedup for performance analysis

**Target Use Case:** Educational demonstration of parallel computing paradigms and performance optimization.

---

## 📁 Project Structure

```
bruteForceSimulation/
│
├── serial_password_hash.c          # Sequential baseline implementation
│
├── openmp/
│   └── openmp_password_hash.c      # OpenMP parallel implementation
│
├── mpi/
│   └── mpi_password_hash.c         # MPI distributed implementation
│
├── cuda/
│   ├── cuda_password_hash.cu       # CUDA GPU implementation
│   └── md5_device.cuh              # MD5 hash for CUDA device
│
├── Graphs/
│   ├── grpahs.py                   # Performance visualization script
│   ├── ExecutionTime.png           # Execution time comparison
│   ├── Speedup.png                 # Speedup comparison
│   ├── OpenMP/                     # OpenMP-specific graphs
│   ├── MPI/                        # MPI-specific graphs
│   └── CUDA/                       # CUDA-specific graphs
│
├── Screenshots/                    # Execution results
│   ├── OPENMP/
│   ├── MPI/
│   └── CUDA/
│
├── IT23281950_PC_Assignment3.pdf   # Assignment report
├── emailOfApproval.pdf             # Approval documentation
├── Links.txt                       # Reference links
├── LICENSE                         # Project license
└── README.md                       # This file
```

---

## 🔍 Algorithm

### Password Generation Strategy

The algorithm uses **base-26 conversion** to generate password candidates:

```
Index 0    → "aaa"
Index 1    → "aab"
Index 25   → "aaz"
Index 26   → "aba"
Index 675  → "aza"
```

### Search Process

1. **Input:** Target password (lowercase letters only)
2. **Hash Generation:** Compute MD5 hash of target password
3. **Brute Force Search:** 
   - Generate all possible combinations of given length
   - Hash each candidate
   - Compare with target hash
4. **Output:** Found password, execution time, and performance metrics

### Parallelization Strategies

| Method | Strategy | Work Distribution |
|--------|----------|-------------------|
| **Serial** | Sequential iteration | Single thread checks all combinations |
| **OpenMP** | Shared memory | Dynamic scheduling across threads |
| **MPI** | Distributed memory | Strided distribution (rank-based) |
| **CUDA** | GPU massively parallel | Each thread checks multiple passwords |

---

## 📦 Dependencies

### Required Libraries

#### 1. **OpenSSL** (All implementations)
```bash
# Ubuntu/Debian
sudo apt-get install libssl-dev

# macOS
brew install openssl
```

#### 2. **OpenMP** (OpenMP implementation)
- Included with GCC 4.2+ and Clang 3.7+
- No separate installation needed

#### 3. **MPI** (MPI implementation)
```bash

# Or OpenMPI
sudo apt-get install openmpi-bin openmpi-common libopenmpi-dev

```

#### 4. **CUDA Toolkit** (CUDA implementation)
- Download from [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-downloads)
- Requires NVIDIA GPU with compute capability 3.0+
- Minimum version: CUDA 10.0
- Or Use Google Colab for free access to Nvdia Tesla GPU s

### Verification

```bash
# Check OpenSSL
openssl version

# Check GCC with OpenMP support
gcc -fopenmp --version

# Check MPI
mpirun --version

# Check CUDA
nvcc --version
nvidia-smi
```

---

## 🔨 Compilation Instructions

### 1. Serial Implementation

```bash
gcc serial_password_hash.c -lssl -lcrypto -o serial_password_hash
```

**Flags Explained:**
- `-lssl` - Link OpenSSL library
- `-lcrypto` - Link cryptography library (required for MD5)
- `-o` - Specify output executable name

---

### 2. OpenMP Implementation

```bash
cd openmp/
gcc -fopenmp openmp_password_hash.c -lssl -lcrypto -o openmp_password_hash
```

**Flags Explained:**
- `-fopenmp` - Enable OpenMP support
- `-lssl -lcrypto` - Link OpenSSL libraries
- `-O3` (optional) - Enable optimization level 3 for better performance

**Optimized Compilation:**
```bash
gcc -fopenmp -O3 openmp_password_hash.c -lssl -lcrypto -o openmp_password_hash
```

---

### 3. MPI Implementation

```bash
cd mpi/
mpicc -O3 mpi_password_hash.c -lssl -lcrypto -o mpi_password_hash
```

**Flags Explained:**
- `mpicc` - MPI C compiler wrapper
- `-O3` - Optimization level 3
- `-lssl -lcrypto` - Link OpenSSL libraries

**Alternative with explicit compiler:**
```bash
mpicc -O3 -Wall mpi_password_hash.c -lssl -lcrypto -o mpi_password_hash
```

---

### 4. CUDA Implementation

```bash
cd cuda/
nvcc cuda_password_hash.cu -lssl -lcrypto -o cuda_password_hash
```

**Flags Explained:**
- `nvcc` - NVIDIA CUDA compiler
- `-lssl -lcrypto` - Link OpenSSL libraries (for CPU-side MD5)
- `-arch=sm_XX`  - Specify GPU architecture

**Optimized Compilation with Architecture:**
```bash
# For compute capability 7.5 (RTX 20xx series)
nvcc -arch=sm_75 -O3 cuda_password_hash.cu -lssl -lcrypto -o cuda_password_hash

# For compute capability 8.6 (RTX 30xx series)
nvcc -arch=sm_86 -O3 cuda_password_hash.cu -lssl -lcrypto -o cuda_password_hash

# For compute capability 8.9 (RTX 40xx series)
nvcc -arch=sm_89 -O3 cuda_password_hash.cu -lssl -lcrypto -o cuda_password_hash

# For Tesla GPUs
nvcc -arch=sm_70 -O3 cuda_password_hash.cu -lssl -lcrypto -o cuda_password_hash
```

**Check your GPU compute capability:**
```bash
nvidia-smi --query-gpu=compute_cap --format=csv
```

---

## 🚀 Running Instructions

### 1. Serial Implementation

```bash
./serial_password_hash
```

**Example Session:**
```
Enter password to crack (lowercase letters only): test
```

**Output:**
```
=== Starting Brute Force Search ===
Target password: test
Target hash (MD5): 098f6bcd4621d373cade4e832627b4f6
Password length: 4
Character set: abcdefghijklmnopqrstuvwxyz
Total combinations to try: 456976

✓ PASSWORD FOUND!
Password: test
Hash: 098f6bcd4621d373cade4e832627b4f6
Found at attempt: 371293
Execution time: 1.234 seconds
Passwords per second: 300732
```

---

### 2. OpenMP Implementation

#### Basic Usage

```bash
./openmp_password_hash
```

#### Configure Thread Count

**Method 1: Environment Variable**
```bash
export OMP_NUM_THREADS=8
./openmp_password_hash
```

**Method 2: Runtime Setting**
```bash
OMP_NUM_THREADS=4 ./openmp_password_hash
```

#### Recommended Configurations

| Configuration | Command | Use Case |
|--------------|---------|----------|
| **Single Thread** | `OMP_NUM_THREADS=1 ./openmp_password_hash` | Baseline comparison |
| **Dual Core** | `OMP_NUM_THREADS=2 ./openmp_password_hash` | Minimal parallelism |
| **Quad Core** | `OMP_NUM_THREADS=4 ./openmp_password_hash` | Standard desktop |
| **Octa Core** | `OMP_NUM_THREADS=8 ./openmp_password_hash` | High-end desktop |
| **16 Threads** | `OMP_NUM_THREADS=16 ./openmp_password_hash` | Workstation/Server |

#### Performance Testing Script

```bash
#!/bin/bash
# test_openmp.sh

for threads in 1 2 4 8 16; do
    echo "Testing with $threads threads..."
    OMP_NUM_THREADS=$threads ./openmp_password_hash <<< "test"
    echo "---"
done
```

---

### 3. MPI Implementation

#### Basic Usage

```bash
mpirun -np 4 ./mpi_password_hash
```

**Flags Explained:**
- `-np` or `-n` - Number of processes to spawn
- `4` - Number of processes (adjust based on CPU cores)

#### Recommended Configurations

| Configuration | Command | Use Case |
|--------------|---------|----------|
| **Single Process** | `mpirun -np 1 ./mpi_password_hash` | Baseline comparison |
| **2 Processes** | `mpirun -np 2 ./mpi_password_hash` | Dual-node testing |
| **4 Processes** | `mpirun -np 4 ./mpi_password_hash` | Quad-core system |
| **8 Processes** | `mpirun -np 8 ./mpi_password_hash` | 8-core system |
| **16 Processes** | `mpirun -np 16 ./mpi_password_hash` | Multi-node cluster |

#### Advanced MPI Options

```bash
# Bind processes to cores
mpirun -np 8 --bind-to core ./mpi_password_hash

# Specify hostfile for cluster
mpirun -np 16 -hostfile hosts.txt ./mpi_password_hash

# Verbose output
mpirun -np 4 --display-map ./mpi_password_hash
```

#### Performance Testing Script

```bash
#!/bin/bash
# test_mpi.sh

for procs in 1 2 4 8 16; do
    echo "Testing with $procs processes..."
    mpirun -np $procs ./mpi_password_hash <<< "test"
    echo "---"
done
```

---

### 4. CUDA Implementation

#### Basic Usage

```bash
./cuda_password_hash
```

**Default Configuration:**
- Threads per block: 256
- Blocks: Auto-calculated based on workload

#### Configure Threads Per Block (TPB)

```bash
# Syntax: ./cuda_password_hash [threads_per_block] [num_blocks]

# 128 threads per block (auto blocks)
./cuda_password_hash 128

# 256 threads per block, 1024 blocks
./cuda_password_hash 256 1024

# 512 threads per block (auto blocks)
./cuda_password_hash 512
```

#### Recommended Configurations

| TPB | Command | Use Case |
|-----|---------|----------|
| **1** | `./cuda_password_hash 1` | Baseline (minimal parallelism) |
| **2** | `./cuda_password_hash 2` | Testing |
| **4** | `./cuda_password_hash 4` | Testing |
| **8** | `./cuda_password_hash 8` | Testing |
| **16** | `./cuda_password_hash 16` | Small workload |
| **32** | `./cuda_password_hash 32` | Warp size (optimal) |
| **64** | `./cuda_password_hash 64` | Medium workload |
| **128** | `./cuda_password_hash 128` | Standard configuration |
| **256** | `./cuda_password_hash 256` | **Recommended default** |
| **512** | `./cuda_password_hash 512` | Maximum TPB (most GPUs) |
| **1024** | `./cuda_password_hash 1024` | Maximum TPB (newer GPUs) |

#### Understanding TPB (Threads Per Block)

**What is TPB?**
- Threads Per Block defines how many CUDA threads execute together in a block
- GPU hardware organizes threads into warps (32 threads)
- Optimal TPB is typically a multiple of 32



#### Performance Testing Script

```bash
#!/bin/bash
# test_cuda.sh

for tpb in 1 2 4 8 16 32 64 128 256 512; do
    echo "Testing with $tpb threads per block..."
    ./cuda_password_hash $tpb <<< "test"
    echo "---"
done
```

#### GPU-Specific Optimization

```bash
# Check GPU properties
nvidia-smi --query-gpu=name,compute_cap,memory.total --format=csv

# Run with specific GPU (multi-GPU system)
CUDA_VISIBLE_DEVICES=0 ./cuda_password_hash 256

# Profile with nvprof
nvprof ./cuda_password_hash 256 <<< "test"
```

---

## 📊 Performance Analysis

### Benchmark Results (Password: "oshan")

#### Execution Time Comparison

| Implementation | Config | Time (s) | Speedup vs Serial |
|---------------|--------|----------|-------------------|
| **Serial** | 1 thread | 1.276 | 1.00x |
| **OpenMP** | 2 threads | 0.772 | 1.65x |
| **OpenMP** | 4 threads | 0.421 | 3.03x |
| **OpenMP** | 8 threads | 0.290 | 4.40x |
| **OpenMP** | 16 threads | 0.289 | 4.42x |
| **MPI** | 2 processes | 0.292 | 4.37x |
| **MPI** | 4 processes | 0.161 | 7.93x |
| **MPI** | 8 processes | 0.101 | 12.63x |
| **MPI** | 16 processes | 0.060 | 21.27x |
| **CUDA** | 32 TPB | 0.003 | 425.33x |
| **CUDA** | 128 TPB | 0.003 | 425.33x |
| **CUDA** | 256 TPB | 0.003 | 425.33x |

#### Speedup Analysis

**OpenMP Speedup:**
- 1 thread: 1.00x (baseline)
- 16 threads: 4.42x
- Efficiency: 27.6% (limited by synchronization overhead)

**MPI Speedup:**
- 1 process: 1.00x (baseline)
- 16 processes: 9.70x
- Efficiency: 60.6% (better scalability than OpenMP)

**CUDA Speedup:**
- 1 TPB: 1.00x (baseline)
- 32 TPB: 19.67x
- Efficiency: 61.5% (excellent GPU utilization)

### Key Findings

1. **CUDA dominates** for compute-intensive tasks (425x faster than serial)
2. **MPI scales better** than OpenMP for CPU parallelism
3. **OpenMP efficiency drops** beyond 8 threads due to overhead
4. **CUDA performance plateaus** at 32 TPB (warp size optimization)

---

## 🔧 Technical Details

### MD5 Hash Implementation

#### Why Two Different MD5 Implementations?

**CPU (OpenSSL):**
- Used in: Serial, OpenMP, MPI implementations
- Library: OpenSSL's `MD5()` function
- Reason: Standard, optimized, well-tested CPU implementation
- Performance: Excellent for CPU-based hashing

**GPU (Custom CUDA):**
- Used in: CUDA implementation
- File: `md5_device.cuh`
- Reason: OpenSSL cannot run on GPU (device code)
- Source: Adapted from open-source CUDA MD5 implementation (https://github.com/xpn/CUDA-MD5-Crack.git)

#### Why Not OpenSSL for CUDA?

OpenSSL is a **CPU library** and cannot be called from GPU kernels because:

1. **Host vs Device Code:**
   - OpenSSL functions are compiled for CPU (host)
   - CUDA kernels run on GPU (device)
   - Cannot call host functions from device code

2. **Memory Architecture:**
   - OpenSSL expects CPU memory pointers
   - GPU uses separate device memory
   - Incompatible memory models

3. **Threading Model:**
   - OpenSSL designed for CPU threads
   - CUDA uses massively parallel GPU threads
   - Different execution models

**Solution:** Implement MD5 algorithm directly in CUDA device code (`md5_device.cuh`)

### MD5 Device Implementation (`md5_device.cuh`)

**Attribution:**
- Adapted from open-source CUDA MD5 implementation
- Core MD5 algorithm follows RFC 1321 specification
- Modified for password cracking use case

**Implementation:**
- Password generation strategy
- Work distribution across threads
- Memory management
- Result synchronization

**Adapted Component:**
- MD5 hash computation function (`md5_cuda`)
- Standard MD5 constants and transformations

### Character Set and Limitations

**Current Configuration:**
```c
#define CHARSET "abcdefghijklmnopqrstuvwxyz"
#define CHARSET_SIZE 26
#define MAX_PASSWORD_LENGTH 10
```

**Complexity:**
- 3-character password: 26³ = 17,576 combinations
- 4-character password: 26⁴ = 456,976 combinations
- 5-character password: 26⁵ = 11,881,376 combinations
- 6-character password: 26⁶ = 308,915,776 combinations

**Extending Character Set:**

To support uppercase, numbers, and symbols, modify:
```c
#define CHARSET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()"
#define CHARSET_SIZE 72
```

---



## ⚠️ Important Notes

### Security Disclaimer

**This project is for EDUCATIONAL PURPOSES ONLY.**

- Do NOT use for unauthorized password cracking
- Do NOT use on systems you don't own
- Demonstrates why weak passwords are insecure
- Shows importance of strong password policies

### Performance Warnings

**Long Execution Times:**
- 7+ character passwords may take hours/days
- 8+ character passwords may take weeks/months
- Use short passwords (3-5 chars) for testing

**Resource Usage:**
- CUDA implementation uses significant GPU memory
- MPI may require cluster configuration
- Monitor system resources during execution

---

## 📝 License

See [LICENSE](LICENSE) file for details.

---

## 👥 Contributors

- **Student ID:** IT23281950
- **Course:** Parallel Computing Assignment 3
- **Institution:** [Your Institution Name]



---

## 🔗 Additional Resources

- **Performance Graphs:** See `Graphs/` directory
- **Execution Screenshots:** See `Screenshots/` directory
- **Detailed Report:** See `IT23281950_PC_Assignment3.pdf`

---

