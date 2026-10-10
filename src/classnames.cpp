/* Class-name getters: each returns a pointer to the class's name literal.
   `mov eax, offset "Name"; ret`. The STRING marker pins the literal to the
   original's address so reccmp equates the operands. */

/* KEngineApp's name getter @0x401120 moved to src/kengineapp.cpp — it is slot 1
   of that class's vtable @0x487418, which needs it as a real virtual member. */

/* The KResourceContainer family's name getters (KTextureContainer,
   KAniClipContainer, KBitmapContainer) moved to src/ktexture.cpp and
   src/kcontainers.cpp - each is vtable slot 1 of its class, and the vtable
   pins need them as real virtual members. */

/* EZ2AC's name getter @0x414fb0 moved to src/ez2acapp.cpp — it is slot 1 of
   that class's vtable @0x48cf34. */
