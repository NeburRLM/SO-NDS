/*------------------------------------------------------------------------------

	"garlic_graf.c" : fase 1 / programador G

	Funciones de gestión del entorno gráfico (ventanas de texto), para GARLIC 1.0

------------------------------------------------------------------------------*/
#include <nds.h>

#include "garlic_system.h"	// definición de funciones y variables de sistema
#include "garlic_font.h"	// definición gráfica de caracteres

#define NVENT 4 // número de ventanas totales
#define PPART 2 // número de ventanas horizontales

#define VCOLS 32
#define VFILS 24
#define PCOLS VCOLS * PPART // número de columnas totales
#define PFILS VFILS * PPART // número de filas totales

// Variables
// Fondos 2 i 3
int background_2, background_3;

// Punter als bitmaps dels fons 2 i 3
u16 *mapPtr_2, *mapPtr_3;


/* normalitzarChar: Rutina de soport per convertir els caracters ASCII extended (128-255) que la nostra paleta NO suporta, als seus equivalents 
	Per exemple: à -> a
	Parámetros:
		c	->	char en format ASCII extended a transformar
*/
unsigned char normalitzarChar(unsigned char c)
{
    // Cas caracter ASCII
    if (c < 128) {
        return c;
    }
    
    // Normalitzar caracters
    switch (c) {
        case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5:  // À Á Â Ã Ä Å
            return 'A';
        case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5:  // à á â ã ä å
            return 'a';
        case 0xC8: case 0xC9: case 0xCA: case 0xCB:  // È É Ê Ë
            return 'E';
        case 0xE8: case 0xE9: case 0xEA: case 0xEB:  // è é ê ë
            return 'e';
        case 0xCC: case 0xCD: case 0xCE: case 0xCF:  // Ì Í Î Ï
            return 'I';
        case 0xEC: case 0xED: case 0xEE: case 0xEF:  // ì í î ï
            return 'i';
        case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6:  // Ò Ó Ô Õ Ö
            return 'O';
        case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6:  // ò ó ô õ ö
            return 'o';
        case 0xD9: case 0xDA: case 0xDB: case 0xDC:  // Ù Ú Û Ü
            return 'U';
        case 0xF9: case 0xFA: case 0xFB: case 0xFC:  // ù ú û ü
            return 'u';
        case 0xD1:  // Ñ
            return 'N';
        case 0xF1:  // ñ
            return 'n';
			
        // Cas caracter no suportat, retornar '?'
        default:
            return '?';
    }
}

/* _gg_generarMarco: dibuja el marco de la ventana que se indica por parámetro*/
void _gg_generarMarco(int v)
{
	/* Calcular fila (Fv) i columna (Cv) inicial per cada finestra
	 * Exemple: (v=3) 
	 * 	Fv = (3 / 2) * 24 = 1 * 24 = 24
	 * 	Cv = (3 % 2) * 32 = 1 * 32 = 32
	*/
    int Fv = (v / PPART) * VFILS; // Fila inicial
    int Cv = (v % PPART) * VCOLS; // Columna inicial

	// 1. Dibuixar marc superior-esquerra (103)
	mapPtr_3[Fv * PCOLS + Cv] = 103;
	
	// 2. Dibuixar marc superior-dreta (102)
    mapPtr_3[Fv * PCOLS + (Cv + VCOLS - 1)] = 102;
	
	// 3. Dibuixar marc inferior-esquerra (100)
	mapPtr_3[(Fv + VFILS - 1) * PCOLS + Cv] = 100;

	// 4. Dibuixar marc inferior-dreta (101)
	mapPtr_3[(Fv + VFILS - 1) * PCOLS + (Cv + VCOLS - 1)] = 101;

	// 5. Dibuixar marc superior (99) e inferior (97)
	for (int col = Cv + 1; col < Cv + (VCOLS - 1); col++)
	{
        mapPtr_3[Fv * PCOLS + col] = 99;
        mapPtr_3[(Fv + VFILS - 1) * PCOLS + col] = 97;
    }
	// 6. Dibuixar marc esquerra (96) i dreta (98)
	for (int fila = Fv + 1; fila < Fv + (VFILS - 1); fila++)
	{
        mapPtr_3[fila * PCOLS + Cv] = 96;
		mapPtr_3[fila * PCOLS + (Cv + VCOLS - 1)] = 98;
    }
}

