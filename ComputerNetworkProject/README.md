# EE450
socket programming

a. Full name: jiangrun chen

b. Student ID: 6420194383
 
c. What you have done in the assignment. 
	In the socket programming project, I made simulations of transmiiting messages between client, aws and servers by TCP and UDP. The client connects with the aws server by TCP. The aws exchanged data between serverA and serverB by UDP. First I created and initialized the TCP socket and UDP socket which were needed. For file I/O work, I formulated the message format transmitting bewteen each server and client, and used different data structures to save the data. Then I utilized the TCP socket and UDP socket to tranmit messages. Finally I followed the requirement to output the data we needed to the screen.


d. Implementations
	
	1.Client
	* create TCP socket to connect the aws server
	* send map id, source node and file size to aws
	* receive the final result from aws by TCP
	* split the received data into some pieces by ","
	* print the final result. 

	2.AWS
	* create UDP and TCP sockets and bind address to it
	* listen to the socket and accept TCP connection from client
	* receive map id, source node and file size from client by TCP
	* send map id, source node to serverA by UDP
	* receive transmission speed, propgation speed number of nodes, destination node with its minimum length and each small length from server A by UDP
	* print the information from A and send the message from A to B added with file size by UDP
	* receive the calculation result(3 delays) from serverB by UDP
	* print the information from B 
	* send the message from B back to client using TCP
	
	3.ServerA
	* create and bind address to the UDP socket	
	* read "map.txt" and store the map information
	* receive map id and source node from aws by UDP and match the required map
	* using Dijkstra algorithm to calculate the min length with diffrent destinations
	* formulate the message format and send the message to aws by UDP
    
	4.ServerB
	* create and bind address to the UDP socket
	* receive messages from aws by UDP
      * calculate the 3 delays using the minimum length and speed in the message and print it out on screen
      * formulate the message format and send the message to aws by UDP

e. messages formats
(1) client->aws : map id, source node, file size
(2) aws->A : map id, source node
(3) A->aws : propgation speed, transmission speed, number of vertices; destination[0], min_length[0], destination[1], min_length[1]···; small_length, small length···; small_length, small length···;
(4) aws->B : the message from A plus the file in the end;
(5) B->aws : number of vertices; destination[0], Ttrans[0], Tprop[0], Tend-to-end[0], min_length[0]; destination[1], Ttrans[1], Tprop[1], Tend-to-end[1], min_length[1];···
(6) aws->client : number of vertices; destination[0], Ttrans[0], Tprop[0], Tend-to-end[0], min_length[0]; destination[1], Ttrans[1], Tprop[1], Tend-to-end[1], min_length[1];···

Reused code:
(1) reference socket programming from https://www.geeksforgeeks.org/socket-programming-cc/
(2) reference socket programming from https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
(3) reference socket programming from http://www.linuxhowtos.org/C_C++/socket.htm
