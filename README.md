This prime finder app is based on C and Javascript (nodejs).

I am designing it to be run as a cluster, with a central server that coordinates various workers. The server is written in nodejs and the workers are written in C. 

The server needs to be able to provide the workers with a set of numbers to crunch when asked, and also needs to store any number sent to it (in a mysql db at the moment).

Currently, the program is under development (by me, who knew?). 

Right now you can compile the c code using 

gcc -Wall -Pthread threaded_primefinder.c

You can run a worker simply with "./a.out"

You need nodejs installed to use the server.

Once its installed you can run it using the command "nodejs server.js min max"

So, first you launch the server. Then, you launch however many workers.

Biggest problem right now is the mysql db I'm using, which is much to slow at insertions to keep up with the cluster. I'm trying to decide between having the nodejs server keep an array in memory and slowly submit the numbers a couple at a time, vs moving to a faster database. Or maybe both. I guess we'll see.

Anyways, yeah. Thats about that.
