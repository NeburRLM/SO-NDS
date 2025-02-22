/*------------------------------------------------------------------------------

	"garlic_graf.c" : fase 1 / programador G

	Funciones de gesti�n del entorno gr�fico (ventanas de texto), para GARLIC 1.0

------------------------------------------------------------------------------*/
#include <nds.h>

#include "garlic_system.h"	// definicion de funciones y variables de sistema
#include "garlic_font.h"	// definicion grafica de caracteres

#define NVENT 16 // numero de ventanas totales
#define PPART 4 // numero de ventanas horizontales

#define VCOLS 32
#define VFILS 24
#define PCOLS VCOLS * PPART // numero de columnas totales
#define PFILS VFILS * PPART // numero de filas totales

// Constants per als caracters especials (funcio addicional GARLIC_setChar())
#define NUM_CUSTOM_CHARS 128	// Rang: 128 - 255
#define TAM_CARACTER 8			// Mida 8x8 per cada caracter especial

const unsigned int char_colors[] = {240, 96, 64};	// amarillo, verde, rojo

// Variables
// Fondos 2 i 3
int background_2, background_3;

// Punter als bitmaps dels fons 2 i 3
u16 *mapPtr_2, *mapPtr_3;

/* normalitzarChar: Rutina de soport per convertir els caracters ASCII extended (128-255) que la nostra paleta NO suporta, als seus equivalents 
	Per exemple: � -> a
	Parametros:
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
        case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5:  // � � � � � �
            return 'A';
        case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5:  // � � � � � �
            return 'a';
        case 0xC8: case 0xC9: case 0xCA: case 0xCB:  // � � � �
            return 'E';
        case 0xE8: case 0xE9: case 0xEA: case 0xEB:  // � � � �
            return 'e';
        case 0xCC: case 0xCD: case 0xCE: case 0xCF:  // � � � �
            return 'I';
        case 0xEC: case 0xED: case 0xEE: case 0xEF:  // � � � �
            return 'i';
        case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6:  // � � � � �
            return 'O';
        case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6:  // � � � � �
            return 'o';
        case 0xD9: case 0xDA: case 0xDB: case 0xDC:  // � � � �
            return 'U';
        case 0xF9: case 0xFA: case 0xFB: case 0xFC:  // � � � �
            return 'u';
        case 0xD1:  // �
            return 'N';
        case 0xF1:  // �
            return 'n';
			
        // Cas caracter no suportat, retornar '?'
        default:
            return '?';
    }
}

/* _gg_generarMarco: dibuja el marco de la ventana que se indica por par�metro*/
void _gg_generarMarco(int v, int color)
{
	color *= 128;	// Color * 128 per despl. a l'index corresponent al color X del conjunt de baldoses

	/* Calcular fila (Fv) i columna (Cv) inicial per cada finestra
	 * Exemple: (v=3) 
	 * 	Fv = (3 / 2) * 24 = 1 * 24 = 24
	 * 	Cv = (3 % 2) * 32 = 1 * 32 = 32
	*/
    int Fv = (v / PPART) * VFILS; // Fila inicial
    int Cv = (v % PPART) * VCOLS; // Columna inicial

	// 1. Dibuixar marc superior-esquerra (103)
	mapPtr_3[Fv * PCOLS + Cv] = 103 + color;
	
	// 2. Dibuixar marc superior-dreta (102)
    mapPtr_3[Fv * PCOLS + (Cv + VCOLS - 1)] = 102 + color;
	
	// 3. Dibuixar marc inferior-esquerra (100)
	mapPtr_3[(Fv + VFILS - 1) * PCOLS + Cv] = 100 + color;

	// 4. Dibuixar marc inferior-dreta (101)
	mapPtr_3[(Fv + VFILS - 1) * PCOLS + (Cv + VCOLS - 1)] = 101 + color;

	// 5. Dibuixar marc superior (99) e inferior (97)
	for (int col = Cv + 1; col < Cv + (VCOLS - 1); col++)
	{
        mapPtr_3[Fv * PCOLS + col] = 99 + color;
        mapPtr_3[(Fv + VFILS - 1) * PCOLS + col] = 97 + color;
    }
	// 6. Dibuixar marc esquerra (96) i dreta (98)
	for (int fila = Fv + 1; fila < Fv + (VFILS - 1); fila++)
	{
        mapPtr_3[fila * PCOLS + Cv] = 96 + color;
		mapPtr_3[fila * PCOLS + (Cv + VCOLS - 1)] = 98 + color;
    }
}

