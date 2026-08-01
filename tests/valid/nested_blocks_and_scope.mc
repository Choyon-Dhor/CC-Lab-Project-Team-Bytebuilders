// Verifies nested scoping: inner declarations do not leak to outer scope
int x;
x = 100;

if (x > 0) {
    int y;
    y = x + 1;
    while (y > 0) {
        int z;
        z = y - 1;
        print z;
        y = y - 1;
    }
}

print x;
