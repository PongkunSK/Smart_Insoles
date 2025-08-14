// models/testResult.js
const mongoose = require('mongoose');

const testResultSchema = new mongoose.Schema({
  userID: String,
  TUNGTresult: String,
  SBSTresult: String,
  STSTresult: String,
  TDSTresult: String,
  CSUTresult: String,
  timestamp: { type: Date, default: Date.now }
});

module.exports = mongoose.model('testresults', testResultSchema);
