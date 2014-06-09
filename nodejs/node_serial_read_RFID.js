/*
CONFIGURATION :
- mongo vls_db Sites/VLS/mongoDB/vls_user.js ( pour mettre les donnes dans la db )
- RFIDSerialPortOut sur arduino
- node nodejs/node_serial_read_RFID.js  (pour lancer le script )
*/


// npm install serialport
// serial port node : https://github.com/voodootikigod/node-serialport


var serialport = require("serialport");
var SerialPort = serialport.SerialPort;

var serialPort = new SerialPort("/dev/tty.usbserial-AM01VB8R", {
  baudrate: 9600,
  parser: serialport.parsers.readline("\n")
});

var mongodb_vls_user; //the access to vls_user in mongodb

var mongodb = require('mongodb');
var server = new mongodb.Server("127.0.0.1", 27017, {});
new mongodb.Db('vls_db', server, {}).open(function (error, client) {
  if (error) throw error;
  mongodb_vls_user = new mongodb.Collection(client, 'vls_user');
});

/* qd on reçoit le rfid -> on recherche dans la db le code, puis envoie */
function sendUserCode(rfid) {
  var mongo_response;
  if (mongodb_vls_user) {
    mongo_response = mongodb_vls_user.find({ rfid: rfid});
    mongo_response.count(function(err, count) {
      console.log('Mongo return code:' + count);
      if(count == 1) {
        mongo_response.next(function(err, result) {
          console.log("User " + result.name + " takes a bike.");
          console.log("His code:" + result.code);
          serialPort.write(result.code + "\n", function(err, results) {
            if(err) console.log('err: ' + err);
            console.log('Code envoyé');
          });
        });
      }
      else {
        serialPort.write("no_user" + "\n", function(err, results) {
          console.log('err: ' + err);
          console.log('results: ' + results);
          console.log('Erreur envoyé');
        });
      }
    });
  }
}

var tx_buffer = ''; // buffer des donnes lues sur le serial port


/* lecture des donnes sur le serial port */
serialPort.on("open", function () {
  tx_buffer = ''
  console.log('open');
  serialPort.on('data', function(data) {
    var rfid  = '';
    var user_info = '';
    tx_buffer = tx_buffer + data;
    console.log("buffer:" + tx_buffer);
    if(tx_buffer.charAt(0) == '[' && tx_buffer.charAt(tx_buffer.length-2) == ']') {
      rfid = tx_buffer.substr(1,tx_buffer.length-3);
      console.log("RFID:" + rfid);
      sendUserCode(rfid);
    }
    tx_buffer = ''; //empty the buffer
});
});
