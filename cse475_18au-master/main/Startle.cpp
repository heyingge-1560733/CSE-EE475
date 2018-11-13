#include "Startle.h"
#include "Neopixel.h"
#include "Debug.h"

constexpr uint8_t Startle::_localWeights[];

uint8_t Startle::getNumRepeats() {
 return 32;
}

State* Startle::transition() {
  Midi::setSound(Midi::getSound() ? 0 : 1);
  Neopixel::setLight(Neopixel::getLight() ? 0 : 1);
  return this;
}

void Startle::loop(uint32_t dt) {
//  dprintln(F("Waiting..."));
}

const uint8_t* Startle::getLocalWeights() {
  return this->_localWeights;
}

float Startle::getStartleFactor() {
  return 9999999999;
}

bool Startle::rxStartle(int8_t rssi, uint8_t len, uint8_t* payload) {}

void Startle::PIR() {
  dprintln("PIR triggered!");
}

void Startle::startled(uint8_t strength, uint8_t id) {}
