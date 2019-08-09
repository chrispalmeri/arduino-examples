#include <Wiegand.h>

int led = 13;
int buzz = 12;

WIEGAND wg;

void setup() {
  pinMode(led, OUTPUT);
  pinMode(buzz, OUTPUT);
  digitalWrite(led, HIGH);
  digitalWrite(buzz, HIGH);
  
  Serial.begin(9600);  
  
  // default Wiegand Pin 2 and Pin 3 see image on README.md
  // for non UNO board, use wg.begin(pinD0, pinD1) where pinD0 and pinD1 
  // are the pins connected to D0 and D1 of wiegand reader respectively.
  wg.begin();
}

void loop() {
  if(wg.available())
  {
    Serial.print("Wiegand HEX = ");
    Serial.print(wg.getCode(),HEX);
    Serial.print(", DECIMAL = ");
    Serial.print(wg.getCode());
    Serial.print(", Type W");
    Serial.println(wg.getWiegandType()); 

    if(wg.getCode() == 6522233)
    {
      digitalWrite(led, LOW);
      delay(3000);
      digitalWrite(led, HIGH);
    } else {
      digitalWrite(led, HIGH);
      digitalWrite(buzz, LOW);
      delay(700);
      digitalWrite(buzz, HIGH);
    }
  }
}
