// src/SafeVar.h - VERSION POUR TON PROJET ESP32
#ifndef SAFEVAR_H
#define SAFEVAR_H

#include <Arduino.h>

template<typename T>
class SafeVar {
private:
  #ifdef ESP32
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  #endif
  volatile T value;

public:
  SafeVar(T initialValue = T()) : value(initialValue) {}
  
  T get() {
    #ifdef ESP32
      portENTER_CRITICAL(&mux);
      T temp = value;
      portEXIT_CRITICAL(&mux);
      return temp;
    #else
      noInterrupts();
      T temp = value;
      interrupts();
      return temp;
    #endif
  }
  
  void set(T newValue) {
    #ifdef ESP32
      portENTER_CRITICAL(&mux);
      value = newValue;
      portEXIT_CRITICAL(&mux);
    #else
      noInterrupts();
      value = newValue;
      interrupts();
    #endif
  }
  
  operator T() { return get(); }
  SafeVar& operator=(T newValue) {
    set(newValue);
    return *this;
  }
  
  SafeVar& operator+=(T val) {
    set(get() + val);
    return *this;
  }
  
  SafeVar& operator-=(T val) {
    set(get() - val);
    return *this;
  }
};

#endif