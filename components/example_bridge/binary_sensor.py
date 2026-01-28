"""ESPHome Example Bridge Binary Sensor Platform."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import DEVICE_CLASS_MOTION

from . import ExampleBridgeComponent

DEPENDENCIES = ["example_bridge"]

# Parent component ID key (separate from entity ID)
CONF_EXAMPLE_BRIDGE_ID = "example_bridge_id"

# Configuration schema for the binary sensor platform
CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(
    device_class=DEVICE_CLASS_MOTION,
).extend(
    {
        cv.GenerateID(CONF_EXAMPLE_BRIDGE_ID): cv.use_id(ExampleBridgeComponent),
    }
)


async def to_code(config):
    """Generate C++ code for the binary sensor platform."""
    parent = await cg.get_variable(config[CONF_EXAMPLE_BRIDGE_ID])
    sens = await binary_sensor.new_binary_sensor(config)
    cg.add(parent.set_motion_sensor(sens))
