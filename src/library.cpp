/* Address annotations for statically-linked CRT symbols in the original
   binary, resolved against our libcmt by name (reccmp LIBRARY/GLOBAL-by-name
   markers). No code — this TU exists only for the annotations.
   Only symbols referenced by matched src/ functions belong here; DecryptFile's
   extra CRT calls (fseek/ftell/fread/fwrite) live in wip/library-decrypt.cpp. */

// LIBRARY: EZ2AC_UNPACKED 0x0047960e
// fopen

// LIBRARY: EZ2AC_UNPACKED 0x00479d9d
// fscanf

/* atoi is libcmt's one-line `jmp atol`; every game call goes through it.
   (Annotated as `atol` until 2026-10-01.) */
// LIBRARY: EZ2AC_UNPACKED 0x004788c5
// atoi

// LIBRARY: EZ2AC_UNPACKED 0x0047883d
// atol

/* The allocator (include/kcrt.h): the operators are what game code calls;
   malloc/free are only reached through them. 0x478528 and 0x4788ca were
   annotated `malloc` and `free` until 2026-10-01. */
// LIBRARY: EZ2AC_UNPACKED 0x00478528
// ??2@YAPAXI@Z

// LIBRARY: EZ2AC_UNPACKED 0x004788ca
// ??3@YAXPAX@Z

// LIBRARY: EZ2AC_UNPACKED 0x0047854c
// ??_U@YAPAXI@Z

// LIBRARY: EZ2AC_UNPACKED 0x0047927d
// ??_V@YAXPAX@Z

// LIBRARY: EZ2AC_UNPACKED 0x0047a5e8
// malloc

// LIBRARY: EZ2AC_UNPACKED 0x0047afc4
// free

// LIBRARY: EZ2AC_UNPACKED 0x00479354
// fclose

/* The two the audio loaders read their files with (src/loaders.cpp). They were
   parked in wip/library-decrypt.cpp for DecryptFile; the loaders are the first
   MATCHED callers, so they belong here now. fseek is NOT here yet: a LIBRARY
   marker resolves by NAME in our own PDB, so annotating a CRT symbol nothing
   in src/ calls leaves it unmatched and costs one point of Implemented. Its
   only caller is the parked seek (wip/loaders-seek.cpp). */
// LIBRARY: EZ2AC_UNPACKED 0x00479d2b
// ftell

/* fseek's first matched caller is SongSelectDirector::loadRankingFile. */
// LIBRARY: EZ2AC_UNPACKED 0x00479434
// fseek

// LIBRARY: EZ2AC_UNPACKED 0x00479566
// fread

// LIBRARY: EZ2AC_UNPACKED 0x00479b3b
// fwrite

// LIBRARY: EZ2AC_UNPACKED 0x0047a1f3
// _access

// LIBRARY: EZ2AC_UNPACKED 0x0047882f
// __security_check_cookie

/* The stack probe. It has to be named _chkstk, not __chkstk: our libcmt exports
   BOTH symbols, they are different addresses, and the one cl's prologue actually
   calls is _chkstk - annotating the alias made loadModeGds @0x4150d0 read as a
   call mismatch with identical bytes. */
// LIBRARY: EZ2AC_UNPACKED 0x00479240
// _chkstk

// LIBRARY: EZ2AC_UNPACKED 0x004792b1
// vsprintf

/* 0x478740 is the asm strchr (a forward byte scan; its found-exit sits in
   the same object just before it, @0x478730) and 0x479d70 is strrchr (`std;
   repnz scasb` walks backward). The two were swapped here until 2026-10-01,
   which read as a strrchr/strchr call mismatch in KParser::parseToken. */
// LIBRARY: EZ2AC_UNPACKED 0x00478740
// strchr

// LIBRARY: EZ2AC_UNPACKED 0x00479d70
// strrchr

// LIBRARY: EZ2AC_UNPACKED 0x00479630
// memmove

// LIBRARY: EZ2AC_UNPACKED 0x00479de8
// _ftol2

// GLOBAL: EZ2AC_UNPACKED 0x004afca0
// __security_cookie

// LIBRARY: EZ2AC_UNPACKED 0x004788cf
// sprintf

