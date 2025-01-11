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

#define MAX_FILES 10
#define FILE_SIZE 65536  // 64 KB

extern int _gd_pidz;

typedef struct {
    char name[6];       // "fileX", on X es el número del fitxer
    char *data;         // Punter a les dades del fitxer
    size_t size;        // Mida de les dades vàlides
    size_t pos;         // Punter (posició) de lectura/escriptura
	char is_open;     	// Indicador de si el fitxer està obert
	int proces;
	char* mode;
} File;

// Array para almacenar hasta 10 ficheros
File files[MAX_FILES];

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



// Función para abrir un archivo cargado en memoria
FILE* _gm_fopen(const char* filename, const char* mode)
{
    int index = filename[4] - '0';  // Obtener el número del archivo ("fileX")
    if (index < 0 || index >= MAX_FILES) 
	{
        return NULL;  // Fuera del rango de archivos permitidos
    }

    // Verificar si el archivo ya está cargado (ya está en memoria)
    if (files[index].data == NULL) 
	{
        return NULL;  // Si no se pudo cargar el archivo
    }
	
	// Verificar si el archivo ya está abierto
    if (files[index].is_open) 
	{
        return NULL;  // El archivo ya está siendo usado
    }
	
	
	if (strcmp(mode, "w") != 0 && strcmp(mode, "a") != 0) 
    {
        return NULL;  // Modo inválido
    }
    if (strcmp(mode, "w") == 0) 
    {
		files[index].mode = "w";
	}
	else if (strcmp(mode, "a") == 0)
	{
		files[index].mode = "a";
	}
	
	// Marcar el archivo como abierto y devolver el puntero
    files[index].is_open = 1;
	files[index].proces = _gd_pidz;
    files[index].pos = 0;  // Reiniciar la posición
	
    // Devolver el archivo cargado en memoria
    return (FILE*)&files[index]; 
}

// Funció per llegir dades del fitxer carregat a la memòria
size_t _gm_fread(void *buffer, size_t size, size_t numele, FILE *file)
{
    File *f = (File*)file;
    size_t bytesToRead = size * numele;
	
	// Verificar que el archivo está abierto y que el proceso actual es el propietario
    if (f->is_open && f->proces == _gd_pidz) 
    {
		// Si la posició està al final del fitxer, restablim a 0 (llegir des del principi)
		//if (f->pos >= f->size) 
		//{
			f->pos = 0;  // Restableix la posició si ja no hi ha més dades
		//}

		// Assegura't de no llegir més enllà de la mida del fitxer
		if (f->pos + bytesToRead > f->size) 
		{
			bytesToRead = f->size - f->pos;  // Llegir només fins al final del fitxer
		}

		if (bytesToRead > 0) {
			memcpy(buffer, f->data + f->pos, bytesToRead);  // Copia les dades al buffer
			f->pos += bytesToRead;  // Actualitza la posició després de la lectura
		}
	}
	else
	{
		bytesToRead = 0;
	}

    return bytesToRead;  // Retorna els bytes llegits (pot ser 0 si no hi ha més dades)
}

// Funció per escriure dades al fitxer carregat a la memòria
size_t _gm_fwrite(const void *buffer, size_t size, size_t num, FILE *file) 
{
    File *f = (File*)file;
    size_t bytesToWrite = size * num;

    // Verificar que el archivo está abierto y que el proceso actual es el propietario
    if (f->is_open && f->proces == _gd_pidz) 
    {
		
		// Manejamos el modo "a" (append) para empezar desde el final del contenido
		if (strcmp(f->mode, "a") == 0) 
		{
			f->pos = f->size; // Colocar la posición al final del archivo
		}
		// Manejamos el modo "w" (write) para sobrescribir desde el inicio
		else if (strcmp(f->mode, "w") == 0) 
		{
			f->pos = 0;     // Restablecer la posición al inicio
			f->size = 0;    // Vaciar el contenido actual
			memset(f->data, 0, FILE_SIZE);
		}
		
		
		// No excedir la mida màxima del fitxer
		if (f->pos + bytesToWrite > FILE_SIZE) 
		{
			//bytesToWrite = FILE_SIZE - f->pos;
			bytesToWrite = 0;
		}
		else
		{	
			// Escriure les dades al fitxer
			memcpy(f->data + f->pos, buffer, bytesToWrite);
			f->pos += bytesToWrite;
			
			// Actualitzar la mida del fitxer si és necessari
			if (f->pos > f->size) 
			{
				f->size = f->pos;
			}
		}
	}
	else
	{
		bytesToWrite = 0;
	}
    return bytesToWrite;  // Retorna els bytes escrits
}

// Funció per tancar el fitxer carregat a la memòria
int _gm_fclose(FILE *file)
{
    File *f = (File*)file;
    if (f == NULL) 
	{
        return -1;  // Error: puntero inválido
    }

    f->is_open = 0;	// Marcar el archivo como cerrado
	f->proces = -1;
    return 0;  // Operació exitosa  
}

