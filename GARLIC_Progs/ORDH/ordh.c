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
	
	////////////// Llegir arxiu de /Datos //////////////
	char nomFile[] = "file1";
	GARLIC_printf("\n\n\n### Llegint fitxer %s ###\n", nomFile);

	// Definir el buffer estàtic de tamany fixe per la lectura de el fitxer
	char buffer[64];  // Reducció del tamany per no sobrepassar la pila a la DTCM

	// Obrim el fitxer per llegir
	GARLIC_FILE* inf = GARLIC_fopen(nomFile, "a");
	if (inf == 0) 
	{   // Si no s'ha pogut obrir el fitxer
		GARLIC_printf("Error al obrir el %s\n", nomFile);
		return -1;
	}
	else
	{
		GARLIC_printf("#Fitxer %s obert#\n", nomFile);
	}

	// Llegir el fitxer usant GARLIC_fread, llegint el buffer
	size_t bytesRead = GARLIC_fread(buffer, 1, sizeof(buffer), inf);
	if (bytesRead <= 0) 
	{
		GARLIC_printf("#Error al llegir el %s\n", nomFile);
	} else 
	{
		// Mostrar el contingut de el fitxer llegit
		GARLIC_printf("-> Contingut del %s:\n", nomFile);
		GARLIC_printf("%s\n", buffer);  // Mostrar els bytes llegits en text
	}

	// Creem un nou contingut per escriure en el fitxer
	char newContent[] = "Hola, soc el programa de usuari ORDH.\n";

	// Escriure noves dades en el fitxer
	size_t bytesWritten = GARLIC_fwrite(newContent, 1, sizeof(newContent) - 1, inf);
	if (bytesWritten <= 0) {
		GARLIC_printf("Error al escriure al %s\n", nomFile);
	} else 
	{
		GARLIC_printf("-> Escrivint nous continguts:\n%s\n", newContent);
	}


	// Llegirlo novament i verificar l'escriptura
	GARLIC_printf("\n\n### Llegint fitxer de nou ###\n");

	// Llegir el fitxer novament
	bytesRead = GARLIC_fread(buffer, 1, sizeof(buffer), inf);
	if (bytesRead <= 0) 
	{
		GARLIC_printf("#Error al llegir el %s despres de l'escriptura\n", nomFile);
	} else 
	{
		// Mostrar el contingut del fitxer després de l'escriptura
		GARLIC_printf("-> Contingut de %s despres de l'escriptura:\n", nomFile);
		GARLIC_printf("%s\n", buffer);  // Mostrar els bytes llegits en text
	}

	// Tancar el fitxer després de la lectura final
	GARLIC_fclose(inf);
	
	//////////////////////////////////////////////////////////////////////
	
	
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
    GARLIC_printf("\n\n");

    return 0;
}