// LIBRARY: EZ2AC_UNPACKED 0x0047f945
// _stricmp

/* _strnicmp, not strncmp: it fetches the thread's locale info and folds
   case through _tolower_lk per byte. Its six game callers are the gds
   "Track"/"Slot" section tests, which the C wrote case-sensitive until
   2026-10-01 (docs/PORT-DELTAS.md). */
// LIBRARY: EZ2AC_UNPACKED 0x00483322
// _strnicmp

// LIBRARY: EZ2AC_UNPACKED 0x00478600
// strncpy

// LIBRARY: EZ2AC_UNPACKED 0x00479e60
// strstr

// LIBRARY: EZ2AC_UNPACKED 0x00479ee6
// _strlwr

// LIBRARY: EZ2AC_UNPACKED 0x00479282
// srand

// LIBRARY: EZ2AC_UNPACKED 0x0047928f
// rand

// LIBRARY: EZ2AC_UNPACKED 0x0047a032
// time

/* Not CRT: the import-library thunk for USER32's BlockInput (`jmp
   [__imp__BlockInput]`), which is what a non-dllimport declaration calls — see
   src/ez2acapp.cpp. */
// LIBRARY: EZ2AC_UNPACKED 0x00478522
// _BlockInput@4

/* Likewise WINMM's timeGetTime thunk, called by EZ2AC::runGame @0x417e90. */
// LIBRARY: EZ2AC_UNPACKED 0x004145f0
// _timeGetTime@0

/* DirectInput's create entry, called WITHOUT dllimport by createInput @0x40c250,
   so it goes through the linker's thunk rather than a dllimport indirect call. */
// LIBRARY: EZ2AC_UNPACKED 0x00478510
// _DirectInputCreateA@16

/* DirectSound's, likewise, called from Obj40EAC0::init @0x40e7f0 - the
   DirectSound8 entry, imported by ordinal 11 (DSOUND's IAT slot 0x487024). */
// LIBRARY: EZ2AC_UNPACKED 0x0047748e
// _DirectSoundCreate8@12

/* cl's array-member destructor helper. A destructor that shows
   `push ~M; push count; push size; push base; call 0x47a12b` is destroying a
   real ARRAY member; N separate `lea ecx; call ~M` pairs at a constant stride
   would be N distinct members instead. (The constructor's counterpart is
   @0x47a06b.) First needed by ~ModeSelectDirector @0x448f70. */
// LIBRARY: EZ2AC_UNPACKED 0x0047a12b
// `eh vector destructor iterator'

/* cl's array-member CONSTRUCTOR helper, the counterpart of @0x47a12b. First
   needed by ModeSelectDirector's constructor @0x44b8f0, which has three array
   members. */
// LIBRARY: EZ2AC_UNPACKED 0x0047a06b
// `eh vector constructor iterator'

/* The CRT's pure-virtual trap. It is what the ABSTRACT sink base's own vftable
   @0x49ec9c holds, so every test-menu page's constructor stores a table made
   of it before overwriting the slot with its own override. */
// LIBRARY: EZ2AC_UNPACKED 0x00479a20
// _purecall

/* The five-byte thunk KGraphics's device enumeration calls - the API is
   declared without dllimport, so the linker builds one. */
// LIBRARY: EZ2AC_UNPACKED 0x0047851c
// _DirectDrawEnumerateExA@12

/* The combo readouts spell their count out with _itoa before indexing the
   digit textures by it. */
// LIBRARY: EZ2AC_UNPACKED 0x0047a1c9
// _itoa

/* Both called by the config tokenizer @0x413940 (src/parsetoken.cpp) - its
   classifier tests digits, and its float arm parses with atof. */
// LIBRARY: EZ2AC_UNPACKED 0x0047996d
// isdigit

// LIBRARY: EZ2AC_UNPACKED 0x00479ffa
// atof

// LIBRARY: EZ2AC_UNPACKED 0x00478516
// _DirectDrawCreateEx@16

// LIBRARY: EZ2AC_UNPACKED 0x00478fb0
// qsort

/* CRT internals the annotated CRT entry points call - named so those entry
   points compare their call operands (they were 67-99% on unnamed callees
   alone). Found with scratchpad crtnames.py: each is the original operand at
   the diff line where our side shows the libcmt/libcpmt name. */

