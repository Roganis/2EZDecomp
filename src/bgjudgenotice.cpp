/* GFBackGround::m45cb20 @0x45cb20 and CatchBackGround::m46b7a0 @0x46b7a0 -
   flash the background's judgement notice.

   The lane-expiry sinks hand these the GRADE of the hit that just expired.
   Grade 4 or better runs the "COOL over" pair (detach latch 2, then the
   other helper with 1); grade 1 exactly runs the "FAIL" pair the other way
   round (1 then 2). Anything between does nothing.

   Three gates guard it: the background has to be running (+0x20 / +0x18),
   its notice flag has to be up (+0xac / +0xa4), and at least ten script
   steps must have passed since the last notice - which is what the
   +0x10353c / +0x103530 watermark records.

   The catch copy has NO log lines; the other two grades' calls and the
   watermark update are identical.

   In its own TU: both helpers each function calls are defined elsewhere
   (src/bgscripttick.cpp and src/stubs.cpp) and these must keep their
   calls. */

#include "backgrounds.h"
#include "loader.h"   /* Log */

// FUNCTION: EZ2AC_UNPACKED 0x0045cb20
void GFBackGround::m45cb20(int grade, int /*flag*/)
{
    if (f20 != 0 && fac != 0 && f102118 - f10353c >= 10)
    {
        if (grade >= 4)
        {
            m45c840(2);
            m41b2f0(1);
            f10353c = f102118;
            // STRING: EZ2AC_UNPACKED 0x0048e268
            Log("Judgement COOL Over");
        }
        else if (grade == 1)
        {
            m45c840(1);
            m41b2f0(2);
            f10353c = f102118;
            // STRING: EZ2AC_UNPACKED 0x0048e258
            Log("Judgement FAIL");
        }
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0046b7a0
void CatchBackGround::m46b7a0(int grade, int /*flag*/)
{
    if (f18 != 0 && fa4 != 0 && f10210c - f103530 >= 10)
    {
        if (grade >= 4)
        {
            m46b310(2);
            m46b360(1);
            f103530 = f10210c;
        }
        else if (grade == 1)
        {
            m46b310(1);
            m46b360(2);
            f103530 = f10210c;
        }
    }
}
