/*------------------------------------------------------------------------------

	"garlic_graf.c" : fase 1 / programador G

	Funciones de gesti�n del entorno gr�fico (ventanas de texto), para GARLIC 1.0

------------------------------------------------------------------------------*/
#include <nds.h>

#include "garlic_system.h"	// definici�n de funciones y variables de sistema
#include "garlic_font.h"	// definici�n gr�fica de caracteres

/* definiciones para realizar c�lculos relativos a la posici�n de los
caracteres dentro de las ventanas gr�ficas, que pueden ser 4 o 16 */
#define NVENT 4 // n�mero de ventanas totales
#define PPART 2 // n�mero de ventanas horizontales

// (particiones de pantalla)
#define VCOLS 32 // columnas y filas de cualquier ventana
#define VFILS 24
#define PCOLS VCOLS * PPART // n�mero de columnas totales
#define PFILS VFILS * PPART // n�mero de filas totales

// Variables
int background_2, background_3;

/* _gg_generarMarco: dibuja el marco de la ventana que se indica por par�metro*/
void _gg_generarMarco(int v)
{

}


/* _gg_iniGraf: inicializa el procesador gr�fico A para GARLIC 1.0 */
void _gg_iniGrafA()
{
	videoSetMode(MODE_5_2D);	// Inicialitzar processador gr�fic principal (A) en mode 5
	lcdMainOnTop();				// Establir pantalla superior com a principal
	
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);	// Reservar banc de mem�ria de video A

	/* Inicialitzar fons grafic 2
	 * ---
	 * Especificacions generals:
	 * Mida mapa = 32x32 posiciones * 2 bytes/posici�n = 2KB
	 * Mida baldoses = 128 baldosas * 8x8 p�xeles/baldosa * 1 byte/p�xel = 8KB
	 * Dir.ini mapa (norma) = VirtVRAM_Background + mapBase * 2Kbytes
	 * Dir.ini baldoses = VirtVRAM_Background + tileBase * 16 Kbytes
	 * ---
	 * Dir.ini mapa = 0x06000000 + 0 * 2KB = 0x06000000 -> Inicialitzem al principi del banc de memoria
	 * Dir.ini baldoses = 0x06000000 + 1 * 16KB = 0x06016384 -> Direccio inical baldoses
	*/
	background_2 = bgInit(2, BgType_ExRotation, BgSize_ER_512x512, 0, 1);
	
	/* Inicialitzar fons grafic 3
	 * Dir.ini mapa = 0x06000000 + 2 * 2KB = 0x06004096
	 * Dir.ini baldoses = 0x06000000 + 1 * 16KB = 0x06016384 -> Direccio inical baldoses
	*/
	background_3 = bgInit(3, BgType_ExRotation, BgSize_ER_512x512, 2, 1);

	// Prioritzar fons 3 > 2
	bgSetPriority(background_2, 1);
	bgSetPriority(background_3, 0);
	
	// Descomprimir el contingut de la font de lletres -> garlic_fontTiles (garlic_font.s)
	decompress(garlic_fontTiles, bgGetGfxPtr(background_2), LZ77Vram);
	
	// Copiar la paleta de colors
	dmaCopy(garlic_fontPal, BG_PALETTE, sizeof(garlic_fontPal));

	// Generar els marcos de les finestres de text del fons 3
	for (int i=0; i<NVENT; i++)
	{
		_gg_generarMarco(i);
	}

	/* Escalar fons 2 i 3
	 * Los par�metros de escalado se tienen que proporcionar en formato de coma fija 0.24.8.
	 * 50% = 0b0101 = 0x5
	 * Format coma fixa (0.24.8) = 0x00000005
	*/
	bgSetScale(background_2, 0x00000005, 0x00000005);
	bgSetScale(background_3, 0x00000005, 0x00000005);
	
	// Actualiza todos los fondos despues de realizar rotaciones, escalados o desplazamientos
	bgUpdate();
}



/* _gg_procesarFormato: copia los caracteres del string de formato sobre el
					  string resultante, pero identifica las marcas de formato
					  precedidas por '%' e inserta la representaci�n ASCII de
					  los valores indicados por par�metro.
	Par�metros:
		formato	->	string con marcas de formato (ver descripci�n _gg_escribir);
		val1, val2	->	valores a transcribir, sean n�mero de c�digo ASCII (%c),
					un n�mero natural (%d, %x) o un puntero a string (%s);
		resultado	->	mensaje resultante.
	Observaci�n:
		Se asume que el string resultante tiene reservado espacio de memoria
		suficiente para albergar todo el mensaje, incluyendo los caracteres
		literales del formato y la transcripci�n en c�digo ASCII de los valores.
*/
void _gg_procesarFormato(char *formato, unsigned int val1, unsigned int val2,
																char *resultado)
{

}


/* _gg_escribir: escribe una cadena de caracteres en la ventana indicada;
	Par�metros:
		formato	->	string de formato:
					admite '\n' (salto de l�nea), '\t' (tabulador, 4 espacios)
					y c�digos entre 32 y 159 (los 32 �ltimos son caracteres
					gr�ficos), adem�s de marcas de format %c, %d, %h y %s (m�x.
					2 marcas por string)
		val1	->	valor a sustituir en la primera marca de formato, si existe
		val2	->	valor a sustituir en la segunda marca de formato, si existe
					- los valores pueden ser un c�digo ASCII (%c), un valor
					  natural de 32 bits (%d, %x) o un puntero a string (%s)
		ventana	->	n�mero de ventana (0..3)
*/
void _gg_escribir(char *formato, unsigned int val1, unsigned int val2, int ventana)
{

}
