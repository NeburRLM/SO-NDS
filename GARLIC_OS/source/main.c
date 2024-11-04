/*------------------------------------------------------------------------------
	"main.c" : fase 1 / master
------------------------------------------------------------------------------*/
#include <nds.h>

#include "garlic_system.h"	// definici�n de funciones y variables de sistema

#include <GARLIC_API.h>		// inclusi�n del API para simular un proceso

int hola(int);				// funci�n que simula la ejecuci�n del proceso
int div1(int);

extern int prnt(int);		// otra funci�n (externa) de test correspondiente
							// a un proceso de usuario

extern int setCharTest();	// Joc de proves per als caracters custom

extern int sqr1(int);		// Programa usuari divisions

char error1, error2;		// variables para guardar el resultado de crearProc

extern int * punixTime;		// puntero a zona de memoria con el tiempo real


/* Inicializaciones generales del sistema Garlic */
//------------------------------------------------------------------------------
void inicializarSistema() {
//------------------------------------------------------------------------------
	int v;

	_gg_iniGrafA();			// inicializar procesador gr�fico A
	for (v = 0; v < 4; v++)	// para todas las ventanas
		_gd_wbfs[v].pControl = 0;		// inicializar los buffers de ventana
	
	_gd_seed = *punixTime;	// inicializar semilla para n�meros aleatorios con
	_gd_seed <<= 16;		// el valor de tiempo real UNIX, desplazado 16 bits

	irqInitHandler(_gp_IntrMain);	// instalar rutina principal interrupciones
	irqSet(IRQ_VBLANK, _gp_rsiVBL);	// instalar RSI de vertical Blank
	irqEnable(IRQ_VBLANK);			// activar interrupciones de vertical Blank
	REG_IME = IME_ENABLE;			// activar las interrupciones en general
	
	_gd_pcbs[0].keyName = 0x4C524147;	// "GARL"
	
	for(int i = 0; i < 8; i++){
		_gd_sem[i] = 1;
	}
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
	_gg_escribir("*** Inicio fase 1_GP\n", 0, 0, 0);
	
	// PROG_P: Test DIV1
	_gp_crearProc(div1, 14, "DIV1", 1);
	
	_gp_crearProc(hola, 7, "HOLA", 2);
	_gp_crearProc(prnt, 5, "PRNT", 1);
	
	// PROG_G: Test mostrar noves baldoses (custom chars)
	_gp_crearProc(setCharTest, 8, "CUST", 0);
	
	// PROG_G: Test SQR1
	_gp_crearProc(sqr1, 2, "sqr1", 2);
	
	_gg_escribir("*** Prova d'errors\n", 0, 0, 0);					// pruebas de los 2 posibles errores que puede dar crearProc
	
	error1 = _gp_crearProc(hola, 7, "HOLA", 0);
	if (error1 == 2)
		_gg_escribir("El zocalo esta ocupat\n", 0, 0, 0);
		
	error2 = _gp_crearProc(hola, 0, "HOLA", 0);
	if (error2 == 1)
		_gg_escribir("El zocalo esta reservat\n", 0, 0, 0);
	
	
	while (_gp_numProc() > 1)
	{
		_gp_WaitForVBlank();
		//_gg_escribir("*** Test %d:%d\n", _gd_tickCount, _gp_numProc(), 1);
	}						// esperar a que terminen los procesos de usuario

	_gg_escribir("*** Final fase 1_GP\n", 0, 0, 0);

	while (1)
	{
		_gp_WaitForVBlank();
	}							// parar el procesador en un bucle infinito
	return 0;
}


/* Programa que hace el signal */
//------------------------------------------------------------------------------
int hola(int arg) {
//------------------------------------------------------------------------------
	unsigned int i, j, iter, res;
	
	if (arg < 0) arg = 0;			// limitar valor m�ximo y 
	else if (arg > 3) arg = 3;		// valor m�nimo del argumento
	
									// esccribir mensaje inicial
	GARLIC_printf("-- Programa HOLA  -  PID (%d) --\n", GARLIC_pid());
	
	j = 1;							// j = c�lculo de 10 elevado a arg
	for (i = 0; i < arg; i++)
		j *= 10;
						// c�lculo aleatorio del n�mero de iteraciones 'iter'
	GARLIC_divmod(GARLIC_random(), j, &i, &iter);
	iter++;							// asegurar que hay al menos una iteraci�n
	
	for (i = 0; i < iter; i++){		// escribir mensajes
		GARLIC_printf("(%d)\t%d: Hello world!\n", GARLIC_pid(), i);
		res = GARLIC_signal(2);
		if(res != 1)
			GARLIC_printf("(%d)\tEsta libre\n", GARLIC_pid());
		else
			GARLIC_printf("(%d)\tHe desbloqueado un semaforo\n", GARLIC_pid());
	}
	return 0;
}


/* Programa que hace el wait */
int div1(int arg)
{
	unsigned int tam = 10*arg+1;
	unsigned int numAl[tam];
	unsigned int quo, mod;
	int res;
	
	GARLIC_printf("-- Programa DIV1-  PID (%d) --\n", GARLIC_pid());
	
	for(int i=0; i<2; i++){
		numAl[i]=GARLIC_random();
	}
	GARLIC_printf("(%d)\tMe bloqueo\n", GARLIC_pid());
	res = GARLIC_wait(2);
	if(res != 1)
		GARLIC_printf("(%d)\tNo me he bloqueado\n", GARLIC_pid());
	else
		GARLIC_printf("(%d)\tMe han desbloqueado\n", GARLIC_pid());
	for(int j=0; j<tam; j++){
		GARLIC_divmod(numAl[j], 2, &quo, &mod);
		if(mod==0){
			GARLIC_printf("(%d)->(%d) es multiplo de 2\n", GARLIC_pid(),numAl[j]);
		}else{
			GARLIC_divmod(numAl[j], 3, &quo, &mod);
			if(mod==0){
				GARLIC_printf("(%d)->(%d) es multiplo de 3\n", GARLIC_pid(),numAl[j]);
			}else{
				GARLIC_divmod(numAl[j], 5, &quo, &mod);
				if(mod==0){
					GARLIC_printf("(%d)->(%d) es multiplo de 5\n", GARLIC_pid(),numAl[j]);
				}else{
					GARLIC_divmod(numAl[j], 7, &quo, &mod);
					if(mod==0){
						GARLIC_printf("(%d)->(%d) es multiplo de 7\n", GARLIC_pid(),numAl[j]);
					}else{
						GARLIC_printf("(%d)->(%d) no es multiplo de 2, de 3, de 5 ni de 7\n", GARLIC_pid(),numAl[j]);
					}
				}
			}
		}
		
	}
	return 0;
}
