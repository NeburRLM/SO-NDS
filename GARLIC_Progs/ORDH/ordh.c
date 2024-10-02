/*------------------------------------------------------------------------------
    "HeapSort.c": Programa per ordenar una llista de (arg+1)*10 n�meros aleatoris.
    Utilitza l'algorisme de heapsort i les funcions de l'API de GARLIC per a
    generar n�meros aleatoris i mostrar els resultats.
------------------------------------------------------------------------------*/

#include <GARLIC_API.h>
#include "heapsort.h" // Inclou el fitxer de cap�alera que cont� les declaracions de funcions


int _start(int arg)
{
    unsigned int random_num;
    unsigned int quocient;
	short i;
	
	if (arg < 0) arg = 0;         // Limitar valor m�nim de l'argument
    else if (arg > 3) arg = 3;    // Limitar valor m�xim de l'argument

    // Calcular el tamany de la llista
    short size = (arg + 1) * 10;

    // Generar una llista de n�meros aleatoris
    short arr[size];
	
	
	GARLIC_printf("-- Programa ORDH --\n");
	
    // Generar n�meros aleatoris y emmagatzemar-los en la llista generada
	GARLIC_printf("-- Llista desordenada --\n");
    for (i = 0; i < size; i++)
	{
        GARLIC_divmod(GARLIC_random(), 1000, &quocient, &random_num);
        arr[i] = random_num;
		GARLIC_printf("%d ", arr[i]);
    }
	GARLIC_printf("\n");

    // Ordenar la llista utilitzant heapsort
    heapSort(arr, size);

    // Imprimir la llista ordenada
    GARLIC_printf("-- Llista ordenada --\n");
    for (i = 0; i < size; i++)
	{
        GARLIC_printf("%d ", arr[i]);
    }
    GARLIC_printf("\n");

    return 0;
}