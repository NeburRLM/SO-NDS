/*------------------------------------------------------------------------------

	"sqr1.c" : fase 1 / programador G

	Programa per calcular l'arrel quadrada de 20 valors aleatoris (valors: 1 a 10.000 * arg) per tanteig

------------------------------------------------------------------------------*/

#include <GARLIC_API.h>			/* definicio funcions API del SO GARLIC */

// Metode que retorna en format Q12 (0.20.12) l'arrel quadrada d'un valor x per tanteig */
unsigned int sqrtQ12(unsigned int x) {
    
    /* Metode calcul per tanteig:
     * 1. Definir un rang inicial on pot estar l'arrel quadrada (0 a x (Q12))
     * 2. Calcular valor mitja (mid = low + high / 2) i calcular el seu quadrat (midSquared *= mid)
     * 3. Aplicar divisio Q12 i comprovar resultat
     *      3.1. Si quo > x -> arrel quadrada es mes petita -> Ajustar high (high - 1)
     *      3.2. Si quo < x -> arrel quadrada es mes gran -> Ajustar low (low + 1)
     * 4. Repetir el proces fins que el quadrat calculat es proper al valor esperat.
    */

	unsigned int den = 4096; // Format Q12
	x = x * den;			// Valor x en format Q12 (2^12 = 4096)

    long long low = 0;
    long long high = x; // Valor x en format Q12 (2^12 = 4096)
    
	long long mid, midSquared, quo;
    unsigned int mod;
    
    while (low <= high) {
       
        mid = (low + high) / 2; // Calcular valor mitja
        midSquared = mid * mid; // Calcular mid^2
		
        // Convertir midSquared en format Q12
        if (GARLIC_divmodL(&midSquared, &den, &quo, &mod) != 0) {
            GARLIC_printf("Divisio per zero detectada!\n"); // GARLIC_divmod retorna != 0 quan detecta una divisio per zero
            return 0;
        }
        
        // Comprovar si estem a prop de l'arrel esperada
        if (quo == x)
            return mid;     // mid = resultat de l'arrel quadrada en format Q12
        else if (quo > x)   // Cas mid > arrel quadrada
            high = mid - 1;
        else if (quo < x)	// Cas mid < arrel quadrada
            low = mid + 1;
    }
    
    return high;
}

//------------------------------------------------------------------------------
// Funcio principal per calcular arrels quadrades aleatories en coma fixa
//------------------------------------------------------------------------------
int sqr1(int arg) {
    unsigned int randNum, sqrtResult, quoEnter, quoDecimal, quo, mod;
	unsigned int den = 4096;	// Format Q12

    // Limitar el valor de arg de 1 a 10000
    if (arg < 1) arg = 1;
    else if (arg > 10000) arg = 10000;
    
    // Escriure missatge inicial
    GARLIC_printf("* Inici programa SQR1\n");
	
    for (int i = 0; i < 20; i++) {
        // Generar un valor aleatori entre 1 i 10.000 * arg (+ 1 per no ser 0)
        //randNum = GARLIC_random() % (10000 * arg) + 1;
		GARLIC_divmod(GARLIC_random(), 10000 * arg, &quo, &randNum);	// Convertir valor Q12 a decimal
		randNum++;	// Cas GARLIC_random o arg = 0
		
        // Calcular l'arrel quadrada (format Q12) utilitzant el metode de tanteig en coma fixa (0.20.12)
		sqrtResult = sqrtQ12(randNum);
		
		GARLIC_divmod(sqrtResult, den, &quoEnter, &mod);	// Convertir valor Q12 a decimal
		GARLIC_divmod(mod * 1000, den, &quoDecimal, &mod);	// El residu continua en Q12, s'ha de convertir
		
		GARLIC_printf("Valor: %d\n", randNum);
		GARLIC_printf("Arrel Q12: %d,%d\n\n", quoEnter, quoDecimal);
    }
    
    // Escriure missatge final
    GARLIC_printf("* Fi programa SQR1\n");

    return 0;
}