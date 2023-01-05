
void Relay_Init(int PORT = CONFIG_RELAY, int isOn = LOW) {
  pinMode(PORT, OUTPUT);
  digitalWrite(PORT, isOn);
}

void Relay_On(int PORT = CONFIG_RELAY) {
  digitalWrite(PORT, HIGH);
}

void Relay_Off(int PORT = CONFIG_RELAY) {
  digitalWrite(PORT, LOW);
}

void Relay_Open(int wait = 7000) {
  digitalWrite(CONFIG_RELAY, HIGH);
  delay(wait);
  digitalWrite(CONFIG_RELAY, LOW);
}