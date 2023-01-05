//STATUS: Working!

/* Local Server */
const char *svrAp = "Syntry-AP";
const char *svrPw = "bytescrafter";

/* Put IP Address details */
IPAddress local_ip(10,10,10,1);
IPAddress gateway(10,10,10,1);
IPAddress subnet(255,255,255,0);

// Create WebServer object on port 80
//ESP8266WebServer web(80);
ESP8266WebServer webs(80);

void Hotspot_broadcast() {

  Serial.println();
  Serial.println("Starting Hotspot.");

  //WiFi.disconnect(false);

  WiFi.softAP(svrAp, svrPw);
  IPAddress IP = WiFi.softAPIP();
  //WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  Serial.println();
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //web.on("/", handle_OnConnect);
  //web.on("/test", handle_led1on);
  //web.onNotFound(handle_NotFound);

  // Start server
  //web.begin();
}

// void handle_OnConnect() {
//   Serial.println("Client request hotspot.");
//   web.send(200, "text/html", Helper_TestHtml(1,1)); 
// }

// void handle_NotFound(){
//   //web.send(404, "text/plain", "Not found");
// }

void Hotspot_loop() {
  webs.handleClient();
}