# OpenMP sessions from the Tim Mattson

Vid link: <https://www.youtube.com/watch?v=nE-xN4Bf8XI&list=PLLX-Q6B8xqZ8n8bwjGdzBJ25X2utwnoEG&index=1>

## Concurrency

A condition of a system in which multiple tasks are logically active at one time.

![Alt text](image.png)

Nothing happened at the same time, the CPU switching between tasks quickly. It looks like it is running at the same time.

## Parallelism(A subset of Concurrency)

A condition of a system in which multiple tasks are actually active at one time.

![Alt text](image-1.png)

So the relationship between Parallelism and Concurrency looks like this:

![Alt text](image-2.png)

## concurrent applications

An application for which computations logically execute simultaneously due to the semantics of the application.

## Parallel applications

An application for which computations actually execute simultaneously in order to complete a problem in less time.

## Brief Intro on OpenMP

This is an API for writing multithreaded application.

A set of compiler directives and library routines for parallel application programmers.

Works with Fortran, C, C++.

## Solution Stack

Programming layer

![Alt text](image-3.png)

## Syntax of Open MP

Most of the constructs in Open MP are compiler directives.

Most OpenMP constructs apply to a "Structured Block".

Structured Blocks:

A block of one or more statements with *one point of entry* at the top and *one point of exit* at the bottom.

The syntax is like

```c++

#include <omp.h> 


#pragma omp construct [clause[clause]...]

// example

#pragma omp parallel num_threads(4)

```

## Shared Memory Computer

Any computer composed of multiple processing elements that share an address space. There are two classes:

Symmetric multiprocessor(SMP)

A shared address space with "equal-time" access for each processor and the OS treats every processor the same way.

NUMA Non uniform address space multiprocessor:

Different memory regions have different accessing costs.

There is a difference between reaching far memory and near memory.

![Alt text](image-4.png)

The modern CPUs are not 100% SMP, and coders need to keep this in minds.

## Programming shared memory computers

![Alt text](image-5.png)

Each thread has its own private stack.

But all threads they are sharing the Text, Data, and Heap.

A block of threads are sharing a process, but communication between the threads are fairly cheap because of the shared context.

![Alt text](image-6.png)

## Fork Join Parallelism

![Alt text](image-7.png)

You can nest this pattern in the sub program as well

![Alt text](image-8.png)

## Open MP tells the compiler to generate a func do parallel with pthreads

Open MP actually hides the details and complexity of the pthread.

![Alt text](image-9.png)

## Cyclic distribution of the loop iteration

This is sometimes called round robin distribution.

It is like dealing out a deck of cards to the players.

The card is the task, and the players are the threads.

![Alt text](image-12.png)

**noticed** the i,id,nthrds, x they are declared inside the **#pragma omp parallel**, so each thread will have a copy of those on their stacks.

Outside the parallel block, those variables will be gone, how to keep the result from each thread?

You cannot simply do the accumulation in the parallel block because this will be a race.

What's the solution?

### How about promote the sum into an array, each element represents a sum of that specific thread.

![Alt text](image-14.png)

### The API might give you less threads than you asked for

That's why we check here
![Alt text](image-13.png)

### Results

The scalability sucks. Why is it like this?

![Alt text](image-15.png)

Introduce false sharing below.

## False sharing

![Alt text](image-16.png)

Every time, one thread write, the other thread cannot do anything.

![Alt text](image-17.png)

How to solve this?

Ugly way:

Made the result array into a 2D array. So, it is forcing the results are not on the same cache line.

![Alt text](image-18.png)

But, you have to figure out the size of the cache line adjust the PAD accordingly.
![Alt text](image-19.png)

Are there any more elegant and portable ways to do this?

## Sync

### In open MP, you only need to care about these two sync constructs

1. Barrier:

Each thread wait at the barrier until all threads arrive.

![Alt text](image-10.png)

Example code:

![Alt text](image-21.png)

make sure array A is ready before the calculation on Array B.

BTW, this looks like the perfect example for dependency.

2. Mutual Exclusion:

Define a block of code that only one thread at a time can execute.
(Think about SQL transactions)

![Alt text](image-11.png)

Example Code:

For this block of code, I am only letting one thread execute this line of code.

![Alt text](image-20.png)

Hint: Make sure the big_job is worth all the overhead. Critical overhead does matter for the performance.

There are more sync tools, but we will only focus on these two now.

### There are more in the future but so far this is enough for beginners
