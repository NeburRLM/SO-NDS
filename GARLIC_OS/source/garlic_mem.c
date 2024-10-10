/*------------------------------------------------------------------------------

	"garlic_mem.c" : fase 1 / programador M

	Funciones de carga de un fichero ejecutable en formato ELF, para GARLIC 1.0

------------------------------------------------------------------------------*/
#include <nds.h>
#include <filesystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "garlic_system.h"	// definici�n de funciones y variables de sistema
#include <elf.h> 

int _gm_primeraPosMem = INI_MEM;

/* _gm_initFS: inicializa el sistema de ficheros, devolviendo un valor booleano
					para indiciar si dicha inicializaci�n ha tenido �xito;
*/
int _gm_initFS()
{
	return nitroFSInit(NULL);	// inicializar sistema de ficheros NITRO
}


// funcio per calcular un valor m�ltiple de 4
Elf32_Word ferMultiple(Elf32_Word tSeg)
{
	int multiple;															//per calcular el valor m�ltiple de 4 
	multiple = tSeg % 4;													//_gm_primeraPosMem multiple de 4, per al seg�ent programa a partir de la suma del tamanySeg
	if (multiple != 0)
	{
		tSeg = tSeg + (4 - multiple);
	}
	return tSeg;
}



FILE* _gm_fopen(const char* filename, const char* mode) {
    char path[256]; 
    sprintf(path, "/Datos/%s", filename); 
    
	return fopen(path, mode); 
}


size_t _gm_fread(void * buffer, size_t size, size_t numele, FILE * file) {
    return fread(buffer, size, numele, file); 
}


int _gm_fclose(FILE * file) {
    return fclose(file);  
}



