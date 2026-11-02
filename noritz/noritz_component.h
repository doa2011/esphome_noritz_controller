#pragma once

#include <cstdint>
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace noritz {

class NoritzComponent : public Component, public uart::UARTDevice {
 public:
  explicit NoritzComponent(uart::UARTComponent *parent) : uart::UARTDevice(parent) {}

  void set_power_sensor(sensor::Sensor *sensor) { this->power_sensor_ = sensor; }
  void set_temperature_sensor(sensor::Sensor *sensor) { this->temperature_sensor_ = sensor; }

  void setup() override {}
  void loop() override;

 protected:
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};

  uint8_t frame_[6]{};
  uint8_t frame_pos_{0};
  bool have_frame_{false};

  bool validate_frame_(const uint8_t *frame) const;
  void process_frame_(const uint8_t *frame);
};

}  // namespace noritz
}  // namespace esphome
