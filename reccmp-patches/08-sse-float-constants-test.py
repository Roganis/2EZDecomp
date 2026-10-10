

# --- patch 08: SSE scalar float constants -----------------------------------
# Appended to tests/test_analysis_float_const.py. Verified to FAIL without the
# patch (find_sse_float_instructions_in_buffer does not exist) and pass with it.
# It needs no sample binary, unlike the two tests above it in this module.

from reccmp.analysis.float_const import find_sse_float_instructions_in_buffer


def test_sse_scalar_single():
    """movss/addss/subss and ucomiss read a dword through an absolute operand."""
    code = (
        b"\xf3\x0f\x10\x05\xec\x7b\x48\x00"  # movss xmm0, [0x487bec]
        b"\xf3\x0f\x58\x0d\x84\x50\x49\x00"  # addss xmm1, [0x495084]
        b"\xf3\x0f\x5c\x05\x64\x7f\x48\x00"  # subss xmm0, [0x487f64]
        b"\x0f\x2e\x05\xec\x7b\x48\x00"  # ucomiss xmm0, [0x487bec]
    )
    found = list(find_sse_float_instructions_in_buffer(code, 0x1000))
    assert [(f.pointer, f.size) for f in found] == [
        (0x487BEC, 4),
        (0x495084, 4),
        (0x487F64, 4),
        (0x487BEC, 4),
    ]
    # the displacement field, not the instruction, is what a relocation covers
    assert [f.pointer_address - f.address for f in found] == [4, 4, 4, 3]


def test_sse_scalar_double():
    """The F2 prefix (and 66 for the compares) means an 8-byte operand."""
    code = (
        b"\xf2\x0f\x10\x05\x00\x90\x48\x00"  # movsd xmm0, [0x489000]
        b"\xf2\x0f\x59\x0d\x08\x90\x48\x00"  # mulsd xmm1, [0x489008]
        b"\x66\x0f\x2e\x05\x00\x90\x48\x00"  # ucomisd xmm0, [0x489000]
    )
    found = list(find_sse_float_instructions_in_buffer(code, 0))
    assert [(f.pointer, f.size) for f in found] == [
        (0x489000, 8),
        (0x489008, 8),
        (0x489000, 8),
    ]


def test_sse_ignores_stores_and_int_converts():
    """A store writes the constant region; a convert reads an integer."""
    code = (
        b"\xf3\x0f\x11\x05\x00\x90\x48\x00"  # movss [0x489000], xmm0
        b"\xf3\x0f\x2a\x05\x00\x90\x48\x00"  # cvtsi2ss xmm0, [0x489000]
        b"\xf3\x0f\x2c\x05\x00\x90\x48\x00"  # cvttss2si eax, [0x489000]
    )
    assert list(find_sse_float_instructions_in_buffer(code, 0)) == []


def test_sse_compare_needs_no_scalar_prefix():
    """F3 0F 2E is not ucomiss; only the bare and 66-prefixed forms count."""
    code = b"\xf3\x0f\x2e\x05\x00\x90\x48\x00"
    assert list(find_sse_float_instructions_in_buffer(code, 0)) == []
