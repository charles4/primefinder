var net = require('net');
var redis = require("redis");

//redis.debug_mode = true;

var globalcount = 0;
var maxcount = 0;
var gc_increment = 1000;

if(process.argv.length != 4){
	console.log("usage: nodejs index.js start_count max_count");
	process.exit(0);
}else{
	globalcount = (process.argv[2])*1;
	maxcount = (process.argv[3])*1;
}


  //setup a listening socket (or server)
  //the callback happens when something connects
  var server = net.createServer(function(c) {
	//c is the connection 
	
	//setup a callback for when a connection ends
	c.on('end', function(){
		//nothing right now
	});

	//callback for when connection receives data
	c.on('data', function(data){
		//data is a buffered object, have to convert to string for easy use
		ds = data.toString();
		
		console.log("ds = " + ds);
	
		//insert a prime to the db
		var wrapped_insert = function(data){return function insertPrime(){
			console.log("received: " + data);			

			//defaults to localhost unless otherwise specified				
			rediscli = redis.createClient();
		
			rediscli.on("error", function(err){
				//if the redis server throws back an error
				console.log("Connection error: '" + err  + "'");
			});
			
			rediscli.sadd("primes", "" + (ds.split(" ")[1]));

			rediscli.quit();

		}}(ds);

		//return the next set of numbers
		function returnNextNumSet(){
			
			if(globalcount < maxcount){
				c.write("" + globalcount + "-" + (globalcount+gc_increment));
				globalcount = (globalcount*1) + gc_increment;
				c.end();
			}else{
				c.write("stop");
				c.end();
			}
			
		}


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


	});
  }); 
  
  //set server to listen 
  server.listen(8000, function(){
     console.log("server has aquired port 8000");
  });


