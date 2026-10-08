#!/bin/bash
cd "/Users/pranjali/Downloads/DAA_Project 2"
export GIT_EDITOR=cat

# Remove rebase state
rm -rf .git/rebase-merge .git/rebase-apply

# Add all changes
git add -A

# Create commit
git config user.name "pawarpranjali77"
git config user.email "pranjali@example.com"
git commit -m "Update to C++ only: fix README and run_all.sh for C++ implementation" 2>&1 || true

# Push
git push -u origin main 2>&1

echo "Done!"
