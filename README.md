# Chord-DHT
Implementation of Chord P2P Distributed Hash Table

More Information about Chord Protocol can be found here - https://en.wikipedia.org/wiki/Chord_(peer-to-peer)

Typing "help" will show all supported commands


## Implementation ##
Each node will be assigned a unique ID (within 2^m (m is 48 in this case)) by hashing key which will be "ip:port" of that node by SHA-1 Algorithm

A node can create a DHT ring, other nodes can join the ring by contacting this node using it's ip and port number

When a node creates or joins a ring, it spawns multiple threads which are responsible for tasks like listening to other nodes to join the
ring, sending acknowledgement and other operations , doing stabilization tasks to ensure correct finger table entries, successor 
and predecessor pointers and a Successor list.

Each node contains m(48) entries in it's finger table.

Each node also maintains a successor list having r(10) entries. This list finds it's use when a node's  successor leaves the ring,
the node immediately assigns the next entry in the successor list as it's successor.

Each node regularly asks for acknowledgement from it's successor and predecessor to know that they are still present in the ring. If
no acknowledgement is received then they have left the ring and stabilization is done accordingly

Each node keeps a map for storing key value pairs in it. When a key is entered, it is also assigned a unique ID, then it is stored in 
a node whose ID is just greater than this key's ID

When a node newly joins the ring, it gets all those keys from it's successor which should now belong to it

When a node leaves the ring, it transfers all it's keys to it's successor

m has been set to 48 in this implementation that means a maximum of 2^48 nodes can join the ring

## Files ##

main.cpp is the main file

All important functions are inside the file functions.cpp

A class named NodeInformation is made which contains all the information of a Node and all the functions which every node performs
to maintain correct information about the ring. 

Another class SocketAndPort is made which contains
all the information about socket ,ip address and port number. Object of class SocketAndPort is inside NodeInformation class

Another class HelperClass is made which contains all helper functions which are required by a node to perform various tasks


## Supported Commands ##

typing "help" in the terminal shows all supported commands

__create__ - will create a DHT ring

__join "ip" "port"__ - will connect to the main node running at IP address <ip> and Port Number <port>

__printstate__ - will print successor, predecessor, fingerTable and Successor list of that node

__print__ - will print all keys and values present in that node

__port__ - will display port number on which node is listening

__port "number"__ - will change port number to mentioned number if that port is free (will only run before the node has joined the ring)

__put "key" "value"__ - will put key and value to the node it belongs to

__get "key"__ - will get value of mentioned key


## Execution ##

A Makefile has also been included. Just type make to build the whole project
