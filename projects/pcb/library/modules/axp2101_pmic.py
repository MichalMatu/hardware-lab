from __future__ import annotations

import builtins

from skidl import Net, Part, Pin, SKIDL, TEMPLATE

from library.interfaces import I2CBus, PowerDomain

R_0603 = "Resistor_SMD:R_0603_1608Metric"
C_0603 = "Capacitor_SMD:C_0603_1608Metric"
C_0805 = "Capacitor_SMD:C_0805_2012Metric"
C_1206 = "Capacitor_SMD:C_1206_3216Metric"
L_1210 = "Inductor_SMD:L_1210_3225Metric"
AXP2101_FOOTPRINT = "Package_DFN_QFN:VQFN-40-1EP_5x5mm_P0.4mm_EP3.5x3.5mm"

DEFAULT_BAT_CAP = "1uF"
DEFAULT_SYS_CAP = "22uF"
DEFAULT_VBUS_BULK_CAP = "10uF"
DEFAULT_VBUS_HF_CAP = "2.2uF"
DEFAULT_VREF_CAP = "1uF"
DEFAULT_VIN_CAP = "2.2uF"
DEFAULT_VMID_CAP = "2.2uF"
DEFAULT_VRTC_CAP = "2.2uF"
DEFAULT_ALDOIN_CAP = "2.2uF"
DEFAULT_BLDOIN_CAP = "2.2uF"
DEFAULT_AUX_LDO_CAP = "2.2uF"
DEFAULT_DCDC1_INDUCTOR = "1uH"
DEFAULT_AUX_DCDC_INDUCTOR = "1uH"
DEFAULT_SW_INDUCTOR = "1uH"
DEFAULT_DCDC1_OUT_CAP = "22uF"
DEFAULT_IRQ_PULLUP = "4.7k"
DEFAULT_I2C_PULLUP = "2.2k"
DEFAULT_TS_PULLDOWN = "10k"


def _axp2101_template():
    return Part(
        tool=SKIDL,
        name="AXP2101",
        footprint=AXP2101_FOOTPRINT,
        dest=TEMPLATE,
        pins=[
            Pin(num=1, name="CHGLED", func=Pin.types.OPENCOLL),
            Pin(num=2, name="VREF", func=Pin.types.PWROUT),
            Pin(num=3, name="GND", func=Pin.types.PWRIN),
            Pin(num=4, name="FB3", func=Pin.types.INPUT),
            Pin(num=5, name="LX3", func=Pin.types.PASSIVE),
            Pin(num=6, name="VIN3", func=Pin.types.PWRIN),
            Pin(num=7, name="VIN4", func=Pin.types.PWRIN),
            Pin(num=8, name="LX4", func=Pin.types.PASSIVE),
            Pin(num=9, name="FB4", func=Pin.types.INPUT),
            Pin(num=10, name="CPULDOS", func=Pin.types.PWROUT),
            Pin(num=11, name="DLDO2/DC4SW", func=Pin.types.PWROUT),
            Pin(num=12, name="BLDO1", func=Pin.types.PWROUT),
            Pin(num=13, name="BLDOIN", func=Pin.types.PWRIN),
            Pin(num=14, name="BLDO2", func=Pin.types.PWROUT),
            Pin(num=15, name="ALDO4", func=Pin.types.PWROUT),
            Pin(num=16, name="ALDO3", func=Pin.types.PWROUT),
            Pin(num=17, name="ALDOIN", func=Pin.types.PWRIN),
            Pin(num=18, name="ALDO1", func=Pin.types.PWROUT),
            Pin(num=19, name="ALDO2", func=Pin.types.PWROUT),
            Pin(num=20, name="DLDO1/DC1SW", func=Pin.types.PWROUT),
            Pin(num=21, name="FB1", func=Pin.types.INPUT),
            Pin(num=22, name="LX1", func=Pin.types.PASSIVE),
            Pin(num=23, name="VIN1", func=Pin.types.PWRIN),
            Pin(num=24, name="VIN2", func=Pin.types.PWRIN),
            Pin(num=25, name="LX2", func=Pin.types.PASSIVE),
            Pin(num=26, name="FB2", func=Pin.types.INPUT),
            Pin(num=27, name="VBackup", func=Pin.types.PWRIN),
            Pin(num=28, name="VRTC", func=Pin.types.PWROUT),
            Pin(num=29, name="PWROK", func=Pin.types.OUTPUT),
            Pin(num=30, name="PWRON", func=Pin.types.INPUT),
            Pin(num=31, name="TS", func=Pin.types.INPUT),
            Pin(num=32, name="GPIO1/FB5/RTCLDO2", func=Pin.types.BIDIR),
            Pin(num=33, name="BAT", func=Pin.types.PWRIN),
            Pin(num=34, name="VSYS", func=Pin.types.PWROUT),
            Pin(num=35, name="SW", func=Pin.types.PASSIVE),
            Pin(num=36, name="VMID", func=Pin.types.PWROUT),
            Pin(num=37, name="VBUS", func=Pin.types.PWRIN),
            Pin(num=38, name="IRQ", func=Pin.types.OPENCOLL),
            Pin(num=39, name="SDA", func=Pin.types.BIDIR),
            Pin(num=40, name="SCK", func=Pin.types.INPUT),
            Pin(num=41, name="EP", func=Pin.types.PWRIN),
        ],
    )


