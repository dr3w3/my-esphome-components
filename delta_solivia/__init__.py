import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_FREQUENCY,
    DEVICE_CLASS_DURATION,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_WATT,
    UNIT_KILOWATT,
    UNIT_KILOWATT_HOURS,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_HERTZ,
    UNIT_HOUR,
    UNIT_MINUTE,
)

# maximum number of inverters that we support (pretty randomly chosen)
CONF_MAX_INVERTERS = 4

CODEOWNERS   = ["@robertklep"]
DEPENDENCIES = ["uart"]
AUTO_LOAD    = ["sensor", "text_sensor"]

delta_solivia_ns = cg.esphome_ns.namespace("delta_solivia")
DeltaSolivia     = delta_solivia_ns.class_("DeltaSoliviaComponent", uart.UARTDevice, cg.Component)

# global config
CONF_INVERTERS = "inverters"
CONF_THROTTLE  = "throttle"

# per-inverter config
CONF_INV_ADDRESS = "address"

# per-inverter measurements
CONF_INV_POWER                 = "power"
CONF_INV_TOTAL_ENERGY          = "total_energy"
CONF_INV_TODAY_ENERGY          = "today_energy"
CONF_INV_DC_VOLTAGE            = "dc_voltage"
CONF_INV_DC_CURRENT            = "dc_current"
CONF_INV_AC_VOLTAGE            = "ac_voltage"
CONF_INV_AC_CURRENT            = "ac_current"
CONF_INV_AC_FREQ               = "ac_frequency"
CONF_INV_AC_POWER              = "ac_power"
CONF_INV_GRID_VOLTAGE          = "grid_voltage"
CONF_INV_GRID_FREQ             = "grid_frequency"
CONF_INV_RUNTIME_HOURS         = "runtime_hours"
CONF_INV_RUNTIME_MINUTES       = "runtime_minutes"
CONF_INV_MAX_AC_POWER          = "max_ac_power_today"
CONF_INV_MAX_SOLAR_INPUT_POWER = "max_solar_input_power"

def _validate_inverters(config):
    if len(config) < 1 or len(config) > CONF_MAX_INVERTERS:
        raise cv.Invalid("Number of inverters should be between 1 and %s" % CONF_MAX_INVERTERS)
    # ensure all configured inverters have a unique address
    addresses = { inverter.get(CONF_INV_ADDRESS) for inverter in config }
    if len(addresses) != len(config):
        raise cv.Invalid("Inverter addresses should be unique")
    return config

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.declare_id(DeltaSolivia),
        cv.Optional(CONF_THROTTLE, default = 10000) : cv.int_range(min = 0),
        cv.Required(CONF_INVERTERS): cv.All(
            cv.ensure_list({
                cv.Optional(CONF_INV_ADDRESS, default = 1): cv.int_range(min = 1, max = CONF_MAX_INVERTERS),
                cv.Optional(CONF_INV_POWER): sensor.sensor_schema(
                    unit_of_measurement = UNIT_WATT,
                    icon                = 'mdi:solar_power',
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_POWER,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_TOTAL_ENERGY): sensor.sensor_schema(
                    unit_of_measurement = UNIT_KILOWATT_HOURS,
                    icon                = 'mdi:meter-electric',
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_ENERGY,
                    state_class         = STATE_CLASS_TOTAL_INCREASING
                ),
                cv.Optional(CONF_INV_TODAY_ENERGY): sensor.sensor_schema(
                    unit_of_measurement = UNIT_KILOWATT_HOURS,
                    icon                = 'mdi:meter-electric',
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_ENERGY,
                    state_class         = STATE_CLASS_TOTAL_INCREASING
                ),
                cv.Optional(CONF_INV_DC_VOLTAGE): sensor.sensor_schema(
                    unit_of_measurement = UNIT_VOLT,
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_VOLTAGE,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_DC_CURRENT): sensor.sensor_schema(
                    unit_of_measurement = UNIT_AMPERE,
                    accuracy_decimals   = 1,
                    device_class        = DEVICE_CLASS_CURRENT,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_AC_VOLTAGE): sensor.sensor_schema(
                    unit_of_measurement = UNIT_VOLT,
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_VOLTAGE,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_AC_CURRENT): sensor.sensor_schema(
                    unit_of_measurement = UNIT_AMPERE,
                    accuracy_decimals   = 1,
                    device_class        = DEVICE_CLASS_CURRENT,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_AC_FREQ): sensor.sensor_schema(
                    unit_of_measurement = UNIT_HERTZ,
                    accuracy_decimals   = 2,
                    device_class        = DEVICE_CLASS_FREQUENCY,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_AC_POWER): sensor.sensor_schema(
                    unit_of_measurement = UNIT_WATT,
                    icon                = 'mdi:solar_power',
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_POWER,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_GRID_VOLTAGE): sensor.sensor_schema(
                    unit_of_measurement = UNIT_VOLT,
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_VOLTAGE,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_GRID_FREQ): sensor.sensor_schema(
                    unit_of_measurement = UNIT_HERTZ,
                    accuracy_decimals   = 2,
                    device_class        = DEVICE_CLASS_FREQUENCY,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_RUNTIME_HOURS): sensor.sensor_schema(
                    unit_of_measurement = UNIT_HOUR,
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_DURATION,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_RUNTIME_MINUTES): sensor.sensor_schema(
                    unit_of_measurement = UNIT_MINUTE,
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_DURATION,
                    state_class         = STATE_CLASS_MEASUREMENT,
                ),
                cv.Optional(CONF_INV_MAX_AC_POWER): sensor.sensor_schema(
                    unit_of_measurement = UNIT_WATT,
                    icon                = 'mdi:solar_power',
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_POWER,
                    state_class         = STATE_CLASS_TOTAL_INCREASING
                ),
                cv.Optional(CONF_INV_MAX_SOLAR_INPUT_POWER): sensor.sensor_schema(
                    unit_of_measurement = UNIT_WATT,
                    icon                = 'mdi:solar_power',
                    accuracy_decimals   = 0,
                    device_class        = DEVICE_CLASS_POWER,
                    state_class         = STATE_CLASS_TOTAL_INCREASING
                ),
            }),
            _validate_inverters
        )
    })
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA),
#    _validate
)
