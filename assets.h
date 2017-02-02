
const char asset_date[] PROGMEM = __TIMESTAMP__ " GMT";

const char asset_html_root[] PROGMEM = R"=====(
<!DOCTYPE html>
<title>Gateway</title>
<meta name="viewport" content="width=device-width">
<link rel="stylesheet" href="style.css"/>

<h1>Gateway</h1>

<table class="table-borders">
  <tr>
    <th>Current time</th>
    <td>:time________: (:unixtime:)</td>
  </tr>
  <tr>
    <th>Free RAM</th>
    <td>:freeheap: bytes</td>
  </tr>
  <tr>
    <th>Temperature</th>
    <td>:*T:°C</td>
  </tr>
  <tr>
    <th>Humidity</th>
    <td>:*H:%</td>
  </tr>
</table>
)=====";

const char asset_html_login[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<title>Gateway</title>
<meta name="viewport" content="width=device-width">
<link rel="stylesheet" href="style.css"/>

<h1>Gateway</h1>

<form method="POST" action="./login" style="text-align: center;">
  <!-- Fake username field for Chrome -->
  <div style="display: none;"><input type="text" name="username"></div>
  <b>Password:</b>
  <input type="password" name="password">
  <input type="submit" value="Login">
</form>
)=====";

const char asset_css[] PROGMEM = R"=====(
html {
  margin: 0;
  padding: 0;
  font-family: Roboto, sans-serif;
}
body {
  max-width: 500px;
  margin: 0 auto;
  padding: 8px 0;
}

h1 {
  margin-top: 0;
  margin-bottom: 16px;
  text-align: center;
}

table {
  border-collapse: collapse;
  width: 100%;
}
table.table-borders > tbody > tr > th,
table.table-borders > tbody > tr > td {
  text-align: left;
  vertical-align: top;
  padding: 8px 16px;
}
table.table-borders > tbody > tr:not(:first-child) {
  border-top: #ddd 2px dotted;
}
)=====";
