"""ESPHome Example Actuator Component."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@dragan"]
DEPENDENCIES = []
AUTO_LOAD = ["switch"]

# Namespace
home_esp_ns = cg.esphome_ns.namespace("home_esp")
ExampleActuatorComponent = home_esp_ns.class_("ExampleActuatorComponent", cg.Component)

# Configuration keys
CONF_MIN_ON_TIME = "min_on_time"
CONF_MIN_OFF_TIME = "min_off_time"
CONF_INVERTED = "inverted"

# Configuration schema
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ExampleActuatorComponent),
        cv.Optional(
            CONF_MIN_ON_TIME, default="0s"
        ): cv.positive_time_period_milliseconds,
        cv.Optional(
            CONF_MIN_OFF_TIME, default="0s"
        ): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_INVERTED, default=False): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    """Generate C++ code for the component."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_min_on_time(config[CONF_MIN_ON_TIME]))
    cg.add(var.set_min_off_time(config[CONF_MIN_OFF_TIME]))
    cg.add(var.set_inverted(config[CONF_INVERTED]))
