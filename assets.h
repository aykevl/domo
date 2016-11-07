
const char asset_date[] PROGMEM = __TIMESTAMP__ " GMT";

const char asset_html_root[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<title>Wakeup</title>
<meta name="viewport" content="width=device-width">
<link rel="stylesheet" href="style.css"/>

<h1>Wakeup</h1>

<table class="table-borders">
  <!--<tr>
    <th>D2:</th>
    <td><form method=POST action="">:led1: <input type=submit name=led1 value=Toggle></form></td>
  </tr>
  <tr>
    <th>D3:</th>
    <td><form method=POST action="">:led2: <input type=submit name=led2 value=Toggle></form></td>
  </tr>-->
  <tr>
    <th>Wakeup:</th>
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
    <th>Current time:</th>
    <td>:time________: (:unixtime:)</td>
  </tr>
  <tr>
    <th>Free RAM:</th>
    <td>:freeheap: bytes</td>
  </tr>
</table>
)=====";

const char asset_html_login[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<title>Wakeup</title>
<meta name="viewport" content="width=device-width">
<link rel="stylesheet" href="style.css"/>

<h1>Wakeup</h1>

<form method="POST" action="">
  <table>
    <tr>
      <th>Password:</th>
      <td><input type="password" name="password"/></td>
    </tr>
  </table>
</form>
)=====";

const char asset_css[] PROGMEM = R"=====(
table {
  border-collapse: collapse;
}
table.table-borders > tbody > tr > th,
table.table-borders > tbody > tr > td {
  border: 1px solid #aaa;
  align: left;
}
.input-time {
  border: 1px solid gray;
  white-space: nowrap;
}
input[type=number] {
  border: 1px solid gray;
}
.input-time input[type=number] {
  width: 2em;
  border: none;
  text-align: right;
}
)=====";
