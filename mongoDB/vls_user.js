// db : vls_db
// mongo vls_db mongoDB/vls_user.js
db.vls_user.drop();
db.vls_user.insert({ name : "arnold", id: 111, rfid: "4f00421eeb", code: "1212"});
db.vls_user.insert({ name : "plop", id: 112, rfid: "3501fe7465", code: "2221"});
