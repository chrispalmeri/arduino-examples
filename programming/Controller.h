class Controller {
  private:

    bool programming = false;

    //key data
    unsigned long code = 0;

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

    void handleKey(unsigned long data) {
      if (data == 10) {
        // reset
        Serial.println("ESC");
        code = 0;
      } else if (data == 11) {
        // auth code and reset
        Serial.println("ENT");
        auth(code);
        code = 0;
      } else {
        // add data to code
        Serial.println(data);
        code *= 10;
        code += data;
      }
    }

    void poll() {
      if (programming == true) {

        unsigned long now = micros();
        long elapsed = now - timeclock.last();
        // for some reason it would not work like in Lock but does work like in Wiegand

        if (elapsed > 5000000) {
          programming = false;
          Serial.println("TIMEOUT");
        }
      }
    }
};
