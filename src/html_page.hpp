#pragma once
const char HTML_PAGE[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 WiFi Setup</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; padding: 20px; background-color: #f4f4f4; }
        .container { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); max-width: 400px; margin: auto; }
        h2 { color: #333; }
        input, select { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; font-size: 16px; }
        input[type="submit"] { background-color: #28a745; color: white; border: none; cursor: pointer; }
        input[type="submit"]:hover { background-color: #218838; }
    </style>
</head>
<body>
    <div class="container">
        <h2>WiFi Configuration</h2>
        <form action="/save" method="POST">
            <label for="ssid">SSID:</label>
            <input type="text" id="ssid" name="ssid" required>

            <label for="password">Password:</label>
            <input type="password" id="password" name="password" required>

            <input type="submit" value="Save and Connect">
        </form>
    </div>

    <div class="container" style="margin-top: 20px;">
        <h2>Set Manual Time</h2>
        <form action="/set_manual_time" method="POST">
            <label for="hour">Hour:</label>
            <select id="hour" name="hour">
                <script>
                    for (let i = 1; i <= 23; i++) { document.write(`<option value="${i}">${i}</option>`); }
                </script>
            </select>

            <label for="minute">Minute:</label>
            <select id="minute" name="minute">
                <script>
                    for (let i = 0; i < 60; i++) { document.write(`<option value="${i}">${i}</option>`); }
                </script>
            </select>

            <input type="submit" value="Set Time">
        </form>
    </div>
</body>
</html>
)rawliteral";

