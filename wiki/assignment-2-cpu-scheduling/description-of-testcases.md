# Description of testcases

## Description of testcases

### Test Descriptions

1. **alarm-priority**: Combines the alarm clock with priorities. It checks if sleeping threads wake up in priority order, not just timer order.
2. **priority-preempt**: Tests the core of preemption. When a new thread with higher priority becomes ready, it should immediately preempt the currently running lower-priority thread.
3. **priority-change**: Tests whether a thread's priority can be lowered with `thread_set_priority()` and that this change correctly causes the thread to yield to a higher-priority one.
4. **priority-fifo**: Checks that when multiple threads have the same priority, they are scheduled in a "first-in, first-out" (FIFO) order.

#### Priority Donation with Locks

4. **priority-donate-one**: The simplest donation case. A medium-priority thread holds a lock that a high-priority thread needs. The medium thread's priority should be donated to match the high-priority one.
5. **priority-donate-multiple**: A thread holding a lock is waited on by multiple other threads. Its priority should be elevated to match the highest priority among all waiting threads.
6. **priority-donate-multiple2**: Similar to the above, but with a more complex interaction of three threads and two locks.
7. **priority-donate-lower**: Tests that a thread's priority correctly reverts to its previous value after it releases a lock that had caused a donation.
8. **priority-donate-nest**: This testcase demonstrates nested priority donation: a low-priority thread holding lock A receives a priority boost from a high-priority thread waiting for lock B, via a medium-priority thread blocked on both locks, ensuring the highest-priority thread's progress despite lock dependencies.
9. **priority-donate-chain**: This testcase demonstrates chained priority donation, where increasing thread priorities are passed along a sequence of lock holders all the way up to a low-priority main thread, ensuring higher-priority threads are not starved by nested lock dependencies, with interloper threads confirming correct scheduling order.

***

### Testcase Summary

#### Priority Scheduling (Task 1)

* `alarm-priority`
* `priority-change`
* `priority-fifo`
* `priority-preempt`

#### Priority Donation

* `priority-donate-one`
* `priority-donate-lower`
* `priority-donate-multiple`
* `priority-donate-multiple2`

#### Nested Priority Donation

* `priority-donate-nest`
* `priority-donate-chain`
