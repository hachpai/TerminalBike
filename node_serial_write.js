// npm install serialport
// serial port node : https://github.com/voodootikigod/node-serialport

var SerialPort = require("serialport").SerialPort
var serialPort = new SerialPort("/dev/tty.usbmodem411", {
   baudrate: 9600
});

function serialPortWrite() {
   serialPort.write("Ola ola\n", function(err, results) {
      console.log('err: ' + err);
      console.log('results: ' + results);
      console.log(' ');
   });
};

serialPort.on("open", function () {
   console.log('open');
   setInterval(serialPortWrite,1000);
});