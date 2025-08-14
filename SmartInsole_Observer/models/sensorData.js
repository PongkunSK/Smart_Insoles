const mongoose = require('mongoose');

// Define schema for individual sensor data
const sensorDataSchema = new mongoose.Schema({
    userID: { type: String, required: true }, // User ID for identifying the data owner
    testID: { type: Number, required: true }, // Test ID for identifying the test type
    LFsensor: { type: Number, required: true }, // Single reading from the Left Front sensor
    LBsensor: { type: Number, required: true }, // Single reading from the Left Back sensor
    RFsensor: { type: Number, required: true }, // Single reading from the Right Front sensor
    RBsensor: { type: Number, required: true }, // Single reading from the Right Back sensor
    createdAt: { type: Date, default: Date.now } // Automatically set creation time
});

// Create the model using the schema
const SensorData = mongoose.model('SensorData', sensorDataSchema);

module.exports = SensorData;