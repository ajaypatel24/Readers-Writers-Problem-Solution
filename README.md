# Implementation of a solution to the readers-writers problem 
`The readers-writers problem relates to an object such as a file that is shared between multiple processes. Some of these processes are readers i.e. they only want to read the data from the object and some of the processes are writers i.e. they want to write into the object.`
`The readers-writers problem is used to manage synchronization so that there are no problems with the object data. For example - If two readers access the object at the same time there is no problem. However if two writers or a reader and writer access the object at the same time, there may be problems.`

* Common data needs to be read and edited, editors must have exclusive access while many reader can have read access at the same time
* Implemented using pthreads and POSIX semaphores

* 10 writer threads and 500 reader threads used to test
* The code ensures there is not thread starvation for readers or writers
