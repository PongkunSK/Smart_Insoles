const express = require('express');
const mongoose = require('mongoose');
const bodyParser = require('body-parser');
const path = require('path');
const User = require('./models/user');  // Import the User model
const session = require('express-session');
const ushistory = require('./models/userHistory');

const app = express();

// Serve static files (like CSS, images, etc.)
app.use(express.static(path.join(__dirname)));

// Middleware for sessions
app.use(session({
    secret: 'your-secret-key',   // Ensure this key is random and stored securely
    resave: false,
    saveUninitialized: false,
    cookie: { secure: false }  // set `secure: true` for HTTPS (in production)
}));

// Other middleware
app.use(express.json());

// MongoDB Connection
mongoose.connect('mongodb://localhost:27017/', {
}).then(() => console.log('Connected to MongoDB'))
    .catch(err => console.error('MongoDB connection failed:', err));

// Middleware to check if the user is logged in and is an admin
function isAdmin(req, res, next) {
    if (req.session.loggedIn && req.session.isAdmin) {
        return next();  // User is authenticated and is an admin
    } else {
        return res.redirect('/Login');  // Redirect to login if not authenticated or not an admin
    }
}

//landing page
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'page', 'Welcome.html'));
});

//register page
app.get("/Register", (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'page', 'Register.html'));
});

// Serve the Admin page (Min.html) for admins only
app.get('/Admin', isAdmin, (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'page', 'Min.html'));
});

// Serve the Login page
app.get("/Login", (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'page', 'Login.html'));
});

// Serve the graph page
app.get("/Graph", (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'page', 'graph.html'));
});

// Middleware to check if the user is logged in
function isAuthenticated(req, res, next) {
    if (req.session.loggedIn) {
        return next();  // User is authenticated, proceed to the requested page
    } else {
        return res.redirect('/Login');  // Redirect to login if not authenticated
    }
}

// Use the middleware on routes that require authentication
app.get('/Taker', isAuthenticated, (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'page', 'Taker.html'));
});

// Check authentication endpoint (for debugging or testing)
app.get('/check-auth', (req, res) => {
    if (req.session.loggedIn) {
        return res.status(200).json({ message: 'Authenticated' });
    }
    res.status(401).json({ message: 'Not authenticated' });
});

// Endpoint to fetch all users
app.get('/api/users', isAdmin, async (req, res) => {
    try {
        const users = await User.find({}, 'firstName lastName phoneNo rfid isAdmin'); // Select specific fields
        res.status(200).json(users);
    } catch (error) {
        console.error('Error fetching users:', error);
        res.status(500).json({ message: 'Failed to fetch users' });
    }
});

// Register route
app.post('/register', async (req, res) => {
    const { firstName, lastName, phoneNo, rfid, isAdmin } = req.body;
    console.log(req.body);  // Log the incoming data

    try {
        // Check if the user with the same RFID already exists
        const existingUser = await User.findOne({ rfid });
        if (existingUser) {
            return res.status(400).json({ message: 'RFID already registered' });
        }

        // Create a new user with the plain RFID (no hashing)
        const newUser = new User({ firstName, lastName, phoneNo, rfid, isAdmin });
        await newUser.save();

        res.status(201).json({ message: 'User registered successfully!' });
    } catch (error) {
        console.error('Error during registration:', error);  // Log the error for debugging
        res.status(500).json({ message: 'Error registering user', error: error.message });
    }
});

app.post('/login', async (req, res) => {
    const { rfid } = req.body;  // Expecting plain RFID in the request body

    try {
        // Find user by RFID (plain RFID entered by user)
        const user = await User.findOne({ rfid });

        if (!user) {
            return res.status(404).json({ message: 'User not found' });
        }

        // No bcrypt comparison; direct RFID match
        if (user.rfid !== rfid) {
            return res.status(400).json({ message: 'Invalid RFID' });
        }

        // Set session data after successful login
        req.session.loggedIn = true;
        req.session.userId = user._id;
        req.session.isAdmin = user.isAdmin; // Set isAdmin flag in the session

        // Respond with user details for sessionStorage on the frontend
        res.status(200).json({
            firstName: user.firstName,
            lastName: user.lastName,
            rfid: user.rfid, // Add RFID to the response
            isAdmin: user.isAdmin // Include isAdmin if needed
        });
    } catch (error) {
        console.error('Error during login:', error);
        res.status(500).json({ message: 'Error logging in: ' + error.message });
    }
});

