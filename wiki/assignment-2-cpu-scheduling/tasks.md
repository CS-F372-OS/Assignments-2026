# Tasks

## Tasks

Welcome to your first kernel programming assignment! This document describes the tasks you need to complete for Assignment 2 - CPU Scheduling.

### Pre-requisites

Before starting this assignment, we recommend you to read the following documents:

* [Introduction to Threads](https://pkuflyingpig.gitbook.io/pintos/appendix/reference-guide/threads)
* [Syncronization using Locks](https://pkuflyingpig.gitbook.io/pintos/appendix/reference-guide/synchronization#locks) (Note that other synchronization primitives like Semaphores, Monitors, etc. are not required for this assignment).

{% hint style="success" %}
<mark style="color:green;">\*\*Important Note\*\*</mark>

<mark style="color:green;">**You DO NOT need the changes made in Assignment 1 (Shell Assignment) for this assignment. Hence, there is no need to carry over the changes made in that assignment.**</mark>
{% endhint %}

### Set-up

Before starting the assignment, make sure you have the latest version of the `main` branch of the repository. You can do this by running the following commands in your `Assignments-2025` directory:

```bash
git checkout main
git pull public main
```

Next, create a new branch for this assignment:

```bash
git checkout -b scheduler
```

Please ensure that you have named the branch `scheduler`. This is important for our evaluations and consistency across submissions.

Make sure that you are now on the `scheduler` branch by running:

```bash
git branch
```

If you see `* scheduler` in the output, you are on the correct branch. Now, you are ready to start working on the assignment.

### Task 1: Priority Scheduling in Pintos

{% hint style="success" %}
<mark style="color:green;">**Task 1**</mark>

<mark style="color:green;">**Implement**</mark> _<mark style="color:green;">**priority scheduling**</mark>_ <mark style="color:green;">**in Pintos.**</mark>

* <mark style="color:green;">When a thread is added to the ready list that has a higher priority than the currently running thread, the current thread should</mark> <mark style="color:green;">**immediately yield**</mark> <mark style="color:green;">the processor to the new thread.</mark>
* <mark style="color:green;">Similarly, when threads are waiting for a lock,</mark> <mark style="color:green;">**the highest priority waiting thread should be awakened first**</mark><mark style="color:green;">.</mark>
* <mark style="color:green;">A thread may raise or lower its own priority at any time, but</mark> <mark style="color:green;">**lowering its priority such that it no longer has the highest priority must cause it to immediately yield the CPU**</mark><mark style="color:green;">.</mark>
{% endhint %}

* **Thread priorities range from `PRI_MIN` (0) to `PRI_MAX` (63).** Lower numbers correspond to lower priorities, so that priority 0 is the lowest priority and priority 63 is the highest.
* **The initial thread priority is passed as an argument to `thread_create()`**. If there's no reason to choose another priority, use **`PRI_DEFAULT` (31)**.
* The `PRI_` macros are defined in `threads/thread.h`, and you should not change their values.

{% hint style="info" %}
**Hint**

For this task, **you need to consider all the scenarios** where the priority must be enforced.

* For example, **when an alarm clock for a thread fires off**, that thread should be made ready again, which entails a priority check.
* You can find some of these scenarios by **looking for places that modify `ready_list`** (directly and indirectly, rg can be helpful).
{% endhint %}

{% hint style="info" %}
**Hint**

* **To yield the CPU,** you can check the thread APIs in **`threads/thread.h`**.
  * Read the comment and implementation of the corresponding thread function in `threads/thread.c`.
  * **That function may not be used in interrupt context** (i.e., should not call it inside an interrupt handler).
* To yield the CPU _<mark style="color:red;">**in the interrupt context**</mark>_, you can take a look at functions in **`threads/interrupt.c`**.
{% endhint %}

### Task 2.1: Priority Donation

**One issue with priority scheduling is "priority inversion".**

* Consider high, medium, and low priority threads H, M, and L, respectively.
* If H needs to wait for L (for instance, for a lock held by L), and M is on the ready list, then H will never get the CPU because the low priority thread will not get any CPU time.
* **A partial fix for this problem is for H to "donate" its priority to L** while L is holding the lock, then **recall the donation** once L releases (and thus H acquires) the lock.

{% hint style="success" %}
<mark style="color:green;">**Task 2.1**</mark>

<mark style="color:green;">**Implement priority donation.**</mark>

* <mark style="color:green;">You will need to</mark> <mark style="color:green;">**account for all different situations in which priority donation is required**</mark><mark style="color:green;">.</mark>
* <mark style="color:green;">You must implement priority donation</mark> <mark style="color:green;">**for locks**</mark><mark style="color:green;">.</mark>
* <mark style="color:green;">You do need to</mark> <mark style="color:green;">**implement**</mark> _<mark style="color:green;">**priority scheduling**</mark>_ <mark style="color:green;">**in all cases**</mark><mark style="color:green;">.</mark>
* <mark style="color:green;">Be sure to</mark> <mark style="color:green;">**handle multiple donations**</mark><mark style="color:green;">, in which multiple priorities are donated to a single thread</mark>.
{% endhint %}

### Task 2.2: Nested Priority Donation

{% hint style="success" %}
<mark style="color:green;">**Task 2.2**</mark>

<mark style="color:green;">**Support nested priority donation:**</mark>

* <mark style="color:green;">if H is waiting on a lock that M holds and M is waiting on a lock that L holds, then both M and L should be boosted to H's priority.</mark>
* <mark style="color:green;">If necessary, you may impose</mark> <mark style="color:green;">**a reasonable limit**</mark> <mark style="color:green;">on depth of nested priority donation, such as 8 levels.</mark>
{% endhint %}

**Note:** if you support nested priority donation, you need to pass the `priority-donate-nest` and `priority-donate-chain` tests.

### Task 2.3: Thread Priorities

{% hint style="success" %}
<mark style="color:green;">**Exercise 2.3**</mark>

<mark style="color:green;">**Implement the following functions that allow a thread to examine and modify its own priority.**</mark>

<mark style="color:green;">Skeletons for these functions are provided in</mark> <mark style="color:green;">**threads/thread.c**</mark><mark style="color:green;">.</mark>
{% endhint %}

* <mark style="color:blue;">**Function: void thread\_set\_priority (int new\_priority)**</mark>
  * **Sets the current thread's priority to new\_priority.**
  * If the current thread no longer has the highest priority, **yields**.
* <mark style="color:blue;">**Function: int thread\_get\_priority (void)**</mark>
  * **Returns the current thread's priority.** In the presence of priority donation, returns the higher (donated) priority.

You need not provide any interface to allow a thread to directly modify other threads' priorities.

### Some Implementation Hints

*   You may find it useful to direct most of your code-writing efforts to the following files:

    * `threads/thread.c`
    * `threads/thread.h`
    * `threads/synch.c`
    * `threads/synch.h`
    * `threads/interrupt.c`

    Note that you may choose a different implementation strategy that requires you to not modify all of these files, or that requires you to modify additional files and/or create new files. As long as your implementation meets the project requirements and passes the test cases, you are free to choose your own design.

### Testing Your Implementation

You can test your implementation by running the following command in the `pintos/src/threads` directory:

```bash
make check
```

#### Credits

* This assignment was forked from [PKU's OS Course](https://github.com/CS-F372-OS/Assignments-2025/blob/docs/wiki/pkuflyingpig.gitbook.io/pintos/README.md)