/* _gg_iniGraf: inicializa el procesador grafico A para GARLIC 1.0 */
void _gg_iniGrafA()
{
	videoSetMode(MODE_5_2D);	// Inicialitzar processador grafic principal (A) en mode 5
	lcdMainOnTop();				// Establir pantalla superior com a principal
	
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);	// Reservar banc de memoria de video A

	/* Inicialitzar fons grafic 2
	 * ---
	 * Especificacions generals:
	 *
	 * Finestra = 32x24 (256x192 px) -> Total = 4x4 finestres (1024x768 px)
	 * Mapa = (1024/8 X) x (1024/8 Y) = 128x128 baldoses = 16 KB
	 * Mem. necessaria = Mapa x 2 bytes/pixel = 32 KB
	 *
	 * Mida baldoses = 128 baldosas * 8x8 pixels/baldosa * 1 byte/pixel = 8KB
	 * Dir.ini mapa (norma) = VirtVRAM_Background + mapBase * 2Kbytes
	 * Dir.ini baldoses = VirtVRAM_Background + tileBase * 16 Kbytes
	 * ---
	 * Dir.ini mapa = 0x06000000
	 * Dir.ini baldoses = 0x06010000
	*/
	background_2 = bgInit(2, BgType_ExRotation, BgSize_ER_1024x1024, 0, 4);
	
	/* Inicialitzar fons grafic 3
	 * Dir.ini mapa = 0x06008000
	 * Dir.ini baldoses = 0x06010000
	*/
	background_3 = bgInit(3, BgType_ExRotation, BgSize_ER_1024x1024, 16, 4);

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

	// Cada mapa de color ocupa 4096 bytes
	// 128 baldoses * (64 pixels/baldosa / 2 bytes/pixel) = 4096 bytes
	u16 *mapaColor = bgGetGfxPtr(background_3) + (128 * (64/2));
	
	// Copiar contingut garlic_fontTiles 3 cops per aconseguir les 128 baldoses amb els 4 colors diferents (blanc, groc, verd i roig)
	for (int colorIndx = 0; colorIndx < 3; colorIndx++)
	{
	    // Descomprimir les baldoses originals (en blanc) al mapaColor actual
		decompress(garlic_fontTiles, mapaColor, LZ77Vram);
		
		// Processar cada pixel de la baldosa de les 128 baldoses
		for (int j = 0; j < 4096; j++)
		{
			if (mapaColor[j] & 0xFF00)	// Agafar valor bits alts (bits 15-8)
			{
				mapaColor[j] = (mapaColor[j] & 0x00FF);	// Eliminar valor bits alts
				mapaColor[j] = (mapaColor[j] | (char_colors[colorIndx] << 8));	// Guardar el nou color als bits alts utilitzant l'index de color corresponent de char_colors (groc, verd i roig)
			}
			if (mapaColor[j] & 0x00FF)	// Agafar valor bits baixos (bits 7-0)
			{
				mapaColor[j] = (mapaColor[j] & 0xFF00);	// Eliminar valor bits baixos
				mapaColor[j] = (mapaColor[j] | char_colors[colorIndx]);	// Guardar el nou color als bits baixos utilitzant l'index de color corresponent de char_colors (groc, verd i roig)
			}
		}
		mapaColor += 4096;	// mapaColor++
	}

	// Generar els marcs del fons 3 a cada finestra
	for (int i = 0; i < NVENT; i++)
	{
		_gg_generarMarco(i, 3);	// Generar marcs amb color roig (proces inactiu) per defecte
	}

	/* Escalar fons 2 i 3
	 * Los parametros de escalado se tienen que proporcionar en formato de coma fija 0.24.8.
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
					  precedidas por '%' e inserta la representaci�n ASCII de
					  los valores indicados por par�metro.
	Parametros:
		formato	->	string con marcas de formato (ver descripci�n _gg_escribir);
		val1, val2	->	valores a transcribir, sean n�mero de c�digo ASCII (%c),
					un n�mero natural (%d, %x) o un puntero a string (%s);
		resultado	->	mensaje resultante.
	Observacion:
		Se asume que el string resultante tiene reservado espacio de memoria
		suficiente para albergar todo el mensaje, incluyendo los caracteres
		literales del formato y la transcripci�n en c�digo ASCII de los valores.
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
void _gg_escribir(char *formato, unsigned int val1, unsigned int val2, int ventana)
{
	char resultat[3 * VCOLS + 1];	// Resultat max 3 files (+1 sentinella)
	
	int pControl = _gd_wbfs[ventana].pControl;	// Llegir camp pControl de la finestra actual
	int charPndt = pControl & 0xFFFF;			// Comptador de caracters fins emplenar el buffer (0,32) (16b baixos (0..15)
	int numLinea = pControl >> 16 & 0xFFF;		// Comptador sobre el numero de fila/linea actual (0,23) (12b mitjans (27..16)
	char color = pControl >> 28;				// Index de color (0, 3) (4b alts (31..28)
	
	// Convertir el string de format a text definitiu
	_gg_procesarFormato(formato, val1, val2, resultat);
	
	// Processar text (fi: '\0')
	char charActual;		// Aux per llegir cada caracter del resultat
	int i = 0;				// Index buffer
	while(resultat[i] != '\0')	// Mentre no trobem el sentinella
	{
		charActual = resultat[i];	// LLegir caracter actual
		
		if (charActual == '\n')	/* Cas salt de linea */
		{
			_gp_WaitForVBlank();	// Esperar retroces vertical
			_gg_escribirLinea(ventana, numLinea, charPndt);	// Transferir caracters a la finestra
			
			// Actualitzar comptadors
			numLinea++;
			charPndt = 0;
			
			// Cas hem arribat al final de les files -> Desplacar
			if (numLinea == VFILS)	//numLinea -> [0,23] / VFILS = 24
			{
				_gg_desplazar(ventana);
				numLinea = VFILS - 1;	// Tornar a la ultima fila
			}
		}
		else if(charPndt == VCOLS)	/* Cas buffer ple (tornar a llegir i--) */
		{
			i--;	// Tornar a llegir ultim valor, ja que charPnt ha llegit 33 valors
			_gg_escribirLinea(ventana, numLinea, charPndt);	// Transferir caracters a la finestra
			_gp_WaitForVBlank();	// Esperar retroces vertical
			
			// Actualitzar comptadors
			numLinea++;
			charPndt = 0;
			
			// Cas hem arribat al final de les files -> Desplacar
			if (numLinea == VFILS)	//numLinea -> [0,23] / VFILS = 24
			{
				_gg_desplazar(ventana);
				numLinea = VFILS - 1;	// Tornar a la ultima fila
			}
		}
		else	/* Altres casos */
		{
			if (charActual == '%' && resultat[i + 1] >= '0' && resultat[i + 1] <= '3') /* Cas marca de color */
			{
				color = resultat[i + 1] - '0';  // Actualitzar color actual - 0 (valor ASCII = 48)
				i++; // Saltar al seguent caracter, ja s'ha llegit i + i+1
			}
			else if(charActual == '\t')	/* Cas tabulador */
			{
				// Calcular espais necessaris
				int tab = 4 - (charPndt % 4);
				// Plenar el buffer amb espais fins que no quedi espai (32 posicions)
				for (int j = 0; j < tab && charPndt != VCOLS; j++)
				{
					_gd_wbfs[ventana].pChars[charPndt++] = ' '  - 32;	// Plenar buffer
				}
			}
			else if(charActual >= '\x80' && charActual <= '\x87')	/* Cas caracter custom (128-135 en hexa) */
			{
				/* charActual + 128 * 3 per saltar els 3 colors + 8 * color per agafar el caracter amb el color pertinent */
				_gd_wbfs[ventana].pChars[charPndt++] = charActual + 128 * 3 + (color == 0 ? 0 : (8 * color));
			}
			else	/* Cas caracter literal */
			{
				_gd_wbfs[ventana].pChars[charPndt++] = (charActual - 32) + 128 * color;
			}
		}
		// Actualitzar variable pControl
		_gd_wbfs[ventana].pControl = (color << 28) | (numLinea << 16) | charPndt;	// Color: 31..28 | numLinea 27..16 | charPndt 15..0
		i++;
	}
	_gd_wbfs[ventana].pControl = (color << 28) | (numLinea << 16) | charPndt;
}


