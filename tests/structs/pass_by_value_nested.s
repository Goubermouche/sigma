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
    container my_container;

    my_container.comp.nest.a = 10;
    my_container.comp.nest.b = 123456789012345;
    my_container.comp.nest.c = 3000;
    my_container.comp.x = 100;
    my_container.comp.y = 50;
    my_container.z = 99999;

    printf("Values: %llu %u %u %u %hu %llu\n", my_container.comp.nest.b, my_container.comp.nest.c, my_container.comp.x, my_container.comp.nest.a, my_container.comp.y, my_container.z);
    print(my_container);
    printf("Values: %llu %u %u %u %hu %llu\n", my_container.comp.nest.b, my_container.comp.nest.c, my_container.comp.x, my_container.comp.nest.a, my_container.comp.y, my_container.z);

    ret 0;
}
