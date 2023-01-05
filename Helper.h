
char* Helper_StringToChars(String str) {
  // Length (with one extra character for the null terminator)
  int str_len = str.length() + 1; 

  // Prepare the character array (the buffer) 
  char char_array[str_len];

  // Copy it over 
  str.toCharArray(char_array, str_len);

  return char_array;
}

void Helper_array_to_string(byte array[], unsigned int len, char buffer[])
{
   for (unsigned int i = 0; i < len; i++)
   {
      byte nib1 = (array[i] >> 4) & 0x0F;
      byte nib2 = (array[i] >> 0) & 0x0F;
      buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
      buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
   }
   buffer[len*2] = '\0';
}

// const char* Helper_WifiStatus(wl_status_t status) {
//   switch (status) {
//     case WL_NO_SHIELD: return "WL_NO_SHIELD";
//     case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
//     case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
//     case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
//     case WL_CONNECTED: return "WL_CONNECTED";
//     case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
//     case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
//     case WL_DISCONNECTED: return "WL_DISCONNECTED";
//   }
// }

String Helper_TestHtml(uint8_t led1stat, uint8_t led2stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP8266 Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  
   if(led1stat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";}
  else
  {ptr +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";}

  if(led2stat)
  {ptr +="<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";}
  else
  {ptr +="<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String Helper_ConnectHtml(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
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
  ptr +="<h1>Syntry Mini v1</h1>\n";
  ptr +="<h5>Powered by BytesCarfter</h5>\n";
  
  ptr +="<form action='' method='get'>\n";
    ptr +="<div><input type='text' name='ssid' placeholder='Wifi SSID'></div>\n";
    ptr +="<div><input type='password' name='pw' placeholder='Password'></div>\n";
    ptr +="<input type='submit' value='CONNECT TO WIFI'>\n";
  ptr +="</form>\n\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}