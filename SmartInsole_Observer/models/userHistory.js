const mongoose = require('mongoose');

const userHistorySchema = new mongoose.Schema({
  rfid: String,
  TUNGTresult: String,
  SBSTresult: String,
  STSTresult: String,
  TDSTresult: String,
  CSUTresult: String,
  timestamp: { type: Date, default: Date.now }
});

module.exports = mongoose.model('userhistory', userHistorySchema);