// Logout route (POST request for logout)
app.post('/logout', (req, res) => {
    if (req.session) {
        req.session.destroy(err => {
            if (err) {
                console.error('Error destroying session:', err);
                return res.status(500).json({ message: 'Error logging out' });
            }
            res.status(200).json({ message: 'Logged out successfully' });
        });
    } else {
        res.status(400).json({ message: 'No session to log out' });
    }
});

// Import the TestResult model at the top of your server.js file
const TestResult = require('./models/test_result');

// Endpoint to receive test results
app.post('/api/test_results', async (req, res) => {
    const { userID, TUNGTresult, SBSTresult, STSTresult, TDSTresult, CSUTresult } = req.body;

    try {
        // Find the most recent test result (assuming it is ordered by createdAt or similar field)
        const latestTestResult = await TestResult.findOne().sort({ createdAt: -1 });

        if (!latestTestResult) {
            // If no test results exist, create a new test result
            const newTestResult = new TestResult({ userID, TUNGTresult, SBSTresult, STSTresult, TDSTresult, CSUTresult });
            await newTestResult.save();
            return res.status(201).json({ message: 'Test results saved successfully', newTestResult });
        }

        // If a test result exists, update it with the new values
        latestTestResult.TUNGTresult = TUNGTresult;
        latestTestResult.SBSTresult = SBSTresult;
        latestTestResult.STSTresult = STSTresult;
        latestTestResult.TDSTresult = TDSTresult;
        latestTestResult.CSUTresult = CSUTresult;

        await latestTestResult.save();

        res.status(200).json({ message: 'Test results updated successfully', updatedTestResult: latestTestResult });
    } catch (error) {
        console.error('Error saving/updating test results:', error);
        res.status(500).json({ message: 'Error saving/updating test results', error: error.message });
    }
});

// Fetch latest test result for the user
app.get('/api/test_results', isAuthenticated, async (req, res) => {
    try {
        const { userId } = req.query; // Get userId from query parameters

        if (!userId) return res.status(400).json({ message: 'User ID is required' });

        // Fetch the latest test result for the user
        const result = await TestResult.findOne({ userID: userId }).sort({ createdAt: -1 });

        if (!result) return res.status(404).json({ message: 'No test results found' });

        res.status(200).json(result);
    } catch (error) {
        console.error('Error fetching test results:', error);
        res.status(500).json({ message: 'Fetch failed', error: error.message });
    }
});

// Saved user result
app.post('/api/user_history/:rfid', async (req, res) => {
    const { rfid } = req.params; // Retrieve RFID from URL params
    const { TUNGTresult, SBSTresult, STSTresult, TDSTresult, CSUTresult } = req.body;

    try {
        // Find the user by RFID
        const user = await User.findOne({ rfid });

        if (!user) {
            return res.status(404).json({ message: 'User not found' });
        }

        // Create a new history record
        const newHistory = new ushistory({
            rfid,
            TUNGTresult,
            SBSTresult,
            STSTresult,
            TDSTresult,
            CSUTresult,
        });

        // Save the new history record
        const savedHistory = await newHistory.save();

        // Link the history to the user
        user.histories.push(savedHistory._id);
        await user.save();

        res.status(200).json({ message: 'Test results saved successfully', testResults: savedHistory });
    } catch (error) {
        console.error('Error saving test results:', error);
        res.status(500).json({ message: 'Error saving test results', error: error.message });
    }
});

// Fetch users history with test results TUNGTresult, SBSresult, STSresult, TDSresult, CSUTresult
app.get('/api/users/history/:rfid', isAuthenticated, async (req, res) => {
    const { rfid } = req.params;

    try {
        // Find all test results for the given RFID
        const userHistories = await ushistory.find(
            {
                rfid,
                TUNGTresult: { $ne: null },
                SBSTresult: { $ne: null },
                STSTresult: { $ne: null },
                TDSTresult: { $ne: null },
                CSUTresult: { $ne: null }
            },
            'rfid TUNGTresult SBSTresult STSTresult TDSTresult CSUTresult timestamp' // Exclude `_id`, include only the desired fields
        );

        if (!userHistories.length) {
            return res.status(404).json({ message: 'No test history found for this RFID' });
        }

        res.status(200).json(userHistories);
    } catch (error) {
        console.error('Error fetching user test history:', error);
        res.status(500).json({ message: 'Failed to fetch user history', error: error.message });
    }
});

const SensorData = require('./models/sensorData'); // Import the SensorData model