/* _gg_setChar: escribe una cadena de caracteres en la ventana indicada;
	Parametros:
		n	->	numero de caracter ASCII Extended (entre 128 i 135)
		buffer	->	punter a matriu de 8x8 bytes
*/
void _gg_setChar(unsigned char n, unsigned char *buffer)
{
    if (n < 128 || n > 135) { return; }	// Comprovar n dins del rang

    // Calcular dir. base baldoses[n] (4096 * 3 per saltar la paleta inicial amb totes les variants de color)
    u16 *tileBaseAddress = bgGetGfxPtr(background_2) + (n * 32) + (4096 * 3);	

	// Copiar el buffer (customChar) a la direccio de mem. de les baldoses (VRAM)
    //dmaCopy(buffer, tileBaseAddress, 64);
	_gs_copiaMem(buffer, tileBaseAddress, 64);

	// Cada mapa de color custom ocupa 256 bytes
	// 8 baldoses * (64 pixels/baldosa / 2 bytes/pixel) = 256 bytes    
	u16 *colorTileBase = tileBaseAddress + (8 * 32); // Base de colores
	
    for (int colorIndx = 0; colorIndx < 3; colorIndx++)
	{
		// Copiar la baldosa original al mapa de color corresponent
        //dmaCopy(tileBaseAddress, colorTileBase, 64);
		_gs_copiaMem(tileBaseAddress, colorTileBase, 64);
		
        // Processar cada pixel de la baldosa
        for (int i = 0; i < 32; i++)	// Procesar 32 pixels (16 bits per pixel)
		{ 
            u16 pixel = tileBaseAddress[i]; // Llegir pixel original
            
            // Verificar si el pixel es blanc en bits baixos o alts
            if ((pixel & 0x00FF) == 0x00FF)
			{
                // Aplicar el nou color als bits baixos
                colorTileBase[i] = (colorTileBase[i] & 0xFF00) | char_colors[colorIndx];
            }
            if ((pixel & 0xFF00) == 0xFF00)
			{
                // Aplicar el nou color als bits alts
                colorTileBase[i] = (colorTileBase[i] & 0x00FF) | (char_colors[colorIndx] << 8);
            }
        }
		colorTileBase += 256;
    }
}