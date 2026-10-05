#!/usr/bin/env python3
"""svd/ATSAMC21G17A.svd out of this package's chip.svd (the ATSAMC21E17A).

The E and G parts of the SAM C21 are one die in two packages: the 48-pin G bonds PORTB and adds
SERCOM4, SERCOM5 and CAN1 (SAM C20/C21 data sheet DS60001479M, Table 1-3 "SAM C21 Family
Features": SERCOM 6 vs 4, CAN 2 vs 1; Table 6-2 note 4: "SERCOM4 and SERCOM5 are not supported on
SAM C21E"). Everything else - register layouts, field names, and the hand-made write semantics
in chip.svd (the `<!-- Kvasir: ... -->` edits) - is shared, so the G file is chip.svd with those
three peripherals and their bits put in, rather than Microchip's own ATSAMC21G17A.svd, whose
names the shared drivers are not written against (SERCOM_USART vs USART_INT/USART_EXT, DMAC
trigger names).

    scripts/make_g17a_svd.py           write svd/ATSAMC21G17A.svd
    scripts/make_g17a_svd.py --check   exit 1 if the checked-in file is not what chip.svd gives

Run it after every edit of chip.svd.
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "svd" / "ATSAMC21G17A.svd"
DS = "DS60001479M"

# (name, derivedFrom, base address, interrupt, NVIC line) - Table 10-3 "Interrupt Line Mapping,
# SAM C21" (md l.2181): SERCOM4 13, SERCOM5 14, CAN1 16 ("only for SAM C21 G/J/N"). Addresses:
# the MCLK enable table in src/chip/MCLK.hpp and Microchip's ATSAMC21G17A.svd (SERCOM4 0x42001400,
# SERCOM5 0x42001800, CAN1 0x42002000).
PERIPHERALS = [
    ("SERCOM4", "SERCOM0", "0x42001400", "SERCOM3", 13),
    ("SERCOM5", "SERCOM0", "0x42001800", "SERCOM4", 14),
    ("CAN1", "CAN", "0x42002000", "CAN", 16),
]

# (peripheral, register, field to copy, new field name, new description, bit, source note)
FIELDS = [
    ("MCLK", "AHBMASK", "CAN0_", "CAN1_",
     "CAN1 AHB Clock Mask", 9, "MCLK AHBMASK l.7166"),
    ("MCLK", "APBCMASK", "SERCOM3_", "SERCOM4_",
     "SERCOM4 APB Clock Enable", 5, "MCLK APBCMASK l.7635"),
    ("MCLK", "APBCMASK", "SERCOM4_", "SERCOM5_",
     "SERCOM5 APB Clock Enable", 6, "MCLK APBCMASK l.7626"),
    ("PAC", "INTFLAGC", "SERCOM3_", "SERCOM4_",
     "SERCOM4", 5, "PAC INTFLAGC l.3113"),
    ("PAC", "INTFLAGC", "SERCOM4_", "SERCOM5_",
     "SERCOM5", 6, "PAC INTFLAGC l.3113"),
    ("PAC", "INTFLAGC", "CAN0_", "CAN1_", "CAN1", 8, "PAC INTFLAGC l.3111"),
    ("PAC", "STATUSC", "SERCOM3_", "SERCOM4_",
     "SERCOM4 APB Protect Enable", 5, "PAC STATUSC l.3296"),
    ("PAC", "STATUSC", "SERCOM4_", "SERCOM5_",
     "SERCOM5 APB Protect Enable", 6, "PAC STATUSC l.3296"),
    ("PAC", "STATUSC", "CAN0_", "CAN1_",
     "CAN1 APB Protect Enable", 8, "PAC STATUSC l.3294"),
]

# (enum value to copy, new name, new description, value) in DMAC CHCTRLB.TRIGSRC - Table
# "TRIGSRC" md l.17144-17149.
TRIGGERS = [
    ("SERCOM3_TX", "SERCOM4_RX", "SERCOM4 RX Trigger", "0x0A"),
    ("SERCOM4_RX", "SERCOM4_TX", "SERCOM4 TX Trigger", "0x0B"),
    ("SERCOM4_TX", "SERCOM5_RX", "SERCOM5 RX Trigger", "0x0C"),
    ("SERCOM5_RX", "SERCOM5_TX", "SERCOM5 TX Trigger", "0x0D"),
    ("CAN0_DEBUG", "CAN1_DEBUG", "CAN1 Debug Trigger", "0x0F"),
]


def block(text: str, tag: str, name: str, start: int = 0, end: int | None = None) -> re.Match:
    """The first <tag ...>...</tag> element in text[start:end] whose first <name> is `name`,
    with the indentation before it and the newline after. None of these tags nest in
    themselves, so a non-greedy match is exact."""
    pattern = re.compile(rf"[ \t]*<{tag}(?: [^>]*)?>.*?</{tag}>\n", re.S)
    for m in pattern.finditer(text, start, len(text) if end is None else end):
        first = re.search(r"<name>([^<]+)</name>", m.group(0))
        if first and first.group(1) == name:
            return m
    raise SystemExit(f"no <{tag}> {name}")


def make(svd: str) -> str:
    # a part's own name - in front of everything else, it names the generated headers' part
    svd, n = re.subn(r"<name>ATSAMC21E17A</name>",
                     "<name>ATSAMC21G17A</name>", svd, count=1)
    assert n == 1
    svd, n = re.subn(
        r"<description>[^<]*</description>",
        "<description>Microchip ATSAMC21G17A: Cortex-M0+ microcontroller with 128KB flash, 16KB "
        "SRAM, 48-pin package. Built by scripts/make_g17a_svd.py from chip.svd (ATSAMC21E17A) "
        "with SERCOM4, SERCOM5 and CAN1.</description>",
        svd,
        count=1,
    )
    assert n == 1

    for name, base, address, after, line in PERIPHERALS:
        anchor = block(svd, "peripheral", after)
        indent = anchor.group(
            0)[: len(anchor.group(0)) - len(anchor.group(0).lstrip())]
        i = indent + "   "
        new = (f'{indent}<!-- Kvasir: {name} of the G part ({DS} Table 10-3), '
               f"scripts/make_g17a_svd.py -->\n"
               f'{indent}<peripheral derivedFrom="{base}">\n'
               f"{i}<name>{name}</name>\n"
               f"{i}<baseAddress>{address}</baseAddress>\n"
               f"{i}<interrupt>\n"
               f"{i}   <name>{name}</name>\n"
               f"{i}   <value>{line}</value>\n"
               f"{i}</interrupt>\n"
               f"{indent}</peripheral>\n")
        svd = svd[: anchor.end()] + new + svd[anchor.end():]

    for periph, reg, src, name, desc, bit, note in FIELDS:
        p = block(svd, "peripheral", periph)
        r = block(svd, "register", reg, p.start(), p.end())
        f = block(svd, "field", src, r.start(), r.end())
        # a copy of a field this script put in carries that field's note: drop it
        new = re.sub(
            r"[ \t]*<!-- Kvasir: G part only[^>]*-->\n", "", f.group(0))
        new = new.replace(f"<name>{src}</name>", f"<name>{name}</name>", 1)
        new = re.sub(r"<description>[^<]*</description>", f"<description>{desc}</description>",
                     new, count=1)
        new = re.sub(r"<bitOffset>\d+</bitOffset>", f"<bitOffset>{bit}</bitOffset>", new,
                     count=1)
        indent = new[: len(new) - len(new.lstrip())] + "   "
        new = new.replace(
            f"<name>{name}</name>\n",
            f"<name>{name}</name>\n{indent}<!-- Kvasir: G part only, {DS} {note}, "
            f"scripts/make_g17a_svd.py -->\n", 1)
        svd = svd[: f.end()] + new + svd[f.end():]

    for src, name, desc, value in TRIGGERS:
        p = block(svd, "peripheral", "DMAC")
        r = block(svd, "register", "CHCTRLB", p.start(), p.end())
        e = block(svd, "enumeratedValue", src, r.start(), r.end())
        new = e.group(0)
        new = new.replace(f"<name>{src}</name>", f"<name>{name}</name>", 1)
        new = re.sub(r"<description>[^<]*</description>", f"<description>{desc}</description>",
                     new, count=1)
        new = re.sub(r"<value>[^<]*</value>",
                     f"<value>{value}</value>", new, count=1)
        svd = svd[: e.end()] + new + svd[e.end():]
    return svd


def main() -> None:
    args = sys.argv[1:]
    if args not in ([], ["--check"]):
        raise SystemExit(__doc__)
    svd = make((ROOT / "chip.svd").read_text(encoding="utf-8"))
    if args == ["--check"]:
        if not OUT.exists() or OUT.read_text(encoding="utf-8") != svd:
            raise SystemExit(f"{OUT} is stale: run scripts/make_g17a_svd.py")
        print(f"{OUT}: up to date")
        return
    OUT.parent.mkdir(exist_ok=True)
    OUT.write_text(svd, encoding="utf-8")
    print(f"{OUT}: {len(PERIPHERALS)} peripherals, {len(FIELDS)} fields, "
          f"{len(TRIGGERS)} DMAC triggers in")


if __name__ == "__main__":
    main()
