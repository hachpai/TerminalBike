// npm install serialport
// serial port node : https://github.com/voodootikigod/node-serialport

var SerialPort = require("serialport").SerialPort
var serialPort = new SerialPort("/dev/tty.usbmodem641", {
  baudrate: 9600
});

var data_buffer = '';

serialPort.on("open", function () {
   data_buffer = ''
   console.log('open');
   serialPort.on('data', function(data) {
      console.log(':' + data);
      data_buffer = data_buffer + data;       
      if(data_buffer.charAt( data_buffer.length - 1) == '\n') {
         console.log('data received: ' + data_buffer);
         data_buffer = '';
      }
   });
   /*
   serialPort.write("ls\n", function(err, results) {
      console.log('err ' + err);
      console.log('results ' + results);
   });
   */
});

/*
   serialPort = require("serialport").SerialPort;

   // Create new serialport pointer
   var serial = new serialPort("/dev/tty.usbmodem641" , { baudrate : 9600 });

   // Add data read event listener



      serial.on('data', function(data) {

         console.log('data received: ' + data);
      });



   //serial.write("klsklds");

   //repl.start( "=>" );

   /*
     serialPort.write("ls\n", function(err, results) {
    console.log('err ' + err);
    console.log('results ' + results);
  });
  */