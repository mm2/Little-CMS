#!/usr/bin/env python3
"""Reduce a hybrid ICC profile to a small test fixture by subsampling its CLUTs.

Every CLUT is moved onto a coarser grid whose nodes coincide exactly with
existing nodes, so each retained value is an original value copied verbatim.
Also applies the de-branding metadata edits for the fixture.

Pipeline (iccToXml and iccFromXml come from the DemoIccMAX tools):

    iccToXml   HybridPrinterCMYK.icc       full.xml
    python     subsample_clut.py full.xml  small.xml
    iccFromXml small.xml                   HybridPrinterCMYK_small.icc

Design notes:

  * TableData is handled as whitespace-separated *tokens*, never parsed into
    numbers. Bit-exact retention is therefore structural: no float formatting
    or requantization can creep in.
  * Target grid is chosen by input dimensionality: 4-input CLUTs go to 5
    points per axis, 3-input to 9. Stride is (G-1)/(Gn-1) and must divide
    exactly, which is asserted rather than assumed.
  * The metadata edits de-brand the fixture so it carries no vendor identity:
    creator 'RICC', copyright "Copyright 2026 ICC.", preferred CMM zero, the
    profile description set to the output filename, and the
    UseWithEmbeddedDataOnly header flag cleared.
  * The resulting fixture is structurally faithful -- every retained value is
    an original node value -- but far too coarse to be colorimetrically
    useful. Its ICC.1 and embedded ICC.2 renderings no longer track each
    other. It exists to exercise readers, not to profile a device.
"""

import re
import sys

TARGET_BY_INPUTS = {4: 5, 3: 9}

# Number of CLUTs the source profile is known to contain
EXPECTED_CLUTS = 8

CHANNELS_RE = re.compile(r'<Channels\s+InputChannels="(\d+)"\s+OutputChannels="(\d+)"\s*/>')

CLUT_RE = re.compile(
    r'<CLUT>\s*<GridPoints>([^<]*)</GridPoints>\s*<TableData>(.*?)</TableData>\s*</CLUT>',
    re.DOTALL)

EXTCLUT_RE = re.compile(
    r'(<ExtCLutElement\b[^>]*>)\s*<GridPoints>([^<]*)</GridPoints>'
    r'\s*<TableData>(.*?)</TableData>',
    re.DOTALL)

OUT_ATTR_RE = re.compile(r'OutputChannels="(\d+)"')


def fail(msg):
    raise SystemExit("subsample_clut: " + msg)


def subsample(tokens, grid, out, new_g):
    """Pick the tokens for a grid reduced from `grid` to `new_g` points per axis.

    Returns (list of per-point token lists, stride).
    """
    dims = len(grid)
    g = grid[0]

    if any(x != g for x in grid):
        fail("non-uniform grid %s is not supported" % (grid,))
    if new_g < 2 or (g - 1) % (new_g - 1):
        fail("stride from %d to %d points per axis is not an integer" % (g, new_g))
    stride = (g - 1) // (new_g - 1)

    expected = out
    for x in grid:
        expected *= x
    if len(tokens) != expected:
        fail("TableData holds %d values, expected %d (grid %s, %d out)"
             % (len(tokens), expected, grid, out))

    points = []

    def walk(depth, flat):
        # `flat` accumulates the ORIGINAL flat point index, first channel
        # varying least rapidly (Horner over the original grid size)
        if depth == dims:
            points.append(tokens[flat * out: flat * out + out])
            return
        for j in range(new_g):
            walk(depth + 1, flat * g + j * stride)

    walk(0, 0)
    return points, stride


def render(points, indent):
    return "\n".join(indent + " ".join(p) for p in points)


def replace_exactly(text, old, new, count, what):
    found = text.count(old)
    if found != count:
        fail("expected %d occurrence(s) of %s, found %d" % (count, what, found))
    return text.replace(old, new)


