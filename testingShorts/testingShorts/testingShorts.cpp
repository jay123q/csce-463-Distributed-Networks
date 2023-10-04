#include <iostream>
#include <string>

using namespace std;


class SomeData3
{
public:
    char a;
    short b;
    __int64 c;

};

int main(void)
{
    /*
    char str[] = "hello there";
    SomeData3* p = (SomeData3*)str;
    printf("buffer contents: %d, %d, %I64d\n", p->a, p->b, p->c);
    */
    char x = 0x44, y = 0x20;
    short z = (x >> 8) + y;
    printf("%x", z);
}