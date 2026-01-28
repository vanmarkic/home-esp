"""ESPHome Example Sensor Component."""

import os

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@dragan"]
DEPENDENCIES = []
AUTO_LOAD = ["sensor"]

# Namespace
home_esp_ns = cg.esphome_ns.namespace("home_esp")
ExampleSensorComponent = home_esp_ns.class_(
    "ExampleSensorComponent", cg.PollingComponent
)

# Configuration keys
CONF_OFFSET = "offset"
CONF_MIN_TEMP = "min_temperature"
CONF_MAX_TEMP = "max_temperature"

# Configuration schema
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ExampleSensorComponent),
        cv.Optional(CONF_OFFSET, default=0.0): cv.float_,
        cv.Optional(CONF_MIN_TEMP, default=-40.0): cv.float_,
        cv.Optional(CONF_MAX_TEMP, default=85.0): cv.float_,
    }
).extend(cv.polling_component_schema("60s"))


async def to_code(config):
    """Generate C++ code for the component."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_offset(config[CONF_OFFSET]))
    cg.add(var.set_min_temperature(config[CONF_MIN_TEMP]))
    cg.add(var.set_max_temperature(config[CONF_MAX_TEMP]))

    # Add include path for lib/core headers
    lib_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "..", "lib")
    )
    cg.add_build_flag(f"-I{lib_path}")
