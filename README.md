# ACS

Threading assignment from CSC360, Operating Systems.

Program simulates an Airline Check-in Service, where customers are represented as threads and they are served by "Clerks".

To Run: Compile with make, run ./ACS input0.txt

The program reads from a textfile, which contains information about the customers "arriving".

The input file is a text file and has a simple format. The first line contains the total number of customers that will
be simulated. After that, each line contains the information about a single customer, such that:
The first character specifies the unique ID of customers.
A colon(:) immediately follows the unique number of the customer.
Immediately following is an integer equal to either 1 (indicating the customer belongs to business class) or 0 (indicating the customer belongs to economy class).
A comma(,) immediately follows the previous number.
Immediately following is an integer that indicates the arrival time of the customer. A comma(,) immediately follows the previous number.
Immediately following is an integer that indicates the service time of the customer. A newline (\n) ends a line.
