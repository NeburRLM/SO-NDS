/*------------------------------------------------------------------------------
    "HeapSort.c": Programa per ordenar una llista de (arg+1)*10 números aleatoris.
    Utilitza l'algorisme de heapsort i les funcions de l'API de GARLIC per a
    generar números aleatoris i mostrar els resultats.
------------------------------------------------------------------------------*/

#include <GARLIC_API.h>
#include "heapsort.h" // Inclou el fitxer de capçalera que conté les declaracions de funcions


int _start(int arg)
{
    unsigned int random_num;
    unsigned int quocient;
	short i;
	
	if (arg < 0) arg = 0;         // Limitar valor mínim de l'argument
    else if (arg > 3) arg = 3;    // Limitar valor màxim de l'argument

    // Calcular el tamany de la llista
    short size = 40;	//(arg + 1) * 10

    // Generar una llista de números aleatoris
    short arr[size];
	
	
	GARLIC_printf("-- Programa ORDH --\n");
	
	//////////////Llegir arxiu de /Datos//////////////
	GARLIC_printf("\n\n#####Llegint fitxer /Datos/#####\n");
	// Definir el buffer estàtic de tamany fixe per la lectura de l'arxiu
    char buffer[64];  	//reducció del tamany per no sobrepassar la pila a la DTCM

    // Obrim l'arxiu per llegir
    FILE* inf = GARLIC_fopen("file1", "rb");
	if (inf == 0) {   // Si no s'ha pogut obrir l'arxiu
        GARLIC_printf("Error al obrir l'arxiu\n");
        return -1;
    }
	
    // Llegir l'arxiu usant GARLIC_fread, llegint el buffer
    size_t bytesRead = GARLIC_fread(buffer, 1, sizeof(buffer), inf);
    if (bytesRead <= 0) {
        GARLIC_printf("Error al llegir l'arxiu\n");
    } else {
        // Mostrar el contingut de l'arxiu llegit
        GARLIC_printf("-> Contingut de l'arxiu:\n");
        GARLIC_printf("Bytes read: %d\n%s\n", (int)bytesRead, buffer);  // Mostrar els bytes llegits en text
    }
    // Tancar l'arxiu
    GARLIC_fclose(inf);
	GARLIC_printf("\n\n##########################\n\n");
	//////////////Arxiu llegit de /Datos//////////////
    
	
    // Generar números aleatoris y emmagatzemar-los en la llista generada
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