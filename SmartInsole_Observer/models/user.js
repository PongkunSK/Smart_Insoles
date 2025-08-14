const mongoose = require('mongoose');

const userSchema = new mongoose.Schema({
    firstName: String,
    lastName: String,
    phoneNo: String,
    rfid: String,
    isAdmin: Boolean,
    histories: [{ type: mongoose.Schema.Types.ObjectId, ref: 'userhistory' }] // Reference to history
});

module.exports = mongoose.model('User', userSchema);
