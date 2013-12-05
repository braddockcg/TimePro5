#include "timepro.hpp"
#include <stdio.h>

class A {
    public:
        int a;
        A() {
            a = 7 * 27;
        }
        ~A() {}
};

class B : public A {
    public:
        int b;
        B() {
            b = a * 33;
        }
        ~B() {}
};

class C : public B {
    public:
        int c;
        C() {
            c = b * a;
        }
        ~C() {}
        int result() {
            return c;
        }
};

void test_scoped_timer() {
    TPTimer("scoped_timer");
    int x;
    for (int i=0; i<100000; ++i)
        x = x / 27 + 3; 
}

int main(int argc, char **argv) {
    TPInit();
    test_scoped_timer();
    TPStart("Hello main");
    {
        C c;
        printf("%i\n", c.result());
    }
    TPEnd("Hello main");
    TPPrint(stdout);
}

