/*------------------------------------------------------------------------------

	"garlic_system.h" : definiciones de las variables globales, funciones y
						rutinas del sistema operativo GARLIC (versi�n 1.0)

	Analista-programador: santiago.romani@urv.cat
	Programador P: ines.ortizf@estudiants.urv.cat
	Programador M: ruben.lopezm@estudiants.urv.cat
	Programador G: eros.vilar@estudiants.urv.cat

------------------------------------------------------------------------------*/
#ifndef _GARLIC_SYSTEM_h
#define _GARLIC_SYSTEM_h


//------------------------------------------------------------------------------
//	Variables globales del sistema (garlic_dtcm.s)
//------------------------------------------------------------------------------

extern int _gd_pidz;		// Identificador de proceso (PID) + z�calo
							// (PID en 28 bits altos, z�calo en 4 bits bajos,
							// cero si se trata del propio sistema operativo)

extern int _gd_pidCount;	// Contador de PIDs: se incrementa cada vez que
							// se crea un nuevo proceso (m�x. 2^28)

extern int _gd_tickCount;	// Contador de tics: se incrementa cada IRQ_VBL,
							// permite contabilizar el paso del tiempo

extern int _gd_sincMain;	// Sincronismos con programa principal:
							// bit 0 = 1 indica si se ha acabado de calcular el
							// 				el uso de la CPU,
							// bits 1-15 = 1 indica si el proceso del zócalo
							//				correspondiente ha terminado.

extern int _gd_seed;		// Semilla para generaci�n de n�meros aleatorios
							// (tiene que ser diferente de cero)


extern int _gd_nReady;		// N�mero de procesos en cola de READY (0..15)

extern char _gd_qReady[16];	// Cola de READY (procesos preparados) : vector
							// ordenado con _gd_nReady entradas, conteniendo
							// los identificadores (0..15) de los z�calos de los
							// procesos (m�x. 15 procesos + sistema operativo)

extern int _gd_nDelay;		// Número de procesos en cola de DELAY (0..15)

extern int _gd_qDelay[16];	// Cola de DELAY (procesos retardados) : vector
							// con _gd_nDelay entradas, conteniendo los
							// identificadores de los zócalos (8 bits altos)
							// más el número de tics restantes (16 bits bajos)
							// para desbloquear el proceso

typedef struct				// Estructura del bloque de control de un proceso
{							// (PCB: Process Control Block)
	int PID;				//	identificador del proceso (Process IDentifier)
	int PC;					//	contador de programa (Program Counter)
	int SP;					//	puntero al top de pila (Stack Pointer)
	int Status;				//	estado del procesador (CPSR)
	int keyName;			//	nombre en clave del proceso (cuatro chars)
	int workTicks;			//	contador de ciclos de trabajo (24 bits bajos)
							//		8 bits altos: uso de CPU (%)
} PACKED garlicPCB;

extern garlicPCB _gd_pcbs[16];	// vector de PCBs de los procesos activos


typedef struct				// Estructura del buffer de una ventana
{							// (WBUF: Window BUFfer)
	int pControl;			//	control de escritura en ventana
							//		16 bits altos: n�mero de l�nea (0..23)
							//		16 bits bajos: caracteres pendientes (0..32)
	char pChars[32];		//	vector de 32 caracteres pendientes de escritura
							//		indicando el c�digo ASCII de cada posici�n
} PACKED garlicWBUF;

extern garlicWBUF _gd_wbfs[4];	// vector con los buffers de 16 ventanas


extern int _gd_stacks[15*128];	// vector de pilas de los procesos de usuario

extern char _gd_sem[8];				// vector de semaforos

extern int _gm_primeraPosMem;	// contindr� la primera posici� de mem�ria lliure


//------------------------------------------------------------------------------
//	Rutinas de gesti�n de procesos (garlic_itcm_proc.s)
//------------------------------------------------------------------------------

/* intFunc:		nuevo tipo de dato para representar puntero a funci�n que
				devuelve un int, concretamente, el puntero a la funci�n de
				inicio de los procesos cargados en memoria */
typedef int (* intFunc)(int);

/* _gp_WaitForVBlank:	sustituto de swiWaitForVBlank() para el sistema Garlic;*/
extern void _gp_WaitForVBlank();


/* _gp_IntrMain:	manejador principal de interrupciones del sistema Garlic; */
extern void _gp_IntrMain();

/* _gp_rsiVBL:	manejador de interrupciones VBL (Vertical BLank) de Garlic; */
extern void _gp_rsiVBL();


/* _gp_numProc:	devuelve el n�mero de procesos cargados en el sistema,
				incluyendo el proceso en RUN y los procesos en READY; */
extern int _gp_numProc();


