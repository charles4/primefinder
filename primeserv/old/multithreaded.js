var cluster = require('cluster');
var net = require('net');
var numCPUs = require('os').cpus().length;
var mysql = require('mysql');

//i might need to keep globalcount in the mysql db if simlutaneous access becomes a problem
//or if there is actually a globalcount var for each thread. I'm not sure how fork works in nodejs exactly
var globalcount = 0;
var maxcount = 0;

if(process.argv.length != 4){
	console.log("usage: nodejs index.js start_count max_count");
	process.exit(0);
}else{
	globalcount = process.argv[2];
	maxcount = process.argv[3];
}


//check if this is the master thread or not
if (cluster.isMaster) {
	
	// Fork workers.
	for (var i = 0; i < numCPUs; i++) {
    		var worker = cluster.fork();
   		console.log("created worker " + i);
 	}

 	 cluster.on('exit', function(worker, code, signal) {
		 console.log('worker ' + worker.process.pid + ' died');
 	});



} else {

  //setup a listening socket (or server)
  //the callback happens when something connects
  var server = net.createServer(function(c) {
	//c is the connection 
	
	//setup a callback for when the connection ends
	c.on('end', function(){
		//does nothing right now
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
				user: 'charles',
				password: '6andromeda9'
				//debug: true
    			}); 

			function handleConnectionError(connection){
				connection.on("error", function(err){
					//if the mysql server has too many connections open
					if(err.code == "ER_CON_COUNT_ERROR"){
						connection.end();	
						console.log("Connection error: retrying in 5");
						//have to recreate connection after error
						connection = mysql.createConnection({host:'charles.sytes.net', 
											database:'db_primes', 
											user:'charles', 
											password:'6andromeda9'});
						//wait 5 seconds and try to connect again
						setTimeout(handleConnectionError(connection), 5000);
					}else{
						return;
					}
				});
				
				connection.connect();
				
			}
   
			handleConnectionError(mysql_connection);

			//escape the input
   			var myprime = mysql_connection.escape(data.split(" ")[1]);

			//the insert query
			mysql_connection.query("INSERT INTO `primes` (`prime`) VALUES (" + myprime + ")", function(err, results){
				if(err) throw err;
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
