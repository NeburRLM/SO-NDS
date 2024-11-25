/*------------------------------------------------------------------------------

	"garlic_mem.c" : fase 1 / programador M

	Funciones de carga de un fichero ejecutable en formato ELF, para GARLIC 1.0

------------------------------------------------------------------------------*/
#include <nds.h>
#include <filesystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "garlic_system.h"	// definición de funciones y variables de sistema
#include <elf.h> 

int _gm_primeraPosMem = INI_MEM;

/* _gm_initFS: inicializa el sistema de ficheros, devolviendo un valor booleano
					para indiciar si dicha inicialización ha tenido éxito;
*/
int _gm_initFS()
{
	return nitroFSInit(NULL);	// inicializar sistema de ficheros NITRO
}


/* _gm_listaProgs: devuelve una lista con los nombres en clave de todos
			los programas que se encuentran en el directorio "Programas".
			 Se considera que un fichero es un programa si su nombre tiene
			8 caracteres y termina con ".elf"; se devuelven sólo los
			4 primeros caracteres de los programas (nombre en clave).
			 El resultado es un vector de strings (paso por referencia) y
			el número de programas detectados */
int _gm_listaProgs(char* progs[])
{
	//Inicialitzacions de variable de retorn i contador
	int ret=0;	//guardarà el valor de retorn de la funció
	int i = 0;	//guardarà el contador per guardar cada programa llegit
	struct dirent *entrada;	//guardarà el nom i el tipus de l'arxiu
	
	// Obre el directori "Programas/"
	DIR *directori = opendir("Programas/");
	
	// Verifica si el directori s'ha obert correctament
	if (directori != NULL) 
	{
		// Bucle per recórrer les entrades del directori a partir del punter directori
		while ((entrada = readdir(directori)) != NULL) 
		{
			// Filtra per longitud del nom i tipus d'entrada (arxiu regular)
			if (entrada->d_type == DT_REG && strlen(entrada->d_name) == 8 && strstr(entrada->d_name, ".elf") != NULL) 
			{
				// Assigna memòria per emmagatzemar el nom en clau (4 caràcters + terminador nul)
				char *buffer = malloc(5);
				// Copia els primers 4 caràcters del nom de l'entrada al buffer
				strncpy(buffer, entrada->d_name, 4);
				// Agrega el terminador nul al final del buffer
				buffer[4] = '\0';
				// Emmagatzema el buffer al vector progs
				progs[i] = buffer;
				// Incrementa el contador de programes
				i++;
			}
		}
		ret = i;	// Assigna el valor de i al resultat de la funció
	}
	//Tanca el stream del directori
	closedir(directori);
	//Retorna el número total de programes trobats
	return ret;
}


// funcio per calcular un valor múltiple de 4
Elf32_Word ferMultiple(Elf32_Word tSeg)
{
	int multiple;															//per calcular el valor múltiple de 4 
	multiple = tSeg % 4;													//_gm_primeraPosMem multiple de 4, per al següent programa a partir de la suma del tamanySeg
	if (multiple != 0)
	{
		tSeg = tSeg + (4 - multiple);
	}
	return tSeg;
}



FILE* _gm_fopen(const char* filename, const char* mode)
{
	char path[22]; //"Datos/filename"
	sprintf(path, "/Datos/%s", filename); 
	
	return fopen(path, mode); 
}


size_t _gm_fread(void * buffer, size_t size, size_t numele, FILE * file)
{
	return fread(buffer, size, numele, file); 
}


int _gm_fclose(FILE * file)
{
	return fclose(file);  
}



