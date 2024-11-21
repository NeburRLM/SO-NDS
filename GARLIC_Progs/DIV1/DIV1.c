/*------------------------------------------------------------------------------

	"DIV1.c" : programa de prueba GARLIC 1.0;
	
	Determinar múltiples de 2, 3, 5 o 7 de una lista de 10*arg+1 
	numeros aleatorios.

------------------------------------------------------------------------------*/

#include <GARLIC_API.h>

int _start(int arg)
{
	//1.Crear una lista de 10*arg+1 numeros aleatorios
	//2.Crear una función para determinar los multiples
	unsigned int tam = 10*arg+1; //Calculamos el tamaño que tendra la lista
	unsigned int numAl[tam]; //Creamos la lista vacia
	unsigned int quo, mod;
	for(int i=0; i<tam; i++){ //Llenamos la lista de valores aleatorios
		numAl[i]=GARLIC_random();
	}
	
	for(int j=0; j<tam; j++){
		GARLIC_divmod(numAl[j], 2, &quo, &mod);
		if(mod==0){
			GARLIC_printf("(%d)->(%d) es multiplo de 2\n", GARLIC_pid(),numAl[j]);
		}else{
			GARLIC_divmod(numAl[j], 3, &quo, &mod);
			if(mod==0){
				GARLIC_printf("(%d)->(%d) es multiplo de 3\n", GARLIC_pid(),numAl[j]);
			}else{
				GARLIC_divmod(numAl[j], 5, &quo, &mod);
				if(mod==0){
					GARLIC_printf("(%d)->(%d) es multiplo de 5\n", GARLIC_pid(),numAl[j]);
				}else{
					GARLIC_divmod(numAl[j], 7, &quo, &mod);
					if(mod==0){
						GARLIC_printf("(%d)->(%d) es multiplo de 7\n", GARLIC_pid(),numAl[j]);
					}else{
						GARLIC_printf("(%d)->(%d) no es multiplo de 2, de 3, de 5 ni de 7\n", GARLIC_pid(),numAl[j]);
					}
				}
			}
		}
		
	}
	return 0;
}
