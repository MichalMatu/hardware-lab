import os
import sys
from pathlib import Path
from types import SimpleNamespace


def _find_repo_root(start: Path) -> Path:
    for candidate in (start, *start.parents):
        if (candidate / "library" / "interfaces.py").exists():
            return candidate
    raise RuntimeError("Unable to find repo root containing library/interfaces.py")


project_root = Path(__file__).resolve().parents[1]
repo_root = _find_repo_root(project_root)
if str(project_root) not in sys.path:
    sys.path.insert(0, str(project_root))
if str(repo_root) not in sys.path:
    sys.path.insert(0, str(repo_root))

from config.config import prepare_kicad_env, setup_skidl

os.environ.setdefault("XDG_DATA_HOME", str(project_root / ".skidl"))
os.environ.setdefault("MPLCONFIGDIR", str(project_root / ".mplconfig"))
os.chdir(project_root)

prepare_kicad_env()

from skidl import *
from library.modules import axp2101_pmic, i2c_bus

SOCK_1X22 = "Connector_PinSocket_2.54mm:PinSocket_1x22_P2.54mm_Vertical"
BAT_JST = "Connector_JST:JST_PH_S2B-PH-K_1x02_P2.00mm_Horizontal"
QWIIC_JST = "Connector_JST:JST_SH_SM04B-SRSS-TB_1x04-1MP_P1.00mm_Horizontal"
TP_PAD = "TestPoint:TestPoint_Pad_D1.0mm"
LED_0603 = "LED_SMD:LED_0603_1608Metric"
R_0603 = "Resistor_SMD:R_0603_1608Metric"
C_0603 = "Capacitor_SMD:C_0603_1608Metric"
SW_PUSH = "Button_Switch_SMD:SW_SPST_TL3342"
Q_NMOS_SOT23 = "Package_TO_SOT_SMD:SOT-23"
LOAD_SWITCH_SOT23_6 = "Package_TO_SOT_SMD:SOT-23-6"

LEFT_HOST_PINS = [
    ("ESP_3V3", "3V3"),
    ("GND", "GND"),
    ("ESP_EN", "EN"),
    ("ESP_GPIO21_SDA", "SDA"),
    ("ESP_GPIO22_SCL", "SCL"),
    ("ESP_GPIO32", "IRQ"),
    ("ESP_GPIO33", "PWROK"),
    ("ESP_GPIO25", "CHGLED"),
    ("ESP_GPIO27", "PWRON"),
    ("ESP_GPIO_L10", "L10"),
    ("ESP_GPIO_L11", "L11"),
    ("ESP_GPIO_L12", "L12"),
    ("ESP_GPIO_L13", "L13"),
    ("ESP_GPIO_L14", "L14"),
    ("ESP_GPIO_L15", "L15"),
    ("ESP_GPIO_L16", "L16"),
    ("ESP_GPIO_L17", "L17"),
    ("ESP_GPIO_L18", "L18"),
    ("ESP_GPIO_L19", "L19"),
    ("ESP_GPIO_L20", "L20"),
    ("ESP_GPIO_L21", "L21"),
    ("ESP_GPIO_L22", "L22"),
]

RIGHT_HOST_PINS = [
    ("ESP_5V", "5V"),
    ("GND", "GND"),
    ("ESP_GPIO_R3", "R3"),
    ("ESP_GPIO_R4", "R4"),
    ("ESP_GPIO_R5", "R5"),
    ("ESP_GPIO_R6", "R6"),
    ("ESP_GPIO_R7", "R7"),
    ("ESP_GPIO_R8", "R8"),
    ("ESP_GPIO_R9", "R9"),
    ("ESP_GPIO_R10", "R10"),
    ("ESP_GPIO_R11", "R11"),
    ("ESP_GPIO_R12", "R12"),
    ("ESP_GPIO_R13", "R13"),
    ("ESP_GPIO_R14", "R14"),
    ("ESP_GPIO_R15", "R15"),
    ("ESP_GPIO_R16", "R16"),
    ("ESP_GPIO_R17", "R17"),
    ("ESP_GPIO_R18", "R18"),
    ("ESP_GPIO_R19", "R19"),
    ("ESP_GPIO_R20", "R20"),
    ("ESP_GPIO_R21", "R21"),
    ("ESP_GPIO_R22", "R22"),
]


def _get_or_create_net(cache: dict[str, Net], name: str) -> Net:
    net = cache.get(name)
    if net is None:
        net = Net(name)
        cache[name] = net
    return net


def _populate_header(connector: Part, pin_map: list[tuple[str, str]], nets: dict[str, Net]):
    for index, (net_name, _label) in enumerate(pin_map, start=1):
        net = _get_or_create_net(nets, net_name)
        net += connector[index]


