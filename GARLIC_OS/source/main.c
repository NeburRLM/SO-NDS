/*------------------------------------------------------------------------------

	"main.c" : fase 2 / ProgGP

	Programa de control del sistema operativo GARLIC, versión 2.0

------------------------------------------------------------------------------*/
#include <nds.h>
#include <stdlib.h>

#include "garlic_system.h"	// definición de funciones y variables de sistema

extern int * punixTime;		// puntero a zona de memoria con el tiempo real

const short divFreq0 = -33513982/1024;		// frecuencia de TIMER0 = 1 Hz
const short divFreq2 = -33513982/(1024*4);	// frecuencia de TIMER2 = 4 Hz

const char *argumentosDisponibles[4] = { "0", "1", "2", "3"};
		// se supone que estos programas están disponibles en el directorio
		// "Programas" de las estructura de ficheros de Nitrofiles
const char *progs[7] = {"BORR","CRON","CUST","HOLA","PONG","PRNT","SQR1"};
const unsigned char num_progs = 7;


/* función para escribir los porcentajes de uso de la CPU de los procesos de los
		cuatro primeros zócalos, en el caso que la RSI del TIMER0 haya realizado
		el cálculo */
void porcentajeUso()
{
	if (_gd_sincMain & 1)			// verificar sincronismo de timer0
	{
		_gd_sincMain &= 0xFFFE;			// poner bit de sincronismo a cero
		_gg_escribir("***\t%d%%  %d%%", _gd_pcbs[0].workTicks >> 24,
										_gd_pcbs[1].workTicks >> 24, 0);
		_gg_escribir("  %d%%  %d%%\n", _gd_pcbs[2].workTicks >> 24,
										_gd_pcbs[3].workTicks >> 24, 0);
	}
}


/* gestionSincronismos:	función para detectar cuándo un proceso ha terminado
						su ejecución, consultando el bit i-éssimo de la
						variable global _gd_sincMain; en caso de detección,
						borra la línea del proceso del zócalo i-éssimo y pone
						el bit de _gd_sincMain a cero.
*/
void gestionSincronismos()
{
	int i, mask;
	
	if (_gd_sincMain & 0xFFFE)		// si hay algun sincronismo pendiente
	{
		mask = 2;
		for (i = 1; i <= 15; i++)
		{
			if (_gd_sincMain & mask)
			{	// actualizar visualización de tabla de zócalos
				_gg_escribirLineaTabla(i, (i == _gi_za ? 2 : 3));
				if (i != _gi_za)			// si no se trata del propio zócalo actual
					_gg_generarMarco(i, 3);
				_gg_escribir("%3* %d: proceso terminado\n", i, 0, 0);
				_gd_sincMain &= ~mask;		// poner bit a cero
			}
			mask <<= 1;
		}
	}
}


/* Función para presentar una lista de opciones y escoger una: devuelve el índice de la opción
		(0: primera opción, 1: segunda opción, etc.)
		ATENCIóN: para que pueda funcionar correctamente, se supone que no habrá desplazamiento
				  de las líneas de la ventana. */
unsigned char escogerOpcion(char *opciones[], unsigned char num_opciones)
{
	int fil_ini, j, sel, k;
	
	fil_ini = _gd_wbfs[_gi_za].pControl >> 16;		// fil_ini es índice fila inicial
	for (j = 0; j < num_opciones; j++)			// mostrar opciones
		_gg_escribir("%1( ) %s\n", (unsigned int) opciones[j], 0, _gi_za);

	sel = -1;									// marca de no selección
	j = 0;										// j es preselección
	_gg_escribirCar(1, fil_ini, 10, 2, _gi_za);	// marcar preselección
	do
	{
		_gp_WaitForVBlank();
		scanKeys();
		k = keysDown();				// leer botones
		if (k != 0)
			switch (k)
			{
				case KEY_UP:
						if (j > 0)
						{	_gg_escribirCar(1, fil_ini+j, 0, 2, _gi_za);
							j--;
							_gg_escribirCar(1, fil_ini+j, 10, 2, _gi_za);
						}
						break;
				case KEY_DOWN:
						if (j < num_opciones-1)
						{	_gg_escribirCar(1, fil_ini+j, 0, 2, _gi_za);
							j++;
							_gg_escribirCar(1, fil_ini+j, 10, 2, _gi_za);
						}
						break;
				case KEY_START:
						sel = j;			// escoger preselección
						break;
			}
	} while (sel == -1);
	return sel;
}