/* _gm_cargarPrograma: busca un fichero de nombre "(keyName).elf" dentro del
					directorio "/Programas/" del sistema de ficheros y carga
					los segmentos de programa a partir de una posici�n de
					memoria libre, efectuando la reubicaci�n de las referencias
					a los s�mbolos del programa seg�n el desplazamiento del
					c�digo en la memoria destino;
	Par�metros:
		keyName ->	string de 4 caracteres con el nombre en clave del programa
	Resultado:
		!= 0	->	direcci�n de inicio del programa (intFunc)
		== 0	->	no se ha podido cargar el programa
*/
intFunc _gm_cargarPrograma(char *keyName)
{

	char pathFit[20];														//per guardar el keyName rebut per par�metre
	FILE *fitxer;															//per gestionar el fitxer .elf a obrir
	long tamany;															//per guardar el nombre d'elements que cont� el fitxer .elf
	char *buffer;															//per emmagatzemar el contingut del fitxer .elf en mem�ria din�mica com una seq��ncia de car�cters																		//per emmagatzemar el contingut del fitxer .elf en mem�ria din�mica com una seq��ncia de car�cters								
	unsigned int offset;													//offset de la taula de segments
	unsigned short entry;													//primera instruccio a executar del programa
	unsigned short phentsize;												//tamany de cada entrada de la taula de segments
	unsigned short phnum;													//n�mero d'entrades de la taula de segments																//per guardar l'adre�a final del programa a retornar
	int i;																	//bucle per rec�rrer els segments	
	Elf32_Ehdr capcaleraElf;												//per emmagatzemar l'estructura correcta de la cap�alera del .elf
	Elf32_Phdr taulaSeg;													//per emmagatzemar l'estructura correcta de la taula de segments
	Elf32_Word tamanyFile, tamanySeg;										//per emmagatzemar el tamany memSize i el fileSize del programa
	intFunc adrProg = 0;													//variable aux per guardar l'adressa del programa actual
	
	
    //buscar el fitxer
	sprintf(pathFit, "/Programas/%s.elf", keyName);							//guardem el path del "fitxer.elf" en una cadena de car�cters
	fitxer = fopen(pathFit, "rb"); 											//obrim el fitxer anterior en mode lectura binaria
		
	//si fitxer no trobat en programas del nitrofiles retornem 0, sin� passem a carregar-ho a mem�ria
	if (fitxer != NULL)					
	{
		//si fitxer trobat, calculem el tamany (quantitat d'elements) per saber el que es necessita per reservar-ho en mem�ria din�mica
		fseek(fitxer, 0, SEEK_END);											//moure el punter del fitxer al final 
		tamany = ftell(fitxer);												//obtenir la posici� actual del punter (que equivaldr� al tamany del fitxer=num d'elements)
		fseek(fitxer, 0, SEEK_SET);											//situem el punter al principi del fitxer per fer gestions posteriors
			
		//cargar el fitxer �ntegrament dins d'un buffer de mem�ria din�mica per un acc�s al seu contingut m�s eficient 
		buffer = (char*) malloc(tamany);									//assignem din�micament mem�ria en el heap pel buffer de car�cters en funci� del tamany del fitxer 	
		fread(buffer, sizeof(char), tamany, fitxer); 						//guardem el contingut del fitxer al buffer 								
		memcpy(&capcaleraElf, buffer, sizeof(Elf32_Ehdr));					//cap�alera .elf
			
		offset = capcaleraElf.e_phoff;										//e_phoff (offset de la taula de segments)
		entry = capcaleraElf.e_entry;										//e_entry (primera instruccio a executar del programa)								
		phentsize = capcaleraElf.e_phentsize;								//e_phentsize (tamany de cada entrada de la taula de segments)
		phnum = capcaleraElf.e_phnum;										//e_phnum (n�mero d'entrades de la taula de segments)
			
		if (phnum != 0)														//si t� entrada en la taula de segments
		{
			memcpy(&taulaSeg, buffer + offset, sizeof(Elf32_Phdr)); 		//llegim la taula de segments del programa en l'estructura establerta pels segments			
			for (i = 0; i < phnum; i++) 									//guardarem cadascun dels segments del programa a cada iteraci�
			{ 										
				if (taulaSeg.p_type == PT_LOAD) 							//si l'entrada �s de tipus PT_LOAD
				{									
					tamanyFile = taulaSeg.p_filesz;							//obtenim la informaci� del tamany del segment dins del fitxer del programa actual
					tamanySeg = taulaSeg.p_memsz;							//obtenim la informaci� del segment del programa actual (capcaleraElf)																		
					tamanySeg = ferMultiple(tamanySeg);						//fer c�lcul m�ltiple per fer la pr�xima c�pia a mem�ria
					if (_gm_primeraPosMem + tamanySeg <= END_MEM) 			//verifiquem si la posici�n de mem�ria _gm_primeraPosMem no supera la direcci� final de mem�ria
					{
						//copia el contingut del segment de programa des del buffer en la direcci�n de mem�ria _gm_primeraPosMem (_gs_copiaMem(const void *source, void *dest, unsigned int numBytes))
						_gs_copiaMem((const void *) buffer + taulaSeg.p_offset, (void *) _gm_primeraPosMem, tamanyFile);
						//aplica reubicancions per ajustar refer�ncies
						_gm_reubicar(buffer, taulaSeg.p_paddr, (unsigned int *) _gm_primeraPosMem);	
						//direcci� d'inici del programa a la mem�ria f�sica, tenint en compte totes les reubicacions necess�ries perqu� el programa s'executi correctament des de la posici� en mem�ria on s'ha carregat
						adrProg = (intFunc) (_gm_primeraPosMem + entry - taulaSeg.p_paddr);				
						//actualitzem mem�ria pel seg�ent programa tenint en compte el tamany del segment actual carregat a mem�ria											
						_gm_primeraPosMem += tamanySeg;								
					}
				}			
				//comprovem si s'ha de seguir amb les iteracions segons si queden segments a tractar
				if (i+1 < phnum)																	
				{	
					offset = offset + phentsize;							//actualitzem offset per a que apunti al seg�ent segment del .elf				
					// Carga la seg�ent entrada de la taula de segments en taulaSeg directament desde el buffer
					memcpy(&taulaSeg, buffer + offset, sizeof(Elf32_Phdr));
				}														
			}
		}
		fclose(fitxer);														//tanquem fitxer
		free(buffer);														//netejem buffer
	}
    																
	return ((intFunc) adrProg);
}

