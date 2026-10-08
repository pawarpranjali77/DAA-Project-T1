#!/usr/bin/env python3
import os
import shutil
import subprocess

os.chdir("/Users/pranjali/Downloads/DAA_Project 2")

# Remove rebase state
if os.path.exists(".git/rebase-merge"):
    shutil.rmtree(".git/rebase-merge")

# Stage changes
subprocess.run(["git", "add", "-A"], check=False)

# Configure git
subprocess.run(["git", "config", "user.name", "pawarpranjali77"], check=False)
subprocess.run(["git", "config", "user.email", "pranjali@example.com"], check=False)

# Commit
result = subprocess.run(
    ["git", "commit", "-m", "Update to C++ only: fix README and run_all.sh for C++ implementation"],
    capture_output=True,
    text=True
)
print("Commit:", result.returncode)
print(result.stdout)
print(result.stderr)

# Push
result = subprocess.run(
    ["git", "push", "-u", "origin", "main"],
    capture_output=True,
    text=True
)
print("\nPush:", result.returncode)
print(result.stdout)
print(result.stderr)
