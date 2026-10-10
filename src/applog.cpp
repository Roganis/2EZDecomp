/* The engine's logger. Its body is compiled out in the original (release build
   with logging disabled), leaving a bare `ret` @0x402880 that the original's
   linker ICF-folded together with every other empty function — the novtable
   base dtor KBase::~KBase (reached via the thunk @0x426e40) and KEngineApp's
   two compiled-out virtuals (vtable @0x487418 slots 2 and 3).

   All the copies are FUNCTION markers (2026-10-01): an empty body compiles
   to the same bare `ret`, so each scores 100%; while they were STUBs the
   shared address was unscored.

   This TU exists for two reasons:
     - the definition must not be visible to any caller, or cl inlines the empty
       vararg body and deletes the call along with it;
     - among the annotations that share 0x402880, whichever comes first in
       reccmp's (lower-cased, sorted) source-file order wins the address and
       lends its name to the rest. Keeping Log in the alphabetically first file
       makes Log the winner, so every call site symbolizes as `Log`. */

// FUNCTION: EZ2AC_UNPACKED 0x00402880
void Log(const char * /*fmt*/, ...)
{
}

/* A second compiled-out logger, folded onto the same `ret`. The song-select
   constructor's two "Enable FIX TARGETBAR" blocks @0x43645e / 0x4364c8 are
   byte-identical yet stay separate in the original, which cl only does when
   the two calls named different symbols at compile time (the ICF-fold rule,
   docs/cookbook.md); written with Log twice, cl cross-jumps them into one. */
// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void LogAlt(const char * /*fmt*/, ...)
{
}