// GLOBAL: EZ2AC_UNPACKED 0x037e1398
// __active_heap

// GLOBAL: EZ2AC_UNPACKED 0x037e0ea8
// __pPurecall

// GLOBAL: EZ2AC_UNPACKED 0x004afd54
// __ptlocinfo

// GLOBAL: EZ2AC_UNPACKED 0x037e1394
// _crtheap

// GLOBAL: EZ2AC_UNPACKED 0x037e1198
// _newmode

// LIBRARY: EZ2AC_UNPACKED 0x0047a0cd
// __ArrayUnwind

// LIBRARY: EZ2AC_UNPACKED 0x0047a963
// __SEH_epilog

// LIBRARY: EZ2AC_UNPACKED 0x0047a928
// __SEH_prolog

// LIBRARY: EZ2AC_UNPACKED 0x00483290
// __ascii_stricmp

// LIBRARY: EZ2AC_UNPACKED 0x00483d90
// __ascii_strnicmp

// LIBRARY: EZ2AC_UNPACKED 0x0047f3eb
// __crtLCMapStringA

// LIBRARY: EZ2AC_UNPACKED 0x0047d618
// __doserrno

// LIBRARY: EZ2AC_UNPACKED 0x0047aa7d
// __isctype_mt

// LIBRARY: EZ2AC_UNPACKED 0x004803d5
// __sbh_find_block

// LIBRARY: EZ2AC_UNPACKED 0x00480400
// __sbh_free_block

// LIBRARY: EZ2AC_UNPACKED 0x0047a239
// __tolower_mt

// LIBRARY: EZ2AC_UNPACKED 0x0047ac85
// __updatetlocinfo

// LIBRARY: EZ2AC_UNPACKED 0x0047a323
// _amsg_exit

// LIBRARY: EZ2AC_UNPACKED 0x0047f840
// _aulldiv

// LIBRARY: EZ2AC_UNPACKED 0x0047a5fa
// _callnewh

// LIBRARY: EZ2AC_UNPACKED 0x0047d621
// _dosmaperr

// LIBRARY: EZ2AC_UNPACKED 0x0047d60f
// _errno

// LIBRARY: EZ2AC_UNPACKED 0x00479308
// _fclose_lk

// LIBRARY: EZ2AC_UNPACKED 0x0047b035
// _flsbuf

// LIBRARY: EZ2AC_UNPACKED 0x0047f7a7
// _fltin2

// LIBRARY: EZ2AC_UNPACKED 0x0047947d
// _fread_lk

// LIBRARY: EZ2AC_UNPACKED 0x004793a5
// _fseek_lk

// LIBRARY: EZ2AC_UNPACKED 0x004795b2
// _fsopen

// LIBRARY: EZ2AC_UNPACKED 0x00479bc9
// _ftell_lk

// LIBRARY: EZ2AC_UNPACKED 0x00479a34
// _fwrite_lk

// LIBRARY: EZ2AC_UNPACKED 0x0047ace6
// _getptd

// LIBRARY: EZ2AC_UNPACKED 0x0047e494
// _input

// LIBRARY: EZ2AC_UNPACKED 0x0048035c
// _lock

// LIBRARY: EZ2AC_UNPACKED 0x0047d1d8
// _lock_file

// LIBRARY: EZ2AC_UNPACKED 0x0047a5bc
// _nh_malloc

// LIBRARY: EZ2AC_UNPACKED 0x0047b1dc
// _output

// LIBRARY: EZ2AC_UNPACKED 0x0047f308
// _resetstkoflw

// LIBRARY: EZ2AC_UNPACKED 0x004802a7
// _unlock

// LIBRARY: EZ2AC_UNPACKED 0x0047d22a
// _unlock_file

// LIBRARY: EZ2AC_UNPACKED 0x004799e6
// isspace

// LIBRARY: EZ2AC_UNPACKED 0x00478f40
// shortsort

// LIBRARY: EZ2AC_UNPACKED 0x00478590
// ?_Nomemory@std@@YAXXZ

// LIBRARY: EZ2AC_UNPACKED 0x0047f210
// strcpy

