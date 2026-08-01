// Verifies operator precedence, unary minus, parentheses, logical chaining
int a;
int b;
int c;
bool r;

a = 5;
b = 10;
r = false;

c = a + b * 2;        // multiplication before addition
c = -a + 2;            // unary minus binds tighter than binary +
c = (a + b) * 2;       // parentheses override precedence
r = a < b && !r || b >= 3;   // logical operator chaining
print c;
print r;
