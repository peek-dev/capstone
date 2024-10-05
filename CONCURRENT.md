The Go language documentation wisely states:
>Do not communicate by sharing memory; instead, share memory by communicating.

In other words, use synchronized queues to send messages between threads, not complex systems of locks and shared memory.

This is an embedded project with 32K of memory, so we may not be able to achieve this goal, but we will aspire to it. It prevents all manner of concurrency bugs and timing uncertainties. To that end, we define the following conventions:
 - Each thread will have a private `static` message queue associated with it. Public API functions will do little more than call `xQueueSend()`.
 - The receiving thread will loop, and read messages one at a time. This effectively serializes concurrent operations.
 - The thread's internal private state may only be accessed by private functions, one initializer function, and one thread function. To follow FreeRTOS conventions, private functions will be `static` and prefixed with `prv`.

If we follow these conventions, concurrency bugs should be a thing of the past. Private state (and peripherals) will be "owned" by a single thread, and no other thread can access it.
