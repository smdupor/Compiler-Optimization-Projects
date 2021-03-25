int i;
int j;
int *ptr;
int a[10];

int foo3(int k){ return k*k;     }
int foo2(int j){ return foo3(j); }
int foo1(int i){ return foo2(i); }

int main()
{
  ptr = a;
  while (ptr < a+5) {
    *ptr = cin;
    ptr = ptr + 1; 
  }

  for(i = 0; i < 5; i = i+1){
    *ptr = foo1(*(ptr-5));
    ptr = ptr + 1;
  }

  ptr = &(a[3]) + 2;
  while (ptr < a+10) {
    cout = *ptr;
    ptr = ptr + 1;
  }

  return 0;
}



