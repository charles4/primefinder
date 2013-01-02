This prime finder app is based on C and Javascript (nodejs).

I am designing it to be run as a cluster, with a central server that coordinates various workers. The server is written in nodejs and the workers are written in C. 

The server needs to be able to provide the workers with a set of numbers to crunch when asked, and also needs to store any number sent to it (in a mysql db at the moment).

Currently, the program is under development (by me, who knew?). 

Anyways, right now you can compile the c code using 

gcc -Wall -Pthread threaded_primefinder.c

You can run the application with "./a.out somenumber"

You need nodejs installed to use the server.

Once its installed you can run it using the command "nodejs index.js"

The server uses nodejs's cluster module, which is not stable at the moment. So hopefully it will keep working.

As of right now, the client (the c code) can calculate primes and pass them to server. However, the server is not storing them and something is massively slowing down the code (it takes like 3 minutes to find the primes from 1-100).

So that's what I'll be fixing next.

Anyways, yeah. Thats about that.