// LIBRARY: EZ2AC_UNPACKED 0x0047d090
// strlen

// LIBRARY: EZ2AC_UNPACKED 0x0047a18b
// xtoa

// LIBRARY: EZ2AC_UNPACKED 0x004787fe
// report_failure

// GLOBAL: EZ2AC_UNPACKED 0x004b0468
// _XcptActTab

// GLOBAL: EZ2AC_UNPACKED 0x004b03a0
// __badioinfo

// GLOBAL: EZ2AC_UNPACKED 0x004b0734
// __decimal_point

// GLOBAL: EZ2AC_UNPACKED 0x037e0eb4
// __error_mode

// GLOBAL: EZ2AC_UNPACKED 0x037e1250
// __lc_codepage

// GLOBAL: EZ2AC_UNPACKED 0x004a07f8
// __lookuptable

// GLOBAL: EZ2AC_UNPACKED 0x004b0730
// __mb_cur_max

// GLOBAL: EZ2AC_UNPACKED 0x004afe90
// __nullstring

// GLOBAL: EZ2AC_UNPACKED 0x037e13a0
// __pioinfo

// GLOBAL: EZ2AC_UNPACKED 0x037e137c
// __sbh_cntHeaderList

// GLOBAL: EZ2AC_UNPACKED 0x037e1390
// __sbh_indGroupDefer

// GLOBAL: EZ2AC_UNPACKED 0x037e1378
// __sbh_pHeaderDefer

// GLOBAL: EZ2AC_UNPACKED 0x037e1380
// __sbh_pHeaderList

// GLOBAL: EZ2AC_UNPACKED 0x037e1388
// __sbh_pHeaderScan

// GLOBAL: EZ2AC_UNPACKED 0x004afe8c
// __tlsindex

// GLOBAL: EZ2AC_UNPACKED 0x004afe94
// __wnullstring

// GLOBAL: EZ2AC_UNPACKED 0x004afcd0
// _aexit_rtn

// GLOBAL: EZ2AC_UNPACKED 0x004afea0
// _cfltcvt_tab

// GLOBAL: EZ2AC_UNPACKED 0x004affb0
// _iob

// GLOBAL: EZ2AC_UNPACKED 0x004b04f8
// _locktable

// GLOBAL: EZ2AC_UNPACKED 0x037e0ee0
// _osplatform

// GLOBAL: EZ2AC_UNPACKED 0x004b06e0
// _pctype

// GLOBAL: EZ2AC_UNPACKED 0x037e0eb8
// _pnhHeap

// LIBRARY: EZ2AC_UNPACKED 0x0047a73d
// _CxxThrowException

// LIBRARY: EZ2AC_UNPACKED 0x0047fb25
// _FF_MSGBANNER

// LIBRARY: EZ2AC_UNPACKED 0x0047f9ae
// _NMSG_WRITE

// LIBRARY: EZ2AC_UNPACKED 0x00482c46
// __ansicp

// LIBRARY: EZ2AC_UNPACKED 0x00482c89
// __convertcp

// LIBRARY: EZ2AC_UNPACKED 0x004812c8
// __crtGetStringTypeA

// LIBRARY: EZ2AC_UNPACKED 0x0047a7dd
// __security_error_handler

// LIBRARY: EZ2AC_UNPACKED 0x00482e52
// __strgtold12

// LIBRARY: EZ2AC_UNPACKED 0x0047abc4
// __updatetlocinfo_lk

// LIBRARY: EZ2AC_UNPACKED 0x0047af90
// _allmul

// LIBRARY: EZ2AC_UNPACKED 0x0047f8b0
// _aulldvrm

// LIBRARY: EZ2AC_UNPACKED 0x0047d3e0
// _close

// LIBRARY: EZ2AC_UNPACKED 0x0047d27c
// _filbuf

// LIBRARY: EZ2AC_UNPACKED 0x0047d4a6
// _flush

// LIBRARY: EZ2AC_UNPACKED 0x0047d47b
// _freebuf

// LIBRARY: EZ2AC_UNPACKED 0x004819bb
// _getbuf

// LIBRARY: EZ2AC_UNPACKED 0x0047dee5
// _getstream

