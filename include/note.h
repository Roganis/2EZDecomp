#ifndef NOTE_H
#define NOTE_H

/* Raw 13-byte on-disk .ez (v8) note record and the 40-byte internal note.
   Field meanings are partly known from prior RE (src+4 = record type; per-type
   payload from src+5), but the mixed-width accesses are modelled as raw bytes
   for now to control exact codegen; typed fields come once fully named. */

struct EzNoteRaw { unsigned char b[13]; };
struct EzNote    { unsigned char b[40]; };

/* @0x00410ef0 : convert one raw record (src) into an internal note (dst). */
void NoteConvert(EzNoteRaw *src, EzNote *dst, unsigned char p3);

/* @0x00410fd0 : convert `count` raw records, stamping p3 into each. */
void NoteConvertAll(EzNoteRaw *src, EzNote *dst, unsigned char p3, int count);

#endif /* NOTE_H */
