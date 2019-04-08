#include <stdio.h> 
#include <stdlib.h> 
  
int main() 
{ 
    char r = 3, c = 4, i, j, count; 
  
    char **arr = (char **)malloc(r * sizeof(char *)); 
    for (i=0; i<r; i++) 
         arr[i] = (char *)malloc(c * sizeof(char)); 
  
    // Note that arr[i][j] is same as *(*(arr+i)+j) 
    count = 0; 
    for (i = 0; i <  r; i++) 
      for (j = 0; j < c; j++) 
         arr[i][j] = 'c';  // OR *(*(arr+i)+j) = ++count 
  
    for (i = 0; i <  r; i++) 
      for (j = 0; j < c; j++) 
         printf("%c", arr[i][j]); 
  
   /* Code for further processing and free the  
      dynamically allocated memory */
  
   return 0; 
} 