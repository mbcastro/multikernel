Nanvix Multikernel
===================

What is This Repository About?
------------------------------

Nanvix is a Unix like operating system created by Pedro Henrique Penna
in 2011. It has been initially designed to address educational
puporses, but it is now being used for research purposes as well.

In this repository you will find the experimental multikernel version
of Nanvix, which consists into a complete re-design of the operating
system to target emerging lightweight manycore platforms. Overall, in
in this new version, Nanvix is factored into several servers which
distributively run in the platform at user-level and provide usual
operating system abstractions and facilities to user applications.

Currently, the multikernel version of Nanvix features an architectural
port for the Kalray MPPA-256 processor and the following facilities:

* Exokernel: provides low-level platform-independent inter-process
communication primitives.

* Name Service: provides a naming linking and resolution scheme
that enables processes to communicate in the underlying platform
egardless their physical location.

* IPC Module: relies on the Name Service to provide to other
servers and also user applications high-level inter process
communication primitives.

* Semaphores Service: exports to user applications a
POSIX-compliant named semaphore abstraction.

* Shared Memory Service: exports to user applications a
POSIX-compliant named shared memory region abstraction.


License and Maintainers
------------------------

Nanvix is a free operating system that is distributed under the MIT
License. Although it was created by Pedro Henrique, it has been
implemented by himself and many others. If you are interested in
contacting any of the contributors, take a look on our official list
of contributors: https://github.com/nanvix/nanvix/blob/dev/CREDITS.
