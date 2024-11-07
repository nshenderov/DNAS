# **Distributed Network Attached Storage (WIP)**

Provides software for creating a distributed storage network by unifying storage across IoT devices into a single system, implementing RAID 01 level redundancy. Developed in C++11.

The project consist of two parts: `concrete` and `framework`.

## Concrete part

This section defines how data is stored and accessed within the storage network. It centers around the Network Block Device (NBD) protocol.

Key modules include:
* nbd communicator
* nbd communicator proxy
* minion
* minion proxy
* minion manager
* raid manager
* async injection
* scheduler

## Framework part

This part contains generic modules that implement the Reactor design pattern, allowing extensibility through plugins and supporting concurrent task execution.

Key modules include:
* reactor
* dispatcher
* factory
* command
* handleton
* plugin manager
* pqueue
* waitable queue
* thread pool
* logger