/* _gg_iniGraf: inicializa el procesador gráfico A para GARLIC 1.0 */
void _gg_iniGrafA()
{
	videoSetMode(MODE_5_2D);	// Inicialitzar processador gràfic principal (A) en mode 5
	lcdMainOnTop();				// Establir pantalla superior com a principal
	
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);	// Reservar banc de memòria de video A

	/* Inicialitzar fons grafic 2
	 * ---
	 * Especificacions generals:
	 * Mida mapa = 64x48 posicions * 2 bytes/posicio = 6KB
	 * Mida baldoses = 128 baldosas * 8x8 píxeles/baldosa * 1 byte/píxel = 8KB
	 * Dir.ini mapa (norma) = VirtVRAM_Background + mapBase * 2Kbytes
	 * Dir.ini baldoses = VirtVRAM_Background + tileBase * 16 Kbytes
	 * ---
	 * Dir.ini mapa = 0x06000000 + 0 * 2KB = 0x06000000 -> Inicialitzem al principi del banc de memoria
	 * Dir.ini baldoses = 0x06000000 + 1 * 16KB = 0x06016384 -> Direccio inical baldoses
	*/
	background_2 = bgInit(2, BgType_ExRotation, BgSize_ER_512x512, 0, 1);
	
	/* Inicialitzar fons grafic 3
	 * Dir.ini mapa = 0x06000000 + 4 * 2KB = 0x06008192
	 * Dir.ini baldoses = 0x06000000 + 1 * 16KB = 0x06016384 -> Direccio inical baldoses
	*/
	background_3 = bgInit(3, BgType_ExRotation, BgSize_ER_512x512, 4, 1);

	// Prioritzar fons 3 > 2
	bgSetPriority(background_2, 1);
	bgSetPriority(background_3, 0);
	
	// Descomprimir el contingut de la font de lletres -> garlic_fontTiles (garlic_font.s)
	decompress(garlic_fontTiles, bgGetGfxPtr(background_2), LZ77Vram);
	
	// Copiar la paleta de colors
	dmaCopy(garlic_fontPal, BG_PALETTE, sizeof(garlic_fontPal));

	// Obtenir referencia del fons 2 i 3 (per poder escriure sobre el bitmap)
	mapPtr_2 = bgGetMapPtr(background_2);
	mapPtr_3 = bgGetMapPtr(background_3);

	// Generar els marcs del fons 3 a cada finestra
	for (int i=0; i<NVENT; i++)
	{
		_gg_generarMarco(i);
	}

	/* Escalar fons 2 i 3
	 * Los parámetros de escalado se tienen que proporcionar en formato de coma fija 0.24.8.
	 * Escala 2 = 50%
	*/
	int scale = 2 << 8;
	bgSetScale(background_2, scale, scale);
	bgSetScale(background_3, scale, scale);
	
	// Actualiza todos los fondos despues de realizar rotaciones, escalados o desplazamientos
	bgUpdate();
}

/* _gg_procesarFormato: copia los caracteres del string de formato sobre el
					  string resultante, pero identifica las marcas de formato
					  precedidas por '%' e inserta la representación ASCII de
					  los valores indicados por parámetro.
	Parámetros:
		formato	->	string con marcas de formato (ver descripción _gg_escribir);
		val1, val2	->	valores a transcribir, sean número de código ASCII (%c),
					un número natural (%d, %x) o un puntero a string (%s);
		resultado	->	mensaje resultante.
	Observación:
		Se asume que el string resultante tiene reservado espacio de memoria
		suficiente para albergar todo el mensaje, incluyendo los caracteres
		literales del formato y la transcripción en código ASCII de los valores.
*/
void _gg_procesarFormato(char *formato, unsigned int val1, unsigned int val2,
																char *resultado)
{
	int i = 0;					// Comptador per al format
	int j = 0;					// Comptador per al resultat
	unsigned int valorActual;  	// Variable auxiliar per gestionar val1 o val2
	int valControl = 0;   		// Variable de control per utilitzar val1 o val2
	char buffer[11];			// Buffer per les conversions numeriques (10 digits enter + 1 sentinella)
	int codi;					// Codi retorn conversions numeriques (0 -> OK, != 0 -> Error)
	unsigned char *str;			// Buffer per a la marca de tipus string
	
	// Processar text (fi: '\0')
	while (formato[i] != '\0')
	{
		if (formato[i] == '%')	// Cas marca de format
		{
			i++;	// Seguir amb la seguent posicio per obtindre el tipus de format
			
			valorActual = (valControl == 0) ? val1 : val2;	// Usar val1 o val2
			if(valControl > 1)
			{
				// Ignorar altres formats desconeguts - Retornar format (p ex. '%i')
				resultado[j++] = '%';
				resultado[j++] = formato[i];
			}
			else
			{
				switch (formato[i])
				{
					case 'c':	// Caracter ASCII
						resultado[j++] = normalitzarChar((char) valorActual);	// Convertir val1 a caracter ASCII
						valControl++;	// Canviar de val1 a val2 
						break;
						
					case 'd':	// Decimal
						codi = _gs_num2str_dec(buffer, sizeof(buffer), valorActual);	// Convertir val1 a string decimal
						if (codi == 0) {	// Comprovem conversio correcta
							for (int k = 0; buffer[k] != '\0'; k++) {
								// El buffer guarda ' ' si el numero no ocupa totes les posicions del buffer, ignorar
								if (buffer[k] != ' ')
								{
									resultado[j++] = buffer[k];	// Copiar el decimal al resultat
								}
							}
							valControl++;	// Canviar de val1 a val2 
						}
						break;
						
					case 'x':	// Hexadecimal
						codi = _gs_num2str_hex(buffer, sizeof(buffer), valorActual);	// Convertir val1 a string hexa
						if (codi == 0) {	// Comprovem conversio correcta
							int k = 0;
							
							// Ignorar els 0 inicials (p ex. 0x000B -> 0xB)
							while (buffer[k] == '0' && buffer[k+1] != '\0') {
								k++;
							}

							// Copiar valors rellevants al buffer resultat
							while (buffer[k] != '\0') {
								resultado[j++] = buffer[k++];
							}
							valControl++;  // Canviar de val1 a val2 
						}
						break;
						
					case 's':  	// String
						str = (unsigned char *) valorActual;
						while (*str != '\0') {
							resultado[j++] = normalitzarChar(*str++);  // Copiar el string normalitzat al resultat
						}
						valControl++;	// Canviar de val1 a val2
						break;

					case '%':  	// '%'
						resultado[j++] = '%';	// Afegir '%' al resultat
						break;
						
					default:
						// Ignorar altres formats desconeguts - Retornar format (p ex. '%i')
						resultado[j++] = '%';
						resultado[j++] = formato[i];
						break;
				}
			}
			
			i++;	// Continuar al seguent caracter
		}
		else	// Cas caracter literal
		{
			resultado[j++] = formato[i++];	// Copiar caracter literal al resultat
		}
	}
	
	// Assegurar que el resultat acabi amb sentinella
	resultado[j] = '\0';
}

