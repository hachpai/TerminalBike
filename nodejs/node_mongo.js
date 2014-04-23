// npm install mongodb
var mongodb = require('mongodb');
var server = new mongodb.Server("127.0.0.1", 27017, {});
new mongodb.Db('vls_db', server, {}).open(function (error, client) {
   if (error) throw error;
   var vls_user = new mongodb.Collection(client, 'vls_user');
   vls_user.find().each(function(err, doc) {
      console.log("blop");
      console.log(doc);
   });
});