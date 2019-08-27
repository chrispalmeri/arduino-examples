#include <Arduino.h>
#include "Timer.h"

void Timer::punch() {
  timer = micros();
}
