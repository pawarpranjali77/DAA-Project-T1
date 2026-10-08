import os

import matplotlib.pyplot as plt
import pandas as pd

# Read performance results
file = "results/performance.csv"
data = pd.read_csv(file)
data.columns = (
    data.columns
    .str.strip()
    .str.lower()
    .str.replace(r"[^a-z0-9]+", "_", regex=True)
    .str.strip("_")
)
data = data.rename(
    columns={
        "sequential_time_ms": "seq_ms",
        "parallel_time_ms": "par_ms",
    }
)

required_columns = {"size", "threads", "seq_ms", "par_ms", "speedup", "efficiency"}
missing_columns = required_columns.difference(data.columns)
if missing_columns:
    raise ValueError(
        "Performance CSV is missing required columns: "
        + ", ".join(sorted(missing_columns))
    )

# Create graph folder
os.makedirs("results/graphs", exist_ok=True)


# 1. Size vs sequential time
plt.figure()
seq = data.groupby("size", as_index=False)["seq_ms"].mean()
plt.plot(seq["size"], seq["seq_ms"], marker="o", label="Sequential")
plt.xlabel("Sequence Size")
plt.ylabel("Execution Time (ms)")
plt.title("Size vs Sequential Time")
plt.grid(True)
plt.tight_layout()
plt.savefig("results/graphs/size_vs_seq_time.png", dpi=300, bbox_inches="tight")
plt.close()

# 2. Size vs parallel time
plt.figure()
for threads in sorted(data["threads"].unique()):
    subset = data[data["threads"] == threads].groupby("size", as_index=False)["par_ms"].mean()
    label = "Parallel (1 thread)" if threads == 1 else f"Parallel ({threads} threads)"
    plt.plot(subset["size"], subset["par_ms"], marker="o", label=label)
plt.xlabel("Sequence Size")
plt.ylabel("Execution Time (ms)")
plt.title("Size vs Parallel Time")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("results/graphs/size_vs_parallel_time.png", dpi=300, bbox_inches="tight")
plt.close()

# 3. Threads vs time (per size)
plt.figure()
for size in sorted(data["size"].unique()):
    subset = data[data["size"] == size]
    plt.plot(subset["threads"], subset["seq_ms"], marker="o", label=f"Seq @ {size}")
    plt.plot(subset["threads"], subset["par_ms"], marker="s", linestyle="--", label=f"Par @ {size}")
plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (ms)")
plt.title("Threads vs Time")
plt.legend(title="Dataset size")
plt.grid(True)
plt.tight_layout()
plt.savefig("results/graphs/threads_vs_time.png", dpi=300, bbox_inches="tight")
plt.close()

# 4. Threads vs speedup
plt.figure()
for size in sorted(data["size"].unique()):
    subset = data[data["size"] == size]
    plt.plot(subset["threads"], subset["speedup"], marker="o", label=str(size))
plt.xlabel("Number of Threads")
plt.ylabel("Speedup")
plt.title("Threads vs Speedup")
plt.legend(title="Sequence Size")
plt.grid(True)
plt.tight_layout()
plt.savefig("results/graphs/speedup.png", dpi=300, bbox_inches="tight")
plt.close()

# 5. Threads vs efficiency
plt.figure()
for size in sorted(data["size"].unique()):
    subset = data[data["size"] == size]
    plt.plot(subset["threads"], subset["efficiency"], marker="o", label=str(size))
plt.xlabel("Number of Threads")
plt.ylabel("Efficiency")
plt.title("Threads vs Efficiency")
plt.legend(title="Sequence Size")
plt.grid(True)
plt.tight_layout()
plt.savefig("results/graphs/efficiency.png", dpi=300, bbox_inches="tight")
plt.close()

# Backward-compatible existing names
# Keep the old graph names in case scripts expect them.
seq = data.groupby("size", as_index=False)["seq_ms"].mean()
plt.figure()
plt.plot(seq["size"], seq["seq_ms"], marker="o", label="Sequential")
for threads in sorted(data["threads"].unique()):
    parallel = data[data["threads"] == threads].groupby("size", as_index=False)["par_ms"].mean()
    label = "Parallel (1 thread)" if threads == 1 else f"Parallel ({threads} threads)"
    plt.plot(parallel["size"], parallel["par_ms"], marker="o", label=label)
plt.xlabel("Sequence Size")
plt.ylabel("Execution Time (ms)")
plt.title("Sequence Size vs Execution Time")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("results/graphs/execution_time.png", dpi=300, bbox_inches="tight")
plt.close()

print("All graphs generated successfully!")
print("\nGenerated files:")
for path in [
    "results/graphs/size_vs_seq_time.png",
    "results/graphs/size_vs_parallel_time.png",
    "results/graphs/threads_vs_time.png",
    "results/graphs/speedup.png",
    "results/graphs/efficiency.png",
    "results/graphs/execution_time.png",
]:
    print(path)