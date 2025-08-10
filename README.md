# slab-watcher

A lightweight Linux user-space tool that monitors **slab cache growth** in `/proc/slabinfo` and displays the top growing caches in real-time.  
This can help detect potential **kernel memory leaks** or abnormal memory allocation patterns.

---

## Features
- Reads `/proc/slabinfo` and parses slab cache statistics.
- Calculates growth rate of active objects over a time interval.
- Displays top N growing slab caches, sorted by growth rate.
- Alerts when active objects exceed a threshold.
- Adjustable monitoring interval, number of top entries, and threshold via CLI options.

---

## Requirements
- Linux system with `/proc/slabinfo` available.
- GCC (or any C compiler).
- `ncurses` library (for terminal output handling).

Install `ncurses` if not present:
```bash
sudo apt install libncurses5-dev libncursesw5-dev
