"""ESPHome Example Bridge Binary Sensor Platform."""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, DEVICE_CLASS_MOTION

from . import ExampleBridgeComponent, home_esp_ns

DEPENDENCIES = ["example_bridge"]

# Configuration schema for the binary sensor platform
CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(
    device_class=DEVICE_CLASS_MOTION,
).extend(
    {
        cv.GenerateID(): cv.use_id(ExampleBridgeComponent),
    }
)


async def to_code(config):
    """Generate C++ code for the binary sensor platform."""
    parent = await cg.get_variable(config[CONF_ID])
    sens = await binary_sensor.new_binary_sensor(config)
    cg.add(parent.set_motion_sensor(sens))
