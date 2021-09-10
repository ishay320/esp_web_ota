#include <stdint.h>

const char html_insert[] = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'>
    <meta http-equiv='X-UA-Compatible' content='IE=edge'>
    <title>Page Title</title>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <link rel='stylesheet' type='text/css' media='screen' href='main.css'>
    <script src='main.js'></script>
</head>
<body>
    <h2>Welcome to WIFI connection and OTA</h2>
<form action="/data" method="GET">
  <label for="ssid">WIFI SSID:</label>
  <input type="text" id="ssid" name="ssid"><br><br>
  <label for="password">WIFI password:</label>
  <input type="password" id="password" name="password" ><br><br>
  <input type="submit" value="Submit">
</form>
</body>
</html>
)=====";