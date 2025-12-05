import matplotlib.pyplot as plt
import numpy as np

# Data for all three approaches
openmp_threads = [1, 2, 4, 8, 16]
openmp_times = [1.276, 0.772, 0.421, 0.290, 0.289]

mpi_processes = [1, 2, 4, 8, 16]
mpi_times = [0.5799, 0.2922, 0.1607, 0.1013, 0.0598]

cuda_threads = [1, 2, 4, 8, 16, 32, 64, 128]
cuda_times = [0.059, 0.030, 0.016, 0.008, 0.005, 0.003, 0.003, 0.003]

# Calculate speedup (sequential time / parallel time)
openmp_speedup = [openmp_times[0] / t for t in openmp_times]
mpi_speedup = [mpi_times[0] / t for t in mpi_times]
cuda_speedup = [cuda_times[0] / t for t in cuda_times]

# Create the plot
plt.figure(figsize=(14, 7))

# Plot all three approaches
plt.plot(openmp_threads, openmp_speedup, 'o-', linewidth=2.5, markersize=10, 
         label='OpenMP', color='#ef4444')
plt.plot(mpi_processes, mpi_speedup, 's-', linewidth=2.5, markersize=10, 
         label='MPI', color='#3b82f6')
plt.plot(cuda_threads, cuda_speedup, '^-', linewidth=2.5, markersize=10, 
         label='CUDA', color='#10b981')

# Plot ideal linear speedup for reference (only up to 16 for clarity)
ideal_range = list(range(1, 17))
plt.plot(ideal_range, ideal_range, '--', linewidth=2, 
         label='Ideal Linear Speedup', color='gray', alpha=0.6)

# Customize the plot
plt.xlabel('Number of Threads/Processes', fontsize=13, fontweight='bold')
plt.ylabel('Speedup', fontsize=13, fontweight='bold')
plt.title('Password Cracking Speedup Comparison\n(Password: "oshan")', 
          fontsize=15, fontweight='bold', pad=20)

# Add grid
plt.grid(True, alpha=0.3, linestyle='--', linewidth=0.8)

# Customize legend
plt.legend(fontsize=12, loc='upper left', framealpha=0.95, shadow=True)

# Set x-axis with better spacing - use log scale for x-axis
plt.xscale('log', base=2)
plt.xticks([1, 2, 4, 8, 16, 32, 64, 128], ['1', '2', '4', '8', '16', '32', '64', '128'])

# Add annotations for best speedup with better positioning
plt.annotate(f'{max(openmp_speedup):.2f}x', 
             xy=(16, max(openmp_speedup)), xytext=(22, max(openmp_speedup) - 1.5),
             arrowprops=dict(arrowstyle='->', color='#ef4444', lw=2),
             fontsize=10, color='#ef4444', fontweight='bold',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor='#ef4444', alpha=0.8))

plt.annotate(f'{max(mpi_speedup):.2f}x', 
             xy=(16, max(mpi_speedup)), xytext=(22, max(mpi_speedup) + 1.5),
             arrowprops=dict(arrowstyle='->', color='#3b82f6', lw=2),
             fontsize=10, color='#3b82f6', fontweight='bold',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor='#3b82f6', alpha=0.8))

plt.annotate(f'{max(cuda_speedup):.2f}x', 
             xy=(32, max(cuda_speedup)), xytext=(50, max(cuda_speedup) + 1),
             arrowprops=dict(arrowstyle='->', color='#10b981', lw=2),
             fontsize=10, color='#10b981', fontweight='bold',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor='#10b981', alpha=0.8))

# Set y-axis limits with some padding
plt.ylim(bottom=0, top=max(cuda_speedup) + 3)
plt.xlim(left=0.8, right=140)

# Tight layout
plt.tight_layout()

# Display the plot
plt.show()

# Print speedup statistics
print("\n" + "="*60)
print("SPEEDUP ANALYSIS SUMMARY")
print("="*60)

print(f"\nOpenMP:")
print(f"  1 thread: {openmp_speedup[0]:.2f}x (baseline)")
print(f"  2 threads: {openmp_speedup[1]:.2f}x")
print(f"  4 threads: {openmp_speedup[2]:.2f}x")
print(f"  8 threads: {openmp_speedup[3]:.2f}x")
print(f"  16 threads: {openmp_speedup[4]:.2f}x (max)")
print(f"  Efficiency at 16 threads: {(openmp_speedup[4]/16)*100:.1f}%")

print(f"\nMPI:")
print(f"  1 process: {mpi_speedup[0]:.2f}x (baseline)")
print(f"  2 processes: {mpi_speedup[1]:.2f}x")
print(f"  4 processes: {mpi_speedup[2]:.2f}x")
print(f"  8 processes: {mpi_speedup[3]:.2f}x")
print(f"  16 processes: {mpi_speedup[4]:.2f}x (max)")
print(f"  Efficiency at 16 processes: {(mpi_speedup[4]/16)*100:.1f}%")

print(f"\nCUDA:")
print(f"  1 thread: {cuda_speedup[0]:.2f}x (baseline)")
print(f"  2 threads: {cuda_speedup[1]:.2f}x")
print(f"  4 threads: {cuda_speedup[2]:.2f}x")
print(f"  8 threads: {cuda_speedup[3]:.2f}x")
print(f"  16 threads: {cuda_speedup[4]:.2f}x")
print(f"  32 threads: {cuda_speedup[5]:.2f}x (max)")
print(f"  64 threads: {cuda_speedup[6]:.2f}x")
print(f"  128 threads: {cuda_speedup[7]:.2f}x")
print(f"  Efficiency at 32 threads: {(cuda_speedup[5]/32)*100:.1f}%")

print(f"\nScalability Comparison:")
print(f"  OpenMP scales up to: {max(openmp_speedup):.2f}x")
print(f"  MPI scales up to: {max(mpi_speedup):.2f}x")
print(f"  CUDA scales up to: {max(cuda_speedup):.2f}x")

print("="*60)