def _make_low_side_nmos(ref: str, value: str, tag: str):
    return Part(
        tool=SKIDL,
        name="LOW_SIDE_NMOS",
        ref=ref,
        value=value,
        footprint=Q_NMOS_SOT23,
        tag=tag,
        pins=[
            Pin(num="1", name="G", func=Pin.types.INPUT),
            Pin(num="2", name="S", func=Pin.types.PASSIVE),
            Pin(num="3", name="D", func=Pin.types.PASSIVE),
        ],
    )


def _add_status_led(net: Net, rail: Net, ref_led: str, ref_res: str, label: str, gnd: Net):
    ref_suffix = ref_led.removeprefix("D_")
    gate = Net(f"{ref_suffix}_LED_GATE")

    led = Part("Device", "LED", value=label, ref=ref_led, footprint=LED_0603)
    res = Part("Device", "R", value="1k", ref=ref_res, footprint=R_0603)
    q_led = _make_low_side_nmos(f"Q_{ref_suffix}", "2N7002", f"{ref_suffix.lower()}_led_switch")
    r_gate = Part(
        "Device",
        "R",
        value="100R",
        ref=f"R_{ref_suffix}_G",
        footprint=R_0603,
        tag=f"{ref_suffix.lower()}_led_gate_series",
    )
    r_gate_pd = Part(
        "Device",
        "R",
        value="100k",
        ref=f"R_{ref_suffix}_PD",
        footprint=R_0603,
        tag=f"{ref_suffix.lower()}_led_gate_pulldown",
    )

    rail += res[1]
    res[2] += led["A"]
    led["K"] += q_led[3]

    net += r_gate[1]
    gate += r_gate[2], q_led[1], r_gate_pd[1]
    gnd += q_led[2], r_gate_pd[2]


def _add_pwron_button(pwron: Net, pwron_btn: Net, gnd: Net):
    r_pwron = Part("Device", "R", value="510R", ref="R_PWRON", footprint=R_0603)
    c_pwron = Part("Device", "C", value="1nF", ref="C_PWRON", footprint=C_0603)
    sw_pwron = Part("Switch", "SW_Push", ref="SW_PWRON", footprint=SW_PUSH)
    pwron += r_pwron[1], c_pwron[1]
    pwron_btn += r_pwron[2], sw_pwron[1]
    gnd += c_pwron[2], sw_pwron[2]


def _add_testpoint(net: Net, ref: str):
    tp = Part("Connector", "TestPoint", ref=ref, footprint=TP_PAD)
    net += tp[1]
    return tp


def _make_host_3v3_load_switch(ref: str, value: str, tag: str):
    return Part(
        tool=SKIDL,
        name="HOST_3V3_LOAD_SWITCH",
        ref=ref,
        value=value,
        footprint=LOAD_SWITCH_SOT23_6,
        tag=tag,
        pins=[
            Pin(num="1", name="VOUT", func=Pin.types.PASSIVE),
            Pin(num="2", name="QOD", func=Pin.types.PASSIVE),
            Pin(num="3", name="ON", func=Pin.types.INPUT),
            Pin(num="4", name="CT", func=Pin.types.PASSIVE),
            Pin(num="5", name="GND", func=Pin.types.PASSIVE),
            Pin(num="6", name="VIN", func=Pin.types.PASSIVE),
        ],
    )


def _add_host_3v3_auto_switch(hat_3v3: Net, host_3v3: Net, host_5v: Net, gnd: Net):
    ctrl = Net("HOST_3V3_SW_CTRL")

    u_host_sw = _make_host_3v3_load_switch("U_HOST_3V3_SW", "TPS22917L", "host_3v3_auto_switch")
    r_ctrl_5v = Part(
        "Device",
        "R",
        value="100R",
        ref="R_HOST_3V3_OFF",
        footprint=R_0603,
        tag="host_3v3_switch_ctrl_series",
    )
    r_ctrl_gnd = Part(
        "Device",
        "R",
        value="100k",
        ref="R_HOST_3V3_ON",
        footprint=R_0603,
        tag="host_3v3_switch_ctrl_pulldown",
    )

    hat_3v3 += u_host_sw[6]
    host_3v3 += u_host_sw[1]
    gnd += u_host_sw[5], r_ctrl_gnd[2]

    host_5v += r_ctrl_5v[1]
    ctrl += u_host_sw[3], r_ctrl_5v[2], r_ctrl_gnd[1]


