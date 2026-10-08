import os
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

df = pd.read_csv("results/performance.csv")
os.makedirs("results/graphs", exist_ok=True)

def plot(ycol, ylabel, fname, title, ideal=False):
    plt.figure(figsize=(7, 4.5))
    for length, g in df.groupby("length"):
        plt.plot(g["threads"], g[ycol], marker="o", label=f"n={length}")
    if ideal:
        ts = sorted(df["threads"].unique())
        plt.plot(ts, ts, "k--", label="ideal")
    plt.xlabel("Threads"); plt.ylabel(ylabel); plt.title(title)
    plt.xticks(sorted(df["threads"].unique())); plt.grid(alpha=.3); plt.legend()
    plt.tight_layout(); plt.savefig(f"results/graphs/{fname}", dpi=150); plt.close()

plot("par_ms", "Time (ms)", "execution_time.png", "Parallel execution time")
plot("speedup", "Speedup", "speedup.png", "Speedup vs threads", ideal=True)
plot("efficiency", "Efficiency", "efficiency.png", "Efficiency vs threads")
print("Saved graphs to results/graphs/")