def edit_metadata(text):
    text = replace_exactly(text,
                           "<PreferredCMMType>ONYX</PreferredCMMType>",
                           "<PreferredCMMType></PreferredCMMType>",
                           2, "PreferredCMMType")

    text = replace_exactly(text,
                           "<ProfileCreator>ONYX</ProfileCreator>",
                           "<ProfileCreator>RICC</ProfileCreator>",
                           2, "ProfileCreator")

    text = replace_exactly(text,
                           'UseWithEmbeddedDataOnly="true"',
                           'UseWithEmbeddedDataOnly="false"',
                           1, "UseWithEmbeddedDataOnly")

    # The copyright is hex encoded in the source; rewrite the whole
    # LocalizedText element in the CDATA form used by the description
    m = re.search(r'(<copyrightTag>.*?)<LocalizedText\b.*?</LocalizedText>'
                  r'(.*?</copyrightTag>)', text, re.DOTALL)
    if m is None:
        fail("could not locate the copyrightTag LocalizedText element")
    text = (text[:m.start()]
            + m.group(1)
            + '<LocalizedText LanguageCountry="enUS">'
              '<![CDATA[Copyright 2026 ICC.]]></LocalizedText>'
            + m.group(2)
            + text[m.end():])

    text = replace_exactly(text,
                           "<![CDATA[CMYK_Hybrid_Profile.icc]]>",
                           "<![CDATA[HybridPrinterCMYK_small.icc]]>",
                           1, "profileDescriptionTag text")
    return text


def reduce_cluts(text):
    report = []

    # OutputChannels for a bare <CLUT> comes from the nearest preceding
    # <Channels .../> of its enclosing lutAtoBType / lutBtoAType
    channels = [(m.start(), int(m.group(2))) for m in CHANNELS_RE.finditer(text)]

    def out_before(pos):
        best = None
        for start, out in channels:
            if start < pos:
                best = out
            else:
                break
        if best is None:
            fail("no <Channels> element precedes the CLUT at offset %d" % pos)
        return best

    def target_for(grid):
        new_g = TARGET_BY_INPUTS.get(len(grid))
        if new_g is None:
            fail("no target grid defined for %d input channels" % len(grid))
        return new_g

    def do_clut(m):
        grid = [int(x) for x in m.group(1).split()]
        tokens = m.group(2).split()
        out = out_before(m.start())
        new_g = target_for(grid)
        points, stride = subsample(tokens, grid, out, new_g)
        report.append(("CLUT", len(grid), out, grid[0], new_g, stride, len(points)))
        return ('<CLUT>\n'
                '        <GridPoints>%s</GridPoints>\n'
                '        <TableData>\n%s\n'
                '        </TableData>\n'
                '      </CLUT>'
                % (" ".join([str(new_g)] * len(grid)), render(points, "          ")))

    def do_extclut(m):
        opening = m.group(1)
        grid = [int(x) for x in m.group(2).split()]
        tokens = m.group(3).split()
        mo = OUT_ATTR_RE.search(opening)
        if mo is None:
            fail("ExtCLutElement has no OutputChannels attribute")
        out = int(mo.group(1))
        new_g = target_for(grid)
        points, stride = subsample(tokens, grid, out, new_g)
        report.append(("ExtCLutElement", len(grid), out, grid[0], new_g, stride, len(points)))
        return ('%s\n'
                '                <GridPoints>%s</GridPoints>\n'
                '                <TableData>\n%s\n'
                '                </TableData>'
                % (opening, " ".join([str(new_g)] * len(grid)),
                   render(points, "                  ")))

    # <CLUT> first, scanning the original text so out_before offsets stay valid
    text = CLUT_RE.sub(do_clut, text)
    text = EXTCLUT_RE.sub(do_extclut, text)
    return text, report


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: subsample_clut.py <in.xml> <out.xml>")

    src, dst = sys.argv[1], sys.argv[2]

    with open(src, encoding="utf-8") as f:
        text = f.read()

    text = edit_metadata(text)
    text, report = reduce_cluts(text)

    if not report:
        fail("no CLUTs were found, nothing was reduced")

    # HybridPrinterCMYK.icc has exactly this many reducible tables: a partial regex
    # match that quietly skipped some would otherwise produce a fixture that still
    # carries full size CLUTs
    if len(report) != EXPECTED_CLUTS:
        fail("reduced %d CLUTs, expected %d - the regexes probably matched partially"
             % (len(report), EXPECTED_CLUTS))

    with open(dst, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)

    print("%-16s %6s %5s %8s %8s %7s %8s"
          % ("element", "inputs", "out", "oldgrid", "newgrid", "stride", "points"))
    for row in report:
        print("%-16s %6d %5d %8d %8d %7d %8d" % row)
    print("\nwrote %s" % dst)


if __name__ == "__main__":
    main()
