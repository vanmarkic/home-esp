"""ESPHome Example Sensor Platform."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

from . import ExampleSensorComponent

DEPENDENCIES = ["example_sensor"]

# Configuration schema for the sensor platform
CONFIG_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
).extend(
    {
        cv.GenerateID(): cv.use_id(ExampleSensorComponent),
    }
)


async def to_code(config):
    """Generate C++ code for the sensor platform."""
    parent = await cg.get_variable(config[CONF_ID])
    sens = await sensor.new_sensor(config)
    cg.add(parent.set_sensor(sens))
