// db : vls_db
// mongo vls_db mongoDB/vls_user.js 
db.vls_user.drop()
db.vls_user.insert({ name : "arnold", id: 111, rfid: 'ldldl'})
db.vls_user.insert({ name : "plop", id: 112, rfid: 'blopblop' })