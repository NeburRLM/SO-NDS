/*------------------------------------------------------------------------------

	"MEMP.c" : programa de prueba para probar el sistema de ficheros propio;
	

------------------------------------------------------------------------------*/

#include <GARLIC_API.h>			/* definición de las funciones API de GARLIC */
#define FILE_SIZE 65536  // 64 KB
#define L_SIZE 32

int _start(int arg)				/* función de inicio : no se usa 'main' */
{
	int r = 0;
	GARLIC_printf("-- Programa MEMP --\n");

	////////////// Llegir arxiu de /Datos //////////////
	char nomFile[] = "file1";
	GARLIC_printf("\n\n\n### Llegint fitxer %s ###\n", nomFile);
	
	// Definir el buffer estàtic de tamany fixe per la lectura de el fitxer
	char buffer[64];  // Reducció del tamany per no sobrepassar la pila a la DTCM
	
	// Obrim el fitxer per llegir
	GARLIC_FILE* inf = GARLIC_fopen(nomFile, "a");
	if (inf == 0) 
	{   // Si no s'ha pogut obrir el fitxer
		GARLIC_printf("Error al obrir el %s. Obert per un altre proces\n", nomFile);
		r = -1;
	}
	else
	{
		GARLIC_printf("#Fitxer %s obert#\n", nomFile);
	}
	
	if (r == 0)  // Si s'ha pogut obrir el file1 correctament
	{
		// Llegir el fitxer usant GARLIC_fread, llegint el buffer
		size_t bytesRead = GARLIC_fread(buffer, 1, sizeof(buffer), inf);
		if (bytesRead <= 0) 
		{
			GARLIC_printf("#Error al llegir el %s\n", nomFile);
		} 
		else 
		{
			// Mostrar el contingut de el fitxer llegit
			GARLIC_printf("-> Contingut del %s:\n", nomFile);
			GARLIC_printf("%s\n", buffer);  // Mostrar els bytes llegits en text
		}
		// Tancar el fitxer després de la lectura final
		GARLIC_fclose(inf);
	}

	// Obrim el fitxer per llegir
	r = 0; 
	char nomFile9[] = "file9";
	GARLIC_FILE* inf9 = GARLIC_fopen(nomFile9, "a");
	if (inf9 == 0) 
	{   // Si no s'ha pogut obrir el fitxer
		GARLIC_printf("Error al obrir el %s. Inexistent\n", nomFile9);
		r = -1;
	}
	else
	{
		GARLIC_printf("#Fitxer %s obert#\n", nomFile9);
	}
	
	
	// Obrim el fitxer 2 per llegir
	////////////// Llegir arxiu de /Datos //////////////
	r = 0;
	char nomFile2[] = "file2";
	GARLIC_printf("\n\n\n### Llegint fitxer %s ###\n", nomFile2);
	
	// Obrim el fitxer per llegir
	GARLIC_FILE* inf2 = GARLIC_fopen(nomFile2, "w");
	if (inf2 == 0) 
	{   // Si no s'ha pogut obrir el fitxer
		GARLIC_printf("Error al obrir el %s. Obert per un altre proces\n", nomFile2);
		r = -1;
	}
	else
	{
		GARLIC_printf("#Fitxer %s obert#\n", nomFile2);
	}
	
	if (r == 0)  // Si s'ha pogut obrir el file1 correctament
	{
		
		// Definir el buffer estàtic de tamany fixe per la lectura de el fitxer
		char buffer[64];  // Reducció del tamany per no sobrepassar la pila a la DTCM
		
		// Llegir el fitxer usant GARLIC_fread, llegint el buffer
		size_t bytesRead = GARLIC_fread(buffer, 1, sizeof(buffer), inf2);
		if (bytesRead <= 0) 
		{
			GARLIC_printf("#Error al llegir el %s\n", nomFile2);
		} 
		else 
		{
			// Mostrar el contingut de el fitxer llegit
			GARLIC_printf("-> Contingut del %s:\n", nomFile2);
			GARLIC_printf("%s\n", buffer);  // Mostrar els bytes llegits en text
		}

		// Creem un nou contingut per escriure en el fitxer
		char newContent[] = "Hola, soc el programa de usuari MEMP.\n";
	
		// Escriure noves dades en el fitxer
		size_t bytesWritten = GARLIC_fwrite(newContent, 1, sizeof(newContent) - 1, inf2);
		if (bytesWritten <= 0) {
			GARLIC_printf("Error al escriure al %s\n", nomFile2);
		} 
		else 
		{
			GARLIC_printf("-> Escrivint nous continguts:\n%s\n", newContent);
		}
		
		// Llegirlo novament i verificar l'escriptura
		GARLIC_printf("\n\n### Llegint fitxer de nou ###\n");

		// Llegir el fitxer novament
		bytesRead = GARLIC_fread(buffer, 1, sizeof(buffer), inf2);
		if (bytesRead <= 0) 
		{
			GARLIC_printf("#Error al llegir el %s despres de l'escriptura\n", nomFile2);
		} 
		else 
		{
			// Mostrar el contingut del fitxer després de l'escriptura
			GARLIC_printf("-> Contingut de %s despres de l'escriptura:\n", nomFile2);
			GARLIC_printf("%s\n", buffer);  // Mostrar els bytes llegits en text
		}

		// Tancar el fitxer després de la lectura final
		GARLIC_fclose(inf2);
		
		GARLIC_printf("\n\n\n### Llegint fitxer TANCAT %s ###\n", nomFile2);
		// Llegir el fitxer usant GARLIC_fread, llegint el buffer
		bytesRead = GARLIC_fread(buffer, 1, sizeof(buffer), inf2);
		if (bytesRead <= 0) 
		{
			GARLIC_printf("#Error al llegir el %s\n", nomFile2);
		} 
		else 
		{
			// Mostrar el contingut de el fitxer llegit
			GARLIC_printf("-> Contingut del %s:\n", nomFile2);
			GARLIC_printf("%s\n", buffer);  // Mostrar els bytes llegits en text
		}	
	}
	
	// Obrim el fitxer 3 per llegir
	////////////// Llegir arxiu de /Datos //////////////
	r = 0;
	char nomFile3[] = "file3";
	GARLIC_printf("\n\n\n### Escriure fitxer %s ###\n", nomFile3);
	
	// Obrim el fitxer per llegir
	GARLIC_FILE* inf3 = GARLIC_fopen(nomFile3, "a");
	if (inf3 == 0) 
	{   // Si no s'ha pogut obrir el fitxer
		GARLIC_printf("Error al obrir el %s. Obert per un altre proces\n", nomFile3);
		r = -1;
	}
	else
	{
		GARLIC_printf("#Fitxer %s obert#\n", nomFile3);
	}
	
	if (r == 0)  // Si s'ha pogut obrir el file3 correctament
	{
		// Creem un nou contingut per escriure en el fitxer
		char newContent[] = "S";
	
		// Escriure noves dades en el fitxer
		size_t bytesWritten = GARLIC_fwrite(newContent, 1, sizeof(newContent) - 1, inf3);
		if (bytesWritten <= 0) {
			GARLIC_printf("Error al escriure al %s\n", nomFile3);
		} 
		else 
		{
			GARLIC_printf("-> Escrivint nous continguts:\n%s\n", newContent);
		}
		
		// Tancar el fitxer després de la lectura final
		GARLIC_fclose(inf3);
	}
	return 0;
}