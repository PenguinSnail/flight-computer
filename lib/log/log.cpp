#include "log.h"

void setupLog() {
  #ifdef DEBUG
  // start serial for debugging
  Serial.begin(9600);
  #endif
}

void debugLog(const char * msg) {
  #ifdef DEBUG
  Serial.println(msg);
  #endif
}