// LIBRARY: EZ2AC_UNPACKED 0x0047a541
// _heap_alloc

// LIBRARY: EZ2AC_UNPACKED 0x0047e47e
// _inc

// LIBRARY: EZ2AC_UNPACKED 0x004819ff
// _isatty

// LIBRARY: EZ2AC_UNPACKED 0x00481e10
// _ld12tod

// LIBRARY: EZ2AC_UNPACKED 0x0047d708
// _lseek

// LIBRARY: EZ2AC_UNPACKED 0x004802bc
// _mtinitlocknum

// LIBRARY: EZ2AC_UNPACKED 0x0047dd7d
// _openfile

// LIBRARY: EZ2AC_UNPACKED 0x0047d98e
// _read

// LIBRARY: EZ2AC_UNPACKED 0x00482aef
// _ungetc_lk

// LIBRARY: EZ2AC_UNPACKED 0x0047e1d5
// _write

// LIBRARY: EZ2AC_UNPACKED 0x00478f29
// atexit

// LIBRARY: EZ2AC_UNPACKED 0x00481900
// calloc

// LIBRARY: EZ2AC_UNPACKED 0x004799a7
// isxdigit

// LIBRARY: EZ2AC_UNPACKED 0x00482c1b
// mbtowc

// LIBRARY: EZ2AC_UNPACKED 0x0047da40
// memcpy

// LIBRARY: EZ2AC_UNPACKED 0x004817a0
// memset

// LIBRARY: EZ2AC_UNPACKED 0x0047c280
// terminate

// LIBRARY: EZ2AC_UNPACKED 0x00481a89
// wctomb

// LIBRARY: EZ2AC_UNPACKED 0x0047b14e
// write_char

// LIBRARY: EZ2AC_UNPACKED 0x0047b181
// write_multi_char

// LIBRARY: EZ2AC_UNPACKED 0x0047b1a5
// write_string

// GLOBAL: EZ2AC_UNPACKED 0x004b0230
// errtable

// GLOBAL: EZ2AC_UNPACKED 0x037e0f2c
// f_use

// GLOBAL: EZ2AC_UNPACKED 0x037e0ed0
// gpFlsGetValue

// GLOBAL: EZ2AC_UNPACKED 0x037e0ed4
// gpFlsSetValue

// GLOBAL: EZ2AC_UNPACKED 0x037e0e90
// nomem

// LIBRARY: EZ2AC_UNPACKED 0x0047a97c
// _except_handler3

// GLOBAL: EZ2AC_UNPACKED 0x004a03e4
// ExceptionTemplate

// GLOBAL: EZ2AC_UNPACKED 0x004afcd4
// __app_type

// GLOBAL: EZ2AC_UNPACKED 0x004afd00
// __initiallocinfo

// GLOBAL: EZ2AC_UNPACKED 0x037e14a0
// __piob

// GLOBAL: EZ2AC_UNPACKED 0x037e1384
// __sbh_threshold

// GLOBAL: EZ2AC_UNPACKED 0x037e0f30
// _adbgmsg

// GLOBAL: EZ2AC_UNPACKED 0x037e0f28
// _cflush

// GLOBAL: EZ2AC_UNPACKED 0x037e127c
// _commode

// GLOBAL: EZ2AC_UNPACKED 0x037e139c
// _nhandle

// GLOBAL: EZ2AC_UNPACKED 0x037e24c0
// _nstream

// LIBRARY: EZ2AC_UNPACKED 0x00478dda
// _NLG_Notify

// LIBRARY: EZ2AC_UNPACKED 0x0048109f
// _ValidateEH3RN

// LIBRARY: EZ2AC_UNPACKED 0x004829e1
// __crtInitCritSecAndSpinCount

// LIBRARY: EZ2AC_UNPACKED 0x00480fa6
// __crtMessageBoxA

// LIBRARY: EZ2AC_UNPACKED 0x0047aaf4
// __freetlocinfo

// LIBRARY: EZ2AC_UNPACKED 0x00478d04
// __global_unwind2

// LIBRARY: EZ2AC_UNPACKED 0x00478d46
// __local_unwind2

// LIBRARY: EZ2AC_UNPACKED 0x00482b5b
// __mbtowc_mt

