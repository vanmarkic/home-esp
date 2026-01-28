"""ESPHome Example Actuator Switch Platform."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch

from . import ExampleActuatorComponent, home_esp_ns

DEPENDENCIES = ["example_actuator"]

# Parent component ID key (separate from entity ID)
CONF_EXAMPLE_ACTUATOR_ID = "example_actuator_id"

# The switch class
ExampleSwitch = home_esp_ns.class_("ExampleSwitch", switch.Switch, cg.Component)

# Configuration schema for the switch platform
CONFIG_SCHEMA = switch.switch_schema(ExampleSwitch).extend(
    {
        cv.GenerateID(CONF_EXAMPLE_ACTUATOR_ID): cv.use_id(ExampleActuatorComponent),
    }
)


async def to_code(config):
    """Generate C++ code for the switch platform."""
    parent = await cg.get_variable(config[CONF_EXAMPLE_ACTUATOR_ID])
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    cg.add(parent.set_switch(var))
