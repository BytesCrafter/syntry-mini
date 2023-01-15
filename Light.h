/*
  Module: Default LED Module.
  Version: 0.1.0
*/

//Turn the Red LED light
void Light_Red(int status = HIGH, int life = 0) {
  pinMode(LIGHT_RED, OUTPUT);
  if(life) {
    digitalWrite(LIGHT_RED, HIGH);
    delay(life);
    digitalWrite(LIGHT_RED, LOW);
  } else {
    digitalWrite(LIGHT_RED, status);
  }
}

//Turn the Green LED light
void Light_Green(int status = HIGH, int life = 0) {
  pinMode(LIGHT_GREEN, OUTPUT);
  if(life) {
    digitalWrite(LIGHT_GREEN, HIGH);
    delay(life);
    digitalWrite(LIGHT_GREEN, LOW);
  } else {
    digitalWrite(LIGHT_GREEN, status);
  }
}