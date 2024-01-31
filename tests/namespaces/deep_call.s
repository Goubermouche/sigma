namespace a {
    namespace a {
        namespace a {
            namespace a {
                namespace a {
                    i32 test() {
                        ret 20;
                    }
                }
            } 
        }
    }
}

i32 main() {
    printf("%d\n", a::a::a::a::a::test());
    ret 0;
}