
const char asset_date[] PROGMEM = __TIMESTAMP__ " GMT";

const char asset_html_root[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<title>Wakeup</title>
<meta name="viewport" content="width=device-width">
<link rel="stylesheet" href="style.css"/>

<h1>Wakeup</h1>

<table class="table-borders">
  <tr>
    <th>Wakeup</th>
    <td>
      <form method="POST" action="">
        <input type="submit" name="off" value="Off">
        <input type="submit" name="wake" value="Wake">
        <input type="submit" name="on" value="On">
      </form>
      <form method="POST" action="">
        <table>
          <tr>
            <td>State:</td>
            <td>:wakeup_state: – :wkp</td>
          </tr>
          <tr>
            <td>Start time:</td>
            <td>
              <span class="input-time">
                <input type="number" name="wakeup-hour" value=":H" min="0" max="23">:<input type="number" name="wakeup-minute" value=":M" min="0" max="59">
              </span>
            </td>
          </tr>
          <tr>
            <td>Duration:</td>
            <td>
              <input type="number" name="wakeup-duration" value=":D" min="0" max="60" style="width: 2em">
            </td>
          </tr>
        </table>
        <input type=submit value="Change"/>
      </form>
    </td>
  </tr>
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
<title>Wakeup</title>
<meta name="viewport" content="width=device-width">
<link rel="stylesheet" href="style.css"/>

<h1>Wakeup</h1>

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
.input-time {
  border: 1px solid gray;
  white-space: nowrap;
}
input[type=number] {
  border: 1px solid gray;
  background: inherit;
  font-size: inherit;
  padding: 2px;
}
.input-time input[type=number] {
  width: 2em;
  border: none;
  text-align: right;
  margin: 0;
}
)=====";
