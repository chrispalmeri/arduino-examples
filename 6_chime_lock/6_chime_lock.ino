// remove delays and replace with timers
// need a timer for keypresses also - to reset after inactivity
// you do already have a timer
// refactor
// code can only go up to 65000ish and rolls over or something if you keep typing
// not sure it should be a number... 0021 is same as 21

// your input pins
int d0 = 2;
int d1 = 3;
int led = 12;
int buzz = 13;
int door = 6;
int lock = 7;

// last activity
volatile unsigned long timer;
volatile bool reading = false;

// binary data
volatile unsigned long data;

//key data
unsigned long code = 0;

void setup() {
  pinMode(led, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(door, INPUT);
  pinMode(lock, OUTPUT);
  
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
      //Serial.println(data, BIN);

      if(data < 16) {
        // must be a keypress
        handleKey();
      } else {
        // assume it is 26-bit wiegand
        handleCard();
      }
      
      reading = false;
      data = 0;
    }
  }

  if (digitalRead(door) == LOW) {
    chime();
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

void chime() {
  tone(buzz, 2489, 125);
  delay(150);
  tone(buzz, 2093, 500);
  delay(525);
  noTone(buzz);
}

unsigned int parity(unsigned int data, unsigned int p) {
  while(data){
    p ^= (data & 1);
    data >>= 1;
  }
  return p;
}

void handleKey() {
  if(data == 10) {
    // reset
    //Serial.println("escape");
    code = 0;
  } else if (data == 11) {
    // auth code and reset
    //Serial.println("enter");
    auth(code);
    code = 0;
  } else {
    // add data to code
    code *= 10;
    code += data;
    //Serial.println(code);
  }
}

void handleCard() {
  unsigned int evenParity = (data >> 25) & 1;
  unsigned int evenHalf = (data >> 13) & 4095;
  unsigned int oddHalf = (data >> 1) & 4095;
  unsigned int oddParity = data & 1;

  if(parity(evenHalf, 0) == evenParity && parity(oddHalf, 1) == oddParity) {
    unsigned int facility = (data >> 17) & 255;
    unsigned int number = (data >> 1) & 65535;

    //Serial.print("Facility code: ");
    //Serial.print(facility);
    //Serial.print(" Card number: ");
    //Serial.println(number);

    auth(number);
  }
}

void auth(unsigned int number) {
  Serial.print(number);
  if(number == 34169 || number == 5555) {
    Serial.println(" approved");
    digitalWrite(led, HIGH);
    digitalWrite(lock, HIGH);
    delay(3000); // doesn't read anything during this time...
    digitalWrite(led, LOW);
    digitalWrite(lock, LOW);
  } else {
    Serial.println(" denied");
    digitalWrite(led, LOW);
    tone(buzz, 440, 700);
    delay(700);
    noTone(buzz);
  }
}