def _tie_optional(pin, net: Net | None):
    if net is None:
        pin += builtins.NC
        return
    pin += net


def _add_cap(ref: str, value: str, net: Net, gnd: Net, *, footprint: str = C_0603):
    cap = Part("Device", "C", value=value, ref=ref, footprint=footprint)
    net += cap[1]
    gnd += cap[2]
    return cap


def _add_buck_output(
    pmic: Part,
    *,
    lx_pin: str,
    fb_pin: str,
    output_name: str,
    output_net: Net | None = None,
    inductor_ref: str,
    cap_ref: str,
    inductor_value: str,
    output_cap: str,
    gnd: Net,
    inductor_footprint: str = L_1210,
    cap_footprint: str = C_1206,
):
    sw = Net(f"{output_name}_SW")
    out = output_net or Net(output_name)
    ind = Part("Device", "L", value=inductor_value, ref=inductor_ref, footprint=inductor_footprint)
    cap = Part("Device", "C", value=output_cap, ref=cap_ref, footprint=cap_footprint)
    pmic[lx_pin] += sw
    sw += ind[1]
    out += ind[2], cap[1], pmic[fb_pin]
    gnd += cap[2]
    return out


def _mark_unused_buck(pmic: Part, *, lx_pin: str, fb_pin: str):
    pmic[lx_pin] += builtins.NC
    pmic[fb_pin] += builtins.NC


def _add_optional_buck_output(
    pmic: Part,
    *,
    lx_pin: str,
    fb_pin: str,
    output_name: str | None,
    output_net: Net | None = None,
    inductor_ref: str,
    cap_ref: str,
    inductor_value: str,
    output_cap: str,
    gnd: Net,
    inductor_footprint: str = L_1210,
    cap_footprint: str = C_1206,
):
    if output_name is None:
        _mark_unused_buck(pmic, lx_pin=lx_pin, fb_pin=fb_pin)
        return None

    return _add_buck_output(
        pmic,
        lx_pin=lx_pin,
        fb_pin=fb_pin,
        output_name=output_name,
        output_net=output_net,
        inductor_ref=inductor_ref,
        cap_ref=cap_ref,
        inductor_value=inductor_value,
        output_cap=output_cap,
        gnd=gnd,
        inductor_footprint=inductor_footprint,
        cap_footprint=cap_footprint,
    )


def _add_ldo_output(pmic: Part, *, pin_name: str, output_name: str, cap_ref: str, cap_value: str, gnd: Net):
    out = Net(output_name)
    pmic[pin_name] += out
    _add_cap(cap_ref, cap_value, out, gnd)
    return out


def _add_optional_ldo_output(
    pmic: Part,
    *,
    pin_name: str,
    output_name: str | None,
    cap_ref: str,
    cap_value: str,
    gnd: Net,
):
    if output_name is None:
        pmic[pin_name] += builtins.NC
        return None

    return _add_ldo_output(
        pmic,
        pin_name=pin_name,
        output_name=output_name,
        cap_ref=cap_ref,
        cap_value=cap_value,
        gnd=gnd,
    )


