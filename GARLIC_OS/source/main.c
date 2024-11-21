/*------------------------------------------------------------------------------

	"main.c" : fase 1 / master
	
------------------------------------------------------------------------------*/
#include <nds.h>

#include "garlic_system.h"	// definici�n de funciones y variables de sistema

#include <GARLIC_API.h>		// inclusi�n del API para simular un proceso

char error1, error2;		// variables para guardar el resultado de crearProc

extern int * punixTime;		// puntero a zona de memoria con el tiempo real

intFunc start;
intFunc HOLA, PRNT, DIV1, SQR1, CUST, ORDH;

intFunc codiCarregarPrograma[20];	// Vector que guarda la dir de mem al carregar un programa o 0 en cas de error


// Funcio generica per carregar programes
intFunc carregarProg(unsigned int nomProg)
{
	_gg_escribir("*** Carga de programa %s.elf\n", nomProg, 0, 0);
	start = _gm_cargarPrograma((char *) nomProg);
	if (start)
	{
		_gg_escribir("*** Direccion de arranque :\n\t\t%x\n", (unsigned int) start, 0, 0);
		_gg_escribir("*** Pulse tecla \'START\' ::\n\n", 0, 0, 0);
		do
		{	_gp_WaitForVBlank();
			scanKeys();
		} while ((keysDown() & KEY_START) == 0);
		
		//start(val);
	}
	else
		_gg_escribir("*** Programa \"%s\" NO cargado\n", nomProg, 0, 0);

	return start;	// Retornar dir mem o 0 en cas de error
}


/* Inicializaciones generales del sistema Garlic */
//------------------------------------------------------------------------------
void inicializarSistema() {
//------------------------------------------------------------------------------
	int v;

	_gg_iniGrafA();			// inicializar procesador grafico A
	for (v = 0; v < 4; v++)	// para todas las ventanas
		_gd_wbfs[v].pControl = 0;		// inicializar los buffers de ventana
	
	if (!_gm_initFS())
	{
		_gg_escribir("ERROR: no se puede inicializar el sistema de ficheros!", 0, 0, 0);
		exit(0);
	}
	
	_gd_seed = *punixTime;	// inicializar semilla para numeros aleatorios con
	_gd_seed <<= 16;		// el valor de tiempo real UNIX, desplazado 16 bits

	irqInitHandler(_gp_IntrMain);	// instalar rutina principal interrupciones
	irqSet(IRQ_VBLANK, _gp_rsiVBL);	// instalar RSI de vertical Blank
	irqEnable(IRQ_VBLANK);			// activar interrupciones de vertical Blank
	REG_IME = IME_ENABLE;			// activar las interrupciones en general
	
	_gd_pcbs[0].keyName = 0x4C524147;	// "GARL"
	
	for(int i = 0; i < 8; i++)
		_gd_sem[i] = 1;
}


//------------------------------------------------------------------------------
int main(int argc, char **argv) {
//------------------------------------------------------------------------------
	
	inicializarSistema();
	
	_gg_escribir("********************************", 0, 0, 0);
	_gg_escribir("*                              *", 0, 0, 0);
	_gg_escribir("* Sistema Operativo GARLIC 1.0 *", 0, 0, 0);
	_gg_escribir("*                              *", 0, 0, 0);
	_gg_escribir("********************************", 0, 0, 0);
	_gg_escribir("*** Inicio fase 1_GPM\n", 0, 0, 0);
	
	// PROG M: Carregar programes a memoria
	
	codiCarregarPrograma[0] = carregarProg((unsigned int) "HOLA");
	codiCarregarPrograma[1] = carregarProg((unsigned int) "PRNT");
	codiCarregarPrograma[2] = carregarProg((unsigned int) "DIV1");
	codiCarregarPrograma[3] = carregarProg((unsigned int) "ORDH");
	codiCarregarPrograma[4] = carregarProg((unsigned int) "SQR1");
	//codiCarregarPrograma[5] = carregarProg((unsigned int) "CUST");	// Falta fase 2 prog M (2 segments)

	// PROG_P: Crear processos per executar els programes .elf
	
	if(codiCarregarPrograma[0] != 0)
	{
		_gp_crearProc(codiCarregarPrograma[0], 7, "HOLA", 1);
	}
	if(codiCarregarPrograma[1] != 0)
	{
		_gp_crearProc(codiCarregarPrograma[1], 5, "PRNT", 2);
	}
	if(codiCarregarPrograma[2] != 0)
	{
		_gp_crearProc(codiCarregarPrograma[2], 6, "DIV1", 1);
	}
	if(codiCarregarPrograma[3] != 0)
	{
		_gp_crearProc(codiCarregarPrograma[3], 10, "ORDH", 1);
	}
	if(codiCarregarPrograma[4] != 0)
	{
		_gp_crearProc(codiCarregarPrograma[4], 11, "SQR1", 1);
	}
	if(codiCarregarPrograma[5] != 0)
	{
		//_gp_crearProc(codiCarregarPrograma[5], 8, "CUST", 0);				// Falta fase 2 prog M (2 segments)
	}
	
	// PROG_P: Prova disponibilitat del zocalo
	_gg_escribir("*** Prova d'errors\n", 0, 0, 0);					// pruebas de los 2 posibles errores que puede dar crearProc
	
	error1 = _gp_crearProc(codiCarregarPrograma[0], 7, "HOLA", 0);
	if (error1 == 2)
		_gg_escribir("El zocalo esta ocupat\n", 0, 0, 0);
		
	error2 = _gp_crearProc(codiCarregarPrograma[0], 0, "HOLA", 0);
	if (error2 == 1)
		_gg_escribir("El zocalo esta reservat\n", 0, 0, 0);
	
	while (_gp_numProc() > 1)	// esperar a que terminen los procesos de usuario
	{
		_gp_WaitForVBlank();
		//_gg_escribir("*** Test %d:%d\n", _gd_tickCount, _gp_numProc(), 1);
	}

	_gg_escribir("*** Final fase 1_GPM\n", 0, 0, 0);

	while (1)
	{
		_gp_WaitForVBlank();
	}
	
	return 0;
}
