#ifndef WEBPAGES_H
#define WEBPAGES_H

const char WEBPAGE_MAIN[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>

    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Split Flap Config</title>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }
        
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            background-color: #f4f4f4;
        }

        .container {
            align-items: center;
            justify-content: center;
            display: flex;
            flex-direction: column;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0px 4px 10px rgba(0, 0, 0, 0.1);
            max-width: 400px;
            width: 90%;
            text-align: center;
        }

        h1 {
            font-size: 2em;
            margin-bottom: 30px;
        }

        .button {
            display: block;
            width: 80%;
            padding: 20px;
            margin: 15px 0;
            font-size: 1.5em;
            text-align: center;
            background-color: #007BFF;
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            transition: background-color 0.3s;
        }

        .button:hover {
            background-color: #0056b3;
        }

        .button:active {
            background-color: #004080;
        }

        .footer {
            font-size: 0.8em;
            color: #777;
            margin-top: 30px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Split Flap Config</h1>
        
        <button class="button" onclick="location.href='/mode'">Mode</button>
        <button class="button" onclick="location.href='/custom-text'">Custom Text</button>
        <button class="button" onclick="location.href='/settings'">Settings</button>

        <p class="footer">Morgan Manly 2025</p>
    </div>
</body>
</html>
)rawliteral";


const char WEBPAGE_SETTINGS[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Split Flap Config</title>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }
        
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            background-color: #f4f4f4;
        }

        .container {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0px 4px 10px rgba(0, 0, 0, 0.1);
            max-width: 400px;
            width: 90%;
            text-align: center;
        }

        h1 {
            font-size: 1.8em;
            margin-bottom: 20px;
        }

        label {
            display: block;
            margin-top: 15px;
            font-size: 1.1em;
            text-align: left;
        }

        input, select, button {
            width: 100%;
            padding: 12px;
            margin-top: 5px;
            font-size: 1em;
            border: 1px solid #ccc;
            border-radius: 5px;
        }

        button {
            background-color: #007BFF;
            color: white;
            border: none;
            cursor: pointer;
            margin-top: 15px;
        }

        button:disabled {
            background-color: #ccc;
            cursor: not-allowed;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Split Flap Settings</h1>

        <label>Update Wi-Fi SSID:</label>
        <input type="text" id="ssid" placeholder="Enter WiFi SSID">

        <label>Update Wi-Fi Password:</label>
        <input type="password" id="password" placeholder="Enter WiFi Password">

        <button id="saveBtn" onclick="sendData()">Save Settings</button>
    </div>
</body>
</html>
)rawliteral";


#endif // WEBPAGES_H