// LIBRARY: EZ2AC_UNPACKED 0x00483622
// __mtold12

// LIBRARY: EZ2AC_UNPACKED 0x00483d01
// __multtenpow12

// LIBRARY: EZ2AC_UNPACKED 0x00480bb4
// __sbh_alloc_block

// LIBRARY: EZ2AC_UNPACKED 0x00481a29
// __wctomb_mt

// LIBRARY: EZ2AC_UNPACKED 0x0047d35d
// _close_lk

// LIBRARY: EZ2AC_UNPACKED 0x0047cafb
// _exit

// LIBRARY: EZ2AC_UNPACKED 0x00481cb8
// _ld12cvt

// LIBRARY: EZ2AC_UNPACKED 0x0048239b
// _lock_fhandle

// LIBRARY: EZ2AC_UNPACKED 0x0047d207
// _lock_file2

// LIBRARY: EZ2AC_UNPACKED 0x0047d694
// _lseek_lk

// LIBRARY: EZ2AC_UNPACKED 0x00478ef1
// _onexit

// LIBRARY: EZ2AC_UNPACKED 0x0047d7b3
// _read_lk

// LIBRARY: EZ2AC_UNPACKED 0x0048297c
// _sopen

// LIBRARY: EZ2AC_UNPACKED 0x0048243b
// _unlock_fhandle

// LIBRARY: EZ2AC_UNPACKED 0x0047d259
// _unlock_file2

// LIBRARY: EZ2AC_UNPACKED 0x0047e007
// _write_lk

// LIBRARY: EZ2AC_UNPACKED 0x0047f220
// strcat

// LIBRARY: EZ2AC_UNPACKED 0x00481b00
// abort

// GLOBAL: EZ2AC_UNPACKED 0x004b073c
// DoubleFormat

// GLOBAL: EZ2AC_UNPACKED 0x004b03d0
// rterrs

// GLOBAL: EZ2AC_UNPACKED 0x037e0ebc
// user_handler


// GLOBAL: EZ2AC_UNPACKED 0x004afca4
// __NLG_Destination

// LIBRARY: EZ2AC_UNPACKED 0x004829d1
// __crtInitCritSecNoSpinCount

// GLOBAL: EZ2AC_UNPACKED 0x037e136c
// __ctype1_refcount

// GLOBAL: EZ2AC_UNPACKED 0x037e120c
// __lc_time_intl

// GLOBAL: EZ2AC_UNPACKED 0x037e1210
// __lconv_intl

// GLOBAL: EZ2AC_UNPACKED 0x037e1370
// __lconv_mon_refcount

// GLOBAL: EZ2AC_UNPACKED 0x037e1374
// __lconv_num_refcount

// GLOBAL: EZ2AC_UNPACKED 0x004b09f0
// _pow10neg

// GLOBAL: EZ2AC_UNPACKED 0x004b0890
// _pow10pos

// GLOBAL: EZ2AC_UNPACKED 0x037e0eec
// _winmajor

// LIBRARY: EZ2AC_UNPACKED 0x00481c09
// _CopyMan

// LIBRARY: EZ2AC_UNPACKED 0x00481c24
// _IsZeroMan

// LIBRARY: EZ2AC_UNPACKED 0x00481b97
// _RoundMan

// LIBRARY: EZ2AC_UNPACKED 0x00481c3d
// _ShrMan

// LIBRARY: EZ2AC_UNPACKED 0x00483569
// __add_12

// LIBRARY: EZ2AC_UNPACKED 0x00481482
// __free_lc_time

// LIBRARY: EZ2AC_UNPACKED 0x00481671
// __free_lconv_mon

// LIBRARY: EZ2AC_UNPACKED 0x00481612
// __free_lconv_num

// LIBRARY: EZ2AC_UNPACKED 0x00483acf
// __ld12mul

// LIBRARY: EZ2AC_UNPACKED 0x004807cf
// __sbh_alloc_new_group

// LIBRARY: EZ2AC_UNPACKED 0x00480718
// __sbh_alloc_new_region

// LIBRARY: EZ2AC_UNPACKED 0x004835c7
// __shl_12

// LIBRARY: EZ2AC_UNPACKED 0x004822db
// _free_osfhnd

