
#define HEADER_COOKIE "Cookie"
#define TOKEN "Token="

const int TOKEN_LEN = strlen( TOKEN);

String getAccessToken() {
  if (server.hasHeader(HEADER_COOKIE)) {
    String cookie = server.header( HEADER_COOKIE);
    int idxToken = cookie.indexOf( TOKEN);
    if( idxToken >= 0) {
      int idxSemicolon = cookie.indexOf( ';', idxToken);
      String accessToken;
      if( idxSemicolon > 0) {
        accessToken = cookie.substring( idxToken + TOKEN_LEN, idxSemicolon);
      } else {
        accessToken = cookie.substring( idxToken + TOKEN_LEN);
      }
      Serial.print("AccessToken: ");
      Serial.println(accessToken);
      return accessToken;
    }
  }
  return (String)NULL;
}

void redirectToLogin() {
  server.sendHeader("Location", "/auth.html", true);
  server.send(302);
}

bool redirectIfNoAuth() {
  if( NULL == getAccessToken()) {
    redirectToLogin();
    return true;
  }
  return false;
}

bool isAuthorized() {
  String accessToken = getAccessToken();

//https://www.googleapis.com/oauth2/v3/tokeninfo?access_token=ya29.GlsFBt3rcGks1-T9Q7DeMuXXWEaejygIxhsmetttruKgKTewO22V28XP7b_HEwls2CQoJxQ4x0Og8a3k-wMle4QpUrqUQ4OuM6OvlDvJ6dy_nE99HyghMj3WzMU0


  if( accessToken == NULL) {
    redirectToLogin();
    return false;
  }
  return true;  
}