/* _gp_crearProc:	prepara un proceso para ser ejecutado, creando su entorno
				de ejecuci�n y coloc�ndolo en la cola de READY;
	Par�metros:
		funcion	->	direcci�n de memoria de entrada al c�digo del proceso
		zocalo	->	identificador del z�calo (0..15)
		nombre	->	string de 4 caracteres con el nombre en clave del programa
		arg		->	argumento del programa (0..3)
	Resultado:	0 si no hay problema, >0 si no se puede crear el proceso
*/
extern int _gp_crearProc(intFunc funcion, int zocalo, char *nombre, int arg);

/* _gp_retardarProc:	retarda la ejecución del proceso actual durante el
				número de segundos que se especifica por parámetro,
				colocándolo en el vector de DELAY;
	Parámetros:
		nsec ->	número de segundos (máx. 600); si se especifica 0, el proceso
				solo se desbanca y el retardo será el tiempo que tarde en ser
				restaurado (depende del número de procesos activos del sistema)
	ATENCIÓN:
				¡el proceso del sistema operativo (PIDz = 0) NO podrá utilizar
				esta función, para evitar que el procesador se pueda quedar sin
				procesos a ejecutar!
*/
extern int _gp_retardarProc(int nsec);


/* _gp_matarProc:	elimina un proceso de las colas de READY o DELAY, según
				donde se encuentre, libera memoria y borra el PID de la
				estructura _gd_pcbs[zocalo] correspondiente al zócalo que se
				pasa por parámetro;
	ATENCIÓN:	Esta función solo la llamará el sistema operativo, por lo tanto,
				no será necesario realizar comprobaciones del parámetro; por
				otro lado, el proceso del sistema operativo (zocalo = 0) ¡NO se
				tendrá que destruir a sí mismo!
*/
extern int _gp_matarProc(int zocalo);



/* _gp_rsiTIMER0:	servicio de interrupciones del TIMER0 de la plataforma NDS,
				que refrescará periódicamente la información de la tabla de
				procesos relativa al tanto por ciento de uso de la CPU; */
extern void _gp_rsiTIMER0();

//------------------------------------------------------------------------------
//	Funciones de gesti�n de memoria (garlic_mem.c)
//------------------------------------------------------------------------------

/* _gm_initFS: inicializa el sistema de ficheros, devolviendo un valor booleano
					para indiciar si dicha inicializaci�n ha tenido �xito;
*/
extern int _gm_initFS();

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
extern intFunc _gm_cargarPrograma(char *keyName);


//------------------------------------------------------------------------------
//	Rutinas de soporte a la gesti�n de memoria (garlic_itcm_mem.s)
//------------------------------------------------------------------------------

/* _gm_reubicar: rutina de soporte a _gm_cargarPrograma(), que interpreta los
					'relocs' de un fichero ELF contenido en un buffer *fileBuf,
					y ajusta las direcciones de memoria correspondientes a las
					referencias de tipo R_ARM_ABS32, restando la dirección de
					inicio de segmento (pAddr) y sumando la dirección de destino
					en la memoria (*dest) */
extern void _gm_reubicar(char *fileBuf, unsigned int pAddr, unsigned int *dest);

//------------------------------------------------------------------------------
//	Funciones de gesti�n de gr�ficos (garlic_graf.c)
//------------------------------------------------------------------------------

/* _gg_iniGraf: inicializa el procesador gr�fico A para GARLIC 1.0 */
extern void _gg_iniGrafA();


/* _gg_generarMarco: dibuja el marco de la ventana que se indica por parámetro,
												con el color correspondiente; */
extern void _gg_generarMarco(int v);


/* _gg_escribir: escribe una cadena de carácteres en la ventana indicada;
	Parámetros:
		formato	->	string de formato:
					admite '\n' (salto de línea), '\t' (tabulador, 4 espacios)
					y códigos entre 32 y 159 (los 32 últimos son carácteres
					gráficos), además de marcas de formato %c, %d, %h y %s (máx.
					2 marcas por string) y de las marcas de cambio de color 
					actual %0 (blanco), %1 (amarillo), %2 (verde) y %3 (rojo);
		val1	->	valor a sustituir en la primera marca de formato, si existe
		val2	->	valor a sustituir en la segunda marca de formato, si existe
					- los valores pueden ser un código ASCII (%c), un valor
					  natural de 32 bits (%d, %x) o un puntero a string (%s)
		ventana	->	número de ventana (0..3)
*/
extern void _gg_escribir(char *formato, unsigned int val1, unsigned int val2,
																   int ventana);


//------------------------------------------------------------------------------
//	Rutinas de soporte a la gesti�n de gr�ficos (garlic_itcm_graf.s)
//------------------------------------------------------------------------------

