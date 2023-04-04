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