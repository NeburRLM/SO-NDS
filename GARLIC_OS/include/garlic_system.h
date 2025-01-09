/*------------------------------------------------------------------------------

	"garlic_system.h" : definiciones de las variables globales, funciones y
						rutinas del sistema operativo GARLIC (versión 2.0)

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

extern int _gd_pidz;		// Identificador de proceso (PID) + zï¿½calo
							// (PID en 28 bits altos, zï¿½calo en 4 bits bajos,
							// cero si se trata del propio sistema operativo)

extern int _gd_pidCount;	// Contador de PIDs: se incrementa cada vez que
							// se crea un nuevo proceso (mï¿½x. 2^28)

extern int _gd_tickCount;	// Contador de tics: se incrementa cada IRQ_VBL,
							// permite contabilizar el paso del tiempo

extern int _gd_sincMain;	// Sincronismos con programa principal:
							// bit 0 = 1 indica si se ha acabado de calcular el
							// 				el uso de la CPU,
							// bits 1-15 = 1 indica si el proceso del zÃ³calo
							//				correspondiente ha terminado.

extern int _gd_seed;		// Semilla para generaciï¿½n de nï¿½meros aleatorios
							// (tiene que ser diferente de cero)


extern int _gd_nReady;		// Nï¿½mero de procesos en cola de READY (0..15)

extern char _gd_qReady[16];	// Cola de READY (procesos preparados) : vector
							// ordenado con _gd_nReady entradas, conteniendo
							// los identificadores (0..15) de los zï¿½calos de los
							// procesos (mï¿½x. 15 procesos + sistema operativo)

extern int _gd_nDelay;		// NÃºmero de procesos en cola de DELAY (0..15)

extern int _gd_qDelay[16];	// Cola de DELAY (procesos retardados) : vector
							// con _gd_nDelay entradas, conteniendo los
							// identificadores de los zÃ³calos (8 bits altos)
							// mÃ¡s el nÃºmero de tics restantes (16 bits bajos)
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
	int pControl;			// control de escritura en ventana
							// 4 bits altos: cÃƒÂ³d. color actual (0..3)
							// 12 bits medios: nÃƒÂºmero de lÃƒÂ­nea (0..23) --- (ANTERIOR) 16 bits altos: numero de linea (0..23)
							// 16 bits bajos: caracteres pendientes (0..32)
	short pChars[32];		// vector de 32 caracteres pendientes de escritura
							// indicando el codigo ASCII de cada posiciÃ¯Â¿Â½n
} PACKED garlicWBUF;

extern garlicWBUF _gd_wbfs[16];	// vector con los buffers de 16 ventanas


extern int _gd_stacks[15*128];	// vector de pilas de los procesos de usuario

extern char _gd_sem[8];				// vector de semaforos

extern int _gm_primeraPosMem;	// contindrï¿½ la primera posiciï¿½ de memï¿½ria lliure

extern int _gd_perc;			// contendrÃ¡ el porcentage en string

extern int _gd_quo;				// contendrÃ¡ el cuociente de la division

extern int _gd_mod;				// contendrÃ¡ el modulo de la division

//------------------------------------------------------------------------------
//	Rutinas de gestión de procesos (garlic_itcm_proc.s)
//------------------------------------------------------------------------------

/* intFunc:		nuevo tipo de dato para representar puntero a función que
				devuelve un int, concretamente, el puntero a la función de
				inicio de los procesos cargados en memoria */
typedef int (* intFunc)(int);

/* _gp_WaitForVBlank:	sustituto de swiWaitForVBlank() para el sistema Garlic;*/
extern void _gp_WaitForVBlank();


/* _gp_IntrMain:	manejador principal de interrupciones del sistema Garlic; */
extern void _gp_IntrMain();

/* _gp_rsiVBL:	manejador de interrupciones VBL (Vertical BLank) de Garlic; */
extern void _gp_rsiVBL();


/* _gp_numProc:	devuelve el número de procesos cargados en el sistema,
				incluyendo el proceso en RUN y los procesos en READY; */
extern int _gp_numProc();