// Funció per carregar els fitxers a la memòria
void loadFiles() 
{
    for (int i = 0; i < MAX_FILES; i++) 
	{
        // Inicialitzar el nom del fitxer "fileX"
        snprintf(files[i].name, sizeof(files[i].name), "file%d", i);
		
        // Llegir el contingut del fitxer des de NitroFS (només al principi)
        char path[13];
        snprintf(path, sizeof(path), "/Datos/file%d", i);
		
        // Fer servir fopen estàndard per accedir al fitxer al sistema de fitxers
        FILE* f = fopen(path, "rb"); // Utilitzem fopen estàndard per accedir al fitxer
        if (f != NULL) 
		{
            
			// Assignar memòria dinàmica de 64 KB per als fitxers
			files[i].data = (char*)malloc(FILE_SIZE);
			files[i].size = 0;
			files[i].pos = 0;
			// Llegir el contingut del fitxer utilitzant fread estàndard
            size_t bytesRead = fread(files[i].data, 1, FILE_SIZE, f);
			
            // Actualitzar la mida del fitxer llegit
            files[i].size = bytesRead;
			
            // Tancar el fitxer després de la lectura
            fclose(f);
        } 
		else 
		{
            // Si no es pot obrir el fitxer, deixar la mida a 0 (buit)
            files[i].size = 0;
        }
    }
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
	unsigned char cap = 1;															//variable per controlar si el programa ha capigut i s'ha carregat correctament

	
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
					if (taulaSeg.p_flags == 5) 										//si els flags del segment són lectura i executable (segment codi)
					{
						tamanySegCodi = taulaSeg.p_memsz;							//obtenim la informació del tamany del segment de codi del programa actual (capcaleraElf)
						tamanyFileCodi = taulaSeg.p_filesz;							//obtenim la informació del tamany del fitxer del segment de codi del programa actual (capcaleraElf)
						offsetSegCodi = taulaSeg.p_offset;							//obtenim la informació del offset del segment de codi del programa actual (capcaleraElf)
						paddrSegCodi = taulaSeg.p_paddr;							//obtenim la informació de la direcció física del segment de codi del programa actual (capcaleraElf)
						
						//reserva memoria, retorna la primera direcció de l'espai reservat per al segment de codi
						primerDirCodi = (int)_gm_reservarMem(zocalo, tamanySegCodi, (unsigned char)i);
						//en el cas de que quedi un espai de memòria consecutiu del tamany requerit
						if (primerDirCodi != 0) 
						{
							//copia el contingut del segment de programa des del buffer en la direcció de memòria primerDirCodi (_gs_copiaMem(const void *source, void *dest, unsigned int numBytes))
							_gs_copiaMem((const void *) buffer + offsetSegCodi, (void *)primerDirCodi, tamanyFileCodi);
							//si només té una entrada en la taula de segments
							if (phnum == 1) 
							{
								//aplica reubicancions per ajustar referències amb respecte al segment de codi en funció de taulaSeg.p_paddr
								_gm_reubicar(buffer, paddrSegCodi, (unsigned int *)primerDirCodi, 0, (unsigned int *)0);
							}
								
						} else {
							_gm_liberarMem(zocalo);								//si no s'ha pogut reservar la memòria requerida, alliberem a partir del zocalo
							cap = 0;											//programa no carregat ja que no hi ha espai
						}								
						
					}
					else if ((taulaSeg.p_flags == 6) && primerDirCodi != 0) 		//si els flags del segment són lectura i escriptura (segment dades) i s'ha reservat correctament memòria al segment de codi
					{
						tamanySegDades = taulaSeg.p_memsz;							//obtenim la informació del tamany del segment de dades del programa actual (capcaleraElf)
						tamanyFileDades = taulaSeg.p_filesz;						//obtenim la informació del tamany del fitxer del segment de dades del programa actual (capcaleraElf)
						offsetSegDades = taulaSeg.p_offset;							//obtenim la informació del offset del segment de dades del programa actual (capcaleraElf)
						paddrSegDades = taulaSeg.p_paddr;							//obtenim la informació de la direcció física del segment de dades del programa actual (capcaleraElf)
									
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
							cap = 0;											//programa no carregat ja que no hi ha espai
						}
							
					}														
				}
				offset = offset + capcaleraElf.e_phentsize;							//actualitzem offset per a que apunti al següent segment del .elf a partir del tamany de cada entrada
			}
			if (cap == 1)															//mirem si el programa ha cabut correctament
			{
				//direcció d'inici del programa a la memòria física, tenint en compte totes les reubicacions necessàries perquè el programa s'executi correctament des de la posició en memòria on s'ha carregat
				adrProg = (intFunc) (primerDirCodi + entry - paddrSegCodi);
			}
		}
		free(buffer);																//netejem buffer
	}																
	return ((intFunc) adrProg);
}

