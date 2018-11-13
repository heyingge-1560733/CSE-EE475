#include "Creature.h"
#include "State.h"

State::State(Creature& creature, char* const name, const uint8_t id) : _creature(creature), _id(id) {
  strncpy(_name, name, MAX_NAME_LEN);
  _name[MAX_NAME_LEN] = 0;
};

uint8_t State::getId() {
  return _id;
}
 
char* State::getName() {
  return _name;
}

void State::playSound(uint8_t sound_idx) {
  // TODO: implement
}

void State::playEffect(uint8_t effect_idx) {
  // TODO: implement
}

bool State::rxPlaySound(uint8_t len, uint8_t* payload) {
  // TODO: implement
}

bool State::rxPlayEffect(uint8_t len, uint8_t* payload) {
  // TODO: implement
}

bool State::rxStartle(int8_t rssi, uint8_t strength, uint8_t* payload) {
  double power = 1 - rssi / _creature.GLOBALS.STARTLE_DECAY;
  double sigma = 1 / (1 + exp(-1 * power));
  double decay = sigma * STARLE_FACTOR;
  startled(decay * strength, *payload);
}

void State::txStartle(uint8_t strength, uint8_t id) {
  // TODO: implement
  uint8_t payload[] = {strength, id};
  _creature.tx(0x06, 0xFF, sizeof(payload), payload);
}

State* State::transition() {
  // TODO: implement
}

void State::PIR() {
  // TODO: implement
}

void State::startled(uint8_t strength, uint8_t id) {
  if (id != _creature.getLastStartleId()) {
    uint64_t curr_time = millis();
    uint8_t threshold = _creature.getLastStartleThreshold();
    float threshold_decay = _creature.GLOBALS.STARTLE_THRESHOLD_DECAY;
    threshold = threshold - threshold * (curr_time - _creature.getLastStartle())
                                          * threshold_decay * STARLE_FACTOR;
    _creature.setLastStartleThreshold(threshold);
    if (!(strength < _creature.GLOBALS.STARTLE_THRESHOLD)) {
      //Transition into Startle State
      txStartle(strength, id);
      _creature.setLastStartleId(id);
    }
    _creature.setLastStartle(curr_time);
  }
}

int8_t* State::getGlobalWeights() {
  return _globalWeights;
}
