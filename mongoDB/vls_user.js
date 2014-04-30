// db : vls_db
// mongo vls_db mongoDB/vls_user.js 
db.vls_user.drop();
db.vls_user.insert({ name : "arnold", id: 111, rfid: "4f 00 42 1e eb", code: "0111"});
db.vls_user.insert({ name : "plop", id: 112, rfid: "35 01 fe 74 65", code: "1010"});