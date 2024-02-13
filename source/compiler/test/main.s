// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// KNOWN BUGS
// -   numerical literals throw cast warnings, which shouldnt happen
// -   numerical literals are always interpreted as i32

// TODO: 
// -   DIAGNOSTICS:
//     -   better messages
//     -   more info related to numerical errors (hex etc)
//     -   add namespaces to messages, whenever applicable (ie. x::y::test)
// -   BUGS:

struct nested {
    u8 a;
    u64 b;
    u32 c;
};

struct complex {
    nested nest;
    u32 x;
    u16 y;
};

struct container {
    complex comp;
    u64 z;
};

void print(container cont) {
    printf("Values: %llu %u %u %u %hu %llu\n", cont.comp.nest.b, cont.comp.nest.c, cont.comp.x, cont.comp.nest.a, cont.comp.y, cont.z);
}

u64 main() {
    container a;

    a.comp.nest.a = 10;
    a.comp.nest.b = 123456789012345;
    a.comp.nest.c = 3000;
    a.comp.x = 100;
    a.comp.y = 50;
    a.z = 99999;

    printf("Values: %llu %u %u %u %hu %llu\n", a.comp.nest.b, a.comp.nest.c, a.comp.x, a.comp.nest.a, a.comp.y, a.z);
    print(a);
    printf("Values: %llu %u %u %u %hu %llu\n", a.comp.nest.b, a.comp.nest.c, a.comp.x, a.comp.nest.a, a.comp.y, a.z);

    ret 0;
}
