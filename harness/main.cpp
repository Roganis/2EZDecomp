/* Link harness only. Provides an entry point so the reconstructed functions can
   be linked into a PDB-bearing binary for reccmp. Not part of the game; never
   annotated with FUNCTION markers, never matched. Reconstructed functions are
   kept by /OPT:NOREF (they need not be referenced here). */
int main()
{
    return 0;
}
