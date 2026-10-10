# Unit test accompanying 05-icf-aliases.patch.
# Append to tools/reccmp-0.1.6/tests/test_compare_ingest_code.py.
# Verified to FAIL without the patch (the alias reports its own name) and pass with it.

@pytest.fixture(name="fake_binfile")
def fixture_fake_binfile():
    """The `binfile` fixture used by the rest of this module needs a sample
    binary that is not shipped with this source distribution, so every test
    depending on it errors out at collection. This one only needs address
    validation, so a stub stands in and the test below actually runs."""
    mock = Mock()
    mock.is_valid_vaddr = lambda addr: 0x10000000 <= addr < 0x20000000
    return mock


def test_load_code_folded_recomp_not_folded(
    db: EntityDb, lines_db: LinesDb, fake_binfile
):
    """EZ2DECOMP patch 05 (ICF aliases). The test above covers the case where the
    recompiled build folds the duplicates too, so one recomp address serves them
    all. When the recomp does NOT fold them - e.g. deliberately linked with
    /opt:noicf so that every other function keeps a 1:1 address mapping - each
    duplicate keeps its own recomp address. Only the first can win the match, but
    the others must still SYMBOLIZE as the winner: otherwise any operand that
    refers to them (a call, or a vtable slot) differs between the two images even
    though the encoded bytes are identical."""
    files = (
        TextFile(
            PurePath("test.cpp"),
            dedent("""\
                // FUNCTION: TEST 0x10001000 FOLDED
                void Log()
                {
                }

                // FUNCTION: TEST 0x10001000 FOLDED
                void BaseDtor()
                {
                }
                """),
        ),
    )

    # Two DISTINCT recomp addresses for the one original address.
    lines_db.add_line(PureWindowsPath("test.cpp"), 3, 0x1234)
    lines_db.add_line(PureWindowsPath("test.cpp"), 8, 0x5555)
    lines_db.mark_function_starts([0x1234, 0x5555])

    with db.batch() as batch:
        batch.set(ImageId.RECOMP, 0x1234, name="Log")
        batch.set(ImageId.RECOMP, 0x5555, name="BaseDtor")

    load_markers(files, lines_db, fake_binfile, "TEST", db)

    # The first annotation wins the address and the match.
    entity = db.get(ImageId.ORIG, 0x10001000)
    assert entity is not None
    assert entity.recomp_addr == 0x1234
    assert entity.best_name() == "Log"

    # The duplicate now reports the SAME name, so an operand pointing at it
    # symbolizes identically on both sides.
    alias = db.get(ImageId.RECOMP, 0x5555)
    assert alias is not None
    assert alias.best_name() == "Log"

    # ...but it is deliberately NOT a match, so it is never scored.
    assert alias.orig_addr is None


