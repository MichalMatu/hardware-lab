from __future__ import annotations

from dataclasses import dataclass


SUPPORTED_POWER = {"usb_5v"}
SUPPORTED_MCU = {"esp32-s3-wroom-1"}


@dataclass(frozen=True)
class ResourceDef:
    setup_line: str
    interface_import: str | None = None


@dataclass(frozen=True)
class ModuleDef:
    import_name: str
    call_line: str
    resources: tuple[str, ...]
    description: str


RESOURCE_ORDER = (
    "gnd",
    "input_pwr",
    "bat_raw_pwr",
    "mcu_pwr",
    "i2c0",
)

MODULE_RENDER_ORDER = (
    "axp2101_pmic",
    "i2c_bus",
)


RESOURCE_DEFS = {
    "gnd": ResourceDef('    gnd = Net("GND")'),
    "input_pwr": ResourceDef(
        '    input_pwr = PowerDomain(vcc_name="V_USB", gnd_net=gnd)',
        "PowerDomain",
    ),
    "bat_raw_pwr": ResourceDef(
        '    bat_raw_pwr = PowerDomain(vcc_name="BAT_RAW_VCC", gnd_net=gnd)',
        "PowerDomain",
    ),
    "mcu_pwr": ResourceDef(
        '    mcu_pwr = PowerDomain(vcc_name="V_3V3", gnd_net=gnd)',
        "PowerDomain",
    ),
    "i2c0": ResourceDef('    i2c0 = I2CBus(name="I2C0")', "I2CBus"),
}


MODULE_DEFS = {
    "axp2101_pmic": ModuleDef(
        import_name="axp2101_pmic",
        call_line="    axp2101_pmic.add_axp2101_pmic(input_pwr, bat_raw_pwr, mcu_pwr, i2c0)",
        resources=("gnd", "input_pwr", "bat_raw_pwr", "mcu_pwr", "i2c0"),
        description="Integrated AXP2101 PMIC path with charge, power-path, gauge and 3V3 rail.",
    ),
    "i2c_bus": ModuleDef(
        import_name="i2c_bus",
        call_line="    i2c_bus.add_i2c_bus(mcu_pwr, i2c0)",
        resources=("gnd", "mcu_pwr", "i2c0"),
        description="Shared I2C bus infrastructure with pull-ups and breakout header.",
    ),
}


DEFAULT_MODULES = ("axp2101_pmic", "i2c_bus")


def ordered_resources_for(modules: tuple[str, ...]) -> list[str]:
    required = {
        resource
        for module_name in modules
        for resource in MODULE_DEFS[module_name].resources
    }
    return [resource for resource in RESOURCE_ORDER if resource in required]


def ordered_modules_for(modules: tuple[str, ...]) -> list[str]:
    return [module for module in MODULE_RENDER_ORDER if module in modules]


def render_call_line(module_name: str, active_modules: tuple[str, ...]) -> str:
    if module_name == "i2c_bus" and "axp2101_pmic" in active_modules:
        return "    i2c_bus.add_i2c_bus(mcu_pwr, i2c0, pullup_value='2.2k')"
    return MODULE_DEFS[module_name].call_line


def validate_modules(modules: tuple[str, ...]):
    if not modules:
        raise ValueError("At least one module must be enabled in [composition].")

    unknown = [module for module in modules if module not in MODULE_DEFS]
    if unknown:
        supported = ", ".join(sorted(MODULE_DEFS))
        raise ValueError(
            f"Unsupported modules: {', '.join(unknown)}. Supported: {supported}."
        )

    duplicates = []
    seen = set()
    for module in modules:
        if module in seen and module not in duplicates:
            duplicates.append(module)
        seen.add(module)

    if duplicates:
        raise ValueError(f"Duplicate modules in [composition].modules: {', '.join(duplicates)}.")

    if "axp2101_pmic" in modules and "i2c_bus" not in modules:
        raise ValueError("Module 'axp2101_pmic' currently requires 'i2c_bus' for its control interface.")