/* _gg_escribirLinea: rutina de soporte a _gg_escribir(), para escribir sobre la
					fila (f) de la ventana (v) los caracters pendientes (n) del
					buffer de ventana correspondiente.
*/
extern void _gg_escribirLinea(int v, int f, int n);


/* _gg_desplazar: rutina de soporte a _gg_escribir(), para desplazar una posici�n
					hacia arriba todas las filas de la ventana (v) y borrar el
					contenido de la �ltima fila.
*/
extern void _gg_desplazar(int v);

 /* _gg_setChar: Defineix un nou caracter grafic a partir del codi 128
	Par�metros:
		n	->	numero de caracter, entre 128 i 255
		buffer	->	punter a una matriu de 8x8 bytes (emmagatzemada per files)
					on cada byte indica l'index de color d'un pixel.
*/
extern void _gg_setChar(unsigned char n, unsigned char *buffer);

//------------------------------------------------------------------------------
//	Rutinas de soporte al sistema (garlic_itcm_sys.s)
//------------------------------------------------------------------------------

/* _gs_num2str_dec: convierte el número pasado por valor en el parámetro num
					a una representación en códigos ASCII de los dígitos
					decimales correspondientes, escritos dentro del vector de
					carácteres numstr, que se pasa por referencia; el parámetro
					length indicará la longitud del vector; la rutina coloca un
					carácter centinela (cero) en la última posición del vector
					(numstr[length-1]) y, a partir de la penúltima posición,
					empieza a colocar los códigos ASCII correspondientes a las
					unidades, decenas, centenas, etc.; en el caso que después de
					trancribir todo el número queden posiciones libres en el
					vector, la rutina rellenará dichas posiciones con espacios
					en blanco y devolverá un cero; en el caso que NO hayan
					suficientes posiciones para transcribir todo el número, la
					función abandonará el proceso y devolverá un valor diferente
					de cero.
		ATENCIóN:	solo procesa números naturales de 32 bits SIN signo. */
extern int _gs_num2str_dec(char * numstr, unsigned int length, unsigned int num);


/* _gs_num2str_hex:	convierte el parámetro num en una representación en códigos
					ASCII sobre el vector de carácteres numstr, en base 16
					(hexa), siguiendo las mismas reglas de gestión del espacio
					del string que _gs_num2str_dec(), salvo que las posiciones
					de más peso vacías se rellenarán con ceros, no con espacios
					en blanco */
extern int _gs_num2str_hex(char * numstr, unsigned int length, unsigned int num);


/* _gs_copiaMem: copia un bloque de numBytes bytes, desde una posición de
				memoria inicial (*source) a partir de otra posición de memoria
				destino (*dest), asumiendo que ambas posiciones de memoria están
				alineadas a word */
extern void _gs_copiaMem(const void *source, void *dest, unsigned int numBytes);

/* _gs_borrarVentana: borra el contenido de la ventana que se pasa por parámetro,
				así como el campo de control del buffer de ventana
				_gd_wbfs[ventana].pControl; la rutina puede operar en una
				configuración de 4 o 16 ventanas, según el parámetro de modo;
	Parámetros:
		ventana ->	número de ventana
		modo 	->	(0 -> 4 ventanas, 1 -> 16 ventanas)
*/
extern void _gs_borrarVentana(int zocalo, int modo);


/* _gs_iniGrafB: inicializa el procesador gráfico B para GARLIC 2.0 */
extern void _gs_iniGrafB();


/* _gs_escribirStringSub: escribe un string (terminado con centinela cero) a
				partir de la posición indicada por parámetros (fil, col), con el
				color especificado, en la pantalla secundaria; */
extern void _gs_escribirStringSub(char *string, int fil, int col, int color);


/* _gs_dibujarTabla: dibujar la tabla de procesos; */
extern void _gs_dibujarTabla();


/* _gs_pintarFranjas: rutina para pintar las líneas verticales correspondientes
				a un conjunto de franjas consecutivas de memoria asignadas a un
				segmento (de código o datos) del zócalo indicado por parámetro.
	Parámetros:
		zocalo		->	el zócalo que reserva la memoria (0 para borrar)
		index_ini	->	el índice inicial de las franjas
		num_franjas	->	el número de franjas a pintar
		tipo_seg	->	el tipo de segmento reservado (0 -> código, 1 -> datos)
*/
extern void _gs_pintarFranjas(unsigned char zocalo, unsigned short index_ini,
							unsigned short num_franjas, unsigned char tipo_seg);


/* _gs_representarPilas: rutina para para representar gráficamente la ocupación
				de las pilas de los procesos de usuario, además de la pila del
				proceso de control del sistema operativo, sobre la tabla de
				control de procesos.
*/
extern void _gs_representarPilas();

#endif // _GARLIC_SYSTEM_h
