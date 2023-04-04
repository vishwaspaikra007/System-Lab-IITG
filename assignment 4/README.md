# Q2

Write a C/C++ program that simulates a simple online exam system using message
passing for inter-process communication. The program should prompt the user for the
number of students taking the exam and the number of questions on the exam. The
program should then create a child process for each student and use message queues to
communicate the exam questions to the child process.
1. Each child process should randomly answer the questions and send the answers
back to the parent process using another message queue.
2. The parent process should collect all the answers and grade each student's exam.
3. The program should display the grade for each student and the overall grade
distribution for the exam.
4. After grading all the exams, the parent process should wait for all the child
processes to finish using the message queue synchronization mechanism. If any
child process fails to respond or terminates unexpectedly, the program should
display a warning message and take appropriate action to handle the situation

# Q3

In this question, you write a multi-threaded Event-reservation system for Nehru Centre.
Suppose that you have e events. The auditorium has a capacity c. Queries made to the
reservation system are of three types:
1) Inquire the number of available seats in a events,
2) Book k tickets in a events, and
3) Cancel a booked ticket.

In order that the reservation system is not overloaded, there is a limit – call it MAX – on
the maximum number of active queries at any instant. Moreover, in order to insure
consistency of the database, different threads reading/modifying the reservation for the
same event must go through a mechanism of mutual exclusion. You are asked to use the
pthread API calls in order to implement a simulation of this reservation system.

