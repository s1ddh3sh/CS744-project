import pandas as pd
import matplotlib.pyplot as plt
import os

output_dir = "results/plots"   # folder to save images
os.makedirs(output_dir, exist_ok=True)

# Read CSV (skip empty lines at the end)
df = pd.read_csv("results/get_popular.csv").dropna()

# Convert columns to numeric (if needed)
df = df.apply(pd.to_numeric, errors='ignore')


def save_plot(filename):
    plt.grid(True)
    plt.xticks(df['Threads'])  # show ALL thread values on X axis
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, filename), dpi=300)
    # plt.show()


# Plot 1: Throughput vs Threads
plt.figure(figsize=(8,5))
plt.plot(df['Threads'], df['Throughput'], marker='o')
plt.title("get_popular: Throughput")
plt.xlabel("Load Level")
plt.ylabel("Throughput req/s")
plt.grid(True)
plt.tight_layout()
# plt.show()
save_plot("get_popular_throughput.png")

# Plot 2: Response Time vs Threads
plt.figure(figsize=(8,5))
plt.plot(df['Threads'], df['Response Time'], marker='o', color='orange')
plt.title("get_popular: Response Time")
plt.xlabel("Load Level")
plt.ylabel("Response Time (ms)")
plt.grid(True)
plt.tight_layout()
# plt.show()
save_plot("get_popular_response.png")

# Plot 3: CPU Util vs Threads
plt.figure(figsize=(8,5))
plt.plot(df['Threads'], df['CPU util'], marker='o', color='green')
plt.title("get_popular: CPU Utilization")
plt.xlabel("Load Level")
plt.ylabel("CPU Util (%)")
plt.grid(True)
plt.tight_layout()
# plt.show()
save_plot("get_popular_cpu.png")

# Plot 4: Mem Util vs Threads
plt.figure(figsize=(8,5))
plt.plot(df['Threads'], df['DB util'], marker='o', color='green')
plt.title("get_popular Mem Utilization")
plt.xlabel("Load Level")
plt.ylabel("DB Util (%)")
plt.grid(True)
plt.tight_layout()
# plt.show()
save_plot("get_popular_db.png")

df = pd.read_csv("results/put_all.csv").dropna()

# Convert columns to numeric (if needed)
df = df.apply(pd.to_numeric, errors='ignore')

# Plot 1: Throughput vs Threads
plt.figure(figsize=(8,5))
plt.plot(df['Threads'], df['Throughput'], marker='o')
plt.title("put_all: Throughput")
plt.xlabel("Load Level")
plt.ylabel("Throughput req/s")
plt.grid(True)
plt.tight_layout()
# plt.show()
save_plot("put_all_throughput.png")

# Plot 2: Response Time vs Threads
plt.figure(figsize=(8,5))
plt.plot(df['Threads'], df['Response Time'], marker='o', color='orange')
plt.title("put_all: Response Time")
plt.xlabel("Load Level")
plt.ylabel("Response Time (ms)")
plt.grid(True)
plt.tight_layout()
# plt.show()
save_plot("put_all_response.png")

# Plot 3: CPU Util vs Threads
plt.figure(figsize=(8,5))
plt.plot(df['Threads'], df['CPU util'], marker='o', color='green')
plt.title("put_all CPU Utilization")
plt.xlabel("Load Level")
plt.ylabel("CPU Util (%)")
plt.grid(True)
plt.tight_layout()
# plt.show()
save_plot("put_all_cpu.png")

# Plot 4: Mem Util vs Threads
plt.figure(figsize=(8,5))
plt.plot(df['Threads'], df['DB util'], marker='o', color='green')
plt.title("put_all: Disk Utilization (nvme0n1p5)")
plt.xlabel("Load Level")
plt.ylabel("DB Util (%)")
plt.grid(True)
plt.tight_layout()
# plt.show()
save_plot("put_all_db.png")