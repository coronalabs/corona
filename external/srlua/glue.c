/*
* glue.c
* glue exe and script
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 19 Feb 2005 09:14:06
* This code is hereby placed in the public domain.
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glue.h"

static void cannot(const char* what, const char* name)
{
 fprintf(stderr,"cannot %s %s: %s\n",what,name,strerror(errno));
 exit(EXIT_FAILURE);
}

static long copy(const char* name, FILE* out, const char* outname)
{
 FILE* f;
 long size;
 f=fopen(name,"rb");
 if (f==NULL) cannot("open",name);
 if (fseek(f,0,SEEK_END)!=0) cannot("seek",name);
 size=ftell(f);
 if (fseek(f,0,SEEK_SET)!=0) cannot("seek",name);
 for (;;)
 {
  char b[BUFSIZ];
  int n=fread(&b,1,sizeof(b),f);
  if (n==0) { if (ferror(f)) cannot("read",name); else break; }
  if (fwrite(&b,n,1,out)!=1) cannot("write",outname);
 }
 fclose(f);
 return size;
}

int main(int argc, char* argv[])
{
 Glue t= { GLUESIG, 0, 0 };
 FILE* f;
 if (argc<4)
 {
  fprintf(stderr,"usage: glue c.exe prog.lua prog.exe\n");
  return 1;
 }
 f=fopen(argv[3],"wb");
 if (f==NULL) cannot("open",argv[3]);
 t.size1=copy(argv[1],f,argv[3]);
 t.size2=copy(argv[2],f,argv[3]);
 t.sig[GLUETYP]= (argv[4]!=NULL) ? argv[4][0] : 'L';
 if (fwrite(&t,sizeof(t),1,f)!=1) cannot("write",argv[3]);
 if (fclose(f)!=0) cannot("close",argv[3]);
 return 0;
}
