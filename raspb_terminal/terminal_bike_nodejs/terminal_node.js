var net = require('net');
var fs = require('fs');
var request = require('request');
var socketPath = "\0terminal";

var rf_process = net.createConnection(socketPath);

url_withdraw="http://srv.bikino.be/loans/loan"
url_return="http://srv.bikino.be/loans/return"

var options_server_request = {
  url: "",
  method: "POST",
  headers: {
    'X-Requested-With':'XMLHttpRequest'
  },
  json: {}
};

//options.url = "http://srv.bikino.be/loans/return";
//options.json = {"bike_id":'1'};
function callback(error, response, body) {
  console.log(body) // Show the HTML for the Google homepage.

  if(!error && response.statusCode == 200) {
    if(body.response_code=='1'){
      console.log("Sending OK");
      rf_process.write("OK");
    }
    else{
      console.log("Sending KO");
      rf_process.write("KO");
    }
  }
  else{
    console.log("Sending KO");
    rf_process.write("KO");
  }
}

rf_process.on("data", function(data) {
  console.log("data received!");
  //if the the byte is 0 then it's a return and the bike id is in the first byte
  if(data[7]!=0){
    rfid_user = "";
    for(i=0;i<6;i++){
      if(data[i]<16) rfid_user += 0;
      rfid_user += parseInt(data[i]).toString(16);
    }
    user_code= data[6];
    bike_id=data[7];
    console.log("Withdraw, bike id:"+bike_id);
    console.log("RFID:"+rfid_user);
    console.log("User code:"+user_code);
    options_server_request.url = url_withdraw;
    options_server_request.json={};
    options_server_request.json.rfid=rfid_user;
    options_server_request.json.bike_id=bike_id.toString();
    options_server_request.json.user_code=user_code.toString();
  }else{
    options_server_request.url = url_return;
    // we empty json to avoid key error
    options_server_request.json={};
    bike_id=data[0];
    console.log("return, Bike id:"+bike_id);
    options_server_request.json.bike_id=bike_id;
  }
  console.log("options request:");
  console.log(options_server_request);
  request(options_server_request, callback);
});

rf_process.on("connect", function() {
  console.log("CONNECTED!");
});
/*
curl http://srv.bikino.be/loans/loan  --header "X-Requested-With:XMLHttpRequest" --data '{"rfid":"aabbccddee","bike_id":1,"user_code":"1111"}'

curl http://srv.bikino.be/loans/return  --header
"X-Requested-With:XMLHttpRequest" --data '{"bike_id":1}'
*/
