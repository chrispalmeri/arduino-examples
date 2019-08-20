// only blocking delay now is for the buzzer which maybe is fine
// add more classes
// need a timer for keypresses also - to reset after inactivity
// code can only go up to 65000ish and rolls over or something if you keep typing
// not sure it should be a number... 0021 is same as 21


class Lock {
  private:

    byte pin = 7;
    byte led = 4;
    unsigned long openduration = 3000000;

    unsigned long unlocktime;
    bool secure = true;

  public:

    void enable() {
      pinMode(pin, OUTPUT);
      pinMode(led, OUTPUT);
    }

    void lock() {
      digitalWrite(led, LOW);
      digitalWrite(pin, LOW);
      secure = true;
    }

    void unlock() {
      unlocktime = micros();
      secure = false;
      digitalWrite(led, HIGH);
      digitalWrite(pin, HIGH);
    }

    bool loose() {
      return !secure;
    }

    bool timeout() {
      return (micros() - unlocktime > openduration);
    }
};

Lock strike;

class Piezo {
  private:

    byte buzz = 5;

  public:

    void enable() {
      pinMode(buzz, OUTPUT);
    }

    void deny() {
      tone(buzz, 440, 700);
      delay(700);
      noTone(buzz);
    }

    void chime() {
      tone(buzz, 2489, 125);
      delay(150);
      tone(buzz, 2093, 500);
      delay(525);
      noTone(buzz);
    }
};

Piezo buzzer;

class Contact {
  private:

    byte pin = 6;

  public:

    void enable() {
      pinMode(pin, INPUT);
    }

    bool trigger() {
      return (digitalRead(pin) == LOW);
    }
};

Contact door;

class Wiegand {
  private:

    //key data
    unsigned long code = 0;

  public:

    // your input pins
    byte d0 = 2;
    byte d1 = 3;

    // last read activity
    volatile unsigned long timer;
    volatile bool reading = false;

    // binary data
    volatile unsigned long data;

    //public:

    unsigned int parity(unsigned int data, unsigned int p) {
      while (data) {
        p ^= (data & 1);
        data >>= 1;
      }
      return p;
    }

    void handleKey() {
      if (data == 10) {
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

      if (parity(evenHalf, 0) == evenParity && parity(oddHalf, 1) == oddParity) {
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
      if (number == 34169 || number == 5555) {
        Serial.println(" approved");
        strike.unlock();
      } else {
        Serial.println(" denied");
        buzzer.deny();
      }
    }

};

Wiegand reader;

void interrupt() {
  // punch the clock for last activity
  reader.timer = micros();
  reader.reading = true;

  int d0state = digitalRead(reader.d0);
  int d1state = digitalRead(reader.d1);

  if (d0state == LOW && d1state == HIGH) {
    // tag a 0 on the end of the reader.data
    reader.data = (reader.data << 1);
  }
  if (d0state == HIGH && d1state == LOW) {
    // tag a 1 on the end of the reader.data
    reader.data = (reader.data << 1) | 1;
  }
}

void setup() {
  // just cause I don't want the board LED on
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  strike.enable();
  buzzer.enable();
  door.enable();

  attachInterrupt(digitalPinToInterrupt(reader.d0), interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(reader.d1), interrupt, FALLING);

  Serial.begin(9600);
}

void loop() {
  if (reader.reading == true) {
    // how long since last activity
    unsigned long now = micros();
    long elapsed = now - reader.timer;

    // if its been more than 3ms then tie it off
    if (elapsed > 3000) {
      //Serial.println(reader.data, BIN);

      if (reader.data < 16) {
        // must be a keypress
        reader.handleKey();
      } else {
        // assume it is 26-bit wiegand
        reader.handleCard();
      }

      reader.reading = false;
      reader.data = 0;
    }
  }

  if (strike.loose() && strike.timeout()) {
    strike.lock();
  }

  if (door.trigger()) {
    buzzer.chime();
  }
}