// POST Route for sensor data with multiple entries
app.post('/api/sensor_data', async (req, res) => {
    const { userId, data } = req.body;

    try {
        if (data && Array.isArray(data)) {
            // Loop through the received data and store each test's sensor readings
            for (let i = 0; i < data.length; i++) {
                const testEntry = data[i];

                if (
                    Array.isArray(testEntry.LFsensor) &&
                    Array.isArray(testEntry.LBsensor) &&
                    Array.isArray(testEntry.RFsensor) &&
                    Array.isArray(testEntry.RBsensor)
                ) {
                    // Iterate through the individual sensor readings and save them
                    const numReadings = Math.min(
                        testEntry.LFsensor.length,
                        testEntry.LBsensor.length,
                        testEntry.RFsensor.length,
                        testEntry.RBsensor.length
                    );

                    for (let j = 0; j < numReadings; j++) {
                        const sensorData = new SensorData({
                            userID: userId,
                            RFsensor: testEntry.RFsensor[j],
                            RBsensor: testEntry.RBsensor[j],
                            LFsensor: testEntry.LFsensor[j],
                            LBsensor: testEntry.LBsensor[j]
                        });

                        await sensorData.save();
                    }
                } else {
                    return res.status(400).send("Invalid sensor data format");
                }
            }

            res.status(200).send("Data saved successfully");
        } else {
            res.status(400).send("Invalid data format");
        }
    } catch (error) {
        console.error(error);
        res.status(500).send("Error saving data");
    }
});

// Endpoint to delete a user by RFID
app.delete('/api/users/:rfid', isAdmin, async (req, res) => {
    const rfid = req.params.rfid;  // Get the RFID from the URL parameter

    try {
        // Find and delete the user by RFID
        const deletedUser = await User.findOneAndDelete({ rfid });

        if (!deletedUser) {
            return res.status(404).json({ message: 'User not found' });
        }

        res.status(200).json({ message: 'User deleted successfully' });
    } catch (error) {
        console.error('Error deleting user:', error);
        res.status(500).json({ message: 'Error deleting user' });
    }
});

// GET Route to retrieve sensor data for a user
app.get('/api/sensor_data/:userId', async (req, res) => {
    const { userId } = req.params;

    try {
        const sensorData = await SensorData.find({ userId }).sort( 'createdAt' ).limit(200);
        res.status(200).json(sensorData);
    } catch (error) {
        console.error('Error fetching sensor data:', error);
        res.status(500).json({ error: 'Failed to fetch sensor data' });
    }
});

// const generateSmoothData = () => {
//     const userID = "insole1";
//     const baseValue = 500; // Base value of the sensor readings
//     const variationRate = 50; // Smooth rate of change
//     const numEntries = 200;
//     const interval = 10000; // 10 seconds between each data entry

//     let mockData = [];
//     let previousValues = {
//         RFsensor: baseValue,
//         RBsensor: baseValue,
//         LFsensor: baseValue,
//         LBsensor: baseValue,
//     };

//     for (let i = 0; i < numEntries; i++) {
//         const timestamp = new Date(Date.now() + (i * interval)); // Increment timestamp by 10 seconds

//         // Smoothly adjust the sensor values by a small drift
//         previousValues.RFsensor += (Math.random() * variationRate - variationRate / 2);
//         previousValues.RBsensor += (Math.random() * variationRate - variationRate / 2);
//         previousValues.LFsensor += (Math.random() * variationRate - variationRate / 2);
//         previousValues.LBsensor += (Math.random() * variationRate - variationRate / 2);

//         // Round values for stability
//         mockData.push({
//             userID,
//             RFsensor: parseFloat(previousValues.RFsensor.toFixed(2)),
//             RBsensor: parseFloat(previousValues.RBsensor.toFixed(2)),
//             LFsensor: parseFloat(previousValues.LFsensor.toFixed(2)),
//             LBsensor: parseFloat(previousValues.LBsensor.toFixed(2)),
//             timestamp,
//         });
//     }

//     return mockData;
// };

// // Generate smooth sensor data
// const smoothSensorData = generateSmoothData();
// console.log(smoothSensorData); // This data can be used for simulation or sending via an API

// const axios = require('axios');

// async function sendSmoothData(smoothData) {
//     try {
//         for (let data of smoothData) {
//             await axios.post('http://localhost:3000/api/sensor_data', data);
//             console.log(`Data sent for timestamp: ${data.timestamp}`);
//         }
//     } catch (error) {
//         console.error('Error sending data:', error);
//     }
// }

// // Send the generated smooth data to the backend
// sendSmoothData(smoothSensorData);


// Start the server
app.listen(3000, () => console.log('Server is running on http://localhost:3000'));