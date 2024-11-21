/*------------------------------------------------------------------------------

	"GARLIC_API.h" : cabeceras de funciones del API (Application Program
					Interface) del sistema operativo GARLIC 2.0 (código fuente
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
		parámetro, insertando los valores que se pasan en los siguientes
		parámetros (hasta 2) en la posición y forma (tipo) que se especifique
		con los marcadores incrustados en el string de formato:
			%c	: inserta un carácter (según código ASCII)
			%d	: inserta un natural (32 bits) en formato decimal
			%x	: inserta un natural (32 bits) en formato hexadecimal
			%s	: inserta un string
			%%	: inserta un carácter '%' literal
		Además, también procesa los metacarácteres '\t' (tabulador) y '\n'
		(salto de línia), junto con códigos de formato para cambiar el color
		actual de los caracteres:
			%0	:	fija el color blanco
			%1	:	fija el color amarillo
			%2	:	fija el color verde
			%3	:	fija el color rojo
		El último color seleccionado será persistente en las siguientes llamadas
		a la función. */
extern void GARLIC_printf(char * format, ...);

/* GARLIC_printchar: escribe un carácter (c), especificado como código de
		baldosa (código ASCII - 32), en la posición (vx, vy) de la ventana del
		proceso actual, donde (vx) tiene rango [0..31] y (vy) tiene rango
		[0..23], con el color especificado por parámetro (0 -> blanco,
		1 -> amarillo, 2 -> verde, 3 -> rojo) */
extern void GARLIC_printchar(int vx, int vy, char c, int color);


	/* GARLIC_printmat: escribe una matriz de carácteres (m) en la posición
		(vx, vy) de la ventana del proceso actual, donde (vx) tiene rango
		[0..31] y (vy) tiene rango [0..23], con el color especificado por
		parámetro (0 -> blanco,	1 -> amarillo, 2 -> verde, 3 -> rojo);
		 la matriz consistirá en 8x8 posiciones	con códigos ASCII, aunque las
		posiciones que contengan un código inferior a 32 (espacio en blanco) no
		modificarán las casillas correspondientes de la ventana. */
extern void GARLIC_printmat(int vx, int vy, char m[][8], int color);


	/* GARLIC_delay: retarda la ejecución del proceso actual el número de
		segundos que se especifica por el parámetro (nsec); el rango permitido
		será de [0..600] (max. 10 minutos); el valor 0 provocará que el proceso
		se desbanque y pase a READY, lo cual corresponde a un tiempo de retardo
		muy pequeño, aunque no se puede determinar exactamente cuál (el que
		resulte de volver a restaurar el proceso). */
extern void GARLIC_delay(unsigned int nsec);


	/* GARLIC_clear: borra todo el contenido de la ventana del proceso que
		invoca esta función. La siguiente llamada a la función GARLIC_print
		empezará a escribir a partir de la primera fila de la ventana. */
extern void GARLIC_clear();

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
