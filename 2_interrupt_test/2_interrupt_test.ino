int d0 = 2;
int d1 = 3;

volatile int reading = 0;
volatile unsigned long timer;
volatile unsigned long data;

void setup() {
  Serial.begin(9600);
  
  attachInterrupt(digitalPinToInterrupt(d0), zeros, FALLING);
  attachInterrupt(digitalPinToInterrupt(d1), ones, FALLING);
}

void loop() {
  unsigned long now = micros();
  long elapsed = now - timer;

  // if reading but its been 7ms since any interrupt then wrap it up
  if (reading == 1 && elapsed > 7000) {
    Serial.println(data, BIN);
    reading = 0;
    data = 0;
  }
}

void zeros() {
  if (digitalRead(d0) == LOW && digitalRead(d1) == HIGH) {
    timer = micros();
    reading = 1;
    data = (data << 1);
  }
}

void ones() {
  if (digitalRead(d1) == LOW && digitalRead(d0) == HIGH) {
    timer = micros();
    reading = 1;
    data = (data << 1) | 1;
  }
}