/* _gm_cargarPrograma: busca un fichero de nombre "(keyName).elf" dentro del
					directorio "/Programas/" del sistema de ficheros y carga
					los segmentos de programa a partir de una posición de
					memoria libre, efectuando la reubicación de las referencias
					a los símbolos del programa según el desplazamiento del
					código y los datos en la memoria destino;
	Parámetros:
		zocalo	->	índice del zócalo que indexará el proceso del programa
		keyName ->	string de 4 carácteres con el nombre en clave del programa
	Resultado:
		!= 0	->	dirección de inicio del programa (intFunc)
		== 0	->	no se ha podido cargar el programa
*/
intFunc _gm_cargarPrograma(int zocalo, char *keyName)
{

	char pathFit[20];																//per guardar el keyName rebut per paràmetre
	FILE *fitxer;																	//per gestionar el fitxer .elf a obrir
	Elf32_Ehdr capcaleraElf;														//per emmagatzemar l'estructura correcta de la capçalera del .elf
	Elf32_Phdr taulaSeg;															//per emmagatzemar l'estructura correcta de la taula de segments
	long tamany;																	//per guardar el nombre d'elements que conté el fitxer .elf
	char *buffer;																	//per emmagatzemar el contingut del fitxer .elf en memòria dinámica com una seqüència de caràcters
	unsigned int offset;															//offset de la taula de segments
	unsigned int offsetSegCodi;														//offset segment codi
	unsigned int offsetSegDades;													//offset segment dades
	unsigned short entry;															//primera instruccio a executar del programa
	unsigned short phnum;															//número d'entrades de la taula de segments							
	unsigned int tamanySegCodi; 													//per emmagatzemar el tamany memSize del segment de codi
	unsigned int tamanySegDades;													//per emmagatzemar el tamany memSize del segment de dades
	unsigned int tamanyFileCodi;													//per emmagatzemar el tamany p_filesz del fitxer de codi
	unsigned int tamanyFileDades;													//per emmagatzemar el tamany p_filesz del fitxer de dades
	unsigned int paddrSegCodi;														//direcció física segment codi
	unsigned int paddrSegDades;														//direcció física segment dades
	unsigned int primerDirCodi = 0; 												//guardarà la primera direcció de l'espai reservat del segment de codi
	unsigned int primerDirDades = 0; 												//guardarà la primera direcció de l'espai reservat del segment de dades
	int i;																			//bucle per recòrrer els segments	
	intFunc adrProg = 0;															//variable aux per guardar l'adressa del programa actual

	
	//buscar el fitxer
	sprintf(pathFit, "/Programas/%s.elf", keyName);									//guardem el path del "fitxer.elf" en una cadena de caràcters
	fitxer = fopen(pathFit, "rb"); 													//obrim el fitxer anterior en mode lectura binaria
		
	//si fitxer no trobat en programas del nitrofiles retornem 0, sinò passem a carregar-ho a memòria
	if (fitxer != NULL)					
	{
		//si fitxer trobat, calculem el tamany (quantitat d'elements) per saber el que es necessita per reservar-ho en memòria dinàmica
		fseek(fitxer, 0, SEEK_END);													//moure el punter del fitxer al final 
		tamany = ftell(fitxer);														//obtenir la posició actual del punter (que equivaldrà al tamany del fitxer=num d'elements)
		fseek(fitxer, 0, SEEK_SET);													//situem el punter al principi del fitxer per fer gestions posteriors
			
		//cargar el fitxer íntegrament dins d'un buffer de memòria dinàmica per un accés al seu contingut més eficient 
		buffer = (char*) malloc(tamany);											//assignem dinàmicament memòria en el heap pel buffer de caràcters en funció del tamany del fitxer 	
		fread(buffer, sizeof(char), tamany, fitxer); 								//guardem el contingut del fitxer al buffer 								
		fclose(fitxer);																//tanquem fitxer
		memcpy(&capcaleraElf, buffer, sizeof(Elf32_Ehdr));							//capçalera .elf
			
		offset = capcaleraElf.e_phoff;												//e_phoff (offset de la taula de segments)
		entry = capcaleraElf.e_entry;												//e_entry (primera instruccio a executar del programa)								
		phnum = capcaleraElf.e_phnum;												//e_phnum (número d'entrades de la taula de segments)
			
		if (phnum != 0)																//si té entrada en la taula de segments
		{
			for (i = 0; i < phnum; i++) 											//guardarem cadascun dels segments del programa a cada iteració
			{ 										
				memcpy(&taulaSeg, buffer + offset, sizeof(Elf32_Phdr)); 			//llegim la taula de segments del programa en l'estructura establerta pels segments
				if (taulaSeg.p_type == PT_LOAD) 									//si l'entrada és de tipus PT_LOAD
				{									
					if (taulaSeg.p_flags == (PF_R | PF_X)) 							//si els flags del segment són lectura i executable (segment codi)
					{
						tamanySegCodi = taulaSeg.p_memsz;							//obtenim la informació del tamany del segment de codi del programa actual (capcaleraElf)
						tamanyFileCodi = taulaSeg.p_filesz;							//obtenim la informació del tamany del fitxer del segment de codi del programa actual (capcaleraElf)
						offsetSegCodi = taulaSeg.p_offset;							//obtenim la informació del offset del segment de codi del programa actual (capcaleraElf)
						paddrSegCodi = taulaSeg.p_paddr;							//obtenim la informació de la direcció física del segment de codi del programa actual (capcaleraElf)
						if (_gm_primeraPosMem + tamanySegCodi <= END_MEM) 			//verifiquem si la posición de memòria _gm_primeraPosMem no supera la direcció final de memòria
						{
							//reserva memoria, retorna la primera direcció de l'espai reservat per al segment de codi
							primerDirCodi = (int)_gm_reservarMem(zocalo, tamanySegCodi, (unsigned char)i);
							//en el cas de que quedi un espai de memòria consecutiu del tamany requerit
							if (primerDirCodi != 0) 
							{
								//copia el contingut del segment de programa des del buffer en la dirección de memòria _gm_primeraPosMem (_gs_copiaMem(const void *source, void *dest, unsigned int numBytes))
								_gs_copiaMem((const void *) buffer + offsetSegCodi, (void *)primerDirCodi, tamanyFileCodi);
								//si només té una entrada en la taula de segments
								if (phnum == 1) 
								{
									//aplica reubicancions per ajustar referències amb respecte al segment de codi en funció de taulaSeg.p_paddr
									_gm_reubicar(buffer, paddrSegCodi, (unsigned int *)primerDirCodi, 0, (unsigned int *)0);
								}
								//direcció d'inici del programa a la memòria física, tenint en compte totes les reubicacions necessàries perquè el programa s'executi correctament des de la posició en memòria on s'ha carregat
								adrProg = (intFunc) (_gm_primeraPosMem + entry - paddrSegCodi);				
							} else {
								_gm_liberarMem(zocalo);								//si no s'ha pogut reservar la memòria requerida, alliberem a partir del zocalo
							}								
						}
					}
					else if ((taulaSeg.p_flags == (PF_R | PF_W)) && primerDirCodi != 0) //si els flags del segment són lectura i escriptura (segment dades) i s'ha reservat correctament memòria al segment de codi
					{
						tamanySegDades = taulaSeg.p_memsz;							//obtenim la informació del tamany del segment de dades del programa actual (capcaleraElf)
						tamanyFileDades = taulaSeg.p_filesz;						//obtenim la informació del tamany del fitxer del segment de dades del programa actual (capcaleraElf)
						offsetSegDades = taulaSeg.p_offset;							//obtenim la informació del offset del segment de dades del programa actual (capcaleraElf)
						paddrSegDades = taulaSeg.p_paddr;							//obtenim la informació de la direcció física del segment de dades del programa actual (capcaleraElf)
						if (_gm_primeraPosMem + tamanySegDades <= END_MEM) 			//verifiquem si la posición de memòria _gm_primeraPosMem no supera la direcció final
						{			
							//reserva memoria, retorna la primera direcció de l'espai reservat per al segment de dades
							primerDirDades = (int)_gm_reservarMem(zocalo, tamanySegDades, (unsigned char) i);
							//en el cas de que quedi un espai de memòria consecutiu del tamany requerit
							if (primerDirDades != 0) 
							{
								//copia el contingut del segment de programa des del buffer en la dirección de memòria primerDirDades (_gs_copiaMem(const void *source, void *dest, unsigned int numBytes))
								_gs_copiaMem((const void *)buffer + offsetSegDades, (void *)primerDirDades, tamanyFileDades);
								//aplica reubicancions per ajustar referències
								_gm_reubicar(buffer, paddrSegCodi, (unsigned int *)primerDirCodi, paddrSegDades, (unsigned int *)primerDirDades);
							} 
							else 
							{
								_gm_liberarMem(zocalo);								//si no s'ha pogut reservar la memòria requerida, alliberem a partir del zocalo
							}
						}	
					}														
				}
				offset = offset + capcaleraElf.e_phentsize;							//actualitzem offset per a que apunti al següent segment del .elf a partir del tamany de cada entrada
			}
		}
		free(buffer);																//netejem buffer
	}																
	return ((intFunc) adrProg);
}

