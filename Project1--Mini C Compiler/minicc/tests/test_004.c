int main()
{
  int arr[100];
  int y;
  int *x;
  int **w;

  x = &arr[10];
  
  *x = 5;


  for(y=0; y<100; y=y+1)
    arr[y] = *x + 10;
  
  
  
  if(arr[10] == 100)
    if(arr[9] == 100)
      *x =0;
    else
      *x = 1;

  return y;
}
