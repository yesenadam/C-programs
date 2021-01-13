#include <stdio.h> 
int main (void) 
{ 
//define variables 
unsigned short two, n, sum; 
n=3;
two = 2; 
printf ("Enter a number indicating a range: \n"); 
scanf ("%hu", &n); 
//switch to assembly 
_asm 
{
MOV AX, n //n -> ax 
INC AX //n+1 -> ax 
MUL n //n(n+1) -> dx:ax 
DIV two //n(n+1)/2 -> dx:ax //DIV two
MUL AX //[n(n+1)/2]^2 
MOV sum, AX 
} 
//print sum 
printf ("\nSum of cubes 1 through n = %u\n\n", sum); 
return 0; 
} 