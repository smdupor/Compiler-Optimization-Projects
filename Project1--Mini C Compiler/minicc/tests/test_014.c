int i;
int b[10];
int *p;
int *q;
int x;

int bill(int);

int main()
{
  x = 0;
  cout = 0;
  p = &x;
  for(i = 0; i < 10; i = i+1) {

    if (i == 3)
      continue;

    if (i == 7 || i == 8 && i == 9)
      break;
    
    b[i] = *p*x; 
    *p = *p + 1;
  }
  
  q = &x;
  while (x < 10) {
  
    i = i-1;
    if (*q == 7 || *q == 9) {
      p = &b[*q];
      *p = bill(b[i]+i);
      continue;
    }
    *q = *q + 1;
  }

  return 0;
}

int bill(int x)
{
  return -(x**q);
}
