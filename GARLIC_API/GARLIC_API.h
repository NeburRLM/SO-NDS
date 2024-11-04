/*------------------------------------------------------------------------------

	"GARLIC_API.h" : cabeceras de funciones del API (Application Program
					Interface) del sistema operativo GARLIC 1.0 (c�digo fuente
					disponible en "GARLIC_API.s")

------------------------------------------------------------------------------*/
#ifndef _GARLIC_API_h_
#define _GARLIC_API_h_

// Definir size_t como un entero sin signo
typedef unsigned int size_t;

// Declaraci�n de FILE como una estructura vac�a (placeholder)
typedef struct _FILE FILE;

	/* GARLIC_pid: devuelve el identificador del proceso actual */
extern int GARLIC_pid();


	/* GARLIC_random: devuelve un n�mero aleatorio de 32 bits */
extern int GARLIC_random();


	/* GARLIC_divmod: calcula la divisi�n num / den (numerador / denominador),
		almacenando el cociente y el resto en las posiciones de memoria indica-
		das por *quo y *mod, respectivamente (pasa resultados por referencia);
		la funci�n devuelve 0 si la divisi�n es correcta, o diferente de 0
		si hay alg�n problema (divisi�n por cero).
		ATENCI�N: solo procesa n�meros naturales de 32 bits SIN signo. */
extern int GARLIC_divmod(unsigned int num, unsigned int den,
							unsigned int * quo, unsigned int * mod);


	/* GARLIC_divmodL: calcula la divisi�n num / den (numerador / denominador),
		almacenando el cociente y el resto en las posiciones de memoria indica-
		das por *quo y *mod, respectivamente; los par�metros y los resultados
		se pasan por referencia; el numerador y el cociente son de tipo
		long long (64 bits), mientras que el denominador y el resto son de tipo
		unsigned int (32 bits sin signo).
		la funci�n devuelve 0 si la divisi�n es correcta, o diferente de 0
		si hay alg�n problema (divisi�n por cero). */
extern int GARLIC_divmodL(long long * num, unsigned int * den,
							long long * quo, unsigned int * mod);


	/* GARLIC_printf: escribe un string en la ventana del proceso actual,
		utilizando el string de formato 'format' que se pasa como primer
		par�metro, insertando los valores que se pasan en los siguientes
		par�metros (hasta 2) en la posici�n y forma (tipo) que se especifique
		con los marcadores incrustados en el string de formato:
			%c	: inserta un car�cter (seg�n c�digo ASCII)
			%d	: inserta un natural (32 bits) en formato decimal
			%x	: inserta un natural (32 bits) en formato hexadecimal
			%s	: inserta un string
			%%	: inserta un car�cter '%' literal
		Adem�s, tambi�n procesa los metacar�cteres '\t' (tabulador) y '\n'
		(salto de l�nia). */
extern void GARLIC_printf(char * format, ...);

/* PROG G*/
extern void GARLIC_setChar(unsigned char n, unsigned char * buffer);

/* PROG P*/
extern int GARLIC_wait(unsigned char sem);

extern int GARLIC_signal(unsigned char sem);

/* PROG M*/
extern FILE * GARLIC_fopen(const char * filename, const char * mode);

extern int GARLIC_fread(void * buffer, size_t size, size_t numele, FILE * file);

extern int GARLIC_fclose(FILE * file);

#endif // _GARLIC_API_h_