/* _gp_crearProc:	prepara un proceso para ser ejecutado, creando su entorno
				de ejecución y colocándolo en la cola de READY;
	Parámetros:
		funcion	->	dirección de memoria de entrada al código del proceso
		zocalo	->	identificador del zócalo (0..15)
		nombre	->	string de 4 carácteres con el nombre en clave del programa
		arg		->	argumento del programa (0..3)
	Resultado:	0 si no hay problema, >0 si no se puede crear el proceso
*/
extern int _gp_crearProc(intFunc funcion, int zocalo, char *nombre, int arg);


/* _gp_retardarProc:	retarda la ejecuciÃ³n del proceso actual durante el
				nÃºmero de segundos que se especifica por parÃ¡metro,
				colocÃ¡ndolo en el vector de DELAY;
	ParÃ¡metros:
		nsec ->	nÃºmero de segundos (mÃ¡x. 600); si se especifica 0, el proceso
				solo se desbanca y el retardo serÃ¡ el tiempo que tarde en ser
				restaurado (depende del nÃºmero de procesos activos del sistema)
	ATENCIÃ“N:
				Â¡el proceso del sistema operativo (PIDz = 0) NO podrÃ¡ utilizar
				esta funciÃ³n, para evitar que el procesador se pueda quedar sin
				procesos a ejecutar!
*/
extern int _gp_retardarProc(int nsec);


/* _gp_matarProc:	elimina un proceso de las colas de READY o DELAY, segÃºn
				donde se encuentre, libera memoria y borra el PID de la
				estructura _gd_pcbs[zocalo] correspondiente al zÃ³calo que se
				pasa por parÃ¡metro;
	ATENCIÃ“N:	Esta funciÃ³n solo la llamarÃ¡ el sistema operativo, por lo tanto,
				no serÃ¡ necesario realizar comprobaciones del parÃ¡metro; por
				otro lado, el proceso del sistema operativo (zocalo = 0) Â¡NO se
				tendrÃ¡ que destruir a sÃ­ mismo!
*/
extern int _gp_matarProc(int zocalo);


/* _gp_rsiTIMER0:	servicio de interrupciones del TIMER0 de la plataforma NDS,
				que refrescarÃ¡ periÃ³dicamente la informaciÃ³n de la tabla de
				procesos relativa al tanto por ciento de uso de la CPU; */
extern void _gp_rsiTIMER0();


//------------------------------------------------------------------------------
//	Funciones de gestión de memoria (garlic_mem.c)
//------------------------------------------------------------------------------

/* _gm_initFS: inicializa el sistema de ficheros, devolviendo un valor booleano
					para indiciar si dicha inicialización ha tenido éxito;
*/
extern int _gm_initFS();


/* _gm_listaProgs: devuelve una lista con los nombres en clave de todos
				los programas que se encuentran en el directorio "Programas".
				Se considera que un fichero es un programa si su nombre tiene
				8 carácteres y termina con ".elf"; se devuelven solo los
				4 primeros carácteres del nombre del fichero (nombre en clave),
				que por convenio deben estar en mayúsculas;
				el resultado es un vector de strings (paso por referencia) y
				el número de programas detectados; */
extern int _gm_listaProgs(char* progs[]);


extern void loadFiles();


/* _gm_cargarPrograma: busca un fichero de nombre "(keyName).elf" dentro del
					directorio "/Programas/" del sistema de ficheros y carga
					los segmentos de programa a partir de una posiciÃ³n de
					memoria libre, efectuando la reubicaciÃ³n de las referencias
					a los sÃ­mbolos del programa segÃºn el desplazamiento del
					cÃ³digo y los datos en la memoria destino;
	ParÃ¡metros:
		zocalo	->	Ã­ndice del zÃ³calo que indexarÃ¡ el proceso del programa
		keyName ->	string de 4 carÃ¡cteres con el nombre en clave del programa
	Resultado:
		!= 0	->	direcciÃ³n de inicio del programa (intFunc)
		== 0	->	no se ha podido cargar el programa
*/
extern intFunc _gm_cargarPrograma(int zocalo, char *keyName);