/* Función para permitir seleccionar un programa entre los ficheros ELF
		disponibles, así como un argumento para el programa (0, 1, 2 o 3) */
void seleccionarPrograma()
{
	intFunc start;
	int ind_prog, argumento, i;

	i = 1;
	while ((i < 16) &&	(_gd_pcbs[i].PID == 0))	// buscar si hay otro proceso en marcha
	{
		i++;
	}
	if (i < 16)						// en caso de encontrar otro proceso activo
	{
		_gd_pcbs[i].PID = 0;		// liberar su PCB
		_gd_nReady = 0;				// eliminar cualquier proceso de cola de READY
		_gg_escribir("* %3%d%0: proceso destruido\n", i, 0, 0);
		_gg_escribirLineaTabla(i, (i == _gi_za ? 2 : 3));
		if (i != _gi_za)			// si no se trata del propio zócalo actual
			_gg_generarMarco(i, 3);
	}
	_gs_borrarVentana(_gi_za, 1);
	_gg_escribir("%1*** Seleccionar programa :\n", 0, 0, _gi_za);
	ind_prog = escogerOpcion((char **) progs, num_progs);
	_gg_escribir("%1*** seleccionar argumento :\n", 0, 0, _gi_za);
	argumento = escogerOpcion((char **) argumentosDisponibles, 4);
	
	start = _gm_cargarPrograma((char *) progs[ind_prog]);
	if (start)
	{
		_gp_crearProc(start, _gi_za, (char *) progs[ind_prog], argumento);
		_gg_escribir("%2* %d:%s.elf", _gi_za, (unsigned int) progs[ind_prog], 0);
		_gg_escribir(" (%d)\n", argumento, 0, 0);
		_gg_escribirLineaTabla(_gi_za, 2);
	}
}

/* Inicializaciones generales del sistema Garlic */
//------------------------------------------------------------------------------
void inicializarSistema() {
//------------------------------------------------------------------------------

	_gd_seed = *punixTime;	// inicializar semilla para números aleatorios con
	_gd_seed <<= 16;		// el valor de tiempo real UNIX, desplazado 16 bits
	
	_gd_pcbs[0].keyName = 0x4C524147;	// "GARL"
	
	_gg_iniGrafA();					// inicializar gráficos
	_gs_iniGrafB();
	_gs_dibujarTabla();
	
	_gi_redibujarZocalo(1);			// marca tabla de zócalos con el proceso
									// del S.O. seleccionado (en verde)
	
	if (!_gm_initFS())
	{	_gg_escribir("%3ERROR: ¡no se puede utilizar sistema de ficheros!", 0, 0, 0);
		exit(0);
	}

	irqInitHandler(_gp_IntrMain);	// instalar rutina principal interrupciones
	irqSet(IRQ_VBLANK, _gp_rsiVBL);	// instalar RSI de vertical Blank
	irqEnable(IRQ_VBLANK);			// activar interrupciones de vertical Blank

	irqSet(IRQ_TIMER0, _gp_rsiTIMER0);
	irqEnable(IRQ_TIMER0);				// instalar la RSI para el TIMER0
	TIMER0_DATA = divFreq0; 
	TIMER0_CR = 0xC3;  	// Timer Start | IRQ Enabled | Prescaler 3 (F/1024)

	irqSet(IRQ_TIMER2, _gg_rsiTIMER2);
	irqEnable(IRQ_TIMER2);			// instalar la RSI para el TIMER2
	TIMER2_DATA = divFreq2; 
	TIMER2_CR = 0xC3;  		// Timer Start | IRQ Enabled | Prescaler 3 (F/1024)

	irqSet(IRQ_VCOUNT, _gi_movimientoVentanas);
	REG_DISPSTAT |= 0xE620;			// fijar linea VCOUNT a 230 y activar int.
	irqEnable(IRQ_VCOUNT);			// de VCOUNT
	
	REG_IME = IME_ENABLE;			// activar las interrupciones en general
}

