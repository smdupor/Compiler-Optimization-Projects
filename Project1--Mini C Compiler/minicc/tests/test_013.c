int i;
int j;
int fibs[10];

int fibonacci(int i, int temp);

int main()
{
  for(i = 5; i < 10; i = i + 1){

    if (i == 3)
      continue;
    
    for(j = 0; j < 5; j = j + 1){

      if(j == 4){
	break;
	fibs[8] = fibonacci(8, 0);
      }
      fibs[i-5+j] = fibonacci(i+j, 0);
    }
  }

  fibs[9] = fibonacci(22, 0);
  return 0;
}

int fibonacci(int i, int temp){
  if(i == 0)
    return 0;

  if(i == 1)
    return 1;

  temp = fibonacci(i-2, 0);

  return fibonacci(i-1, 0) + temp;
}