/* _gg_escribir: escribe una cadena de caracteres en la ventana indicada;
	Parámetros:
		formato	->	string de formato:
					admite '\n' (salto de línea), '\t' (tabulador, 4 espacios)
					y códigos entre 32 y 159 (los 32 últimos son caracteres
					gráficos), además de marcas de format %c, %d, %h y %s (máx.
					2 marcas por string)
		val1	->	valor a sustituir en la primera marca de formato, si existe
		val2	->	valor a sustituir en la segunda marca de formato, si existe
					- los valores pueden ser un código ASCII (%c), un valor
					  natural de 32 bits (%d, %x) o un puntero a string (%s)
		ventana	->	número de ventana (0..3)
*/
void _gg_escribir(char *formato, unsigned int val1, unsigned int val2, int ventana)
{
	char resultat[3 * VCOLS + 1];	// Resultat max 3 files (+1 sentinella)
	
	int pControl = _gd_wbfs[ventana].pControl;	// Llegir camp pControl de la finestra actual
	int charPndt = pControl & 0xFFFF;			// Comptador de caracters fins emplenar el buffer (0,32) (16b)
	int numLinea = pControl >> 16;				// Comptador sobre el numero de fila/linea actual (0,23) (16b)
	
	// Convertir el string de format a text definitiu
	_gg_procesarFormato(formato, val1, val2, resultat);
	
	// Processar text (fi: '\0')
	char charActual;		// Aux per llegir cada caracter del resultat
	for(int i = 0; resultat[i] != '\0'; i++)
	{
		charActual = resultat[i];	// LLegir caracter
		
		// Cas buffer ple o '\n'
		if(charActual == '\n' || charPndt == VCOLS)
		{
			swiWaitForVBlank();	// Esperar retroces vertical
			_gg_escribirLinea(ventana, numLinea, charPndt);	// Transferir caracters a la finestra
			
			charPndt = ventana == 0 ? 1 : 0;	// Reiniciar comptador (bug ventana 0 es mostra diferent a les altres)
			
			numLinea++;	// Comptador +1 fila
			
			//Cas hem arribat al final de les files -> Desplacar
			if (numLinea == VFILS)	//numLinea -> [0,23] / VFILS = 24
			{
				_gg_desplazar(ventana);
				numLinea = VFILS - 1;	// Tornar a la ultima fila
			}
		}
		else if(charActual == '\t')	// Cas tabulador
		{
			// Calcular espais necessaris
			int tab = 4 - (charPndt % 4);
			// Plenar el buffer amb espais fins que no quedi espai (32 posicions)
			for (int j = 0; j < tab && charPndt < VCOLS; j++)
			{
				_gd_wbfs[ventana].pChars[charPndt++] = ' '  - 32;
			}
		}
		else	// Cas caracter literal
		{
			_gd_wbfs[ventana].pChars[charPndt++] = charActual - 32;
		}
		
		// Actualitzar variable pControl amb la linea actual i charPndt
		_gd_wbfs[ventana].pControl = (numLinea << 16) | charPndt;
	}
}
