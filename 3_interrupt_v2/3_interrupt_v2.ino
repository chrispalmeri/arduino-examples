// your input pins
int d0 = 2;
int d1 = 3;

// last activity
volatile unsigned long timer;
volatile bool reading = false;

// binary data
volatile unsigned long data;

void setup() {
  attachInterrupt(digitalPinToInterrupt(d0), interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(d1), interrupt, FALLING);

  Serial.begin(9600);
}

void loop() {
  if (reading == true) {
    // how long since last activity
    unsigned long now = micros();
    long elapsed = now - timer;

    // if its been more than 3ms then tie it off
    if (elapsed > 3000) {
      Serial.println(data, BIN);
      reading = false;
      data = 0;
    }
  }
}

void interrupt() {
  // punch the clock for last activity
  timer = micros();
  reading = true;
  
  int d0state = digitalRead(d0);
  int d1state = digitalRead(d1);
  
  if (d0state == LOW && d1state == HIGH) {
    // tag a 0 on the end of the data
    data = (data << 1);
  }
  if (d0state == HIGH && d1state == LOW) {
    // tag a 1 on the end of the data
    data = (data << 1) | 1;
  }
}
