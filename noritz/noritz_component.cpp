#include "noritz_component.h"
#include "esphome/core/log.h"

namespace esphome {
namespace noritz {

static const char *const TAG = "noritz";

bool NoritzComponent::validate_frame_(const uint8_t *frame) const {
  if (frame[0] != 0xDF || frame[1] != 0xEF || frame[2] != 0xFF)
    return false;

  // Matches the checksum implementation in the upstream noritz.h:
  // data[0] + data[1] + data[2] + data[3] + data[4] - 945 == data[5]
  const int sum = static_cast<int>(frame[0]) + frame[1] + frame[2] +
                  frame[3] + frame[4] - 945;
  return sum == static_cast<int>(frame[5]);
}

void NoritzComponent::process_frame_(const uint8_t *frame) {
  if (!this->validate_frame_(frame)) {
    ESP_LOGD(TAG, "Ignoring invalid frame");
    return;
  }

  const bool power_on = (frame[3] & 0x01) == 0;
  if (this->power_sensor_ != nullptr)
    this->power_sensor_->publish_state(power_on ? 1.0f : 0.0f);

  // Keep the upstream temperature decoding behavior, but only update
  // temperature when the byte matches the recognized temperature pattern.
  if ((frame[4] >> 4) != 13)
    return;

  const uint8_t temp_byte = frame[4] & 0x0F;
  int temperature = 35;
  bool recognized = true;

  if (temp_byte > 6) {
    temperature = 53 - temp_byte;
  } else {
    switch (temp_byte) {
      case 0: temperature = 47; break;
      case 1: temperature = 48; break;
      case 2: temperature = 35; break;
      case 3: temperature = 37; break;
      case 5: temperature = 60; break;
      case 6: temperature = 50; break;
      default: recognized = false; break;
    }
  }

  if (recognized && this->temperature_sensor_ != nullptr)
    this->temperature_sensor_->publish_state(temperature);
}

void NoritzComponent::loop() {
  while (this->available()) {
    uint8_t value;
    if (!this->read_byte(&value))
      break;

    // Synchronize on DF, then collect exactly six bytes.
    if (this->frame_pos_ == 0) {
      if (value != 0xDF)
        continue;
      this->frame_[0] = value;
      this->frame_pos_ = 1;
      continue;
    }

    this->frame_[this->frame_pos_++] = value;
    if (this->frame_pos_ == sizeof(this->frame_)) {
      this->process_frame_(this->frame_);
      this->frame_pos_ = 0;
    }
  }
}

}  // namespace noritz
}  // namespace esphome