//------------------------------------------------------------------------------
int main(int argc, char **argv) {
//------------------------------------------------------------------------------

	int key;
	//intFunc start;
	//intFunc start2;
	//int mtics, v;

	inicializarSistema();
	
	_gg_escribir("%1********************************", 0, 0, 0);
	_gg_escribir("%1*                              *", 0, 0, 0);
	_gg_escribir("%1* Sistema Operativo GARLIC 2.0 *", 0, 0, 0);
	_gg_escribir("%1*                              *", 0, 0, 0);
	_gg_escribir("%1********************************", 0, 0, 0);
	_gg_escribir("%1*** Inicio fase 2 / ProgGP\n", 0, 0, 0);

	while (1)						// bucle infinito
	{
		scanKeys();
		key = keysDown();			// leer botones y controlar la interfaz
		if (key != 0)				// de usuario
		{	_gi_controlInterfaz(key);
			if ((key == KEY_START) && (_gi_za != 0))
				seleccionarPrograma();
		}
		gestionSincronismos();
		_gp_WaitForVBlank();		// retardo del proceso de sistema
	}
	return 0;			
}


/*_gg_escribir("*** Carga de programa HOLA.elf\n", 0, 0, 0);
	start = _gm_cargarPrograma("HOLA");
	start2 = _gm_cargarPrograma("DIV1");
	if (start)
	{	
		_gp_crearProc(start, 1, "HOLA", 3);
		_gp_crearProc(start, 2, "HOLA", 3);
		if (start2)
			_gp_crearProc(start2, 3, "DIV1", 3);
		
		while (_gd_tickCount < 240)			// esperar 4 segundos
		{
			_gp_WaitForVBlank();
			porcentajeUso();
		}
		_gp_matarProc(1);					// matar proceso 1
		_gg_escribir("Proceso 1 eliminado\n", 0, 0, 0);
		_gs_dibujarTabla();
		
		while (_gd_tickCount < 480)			// esperar 4 segundos más
		{
			_gp_WaitForVBlank();
			porcentajeUso();
		}
		_gp_matarProc(3);					// matar proceso 3
		_gg_escribir("Proceso 3 eliminado\n", 0, 0, 0);
		_gs_dibujarTabla();
		
		while (_gp_numProc() > 1)			// esperar a que proceso 2 acabe
		{
			_gp_WaitForVBlank();
			porcentajeUso();
		}
		_gg_escribir("Proceso 2 terminado\n", 0, 0, 0);
	} else
		_gg_escribir("*** Programa NO cargado\n", 0, 0, 0);*/

	/* TEST 2*/
	/* 
	_gg_escribir("*** Carga de programa PONG.elf\n", 0, 0, 0);
	start = _gm_cargarPrograma("PONG");
	if (start)
	{
		for (v = 1; v < 4; v++)	// inicializar buffers de ventanas 1, 2 y 3
			_gd_wbfs[v].pControl = 0;
		
		_gp_crearProc(start, 1, "PONG", 1);
		_gp_crearProc(start, 2, "PONG", 2);
		_gp_crearProc(start, 3, "PONG", 3);
		
		mtics = _gd_tickCount + 960;
		while (_gd_tickCount < mtics)		// esperar 16 segundos más
		{
			_gp_WaitForVBlank();
			porcentajeUso();
		}
		
		_gp_matarProc(1);					// matar los 3 procesos a la vez
		_gp_matarProc(2);
		_gp_matarProc(3);
		_gg_escribir("Procesos 1, 2 y 3 eliminados\n", 0, 0, 0);
		
		while (_gp_numProc() > 1)	// esperar a que todos los procesos acaben
		{
			_gp_WaitForVBlank();
			porcentajeUso();
		}
		
	} else
		_gg_escribir("*** Programa NO cargado\n", 0, 0, 0);


	_gg_escribir("*** Final fase 2_P\n", 0, 0, 0);
	_gs_dibujarTabla();

	while(1) {
		_gp_WaitForVBlank();
	}							// parar el procesador en un bucle infinito
	return 0;
	*/