//------------------------------------------------------------------------------
//	Rutinas de soporte a la gestión de memoria (garlic_itcm_mem.s)
//------------------------------------------------------------------------------

/* _gm_reubicar: rutina de soporte a _gm_cargarPrograma(), que interpreta los
					'relocs' de un fichero ELF contenido en un buffer *fileBuf,
					y ajusta las direcciones de memoria correspondientes a las
					referencias de tipo R_ARM_ABS32, a partir de las direcciones
					de memoria destino de cÃ³digo (dest_code) y datos (dest_data)
					y segÃºn el valor de las direcciones de las referencias a
					reubicar y de las direcciones de inicio de los segmentos de
					cÃ³digo (pAddr_code) y datos (pAddr_data); */
extern void _gm_reubicar(char *fileBuf,
							unsigned int pAddr_code, unsigned int *dest_code,
							unsigned int pAddr_data, unsigned int *dest_data);


/* _gm_reservarMem: rutina para reservar un conjunto de franjas de memoria 
				libres consecutivas que proporcionen un espacio suficiente para
				albergar el tamaÃ±o de un segmento de cÃ³digo o datos del proceso
				(segÃºn indique tipo_seg), asignando al nÃºmero de zÃ³calo que se
				pasa por parÃ¡metro;
				la rutina devuelve la primera direcciÃ³n del espacio reservado; 
				en el caso de que no quede un espacio de memoria consecutivo del
				tamaÃ±o requerido, devuelve cero; */
extern void * _gm_reservarMem(int z, int tam, unsigned char tipo_seg);


/* _gm_liberarMem: rutina para liberar todas las franjas de memoria asignadas
				al proceso del zÃ³calo indicado por parÃ¡metro; */
extern void _gm_liberarMem(int z);


/* _gm_rsiTIMER1:	servicio de interrupciones del TIMER1 de la plataforma NDS,
				que refrescarÃ¡ periÃ³dicamente la informaciÃ³n de la tabla de
				procesos relativa al uso de la pila y el estado del proceso; */
extern void _gm_rsiTIMER1();


//------------------------------------------------------------------------------
//	Funciones de gestión de gráficos (garlic_graf.c)
//------------------------------------------------------------------------------

/* _gg_iniGraf: inicializa el procesador grï¿½fico A para GARLIC 1.0 */
extern void _gg_iniGrafA();


/* _gg_generarMarco: dibuja el marco de la ventana que se indica por parÃƒÂ¡metro,
												con el color correspondiente; */
extern void _gg_generarMarco(int v, int color);


/* _gg_escribir: escribe una cadena de carÃƒÂ¡cteres en la ventana indicada;
	ParÃƒÂ¡metros:
		formato	->	string de formato:
					admite '\n' (salto de lÃƒÂ­nea), '\t' (tabulador, 4 espacios)
					y cÃƒÂ³digos entre 32 y 159 (los 32 ÃƒÂºltimos son carÃƒÂ¡cteres
					grÃƒÂ¡ficos), ademÃƒÂ¡s de marcas de formato %c, %d, %h y %s (mÃƒÂ¡x.
					2 marcas por string) y de las marcas de cambio de color 
					actual %0 (blanco), %1 (amarillo), %2 (verde) y %3 (rojo);
		val1	->	valor a sustituir en la primera marca de formato, si existe
		val2	->	valor a sustituir en la segunda marca de formato, si existe
					- los valores pueden ser un cÃƒÂ³digo ASCII (%c), un valor
					  natural de 32 bits (%d, %x) o un puntero a string (%s)
		ventana	->	nÃƒÂºmero de ventana (0..16)
*/
extern void _gg_escribir(char *formato, unsigned int val1, unsigned int val2,
																   int ventana);


//------------------------------------------------------------------------------
//	Rutinas de soporte a la gestiï¿½n de grï¿½ficos (garlic_itcm_graf.s)
//------------------------------------------------------------------------------

