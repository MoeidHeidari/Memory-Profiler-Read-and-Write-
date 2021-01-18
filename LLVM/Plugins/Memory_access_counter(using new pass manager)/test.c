#include <stdio.h>
int test(int* b, int* c)
{
   int *a;
   int g=0;
   for(int i=0; i<10; i++) {
       a[i] =b[i] + c[i];
       a[i]++;
   }
   return *b;
}
int main(void)
{
  int *b;
  int *c;
  test(b,c);
  return 0;
}
