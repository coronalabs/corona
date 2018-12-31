/*
* glue.h
* glue exe and script
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 25 Aug 2004 22:06:50
* This code is hereby placed in the public domain.
*/

#define GLUESIG		"%%glue:L"
#define GLUELEN		(sizeof(GLUESIG)-1)
#define GLUETYP		(sizeof(GLUESIG)-2)

typedef struct { char sig[GLUELEN]; long size1, size2; } Glue;
