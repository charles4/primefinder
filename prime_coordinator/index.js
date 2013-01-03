var cluster = require('cluster');
var net = require('net');
var numCPUs = require('os').cpus().length;
var mysql = require('mysql');

//i might need to keep globalcount in the mysql db if simlutaneous access becomes a problem
//or if there is actually a globalcount var for each thread. I'm not sure how fork works in nodejs exactly
var globalcount = 1;
var maxcount = 100001;


//check of this is the master thread or not
if (cluster.isMaster) {
  // Fork workers.
  for (var i = 0; i < numCPUs; i++) {
    cluster.fork();
    console.log("created worker " + i);
  }

  cluster.on('exit', function(worker, code, signal) {
    console.log('worker ' + worker.process.pid + ' died');
  });
} else {
    //step one, setup connection to mysql server. We only need 1 mysql connection per server/thread/worker/thing

  //setup a listening socket (or server)
  //the callback happens when something connects
  var server = net.createServer(function(c) {
	//c is the connection 
	console.log("the server established a connection with somewhere or something...");
	
	//setup a callback for when the connection ends
	c.on('end', function(){
		console.log("the server has disconnected from whatever that was");
	});

	//callback for when connection receives data
	c.on('data', function(data){
		//data is a buffered object, have to convert to string for easy use
		ds = data.toString();

		//insert a prime to the db
		var wrapped_insert = function(data){return function insertPrime(){
			var mysql_connection = mysql.createConnection({
				host:'charles.sytes.net',
				database: 'db_primes',
				user: '***',
				password: '***'
				//debug: true
    			});    
   		 	mysql_connection.connect();


			//the tostring func adds a \r\n to the end of the input, but mysql conveniently ignores them
			//because i've defined the column type as an int. So whatever.
			//escape the input
   			var myprime = mysql_connection.escape(data.split(" ")[1]);

			//the insert query
			mysql_connection.query("INSERT INTO `primes` (`prime`) VALUES (" + myprime + ")", function(err, results){
				if (err) throw err;
   			});
		
			mysql_connection.end();

		}}(ds);
		//different cases
		switch(ds.charAt(0)){
			case "n":
				returnNextNumSet();
				break;
			case "i":
				wrapped_insert(ds);
				break;
			default:
				c.write("invalid input\n");
		}

		//return the next set of numbers
		function returnNextNumSet(){
			
			if(globalcount < maxcount){
				c.write(globalcount + "-" + (globalcount+1000));
				globalcount += 1000;
			}else{
				c.write("stop");
			}
			
		}


	});
  }); 
  
  //set server to listen 
  //the workers are threads, so they share system resources and should be able to share a port
  server.listen(8000, function(){
     console.log("server has aquired port 8000");
  });

}
