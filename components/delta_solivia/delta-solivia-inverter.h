#pragma once

#include "esphome.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "constants.h"
#include "delta-solivia-crc.h"
#include "variant-15-parser.h"

namespace esphome {
namespace delta_solivia {

using sensor::Sensor;
using text_sensor::TextSensor;

class DeltaSoliviaInverter {
  protected:
    uint8_t address_;

  public:
    TextSensor* part_number_ { nullptr };
    TextSensor* serial_number_ { nullptr };
    Sensor* solar_voltage_ { nullptr };
    Sensor* solar_current_ { nullptr };
    Sensor* ac_current_ { nullptr };
    Sensor* ac_voltage_ { nullptr };
    Sensor* ac_power_ { nullptr };
    Sensor* ac_frequency_ { nullptr };
    Sensor* grid_ac_voltage_ { nullptr };
    Sensor* grid_ac_frequency_ { nullptr };
    Sensor* inverter_runtime_minutes_ { nullptr };
    Sensor* inverter_runtime_hours_ { nullptr };
    Sensor* day_supplied_ac_energy_ { nullptr };
    Sensor* supplied_ac_energy_ { nullptr };
    Sensor* max_ac_power_today_ { nullptr };
    Sensor* max_solar_input_power_ { nullptr };

    explicit DeltaSoliviaInverter(uint8_t address) : address_(address) {}

    uint8_t get_address() { return address_; }

    void set_part_number(TextSensor* part_number) { part_number_ = part_number; }
    void set_serial_number(TextSensor* serial_number) { serial_number_ = serial_number; }
    void set_solar_voltage(Sensor* solar_voltage) { solar_voltage_ = solar_voltage; }
    void set_solar_current(Sensor* solar_current) { solar_current_ = solar_current; }
    void set_ac_current(Sensor* ac_current) { ac_current_ = ac_current; }
    void set_ac_voltage(Sensor* ac_voltage) { ac_voltage_ = ac_voltage; }
    void set_ac_power(Sensor* ac_power) { ac_power_ = ac_power; }
    void set_ac_frequency(Sensor* ac_frequency) { ac_frequency_ = ac_frequency; }
    void set_grid_ac_voltage(Sensor* grid_ac_voltage) { grid_ac_voltage_ = grid_ac_voltage; }
    void set_grid_ac_frequency(Sensor* grid_ac_frequency) { grid_ac_frequency_ = grid_ac_frequency; }
    void set_inverter_runtime_minutes(Sensor* inverter_runtime_minutes) { inverter_runtime_minutes_ = inverter_runtime_minutes; }
    void set_inverter_runtime_hours(Sensor* inverter_runtime_hours) { inverter_runtime_hours_ = inverter_runtime_hours; }
    void set_day_supplied_ac_energy(Sensor* day_supplied_ac_energy) { day_supplied_ac_energy_ = day_supplied_ac_energy; }
    void set_supplied_ac_energy(Sensor* supplied_ac_energy) { supplied_ac_energy_ = supplied_ac_energy; }
    void set_max_ac_power_today(Sensor* max_ac_power_today) { max_ac_power_today_ = max_ac_power_today; }
    void set_max_solar_input_power(Sensor* max_solar_input_power) { max_solar_input_power_ = max_solar_input_power; }

    void update_sensors(const uint8_t*);

    template <typename F>
    void request_update(const F& callback) {
      ESP_LOGD(LOG_TAG, "INVERTER%u - requesting update", address_);

      // Enquire packet (page 7/8)
      const uint8_t bytes[] = {
        STX,      // start of protocol
        ENQ,      // enquire
        address_, // for inverter with address
        0x02,     // number of data bytes, including commands
        0x96,     // command edited to 0x96, from Hint via ChatGPT, for Delta H4A inverter
        0x01,     // subcommand
        0x00,     // CRC low
        0x00,     // CRC high
        ETX       // end of protocol
      };

      // calculate CRC
      *((uint16_t*) &bytes[6]) = delta_solivia_crc((uint8_t *) bytes + 1, (uint8_t *) bytes + 5);

      // call callback with data, caller will handle writing to UART
      callback(&bytes[0], sizeof(bytes));
    }
};

}
}
