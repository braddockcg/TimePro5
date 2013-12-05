#include <stdio.h>

#include "timepro.h"

void b() {
}

void a() {
    b();
}

int main(int argc, char **argv) {
    TPInit();
    a();
    TPPrint(stdout);
}
