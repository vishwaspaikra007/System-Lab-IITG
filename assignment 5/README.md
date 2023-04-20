# Question 1
To run this program
> g++ Q1.cpp -lpthread - q1\
> ./q1

Consider the following scenario. A town has a very popular restaurant. The restaurant can hold N diners. The number of people in the town who wish to eat at the restaurant, and are waiting outside its doors, is much larger than N. The restaurant runs its service in the following manner. Whenever it is ready for service, it opens its front door and waits for diners to come in. Once N diners enter, it closes its front door and proceeds to serve these diners. Once service finishes, the backdoor is opened and the diners are let out through the backdoor. Once all diners have exited, another batch of  N  diners  is  admitted  again  through  the  front  door.  This  process  continues  indefinitely.  The restaurant does not mind if the same diner is part of multiple batches.

Write  a  C/C++  program  to  model  the  diners  and  the  restaurant  as  threads  in  a  multithreaded program. The threads must be synchronized as follows.
* A diner cannot enter until the restaurant has opened its frontdoor to let people in.
* The restaurant cannot start service until N diners have come in
* The diners cannot exit until the back door is open.
* The restaurant cannot close the backdoor and prepare forthe next batch until all the diners of the previous batch have left.

# Question 2
To run this program
> g++ Q2.cpp -lpthread - q2\
> ./q2

Saraighat Bridge is the only bridge that connects the scenic north and south Guwahati and plays a vital role as it is one of the busiest bridges in the region. The bridge can become deadlocked if northbound and southbound people get on the bridge at the same time. (Let us assume the people are stubborn and are unable to back up.) Write a C /C++ code to implement the scenario of bridge using semaphores and/or mutex locks, that prevents deadlock.
* Case 1 : Consider the scenario in which northbound people prevent southbound people from using the bridge, or vice versa.
* Case  2  :  Consider  the  scenario  in  which  northbound  people  don't  prevent southbound people from using the bridge, or vice versa.

Represent northbound and southbound people as separate threads. Once a person is on the bridge, the  associated  thread  will  sleep for  a  random  period  of  time,  representing travelling  across  the bridge. Design your program so that you can create several threads representingthe northbound and southbound persons

# Question 3
To run this Q3.cpp run command "./q3.sh"\
And make sure "input.txt" is present
> ./q3.sh

or
> g++ Q3.cpp -lpthread -o q3\
> ./q3 < input.txt

Your  task  is  to  design a  process scheduling algorithm  that  can efficiently  allocate  resources  to worker  threadsto  process  incoming  requests.  The  algorithm  should  take  into  account  the following factors:
* The priority level of each worker thread
* The number of available resources assigned to each worker thread
* The type of transaction associated with each request

Your program should read the input from the standard input and write the output to the standard output. The input will contain the following information:
* The number of services nin the system
* The number of worker threads mfor each service
* The  priority  level  and  resources  assigned  to  each  worker  thread.Each  worker  thread should  be  on  a  separate  line  and  its  information  should  be  separated  by  spaces  in  the following format: priority_level resources
* The type of transaction associated with each request, and the number of resources required for that transaction. Each request should be on a separate line and its information should be separated by spaces in the following format: transaction_type resources_required.

Your program should output the following information:➢The order in which requests were processed
* The average waiting time for requests➢The average turnaround time for requests.
* The number of requests that were rejected due to lack of resources•Additionally.

your program should output the following information during periods of high traffic:
* The number of requests that were forced to wait due to lack of available resources.
* The  number  of  requests  that  were  blocked  due  to  the  absence  of  any  available  worker threads