/* _gg_escribirLinea: rutina de soporte a _gg_escribir(), para escribir sobre la
					fila (f) de la ventana (v) los caracters pendientes (n) del
					buffer de ventana correspondiente.
*/
extern void _gg_escribirLinea(int v, int f, int n);


/* _gg_desplazar: rutina de soporte a _gg_escribir(), para desplazar una posiciÃ¯Â¿Â½n
					hacia arriba todas las filas de la ventana (v) y borrar el
					contenido de la Ã¯Â¿Â½ltima fila.
*/
extern void _gg_desplazar(int v);

/* _gg_escribirCar: escribe un carÃƒÂ¡cter (baldosa) en la posiciÃƒÂ³n de la ventana
				indicada, con un color concreto;
	ParÃƒÂ¡metros:
		vx		->	coordenada x de ventana (0..31)
		vy		->	coordenada y de ventana (0..23)
		c		->	cÃƒÂ³digo del carÃƒÂ¡cter, como nÃƒÂºmero de baldosa (0..127)
		color	->	color del texto (0..3)
		ventana	->	nÃƒÂºmero de ventana (0..15)
*/
extern void _gg_escribirCar(int vx, int vy, char c, int color, int ventana);


/* _gg_escribirMat: escribe una matriz de 8x8 carÃƒÂ¡cteres a partir de una
				posiciÃƒÂ³n de la ventana indicada, con un color concreto;
	ParÃƒÂ¡metros:
		vx		->	coordenada x inicial de ventana (0..31)
		vy		->	coordenada y inicial de ventana (0..23)
		m		->	matriz 8x8 de cÃƒÂ³digos ASCII
		color	->	color del texto (0..3)
		ventana	->	nÃƒÂºmero de ventana (0..15)
*/
extern void _gg_escribirMat(int vx, int vy, char m[][8], int color, int ventana);


/* _gg_escribirLineaTabla: escribe los campos bÃƒÂ¡sicos de una lÃƒÂ­nea de la tabla
				de procesos, correspondiente al nÃƒÂºmero de zÃƒÂ³calo que se pasa por
				parÃƒÂ¡metro con el color especificado; los campos a escribir son:
					nÃƒÂºmero de zÃƒÂ³calo, PID y nombre clave del proceso (keyName);
*/
extern void _gg_escribirLineaTabla(int z, int color);


/* _gg_rsiTIMER2:	servicio de interrupciones del TIMER2 de la plataforma NDS,
				que refrescarÃƒÂ¡ periÃƒÂ³dicamente la informaciÃƒÂ³n de la tabla de
				procesos relativa a la direcciÃƒÂ³n actual de ejecuciÃƒÂ³n; */
extern void _gg_rsiTIMER2();


 /* _gg_setChar: Defineix un nou caracter grafic a partir del codi 128
	ParÃ¯Â¿Â½metros:
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

//------------------------------------------------------------------------------
//	Rutinas de soporte a la interficie de usuario (garlic_itcm_ui.s)
//------------------------------------------------------------------------------
extern int _gi_za;				// zÃ³calo seleccionado actualmente


/* _gi_movimientoVentanas:	actualiza el desplazamiento y escalado de los
				fondos 2 y 3 del procesador grÃ¡fico A, para efectuar los
				movimientos de las ventanas segÃºn el comportamiento
				requerido de la interficie de usuario; */
extern void _gi_movimientoVentanas();


/* _gi_redibujarZocalo: rutina para actualizar la tabla de zÃ³calos en funciÃ³n
				del zÃ³calo actual (_gi_za) y del parÃ¡metro (seleccionar):
					si seleccionar == 0, dibuja la lÃ­nea de _gi_za segÃºn el
											color asociado al estado del zÃ³calo
											(blanco -> activo, salmÃ³n -> libre);
					sino, 				dibuja la lÃ­nea en magenta;
*/
extern void _gi_redibujarZocalo(int seleccionar);


/* _gi_controlInterfaz: rutina para gestionar la interfaz del usuario a partir
				del cÃ³digo de tecla que se pasa por parÃ¡metro; */
extern void _gi_controlInterfaz(int key);


#endif // _GARLIC_SYSTEM_h
