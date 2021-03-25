
int fact(int i){
  if(i == 0)
    return 1;
  else
    return fact(i-1) * i;
}

int main()
{
  int i;
  int a[5];
  int facts[5];

  i = 0;
  while(i < 5){
    a[i] = i * (i + 1);

    if(i == 3)
      i = i + 1;
  }

  for(i = 0; i < 5; i = i + 1){
    facts[i] = fact(i);
   }
   return facts[2];
}

