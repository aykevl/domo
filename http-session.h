
const uint32_t SESSION_DURATION = 2592000;

// Compare two byte buffers in constant time.
bool httpSessionConstantTimeEq(const char* buf1, const char* buf2, size_t length) {
  char result = 0;
  for (size_t i=0; i < length; i++) {
    result |= buf1[i] ^ buf2[i];
  }
  return result == 0;
}

// Generate a token from the given time. The token is dependent on that
// time. Anyone in posession of such a token will be able to login.
// See this blog post for an overview of what this system is based on:
// https://aykevl.nl/2015/01/south-stateless-authenticated-sessions-http-golang
String httpSessionGenerateToken(uint32_t ts) {
  String token(ts);
  // Generate a MAC over the payload, with the given key.
  uint8_t hashRaw[32];
  if (blake2s(hashRaw, 32, token.c_str(), token.length(), SECRET, sizeof(SECRET))) {
    return String(F("fail"));
  }
  String hash = base64::encode(hashRaw, 32);
  token.reserve(token.length() + 1 + hash.length()); // save one malloc
  token += ':';
  token += hash;
  return token;
}

bool httpSessionIsAuthenticated(ESP8266WebServer &server) {
  if (!server.hasHeader(String("Cookie"))) {
    //server.sendHeader("X-Debug", "no Cookie header");
    return false;
  }

  // Find the cookie value.
  String cookie = server.header(F("Cookie"));
  int index = cookie.indexOf(F("token="));
  if (index < 0) {
    //server.sendHeader("X-Debug", "index - 1");
    return false;
  }
  if (index > 0) {
    if (!(cookie[index-1] == ' ' || cookie[index-1] == ';')) {
      // Not the start of a token.
      // Could be used in a DoS attack, if the attacker can set cookies.
      // (by setting abctoken=xyz before token=..., for example).
      //server.sendHeader("X-Debug", "cookie not found");
      return false;
    }
  }

  // Found the cookie position.
  index += 6; // "token="
  uint32_t creationTime = 0;
  int startIndex = index;
  for (; cookie[index] >= '0' && cookie[index] <= '9'; index++) {
    creationTime = creationTime * 10 + (cookie[index] - '0');
  }
  int endIndex = index - 1;

  uint32_t now = Clock.timestamp();
  if (creationTime < now - SESSION_DURATION || creationTime > now) {
    //server.sendHeader("X-Debug", "token creation time outside range");
    return false;
  }

  // Verify this is the right token.
  // I think this can be done more efficiently by actually decoding the
  // base64 MAC, instead of generating a new one.
  String token = httpSessionGenerateToken(creationTime);
  if (cookie.length() - startIndex < token.length()) {
    //server.sendHeader("X-Debug", "cookie not long enough");
    return false;
  }
  if (!httpSessionConstantTimeEq(token.c_str(), cookie.c_str()+startIndex, token.length())) {
    //server.sendHeader("X-Debug", "token doesn't match");
    return false;
  }

  return true;
}

bool httpSessionLogin(ESP8266WebServer &server) {
  String password = server.arg("password");
  if (strlen(LOGIN_PASSWORD) != password.length() || httpSessionConstantTimeEq(LOGIN_PASSWORD, password.c_str(), password.length())) {
    server.sendHeader("Set-Cookie", "token=" + httpSessionGenerateToken(Clock.timestamp()) + ";Max-Age=" + String(SESSION_DURATION));
    return true;
  }
  return false;
}
