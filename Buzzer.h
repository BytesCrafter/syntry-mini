/*
  Module: Buzzer Module.
  Version: 0.1.0
*/

void Buzzer_Play(int beat = 1, int hertz = 1000, int gap = 100) {
  pinMode(BUZZER, OUTPUT);

  for (int i=0; i<beat; i++) {
    tone(BUZZER, hertz); // Send sound signal...
    delay(500);
    noTone(BUZZER); 
    delay(gap);
  }
}
