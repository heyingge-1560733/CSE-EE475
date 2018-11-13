#include "Ambient.h"
#include "Neopixel.h"
#include "Debug.h"

constexpr uint8_t Ambient::_localWeights[];

uint8_t Ambient::getNumRepeats() {
 return 32;
}

State* Ambient::transition() {
  Midi::setSound(Midi::getSound() ? 0 : 1);
  Neopixel::setLight(Neopixel::getLight() ? 0 : 1);
  return this;
}

void Ambient::loop(uint32_t dt) {
//  dprintln(F("Waiting..."));
}

const uint8_t* Ambient::getLocalWeights() {
  return this->_localWeights;
}

float Ambient::getStartleFactor() {
  return 9999999999;
}

bool Ambient::rxStartle(int8_t rssi, uint8_t len, uint8_t* payload) {}

void Ambient::PIR() {
  dprintln("PIR triggered!");
}

void Ambient::startled(uint8_t strength, uint8_t id) {}
