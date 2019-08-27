#include "Timer.h"

Timer timeclock;

#include "Controller.h"

Controller controller;

class Wiegand {
  private:

    // your input pins
    byte d0pin = 2;
    byte d1pin = 3;

    // last read activity
    //volatile unsigned long timer;
    volatile bool reading = false;

    // binary data
    volatile unsigned long data;

    unsigned int parity(unsigned int data, unsigned int p) {
      while (data) {
        p ^= (data & 1);
        data >>= 1;
      }
      return p;
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
      //timer = micros();
      timeclock.punch();
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
        //long elapsed = now - timer;
        long elapsed = now - timeclock.last();

        // if its been more than 3ms then tie it off
        if (elapsed > 3000) {
          //Serial.println(reader.data, BIN);

          if (data < 16) {
            // must be a keypress
            controller.handleKey(data);
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
  controller.poll();
}
