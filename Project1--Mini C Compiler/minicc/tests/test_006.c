struct complex {
  int real;
  int imag;
};

int main()
{
  struct complex c;
  struct complex *p;
  struct complex **pp;
  struct complex ***ppp;
  struct complex arr[100];

  c.real = 10;
  c.imag = 1;
  p = &c;
  pp = &p;
  ppp = &pp;

  return 0;
}
