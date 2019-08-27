#ifndef TIMER_H
#define TIMER_H

class Timer {
  private:
    volatile unsigned long timer;

  public:
    void punch();

    unsigned long last() {
      return timer;
    }
};

#endif
