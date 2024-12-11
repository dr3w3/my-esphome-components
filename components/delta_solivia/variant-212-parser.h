#pragma once

#include <string>
#include <cstdint>

// Utility functions for byte extraction and scaling
static int16_t extractInt16(const uint8_t *data) {
  return (data[0] << 8) | data[1];
}

static uint32_t extractInt32(const uint8_t *data) {
  return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

static float applyScaling(int16_t value, float scale) {
  return value * scale;
}

static float applyScaling(uint32_t value, float scale) {
  return value * scale;
}

class Variant212Parser {
public:
  // Data fields
  std::string SAP_part_number;
  std::string SAP_serial_number;
  int32_t SAP_date_code; // Date Code
  int16_t SAP_revision;  // Revision
  int8_t Software_rev_ac_major, Software_rev_ac_minor, Software_rev_ac_bugfix;
  int8_t Software_rev_dc_major, Software_rev_dc_minor, Software_rev_dc_bugfix;
  int8_t Software_rev_display_major, Software_rev_display_minor, Software_rev_display_bugfix;
  int8_t Software_rev_sc_major, Software_rev_sc_minor, Software_rev_sc_bugfix;
  float Solar_voltage_input_1; // Volts
  float Solar_current_input_1; // Amps
  float Solar_power_input_1;   // Watts
  float Solar_voltage_input_2; // Volts
  float Solar_current_input_2; // Amps
  float Solar_power_input_2;   // Watts
  float AC_voltage_phase_1;    // Volts
  float AC_current_phase_1;    // Amps
  float AC_power_phase_1;      // Watts
  float AC_frequency_phase_1;  // Hertz
  float AC_voltage_phase_2;    // Volts
  float AC_current_phase_2;    // Amps
  float AC_power_phase_2;      // Watts
  float AC_frequency_phase_2;  // Hertz
  float AC_voltage_phase_3;    // Volts
  float AC_current_phase_3;    // Amps
  float AC_power_phase_3;      // Watts
  float AC_frequency_phase_3;  // Hertz
  float Supplied_ac_energy;    // kWh
  uint32_t Inverter_runtime_hours; // Hours
  uint8_t Status_ac_output_1;
  uint8_t Status_ac_output_2;
  uint8_t Status_ac_output_3;
  uint8_t Error_status;

  // Constructor
  Variant212Parser(const uint8_t* data, bool skipHeader = false) : data(data), pos(skipHeader ? 6 : 0) {}

  void parse() {
    parseSAPInfo();
    parseSoftwareRevisions();
    parseSolarInputs();
    parseACInputs();
    parseEnergyInfo();
    parseStatusAndErrors();
  }

private:
  const uint8_t* data;
  size_t pos;

  void parseSAPInfo() {
    SAP_part_number = parseString(11);
    SAP_serial_number = parseString(13);
    SAP_date_code = extractInt32(&data[pos]); // Date Code
    pos += 4;
    SAP_revision = extractInt16(&data[pos]); // Revision
    pos += 2;
  }

  void parseSoftwareRevisions() {
    Software_rev_ac_major = data[pos++];
    Software_rev_ac_minor = data[pos++];
    Software_rev_ac_bugfix = data[pos++];
    Software_rev_dc_major = data[pos++];
    Software_rev_dc_minor = data[pos++];
    Software_rev_dc_bugfix = data[pos++];
    Software_rev_display_major = data[pos++];
    Software_rev_display_minor = data[pos++];
    Software_rev_display_bugfix = data[pos++];
    Software_rev_sc_major = data[pos++];
    Software_rev_sc_minor = data[pos++];
    Software_rev_sc_bugfix = data[pos++];
  }

  void parseSolarInputs() {
    Solar_voltage_input_1 = applyScaling(extractInt16(&data[pos]), 0.1); // Volts
    pos += 2;
    Solar_current_input_1 = applyScaling(extractInt16(&data[pos]), 0.1); // Amps
    pos += 2;
    Solar_power_input_1 = extractInt16(&data[pos]); // Watts
    pos += 2;

    Solar_voltage_input_2 = applyScaling(extractInt16(&data[pos]), 0.1); // Volts
    pos += 2;
    Solar_current_input_2 = applyScaling(extractInt16(&data[pos]), 0.1); // Amps
    pos += 2;
    Solar_power_input_2 = extractInt16(&data[pos]); // Watts
    pos += 2;
  }

  void parseACInputs() {
    parseACPhase(1);
    parseACPhase(2);
    parseACPhase(3);
  }

  void parseACPhase(int phase) {
    float &voltage = (phase == 1) ? AC_voltage_phase_1 : (phase == 2) ? AC_voltage_phase_2 : AC_voltage_phase_3;
    float &current = (phase == 1) ? AC_current_phase_1 : (phase == 2) ? AC_current_phase_2 : AC_current_phase_3;
    float &power = (phase == 1) ? AC_power_phase_1 : (phase == 2) ? AC_power_phase_2 : AC_power_phase_3;
    float &frequency = (phase == 1) ? AC_frequency_phase_1 : (phase == 2) ? AC_frequency_phase_2 : AC_frequency_phase_3;

    voltage = applyScaling(extractInt16(&data[pos]), 0.1); // Volts
    pos += 2;
    current = applyScaling(extractInt16(&data[pos]), 0.01); // Amps
    pos += 2;
    power = extractInt16(&data[pos]); // Watts
    pos += 2;
    frequency = applyScaling(extractInt16(&data[pos]), 0.01); // Hertz
    pos += 2;
  }

  void parseEnergyInfo() {
    Supplied_ac_energy = applyScaling(extractInt32(&data[pos]), 0.1); // kWh
    pos += 4;
    Inverter_runtime_hours = extractInt32(&data[pos]); // Hours
    pos += 4;
  }

  void parseStatusAndErrors() {
    Status_ac_output_1 = data[pos++];
    Status_ac_output_2 = data[pos++];
    Status_ac_output_3 = data[pos++];
    Error_status = data[pos++];
  }

  std::string parseString(int length) {
    std::string result(data + pos, data + pos + length);
    pos += length;
    return result;
  }
};