def main():
    setup_skidl()

    nets: dict[str, Net] = {}
    gnd = _get_or_create_net(nets, "GND")

    j_host_l = Part(
        "Connector_Generic",
        "Conn_01x22",
        ref="J_HOST_L",
        footprint=SOCK_1X22,
        tag="esp32_host_left",
    )
    j_host_r = Part(
        "Connector_Generic",
        "Conn_01x22",
        ref="J_HOST_R",
        footprint=SOCK_1X22,
        tag="esp32_host_right",
    )
    _populate_header(j_host_l, LEFT_HOST_PINS, nets)
    _populate_header(j_host_r, RIGHT_HOST_PINS, nets)

    host_5v = SimpleNamespace(vcc=_get_or_create_net(nets, "ESP_5V"), gnd=gnd)
    host_3v3 = SimpleNamespace(vcc=_get_or_create_net(nets, "ESP_3V3"), gnd=gnd)
    bat_pwr = SimpleNamespace(vcc=_get_or_create_net(nets, "BAT_RAW_VCC"), gnd=gnd)
    hat_pwr = SimpleNamespace(vcc=_get_or_create_net(nets, "HAT_3V3"), gnd=gnd)

    _add_host_3v3_auto_switch(hat_pwr.vcc, host_3v3.vcc, host_5v.vcc, gnd)

    i2c0 = SimpleNamespace(
        sda=_get_or_create_net(nets, "ESP_GPIO21_SDA"),
        scl=_get_or_create_net(nets, "ESP_GPIO22_SCL"),
    )

    irq = _get_or_create_net(nets, "ESP_GPIO32")
    pwrok = _get_or_create_net(nets, "ESP_GPIO33")
    chgled = _get_or_create_net(nets, "ESP_GPIO25")
    pwron_btn = _get_or_create_net(nets, "ESP_GPIO27")
    pwron = _get_or_create_net(nets, "AXP_PWRON")
    ts = _get_or_create_net(nets, "BAT_TS")

    rails = axp2101_pmic.add_axp2101_pmic(
        host_5v,
        bat_pwr,
        hat_pwr,
        i2c0,
        irq=irq,
        pwrok=pwrok,
        chgled=chgled,
        pwron=pwron,
        ts=ts,
        irq_pullup="4.7k",
        irq_pullup_rail=host_3v3.vcc,
        ts_pull_down=None,
        dcdc1_net_name=hat_pwr.vcc.name,
        dcdc1_output_net=hat_pwr.vcc,
    )
    i2c_bus.add_i2c_bus(host_3v3, i2c0, add_header=False, pullup_value="2.2k")

    _add_pwron_button(pwron, pwron_btn, gnd)
    _add_status_led(chgled, rails["vsys"], "D_CHG", "R_LED_CHG", "CHG", gnd)

    j_bat = Part(
        "Connector_Generic",
        "Conn_01x02",
        ref="J_BAT",
        footprint=BAT_JST,
        tag="battery_input",
    )
    bat_pwr.vcc += j_bat[1]
    gnd += j_bat[2]

    j_qwiic = Part(
        "Connector_Generic",
        "Conn_01x04",
        ref="J_QWIIC",
        footprint=QWIIC_JST,
        tag="qwiic_i2c",
    )
    gnd += j_qwiic[1]
    hat_pwr.vcc += j_qwiic[2]
    i2c0.sda += j_qwiic[3]
    i2c0.scl += j_qwiic[4]

    _add_testpoint(gnd, "TP_GND")
    _add_testpoint(rails["vsys"], "TP_VSYS")
    _add_testpoint(rails["vrtc"], "TP_VRTC")
    _add_testpoint(rails["vref"], "TP_VREF")
    _add_testpoint(rails["vmid"], "TP_VMID")
    _add_testpoint(ts, "TP_TS")
    _add_testpoint(rails["pwron"], "TP_PWRON")

    host_5v.vcc.drive = POWER
    host_3v3.vcc.drive = POWER
    bat_pwr.vcc.drive = POWER
    hat_pwr.vcc.drive = POWER
    gnd.drive = POWER

    out_dir = project_root / "out"
    out_dir.mkdir(parents=True, exist_ok=True)

    print("\n--- Uruchamiam ERC ---")
    ERC()
    print("--- Generuję Netlistę ---")
    generate_netlist(file_=str(out_dir / "project.net"))
    print("--- Generuję Schematic ---")
    try:
        generate_schematic(filepath=str(out_dir))
    except Exception as exc:
        print(f"--- Pomijam schematic: {exc}")
    print("--- Generuję PCB ---")
    generate_pcb(file_=str(out_dir / "project.kicad_pcb"))
    print(f"Sukces! Pliki w: {out_dir}")


if __name__ == "__main__":
    main()