// LIBRARY: EZ2AC_UNPACKED 0x0048235a
// _get_osfhandle

// LIBRARY: EZ2AC_UNPACKED 0x0047c993
// _lockexit

// LIBRARY: EZ2AC_UNPACKED 0x00482a6c
// _lseeki64_lk

// LIBRARY: EZ2AC_UNPACKED 0x00478e49
// _onexit_lk

// LIBRARY: EZ2AC_UNPACKED 0x00482695
// _tsopen_lk

// LIBRARY: EZ2AC_UNPACKED 0x0047c99c
// _unlockexit

// LIBRARY: EZ2AC_UNPACKED 0x0047ca27
// doexit

// LIBRARY: EZ2AC_UNPACKED 0x004833cf
// raise

// GLOBAL: EZ2AC_UNPACKED 0x037e11b8
// nValidPages

// GLOBAL: EZ2AC_UNPACKED 0x037e11a4
// pfnGetActiveWindow

// GLOBAL: EZ2AC_UNPACKED 0x037e11a8
// pfnGetLastActivePopup

// GLOBAL: EZ2AC_UNPACKED 0x037e11ac
// pfnGetProcessWindowStation

// GLOBAL: EZ2AC_UNPACKED 0x037e11b0
// pfnGetUserObjectInformationA

// GLOBAL: EZ2AC_UNPACKED 0x037e11a0
// pfnMessageBoxA

// LIBRARY: EZ2AC_UNPACKED 0x00478d24
// __unwind_handler

// GLOBAL: EZ2AC_UNPACKED 0x037e1200
// lModifying

// GLOBAL: EZ2AC_UNPACKED 0x037e0f20
// _C_Exit_Done

// GLOBAL: EZ2AC_UNPACKED 0x037e0f1c
// _C_Termination_Done

// GLOBAL: EZ2AC_UNPACKED 0x004b04e0
// _First_FPE_Indx

// GLOBAL: EZ2AC_UNPACKED 0x004b04e4
// _Num_FPE

// GLOBAL: EZ2AC_UNPACKED 0x004b071c
// __lconv

// GLOBAL: EZ2AC_UNPACKED 0x004b06ec
// __lconv_c

// GLOBAL: EZ2AC_UNPACKED 0x037e2708
// __onexitbegin

// GLOBAL: EZ2AC_UNPACKED 0x037e2704
// __onexitend

// GLOBAL: EZ2AC_UNPACKED 0x037e138c
// __sbh_sizeHeaderList

// GLOBAL: EZ2AC_UNPACKED 0x004a50b0
// __xp_a

// GLOBAL: EZ2AC_UNPACKED 0x004a50bc
// __xt_a

// GLOBAL: EZ2AC_UNPACKED 0x037e0f18
// _exitflag

// GLOBAL: EZ2AC_UNPACKED 0x037e135c
// _fmode

// GLOBAL: EZ2AC_UNPACKED 0x037e0edc
// _umaskval

// GLOBAL: EZ2AC_UNPACKED 0x037e1350
// abort_action

// LIBRARY: EZ2AC_UNPACKED 0x00481b4a
// _IncMan

// LIBRARY: EZ2AC_UNPACKED 0x00481b18
// _ZeroTail

// LIBRARY: EZ2AC_UNPACKED 0x00483548
// __addl

// LIBRARY: EZ2AC_UNPACKED 0x0047c963
// __crtExitProcess

// LIBRARY: EZ2AC_UNPACKED 0x004835f5
// __shr_12

// LIBRARY: EZ2AC_UNPACKED 0x0048245d
// _alloc_osfhnd

// LIBRARY: EZ2AC_UNPACKED 0x0048398e
// _chsize_lk

// LIBRARY: EZ2AC_UNPACKED 0x0047c9a5
// _initterm

// LIBRARY: EZ2AC_UNPACKED 0x0047cb2a
// _msize

// LIBRARY: EZ2AC_UNPACKED 0x0048225f
// _set_osfhnd

// LIBRARY: EZ2AC_UNPACKED 0x0047c7b6
// realloc

// LIBRARY: EZ2AC_UNPACKED 0x004833a1
// siglookup

