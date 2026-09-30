# Introduction to Demand Paging

## What is Demand Paging?

Demand paging is a **virtual memory management technique** where pages of a process are **loaded into memory only when they are needed**, rather than loading the entire process at once. This helps:

* Reduce memory usage.
* Improve process startup time.
* Handle large programs that cannot fit entirely in RAM.

Instead of allocating all pages at process start, the operating system **waits until a page is accessed** before loading it into physical memory.

---

## Key Concepts

1. **Page:** A fixed-size block of virtual memory. Typically 4 KB.
2. **Page Table:** Data structure used to map virtual pages to physical frames.
3. **Frame:** A fixed-size block of physical memory.
4. **Page Fault:** An exception triggered when a process accesses a page that is **not currently in memory**.

---

## How Demand Paging Works

1. A process tries to access a virtual address.
2. If the page is **not in memory**, the CPU raises a **page fault**.
3. The OS handles the page fault by:

   * Allocating a free **frame**.
   * Loading the page from **disk** (executable, swap, or file-backed memory).
   * Updating the **page table** to map the virtual page to the frame.
4. The instruction that caused the fault is restarted.
5. The process continues as if the page had always been in memory.

---

## Advantages

* **Memory Efficiency:** Only loads pages that are actually used.
* **Faster Start-up:** Processes can start without loading their entire memory.
* **Supports Large Processes:** Allows programs larger than physical memory to run using **swap space**.

---

## Things to Keep in Mind

* Page faults are **normal** in demand paging.
* Handling page faults correctly is crucial for program correctness.
* Pages can be **evicted** from memory if physical memory is full, which may involve writing them to swap space.