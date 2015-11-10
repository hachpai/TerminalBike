var net = require('net');
var fs = require('fs');
var socketPath = "\0terminal";

var rf_process = net.createConnection(socketPath);

rf_process.on("data", function(data) {
  console.log("data received!");
  rfid_user = "";
  for(i=0;i<6;i++){
    if(data[i]<16) rfid_user += 0;
    rfid_user += parseInt(data[i]).toString(16);
  }
  user_code= data[6];
  bike_id=data[7];
  console.log("RFID:"+rfid_user);
  console.log("User code:"+user_code);
  console.log("Bike id:"+bike_id);

  rf_process.write("OK"+bike_id);

});

rf_process.on("connect", function() {
  console.log("CONNECTED!");
});
/*
curl http://srv.bikino.be/loans/loan  --header "X-Requested-With:XMLHttpRequest" --data '{"rfid":"aabbccddee","bike_id":1,"user_code":"1111"}'

curl http://srv.bikino.be/loans/return  --header
"X-Requested-With:XMLHttpRequest" --data '{"bike_id":1}'
*/
