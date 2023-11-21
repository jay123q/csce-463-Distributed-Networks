#include <iostream>
#include <string>

using namespace std;
typedef unsigned int UINT64;
#include <stdint.h>
#include <stdio.h>  // Include standard input/output for debugging purposes

// Define the uint64 type (assuming it's an unsigned 64-bit integer)
typedef unsigned long long uint64;


int main(void)
{
    /*
    char str[] = "hello there";
    SomeData3* p = (SomeData3*)str;
    printf("buffer contents: %d, %d, %I64d\n", p->a, p->b, p->c);
    */
    char buf[128] = "hello world!!";
    int* ptr1 = (int*)buf;
    uint64* ptr2 = (uint64*)(ptr1 + 1);
    printf("print ptr2 rotated %x\n", ptr2);
    int a = *ptr1;
    uint64 b = *ptr2;
    cout << " int a is = " << a << " uint64 b is " << b << " \n";
}