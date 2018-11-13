#include "Active.h"
#include "Neopixel.h"
#include "Debug.h"

constexpr uint8_t Active::_localWeights[];

uint8_t Active::getNumRepeats() {
 return 32;
}

State* Active::transition() {
  Midi::setSound(Midi::getSound() ? 0 : 1);
  Neopixel::setLight(Neopixel::getLight() ? 0 : 1);
  return this;
}

void Active::loop(uint32_t dt) {
//  dprintln(F("Waiting..."));
}

const uint8_t* Active::getLocalWeights() {
  return this->_localWeights;
}

float Active::getStartleFactor() {
  return 9999999999;
}

bool Active::rxStartle(int8_t rssi, uint8_t len, uint8_t* payload) {}

void Active::PIR() {
  dprintln("PIR triggered!");
}

void Active::startled(uint8_t strength, uint8_t id) {}
