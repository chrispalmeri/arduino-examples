class Control {
  private:

    bool programming = false;

  public:

    // handleKey probably needs to come in here so controller can handle ENT and ESC
    // you also need a timer to enter programming after a card is approved

    void auth(unsigned int number) {
      if (programming) {
        switch (number) {
          case 1:
            Serial.println("permit all");
            break;
          case 2:
            Serial.println("use creds");
            break;
          case 3:
            Serial.println("deny all");
            break;
          case 4:
            Serial.println("audit creds");
            break;
          case 5:
            Serial.println("enroll cred");
            break;
          case 6:
            Serial.println("revoke cred");
            break;
          case 8:
            Serial.println("exit programming");
            programming = false;
            break;
          default:
            Serial.println("unexpected input");
        }
      } else {
        switch (number) {
          case 8:
            Serial.println("enter programming");
            programming = true;
            break;
          default:
            Serial.print(number);
            if (number == 34169 || number == 5555) {
              Serial.println(" approved");
            } else {
              Serial.println(" denied");
            }
        }
      }

    }
};

Control controller;

class Wiegand {
  private:

    // your input pins
    byte d0pin = 2;
    byte d1pin = 3;

    // last read activity
    volatile unsigned long timer;
    volatile bool reading = false;

    // binary data
    volatile unsigned long data;

    //key data
    unsigned long code = 0;

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
        Serial.println("ESC");
        code = 0;
      } else if (data == 11) {
        // auth code and reset
        Serial.println("ENT");
        controller.auth(code);
        code = 0;
      } else {
        // add data to code
        Serial.println(data);
        code *= 10;
        code += data;
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

        controller.auth(number);
      }
    }

  public:

    byte d0() {
      return digitalPinToInterrupt(d0pin);
    }

    byte d1() {
      return digitalPinToInterrupt(d1pin);
    }

    void interrupt() {
      // punch the clock for last activity
      timer = micros();
      reading = true;

      int d0state = digitalRead(d0pin);
      int d1state = digitalRead(d1pin);

      if (d0state == LOW && d1state == HIGH) {
        // tag a 0 on the end of the reader.data
        data = (data << 1);
      }
      if (d0state == HIGH && d1state == LOW) {
        // tag a 1 on the end of the reader.data
        data = (data << 1) | 1;
      }
    }

    void poll() {
      if (reading == true) {
        // how long since last activity
        unsigned long now = micros();
        long elapsed = now - timer;

        // if its been more than 3ms then tie it off
        if (elapsed > 3000) {
          //Serial.println(reader.data, BIN);

          if (data < 16) {
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
    }

};

Wiegand reader;

void setup() {
  attachInterrupt(reader.d0(), [] () {
    reader.interrupt();
  }, FALLING);

  attachInterrupt(reader.d1(), [] () {
    reader.interrupt();
  }, FALLING);

  Serial.begin(9600);
}

void loop() {
  reader.poll();
}
