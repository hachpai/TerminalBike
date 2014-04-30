/*
CONFIGURATION :
- mongo vls_db Sites/VLS/mongoDB/vls_user.js ( pour mettre les donnes dans la db )
- RFIDSerialPortOut sur arduino
- node nodejs/node_serial_read_RFID.js  (pour lancer le script )
*/


// npm install serialport
// serial port node : https://github.com/voodootikigod/node-serialport
var SerialPort = require("serialport").SerialPort
var serialPort = new SerialPort("/dev/tty.usbmodem411", {
   baudrate: 9600
});

var mongodb_vls_user; //the access to vls_user in mongodb

var mongodb = require('mongodb');
var server = new mongodb.Server("127.0.0.1", 27017, {});
new mongodb.Db('vls_db', server, {}).open(function (error, client) {
   if (error) throw error;
   mongodb_vls_user = new mongodb.Collection(client, 'vls_user');
});

/* qd on reçoit le rfid -> on recherche dans la db le code, puis affichage */
function get_user_information(rfid) {
   var mongo_response;
   if (mongodb_vls_user) {
      mongo_response = mongodb_vls_user.find({ rfid: rfid});
      mongo_response.count(function(err, count) {
         if(count == 1) {
            mongo_response.next(function(err, result) {
               console.log("CODE:" + result.code);
               serialPort.write(result.code + "\n", function(err, results) {
                  console.log('err: ' + err);
                  console.log('results: ' + results);
                  console.log('Code envoyé');
               });
            });
         }
      });
   }
}

var data_buffer = ''; // buffer des donnes lues sur le serial port

/* lecture des donnes sur le serial port */
serialPort.on("open", function () {
   data_buffer = ''
   console.log('open');
   serialPort.on('data', function(data) {
      var rfid  = '';
      var user_info = '';
      data_buffer = data_buffer + data;       
      if(data_buffer.charAt( data_buffer.length - 1) == '\n') {
         if(data_buffer.substr(0,5) != "Error") {
            rfid = (data_buffer.substr(0,(data_buffer.length - 2)));
            console.log("RFID:" + rfid);
            get_user_information(rfid);
         }
         data_buffer = '';
      }
   });
});