def add_axp2101_pmic(
    input_pwr: PowerDomain,
    bat_pwr: PowerDomain,
    main_pwr: PowerDomain,
    i2c: I2CBus,
    *,
    irq: Net | None = None,
    pwrok: Net | None = None,
    chgled: Net | None = None,
    pwron: Net | None = None,
    ts: Net | None = None,
    vbackup: Net | None = None,
    add_local_i2c_pullups: bool = False,
    tie_vbackup_to_vrtc: bool = True,
    irq_pullup: str = DEFAULT_IRQ_PULLUP,
    irq_pullup_rail: Net | None = None,
    i2c_pullup: str = DEFAULT_I2C_PULLUP,
    ts_pull_down: str | None = DEFAULT_TS_PULLDOWN,
    bat_cap: str = DEFAULT_BAT_CAP,
    sys_cap: str = DEFAULT_SYS_CAP,
    vbus_bulk_cap: str = DEFAULT_VBUS_BULK_CAP,
    vbus_hf_cap: str = DEFAULT_VBUS_HF_CAP,
    vref_cap: str = DEFAULT_VREF_CAP,
    vin_cap: str = DEFAULT_VIN_CAP,
    vmid_cap: str = DEFAULT_VMID_CAP,
    vrtc_cap: str = DEFAULT_VRTC_CAP,
    aldoin_cap: str = DEFAULT_ALDOIN_CAP,
    bldoin_cap: str = DEFAULT_BLDOIN_CAP,
    aux_ldo_cap: str = DEFAULT_AUX_LDO_CAP,
    dcdc1_inductor: str = DEFAULT_DCDC1_INDUCTOR,
    aux_dcdc_inductor: str = DEFAULT_AUX_DCDC_INDUCTOR,
    sw_inductor: str | None = DEFAULT_SW_INDUCTOR,
    dcdc1_out_cap: str = DEFAULT_DCDC1_OUT_CAP,
    dcdc1_net_name: str = "PMIC_DCDC1",
    dcdc1_output_net: Net | None = None,
    dcdc2_output: str | None = None,
    dcdc3_output: str | None = None,
    dcdc4_output: str | None = None,
    aldo1_output: str | None = None,
    aldo2_output: str | None = None,
    aldo3_output: str | None = None,
    aldo4_output: str | None = None,
    bldo1_output: str | None = None,
    bldo2_output: str | None = None,
    cpuldos_output: str | None = None,
    dldo1_output: str | None = None,
    dldo2_output: str | None = None,
):
    """
    Add an AXP2101 PMIC block for a 1S Li-Ion board.

    Reference-derived default scope of this module:
    - models the primary power path pins (`VBUS`, `BAT`, `VSYS`);
    - models the host control/telemetry interface (`SDA`, `SCK`, `IRQ`);
    - models the critical local passives seen across the design guide and
      the local M5/Core2 reference schematics;
    - exposes `DCDC1` as the main `3V3` rail for the MCU domain;
    - keeps auxiliary `DCDC`/`LDO` outputs opt-in instead of assuming the
      whole PMIC power tree is always populated.

    Important limitations:
    - no USB/solar source mux is included here; the PMIC has a single input;
    - `PWRON`/`PWROK` RC/button networks stay board-level and are not modeled
      inside this PMIC helper;
    - `DLDO1/DC1SW`, `DLDO2/DC4SW`, and `CPULDOS` still depend on the selected
      PMIC mode/config and should be enabled explicitly when used;
    - startup defaults still depend on the actual factory/custom configuration
      of the PMIC;
    - dedicated battery protection is still a separate architectural choice.
    """
    if bat_pwr.gnd is not input_pwr.gnd or main_pwr.gnd is not input_pwr.gnd:
        raise ValueError(
            "AXP2101 skeleton currently assumes shared ground between input, "
            "battery and main output domains."
        )

    v_in = input_pwr.vcc
    v_bat = bat_pwr.vcc
    v_main = main_pwr.vcc
    gnd = input_pwr.gnd

    pmic = _axp2101_template()(ref="U_PMIC")
    v_sys = Net("PMIC_VSYS")
    irq_net = irq or Net("AXP_IRQ")
    ts_net = ts or Net("BAT_TS")
    pwrok_net = pwrok or Net("AXP_PWROK")
    chgled_net = chgled or Net("AXP_CHGLED")
    pwron_net = pwron or Net("AXP_PWRON")
    vref = Net("PMIC_VREF")
    vmid = Net("PMIC_VMID")
    vrtc = Net("PMIC_VRTC")

    pmic["GND"] += gnd
    pmic["EP"] += gnd
    pmic["VBUS"] += v_in
    pmic["BAT"] += v_bat
    pmic["VSYS"] += v_sys
    pmic["VIN1"] += v_sys
    pmic["VIN2"] += v_sys
    pmic["VIN3"] += v_sys
    pmic["VIN4"] += v_sys
    pmic["ALDOIN"] += v_sys
    pmic["BLDOIN"] += v_sys
    pmic["SDA"] += i2c.sda
    pmic["SCK"] += i2c.scl
    pmic["TS"] += ts_net
    pmic["IRQ"] += irq_net
    pmic["VREF"] += vref
    pmic["VMID"] += vmid
    pmic["VRTC"] += vrtc

    pmic["PWROK"] += pwrok_net
    pmic["CHGLED"] += chgled_net
    pmic["PWRON"] += pwron_net

    # Keep the complex DCDC5 / RTCLDO2 mux path explicit as not-yet-modeled,
    # because pin 32 multiplexing depends on the chosen PMIC configuration.
    for unused_pin in ("GPIO1/FB5/RTCLDO2",):
        pmic[unused_pin] += builtins.NC

    if vbackup is not None:
        pmic["VBackup"] += vbackup
        vbackup_net = vbackup
    elif tie_vbackup_to_vrtc:
        pmic["VBackup"] += vrtc
        vbackup_net = vrtc
    else:
        pmic["VBackup"] += builtins.NC
        vbackup_net = None

    if sw_inductor:
        sw_net = Net("PMIC_SW")
        sw_link = Part("Device", "L", value=sw_inductor, ref="L_PMIC_SW", footprint=L_1210)
        pmic["SW"] += sw_net
        sw_net += sw_link[1]
        v_sys += sw_link[2]
    else:
        sw_net = None
        pmic["SW"] += builtins.NC

    _add_cap("C_PMIC_VREF", vref_cap, vref, gnd)
    _add_cap("C_PMIC_VBUSA", vbus_bulk_cap, v_in, gnd, footprint=C_0805)
    _add_cap("C_PMIC_VBUSB", vbus_hf_cap, v_in, gnd)

    _add_cap("C_PMIC_BAT", bat_cap, v_bat, gnd)
    _add_cap("C_PMIC_VMID", vmid_cap, vmid, gnd)
    _add_cap("C_PMIC_VRTC", vrtc_cap, vrtc, gnd)
    _add_cap("C_PMIC_VIN1", vin_cap, v_sys, gnd)
    _add_cap("C_PMIC_VIN2", vin_cap, v_sys, gnd)
    _add_cap("C_PMIC_VIN3", vin_cap, v_sys, gnd)
    _add_cap("C_PMIC_VIN4", vin_cap, v_sys, gnd)
    _add_cap("C_PMIC_ALDOIN", aldoin_cap, v_sys, gnd)
    _add_cap("C_PMIC_BLDOIN", bldoin_cap, v_sys, gnd)

    _add_cap("C_PMIC_SYSA", sys_cap, v_sys, gnd, footprint=C_1206)
    _add_cap("C_PMIC_SYSB", sys_cap, v_sys, gnd, footprint=C_1206)

    dcdc1 = _add_buck_output(
        pmic,
        lx_pin="LX1",
        fb_pin="FB1",
        output_name=dcdc1_net_name,
        output_net=dcdc1_output_net,
        inductor_ref="L_DCDC1",
        cap_ref="C_DCDC1",
        inductor_value=dcdc1_inductor,
        output_cap=dcdc1_out_cap,
        gnd=gnd,
    )
    dcdc2 = _add_optional_buck_output(
        pmic,
        lx_pin="LX2",
        fb_pin="FB2",
        output_name=dcdc2_output,
        inductor_ref="L_DCDC2",
        cap_ref="C_DCDC2",
        inductor_value=aux_dcdc_inductor,
        output_cap=dcdc1_out_cap,
        gnd=gnd,
    )
    dcdc3 = _add_optional_buck_output(
        pmic,
        lx_pin="LX3",
        fb_pin="FB3",
        output_name=dcdc3_output,
        inductor_ref="L_DCDC3",
        cap_ref="C_DCDC3",
        inductor_value=aux_dcdc_inductor,
        output_cap=dcdc1_out_cap,
        gnd=gnd,
    )
    dcdc4 = _add_optional_buck_output(
        pmic,
        lx_pin="LX4",
        fb_pin="FB4",
        output_name=dcdc4_output,
        inductor_ref="L_DCDC4",
        cap_ref="C_DCDC4",
        inductor_value=aux_dcdc_inductor,
        output_cap=dcdc1_out_cap,
        gnd=gnd,
    )

    aldo1 = _add_optional_ldo_output(pmic, pin_name="ALDO1", output_name=aldo1_output, cap_ref="C_ALDO1", cap_value=aux_ldo_cap, gnd=gnd)
    aldo2 = _add_optional_ldo_output(pmic, pin_name="ALDO2", output_name=aldo2_output, cap_ref="C_ALDO2", cap_value=aux_ldo_cap, gnd=gnd)
    aldo3 = _add_optional_ldo_output(pmic, pin_name="ALDO3", output_name=aldo3_output, cap_ref="C_ALDO3", cap_value=aux_ldo_cap, gnd=gnd)
    aldo4 = _add_optional_ldo_output(pmic, pin_name="ALDO4", output_name=aldo4_output, cap_ref="C_ALDO4", cap_value=aux_ldo_cap, gnd=gnd)
    bldo1 = _add_optional_ldo_output(pmic, pin_name="BLDO1", output_name=bldo1_output, cap_ref="C_BLDO1", cap_value=aux_ldo_cap, gnd=gnd)
    bldo2 = _add_optional_ldo_output(pmic, pin_name="BLDO2", output_name=bldo2_output, cap_ref="C_BLDO2", cap_value=aux_ldo_cap, gnd=gnd)
    cpuldos = _add_optional_ldo_output(pmic, pin_name="CPULDOS", output_name=cpuldos_output, cap_ref="C_CPULDOS", cap_value=aux_ldo_cap, gnd=gnd)
    dldo1 = _add_optional_ldo_output(pmic, pin_name="DLDO1/DC1SW", output_name=dldo1_output, cap_ref="C_DLDO1", cap_value=aux_ldo_cap, gnd=gnd)
    dldo2 = _add_optional_ldo_output(pmic, pin_name="DLDO2/DC4SW", output_name=dldo2_output, cap_ref="C_DLDO2", cap_value=aux_ldo_cap, gnd=gnd)

    if dcdc1 is not v_main:
        v_main += dcdc1

    if irq_pullup:
        irq_pullup_source = irq_pullup_rail or vbackup_net or vrtc
        r_irq = Part("Device", "R", value=irq_pullup, ref="R_PMIC_IRQ", footprint=R_0603)
        irq_pullup_source += r_irq[1]
        irq_net += r_irq[2]

    if ts is None and ts_pull_down:
        r_ts = Part("Device", "R", value=ts_pull_down, ref="R_PMIC_TS", footprint=R_0603)
        ts_net += r_ts[1]
        gnd += r_ts[2]

    if add_local_i2c_pullups:
        r_sda = Part("Device", "R", value=i2c_pullup, ref="R_PMIC_SDA", footprint=R_0603)
        r_sck = Part("Device", "R", value=i2c_pullup, ref="R_PMIC_SCK", footprint=R_0603)
        v_main += r_sda[1], r_sck[1]
        i2c.sda += r_sda[2]
        i2c.scl += r_sck[2]

    return {
        "irq": irq_net,
        "ts": ts_net,
        "vsys": v_sys,
        "pwron": pwron_net,
        "pwrok": pwrok_net,
        "chgled": chgled_net,
        "vref": vref,
        "vmid": vmid,
        "vrtc": vrtc,
        "vbackup": vbackup_net,
        "sw": sw_net,
        "dcdc1": dcdc1,
        "dcdc2": dcdc2,
        "dcdc3": dcdc3,
        "dcdc4": dcdc4,
        "aldo1": aldo1,
        "aldo2": aldo2,
        "aldo3": aldo3,
        "aldo4": aldo4,
        "bldo1": bldo1,
        "bldo2": bldo2,
        "cpuldos": cpuldos,
        "dldo1": dldo1,
        "dldo2": dldo2,
    }