// GLOBAL: EZ2AC_UNPACKED 0x037e134c
// ctrlbreak_action

// GLOBAL: EZ2AC_UNPACKED 0x037e1348
// ctrlc_action

// GLOBAL: EZ2AC_UNPACKED 0x004a50b8
// __xp_z

// GLOBAL: EZ2AC_UNPACKED 0x004a50c4
// __xt_z

// GLOBAL: EZ2AC_UNPACKED 0x037e1354
// term_action

// GLOBAL: EZ2AC_UNPACKED 0x004b04ec
// _XcptActTabCount

// LIBRARY: EZ2AC_UNPACKED 0x004808d5
// __sbh_resize_block

// LIBRARY: EZ2AC_UNPACKED 0x00483df1
// _setmode_lk

/* Two CRT statics their functions test (named from our PDB). The second is
   std::_Nomemory's once-guard, which our PDB calls `$S1`. */
// GLOBAL: EZ2AC_UNPACKED 0x037e11c0
// rgValidPages

// GLOBAL: EZ2AC_UNPACKED 0x037e0e9c
// $S1

/* The CRT's own string literals, pinned so the functions that push them
   compare by name (the recomp side already resolves each by content). A
   STRING marker takes the literal from the next line, comment or not. */

// STRING: EZ2AC_UNPACKED 0x004a02ac
// "bad allocation"

// STRING: EZ2AC_UNPACKED 0x004a0404
// "Microsoft Visual C++ Runtime Library"

// STRING: EZ2AC_UNPACKED 0x004a042c
// "Program: "

// STRING: EZ2AC_UNPACKED 0x004a0438
// "\n\n"

// STRING: EZ2AC_UNPACKED 0x004a043c
// "..."

// STRING: EZ2AC_UNPACKED 0x004a0440
// "<program name unknown>"

// STRING: EZ2AC_UNPACKED 0x004a0458
// "A buffer overrun has been detected which has corrupted the program's\ninternal state.  The program cannot safely continue execution and must\nnow be terminated.\n"

// STRING: EZ2AC_UNPACKED 0x004a04f8
// "Buffer overrun detected!"

// STRING: EZ2AC_UNPACKED 0x004a0518
// "A security error of unknown cause has been detected which has\ncorrupted the program's internal state.  The program cannot safely\ncontinue execution and must now be terminated.\n"

// STRING: EZ2AC_UNPACKED 0x004a05cc
// "Unknown security failure detected!"

// STRING: EZ2AC_UNPACKED 0x004a07d4
// "kernel32.dll"

// STRING: EZ2AC_UNPACKED 0x004a0934
// "CorExitProcess"

// STRING: EZ2AC_UNPACKED 0x004a0944
// "mscoree.dll"

// STRING: EZ2AC_UNPACKED 0x004a09f4
// L"\x00"

// STRING: EZ2AC_UNPACKED 0x004a0dc0
// "Runtime Error!\n\nProgram: "

// STRING: EZ2AC_UNPACKED 0x004a0e0c
// "GetProcessWindowStation"

// STRING: EZ2AC_UNPACKED 0x004a0e24
// "GetUserObjectInformationA"

// STRING: EZ2AC_UNPACKED 0x004a0e40
// "GetLastActivePopup"

// STRING: EZ2AC_UNPACKED 0x004a0e54
// "GetActiveWindow"

// STRING: EZ2AC_UNPACKED 0x004a0e64
// "MessageBoxA"

// STRING: EZ2AC_UNPACKED 0x004a0e70
// "user32.dll"

// STRING: EZ2AC_UNPACKED 0x004a1524
// "InitializeCriticalSectionAndSpinCount"

/* The import thunk __global_unwind2 calls. */
// LIBRARY: EZ2AC_UNPACKED 0x00483e5e
// _RtlUnwind@16

/* Two more CRT statics, named from our PDB: __crtGetStringTypeA's once-only
   API probe and the cached InitializeCriticalSectionAndSpinCount pointer
   (our PDB names that static after its function). */
// GLOBAL: EZ2AC_UNPACKED 0x037e1204
// f_use

// GLOBAL: EZ2AC_UNPACKED 0x037e1280
// __crtInitCritSecAndSpinCount
