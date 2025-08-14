const mongoose = require('mongoose');

const sensorDataSchema = new mongoose.Schema({
    userID: String,
    RFsensor: Number,
    RBsensor: Number,
    LFsensor: Number,
    LBsensor: Number,
    createdAt: { type: Date, default: Date.now }
});

const SensorData = mongoose.model('SensorData', sensorDataSchema);

module.exports = SensorData;
