// Based on Public Solar Inverter Communication Protocol (Version 1.2)
#pragma once

#include "esphome.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "variant-15-parser.h"

namespace esphome {
namespace delta_solivia {

// number of inverters to support (mapped to client addresses)
#define NUM_INVERTERS 2

// protocol characters (page 7)
#define ENQ 0x05
#define ACK 0x06
#define NAK 0x15
#define STX 0x02
#define ETX 0x03

using sensor::Sensor;
using text_sensor::TextSensor;
using uart::UARTDevice;
using uart::UARTComponent;

class Inverter {
  protected:
    uint8_t address_;

  public:
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

    explicit Inverter(uint8_t address) : address_(address) {}

    uint8_t get_address() { return address_; }
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
};

class DeltaSoliviaComponent: public PollingComponent, public UARTDevice {
  uint32_t throttle_ms = 10000;
  std::map<uint8_t, Inverter*> inverters;

  unsigned int calc_crc(unsigned char *sop, unsigned char *eop) {
    unsigned int crc;
    unsigned char *char_ptr;

    char_ptr = sop;
    crc = 0x0000;
    do {
      unsigned char bit_count;
      crc ^= ((*char_ptr) & 0x00ff);
      bit_count = 0;
      do {
        if (crc & 0x0001) {
          crc >>= 1;
          crc ^= 0xA001;
        } else {
          crc >>= 1;
        }
      } while ( bit_count++ < 7 );
    } while ( char_ptr++ < eop );
    return crc;
  }

  public:

    // set throttle interval
    void set_throttle(uint32_t ms) {
      ESP_LOGD("SoliviaG3", "CONFIG - setting throttle interval to %u ms", ms);
      throttle_ms = ms;
    }

    // add an inverter
    void add_inverter(Inverter* inverter) {
      ESP_LOGD("SoliviaG3", "CONFIG - added inverter with address %u", inverter->get_address());
      inverters[inverter->get_address()] = inverter;
    }

    // get inverter
    Inverter* get_inverter(uint8_t address) {
      return inverters.count(address) == 1 ? inverters[address] : nullptr;
    }

    // buffer to store serial data
    std::vector<uint8_t> bytes;

    void update() {
      while (available() > 0) {
        // add new bytes to buffer
        bytes.push_back(read());

        // the minimum amount of data for a valid packet is 9 bytes (Packet 3,
        // page 8)
        if (bytes.size() < 9) {
          continue;
        }

        // perform a quick check to see if this is a protocol header
        if (bytes[0] != STX || bytes[1] != ACK || bytes[2] == 0 || bytes[2] > NUM_INVERTERS || bytes[4] != 0x60 || bytes[5] != 0x01) {
          bytes.erase(bytes.begin());
          continue;
        }

        // read full packet
        unsigned int required_size = 6 + bytes[3] + 3;
        if (bytes.size() != required_size) {
          //ESP_LOGI("SoliviaG3", "BYTES READ = %u NEED %u", bytes.size(), required_size);
          continue;
        }

        // should have a valid header here
        ESP_LOGD("SoliviaG3", "PACKET - header = %02x%02x%02x%02x%02x%02x", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]);
        ESP_LOGD("SoliviaG3", "       - SOP = 0x%02x, PC = 0x%02x, address = %u, data size = %u, cmd = 0x%02x, sub cmd = 0x%02x",
          bytes[0],
          bytes[1],
          bytes[2],
          bytes[3],
          bytes[4],
          bytes[5]
        );

        // validate CRC (page 8/9)
        /*
        const uint16_t packet_crc = *reinterpret_cast<const uint16_t*>(&bytes[155]);
        unsigned int calculated_crc = calc_crc(&bytes[1], &bytes[154]);
        ESP_LOGI("SoliviaG3", "PACKET - packet CRC = 0x%04X (0x%02X%02X), calculated CRC = 0x%X, idx = %u",
            packet_crc,
            bytes[155],
            bytes[156],
            calculated_crc,
            6 + bytes[3]
        );

        bytes.clear();
        continue;
        */

        //ESP_LOGI("SoliviaG3", "PACKET - valid measurement/statistics packet");

        // check if this is addressed to an inverter we know
        uint8_t address = bytes[2];
        auto inverter   = get_inverter(address);

        if (inverter == nullptr) {
          bytes.clear();
          continue;
        }

        // parse buffer and update sensors
        Variant15Parser parser(bytes, true);
        parser.parse();

        if (inverter->solar_voltage_ != nullptr) {
          inverter->solar_voltage_->publish_state(parser.Solar_voltage_input_1);
        }

        if (inverter->solar_current_ != nullptr) {
          inverter->solar_current_->publish_state(parser.Solar_current_input_1);
        }

        if (inverter->ac_current_ != nullptr) {
          inverter->ac_current_->publish_state(parser.AC_current);
        }

        if (inverter->ac_voltage_ != nullptr) {
          inverter->ac_voltage_->publish_state(parser.AC_voltage);
        }

        if (inverter->ac_power_ != nullptr) {
          inverter->ac_power_->publish_state(parser.AC_power);
        }

        if (inverter->ac_frequency_ != nullptr) {
          inverter->ac_frequency_->publish_state(parser.AC_frequency);
        }

        if (inverter->grid_ac_voltage_ != nullptr) {
          inverter->grid_ac_voltage_->publish_state(parser.AC_Grid_voltage);
        }

        if (inverter->grid_ac_frequency_ != nullptr) {
          inverter->grid_ac_frequency_->publish_state(parser.AC_Grid_frequency);
        }

        if (inverter->inverter_runtime_minutes_ != nullptr) {
          inverter->inverter_runtime_minutes_->publish_state(parser.Inverter_runtime_minutes);
        }

        if (inverter->day_supplied_ac_energy_ != nullptr) {
          inverter->day_supplied_ac_energy_->publish_state(parser.Day_supplied_ac_energy);
        }

        if (inverter->max_ac_power_today_ != nullptr) {
          inverter->max_ac_power_today_->publish_state(parser.Max_ac_power_today);
        }

        if (inverter->max_solar_input_power_ != nullptr) {
          inverter->max_solar_input_power_->publish_state(parser.Max_solar_1_input_power);
        }

        if (inverter->inverter_runtime_hours_ != nullptr) {
          inverter->inverter_runtime_hours_->publish_state(parser.Inverter_runtime_hours);
        }

        if (inverter->supplied_ac_energy_ != nullptr) {
          inverter->supplied_ac_energy_->publish_state(parser.Supplied_ac_energy);
        }

        // done
        bytes.clear();
      }
    }
};

}
}