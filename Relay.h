/*
  Module: Relay Module.
  Version: 0.1.0
*/

void Relay_Init(int PORT = CONFIG_RELAY) {
  pinMode(PORT, OUTPUT);
}

void Relay_On(int PORT = CONFIG_RELAY) {
  digitalWrite(PORT, HIGH);
}

void Relay_Off(int PORT = CONFIG_RELAY) {
  digitalWrite(PORT, LOW);
}

//Open relay for some time.
void Relay_Open(int wait = WAIT_OPEN) {
  digitalWrite(CONFIG_RELAY, HIGH);
  delay(wait);
  digitalWrite(CONFIG_RELAY, LOW);
}