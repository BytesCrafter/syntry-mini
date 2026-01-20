/*
  Module: Default LED Module.
  Version: 0.1.0
*/

//Turn the Red LED light
void Light_Blink(int status = HIGH, int life = 0) {
  pinMode(LIGHT_PIN, OUTPUT);
  if(life) {
    digitalWrite(LIGHT_PIN, HIGH);
    delay(life);
    digitalWrite(LIGHT_PIN, LOW);
  } else {
    digitalWrite(LIGHT_PIN, status);
  }
}