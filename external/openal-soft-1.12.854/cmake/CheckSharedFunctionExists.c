#ifdef CHECK_SHARED_FUNCTION_EXISTS

#include <stdlib.h>

#ifndef CALLSTACK
#define CALLSTACK
#endif

#ifdef _WIN32
#ifdef ARGSTACK
char __stdcall CHECK_SHARED_FUNCTION_EXISTS(ARGSTACK);
#else
char __stdcall CHECK_SHARED_FUNCTION_EXISTS(void);
#endif
#else
char CHECK_SHARED_FUNCTION_EXISTS();
#endif

#ifdef __CLASSIC_C__
int main(){
  int ac;
  char*av[];
#else
int main(int ac, char*av[]){
#endif
  CHECK_SHARED_FUNCTION_EXISTS(CALLSTACK);
  if(ac > 1000)
    {
    return *av[0];
    }
  return 0;
}

#else  /* CHECK_SHARED_FUNCTION_EXISTS */

#  error "CHECK_SHARED_FUNCTION_EXISTS has to specify the function"

#endif /* CHECK_SHARED_FUNCTION_EXISTS */
