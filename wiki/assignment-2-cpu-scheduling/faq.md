# FAQ

## General FAQ

<details>

<summary><strong>How do I update the Makefiles when I add a new source file?</strong></summary>

To add a .c file, edit the top-level Makefile.build. Add the new file to variable dir\_SRC, where dir is the directory where you added the file. For this project, that means you should add it to `threads_SRC` or `devices_SRC`. Then run `make`. If your new file doesn't get compiled, run `make clean` and then try again.

When you modify the top-level Makefile.build and re-run `make`, the modified version should be automatically copied to `threads/build/Makefile`. The converse is not true, so any changes will be lost the next time you run `make clean` from the threads directory. Unless your changes are truly temporary, you should prefer to edit Makefile.build.

A new .h file does not require editing the Makefiles.

</details>

<details>

<summary><strong>What does <code>warning: no previous prototype for `func'</code> mean?</strong></summary>

It means that you defined a `non-static` function without preceding it with a prototype. Because `non-static` functions are intended for use by other .c files, for safety they should be prototyped in a header file included before their definition. To fix the problem, add a prototype in a header file that you include, or, if the function isn't actually used by other .c files, make it `static`.

</details>

<details>

<summary><strong>What is the interval between timer interrupts?</strong></summary>

Timer interrupts occur `TIMER_FREQ` times per second. You can adjust this value by editing `devices/timer.h`. The default is 100 Hz.

We don't recommend changing this value, because any changes are likely to cause many of the tests to fail.

</details>

<details>

<summary><strong>How long is a time slice?</strong></summary>

There are `TIME_SLICE` ticks per time slice. This macro is declared in `threads/thread.c`. The default is 4 ticks.

We don't recommend changing this value, because any changes are likely to cause many of the tests to fail.

</details>

<details>

<summary><strong>How do I run the tests?</strong></summary>

See section [Testing](https://cs-f372.gitbook.io/cs-f372-docs/appendix/testing) in the [Appendix](https://cs-f372.gitbook.io/cs-f372-docs/appendix) section of this page.

</details>

<details>

<summary><strong>How do interrupts get re-enabled in the new thread following <code>schedule()</code>?</strong></summary>

Every path into `schedule()` disables interrupts. They eventually get re-enabled by the next thread to be scheduled. Consider the possibilities: the new thread is running in `switch_thread()` (but see below), which is called by `schedule()`, which is called by one of a few possible functions:

* `thread_exit()`, but we'll never switch back into such a thread, so it's uninteresting.
* `thread_yield()`, which immediately restores the interrupt level upon return from `schedule()`.
* `thread_block()`, which is called from multiple places:
  * `sema_down()`, which restores the interrupt level before returning.
  * `idle()`, which enables interrupts with an explicit assembly STI instruction.
  * `wait()` in `devices/intq.c`, whose callers are responsible for re-enabling interrupts.

There is a special case when a newly created thread runs for the first time. Such a thread calls `intr_enable()` as the first action in `kernel_thread()`, which is at the bottom of the call stack for every kernel thread but the first.

</details>

## Priority Scheduling FAQ

<details>

<summary><strong>Doesn't priority scheduling lead to starvation?</strong></summary>

Yes, strict priority scheduling can lead to starvation because a thread will not run if any higher-priority thread is runnable.

Strict priority scheduling is valuable in real-time systems because it offers the programmer more control over which jobs get processing time. High priorities are generally reserved for time-critical tasks. It's not "fair," but it addresses other concerns not applicable to a general-purpose operating system.

</details>

<details>

<summary><strong>What thread should run after a lock has been released?</strong></summary>

When a lock is released, the highest priority thread waiting for that lock should be unblocked and put on the list of ready threads. The scheduler should then run the highest priority thread on the ready list.

</details>

<details>

<summary><strong>If the highest-priority thread yields, does it continue running?</strong></summary>

Yes. If there is a single highest-priority thread, it continues running until it blocks or finishes, even if it calls `thread_yield()`. If multiple threads have the same highest priority, `thread_yield()` should switch among them in "round robin" order.

</details>

<details>

<summary><strong>What happens to the priority of a donating thread?</strong></summary>

Priority donation only changes the priority of the donee thread. The donor thread's priority is unchanged. Priority donation is not additive: if thread A (with priority 5) donates to thread B (with priority 3), then B's new priority is 5, not 8.

</details>

<details>

<summary><strong>Can a thread's priority change while it is on the ready queue?</strong></summary>

Yes. Consider a ready, low-priority thread L that holds a lock. High-priority thread H attempts to acquire the lock and blocks, thereby donating its priority to ready thread L.

</details>

<details>

<summary><strong>Can a thread's priority change while it is blocked?</strong></summary>

Yes. While a thread that has acquired lock L is blocked for any reason, its priority can increase by priority donation if a higher-priority thread attempts to acquire L. This case is checked by the `priority-donate-sema` test.

</details>

<details>

<summary><strong>Can a thread added to the ready list preempt the processor?</strong></summary>

Yes. If a thread added to the ready list has higher priority than the running thread, the correct behavior is to immediately yield the processor. It is not acceptable to wait for the next timer interrupt. The highest priority thread should run as soon as it is runnable, preempting whatever thread is currently running.

</details>

<details>

<summary><strong>How does <code>thread_set_priority()</code> affect a thread receiving donations?</strong></summary>

It sets the thread's base priority. The thread's effective priority becomes the higher of the newly set priority or the highest donated priority. When the donations are released, the thread's priority becomes the one set through the function call. This behavior is checked by the `priority-donate-lower` test.

</details>

<details>

<summary><strong>Doubled test names in output make them fail.</strong></summary>

Suppose you are seeing output in which some test names are doubled, like this:

```
(alarm-priority) begin 
(alarm-priority) (alarm-priority) Thread priority 30 woke up. 
Thread priority 29 woke up. 
(alarm-priority) Thread priority 28 woke up.
```

What is happening is that output from two threads is being interleaved. That is, one thread is printing `"(alarm-priority) Thread priority 29 woke up.\n"` and another thread is printing `"(alarm-priority) Thread priority 30 woke up.\n"`, but the first thread is being preempted by the second in the middle of its output.

This problem indicates a bug in your priority scheduler. After all, a thread with priority 29 should not be able to run while a thread with priority 30 has work to do.

Normally, the implementation of the `printf()` function in the Pintos kernel attempts to prevent such interleaved output by acquiring a console lock during the duration of the `printf` call and releasing it afterwards. However, the output of the test name, e.g., `(alarm-priority)`, and the message following it is output using two calls to `printf`, resulting in the console lock being acquired and released twice.

</details>
