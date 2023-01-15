
String Helper_HttpHeader() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Syntry Mini v1</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #3498db;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="input{ margin-bottom: 30px; padding: 10px;} form{ margin-top: 50px; }\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  return ptr;
}

char* Helper_StringToChars(String str) {
  // Length (with one extra character for the null terminator)
  int str_len = str.length() + 1; 

  // Prepare the character array (the buffer) 
  char char_array[str_len];

  // Copy it over 
  str.toCharArray(char_array, str_len);

  return char_array;
}

void Helper_array_to_string(byte array[], unsigned int len, char buffer[]) {
   for (unsigned int i = 0; i < len; i++)
   {
      byte nib1 = (array[i] >> 4) & 0x0F;
      byte nib2 = (array[i] >> 0) & 0x0F;
      buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
      buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
   }
   buffer[len*2] = '\0';
}

String Helper_Hotspot_Login() {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Syntry Mini v1</h1>\n";
  ptr +="<h5>Powered by BytesCarfter</h5>\n";
  
  ptr +="<form action='/login' method='get'>\n";
    ptr +="<div><input type='text' name='uname' placeholder='Username'></div>\n";
    ptr +="<div><input type='password' name='pword' placeholder='Password'></div>\n";
    ptr +="<input type='submit' value='AUTHENTICATE'>\n";
  ptr +="</form>\n\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Helper_Hotspot_To_Menu() {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Syntry Mini v1</h1>\n";
  ptr +="<h5>Powered by BytesCarfter</h5>\n";

  ptr +="<form action='/verify' method='get'>\n";
    ptr +="<input type='submit' value='Verify Mode'>\n";
  ptr +="</form>\n\n";
  
  ptr +="<form action='/access' method='get'>\n";
    ptr +="<input type='submit' value='Access Mode'>\n";
  ptr +="</form>\n\n";

  ptr +="<form action='/add' method='get'>\n";
    ptr +="<input type='submit' value='Add Mode'>\n";
  ptr +="</form>\n\n";

  ptr +="<form action='/remove' method='get'>\n";
    ptr +="<input type='submit' value='Remove Mode'>\n";
  ptr +="</form>\n\n";

  ptr +="<form action='/wifi-connect' method='get'>\n";
    ptr +="<input type='submit' value='Wifi Connect'>\n";
  ptr +="</form>\n\n";

  ptr +="<form action='/change-password' method='get'>\n";
    ptr +="<input type='submit' value='Change Password'>\n";
  ptr +="</form>\n\n";

  ptr +="<form action='/menu' method='get'>\n";
    ptr +="<div><input type='hidden' name='action' value='restart'></div>\n";
    ptr +="<input type='submit' value='Restart Device'>\n";
  ptr +="</form>\n\n";

  ptr +="<form action='/' method='get'>\n";
    ptr +="<input type='submit' value='Logout'>\n";
  ptr +="</form>\n\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Helper_Hotspot_To_Access() {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Syntry Mini v1</h1>\n";
  ptr +="<h5>Powered by BytesCarfter</h5>\n";

  ptr +="<form action='/access' method='get'>\n";
    ptr +="<div><input type='hidden' name='action' value='override'></div>\n";
    ptr +="<input type='submit' value='Override Access'>\n";
  ptr +="</form>\n\n";
  
  ptr +="<form action='/menu' method='get'>\n";
    ptr +="<input type='submit' value='Back to Menu'>\n";
  ptr +="</form>\n\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Helper_Hotspot_To_Add() {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Syntry Mini v1</h1>\n";
  ptr +="<h5>Powered by BytesCarfter</h5>\n";
  
  ptr +="<form action='/menu' method='get'>\n";
    ptr +="<input type='submit' value='Back to Menu'>\n";
  ptr +="</form>\n\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Helper_Hotspot_To_Remove() {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Syntry Mini v1</h1>\n";
  ptr +="<h5>Powered by BytesCarfter</h5>\n";
  
  ptr +="<form action='/menu' method='get'>\n";
    ptr +="<input type='submit' value='Back to Menu'>\n";
  ptr +="</form>\n\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Helper_Hotspot_To_Verify() {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Syntry Mini v1</h1>\n";
  ptr +="<h5>Powered by BytesCarfter</h5>\n";
  
  ptr +="<form action='/menu' method='get'>\n";
    ptr +="<input type='submit' value='Back to Menu'>\n";
  ptr +="</form>\n\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Helper_Hotspot_ChangePassword(String message = "") {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Syntry Mini v1</h1>\n";
  ptr +="<h5>Powered by BytesCarfter</h5>\n";

  if(message != "") {
    ptr +="<h6>"+message+"</h6>\n";
  }
  
  ptr +="<form action='/update-password' method='get'>\n";
    ptr +="<div><input type='password' name='newpass' placeholder='New Password'></div>\n";
    ptr +="<div><input type='password' name='confirmpass' placeholder='Confirm Password'></div>\n";
    ptr +="<input type='submit' value='UPDATE PASSWORD'>\n";
  ptr +="</form>\n\n";

  ptr +="<form action='/menu' method='get'>\n";
    ptr +="<input type='submit' value='Back to Menu'>\n";
  ptr +="</form>\n\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Helper_Hotspot_ConnectWifi(String message = "") {
  String ptr = Helper_HttpHeader();
  ptr +="<h1>Syntry Mini v1</h1>\n";
  ptr +="<h5>Powered by BytesCarfter</h5>\n";

  if(message != "") {
    ptr +="<h6>"+message+"</h6>\n";
  }
  
  ptr +="<form action='/save-wifi' method='get'>\n";
    ptr +="<div><input type='text' name='wifiname' placeholder='Wifi Name'></div>\n";
    ptr +="<div><input type='password' name='wifipass' placeholder='Wifi Pass'></div>\n";
    ptr +="<input type='submit' value='Save & Connect'>\n";
  ptr +="</form>\n\n";

  ptr +="<form action='/menu' method='get'>\n";
    ptr +="<input type='submit' value='Back to Menu'>\n";
  ptr +="</form>\n\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}