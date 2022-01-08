enum states {
  STANDBY = 0,
  RECORDING,
  CARD_ERROR,
  FILE_ERROR
};

extern states state;

void setupStatus();

void ledOn();
void ledOff();
void ledToggle();

void updateLed();
