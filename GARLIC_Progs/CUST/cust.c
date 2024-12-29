#include <GARLIC_API.h>			/* definicio funcions API del SO GARLIC */

// Def rajoles
unsigned const char rombo[64] = {
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned const char rombo_ratlles[64] = {
0x00, 0x00, 0x00, 0x65, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x65, 0xFF, 0x65, 0x00, 0x00,
0x00, 0x65, 0xFF, 0x65, 0xFF, 0x65, 0xFF, 0x00,
0xFF, 0x65, 0xFF, 0x65, 0xFF, 0x65, 0xFF, 0x65,
0x00, 0x65, 0xFF, 0x65, 0xFF, 0x65, 0xFF, 0x00,
0x00, 0x00, 0xFF, 0x65, 0xFF, 0x65, 0x00, 0x00,
0x00, 0x00, 0x00, 0x65, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned const char smile[64] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00
};

unsigned const char smile_rodo[64] = {
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00,
0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00,
0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00,
0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00
};

unsigned const char avion[64] = {
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00
};

unsigned const char avion_ratlles[64] = {
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0xFF, 0x70, 0xFF, 0xFF, 0xFF, 0xFF, 0x70, 0xFF,
0xFF, 0x70, 0xFF, 0xFF, 0xFF, 0xFF, 0x70, 0xFF,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x70, 0xFF, 0xFF, 0x70, 0x00, 0x00
};

unsigned const char peix[64] = {
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x90, 0xFF, 0x00,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00
};

unsigned const char peix_ratlles[64] = {
0x00, 0x00, 0x00, 0x90, 0xFF, 0x00, 0x00, 0x00,
0xFF, 0x00, 0xFF, 0x90, 0xFF, 0x90, 0x00, 0x00,
0xFF, 0x90, 0xFF, 0x90, 0xFF, 0x80, 0xFF, 0x00,
0xFF, 0x90, 0xFF, 0x90, 0xFF, 0x90, 0xFF, 0x90,
0xFF, 0x90, 0xFF, 0x90, 0xFF, 0x90, 0xFF, 0x90,
0xFF, 0x90, 0xFF, 0x90, 0xFF, 0x90, 0xFF, 0x00,
0xFF, 0x00, 0xFF, 0x90, 0xFF, 0x90, 0x00, 0x00,
0x00, 0x00, 0x00, 0x90, 0xFF, 0x00, 0x00, 0x00
};

/* **** BLANC **** */
void proc1_blanc() {
    // Def rajola rombo en l'ID 130
    GARLIC_printf("%0PROC 1: Def ROMBO (ID 128)\n");
    GARLIC_setChar(128, rombo);

    // Most rajola rombo
    GARLIC_printf("%0PROC 1: Most ROMBO: \x80\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	// Def rajola smile en l'ID 132
    GARLIC_printf("%0PROC 1: Def SMILE (ID 129)\n");
    GARLIC_setChar(129, smile);

    // Most rajola smile
    GARLIC_printf("%0PROC 1: Most SMILE: \x81\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	GARLIC_printf("%0PROC 1: Barreja \x80 de \x81 figures!\n");
	
	// Retrassar el proces per perdre la CPU (simula un canvi de context)
	GARLIC_delay(0);
}

void proc2_blanc() {
    // Def rajola avion en l'ID 130
    GARLIC_printf("%0PROC 2: Def AVION (ID 130)\n");
    GARLIC_setChar(130, avion);

    // Most rajola avion
    GARLIC_printf("%0PROC 2: Most AVION: \x82\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	// Def rajola peix en l'ID 132
    GARLIC_printf("%0PROC 2: Def PEIX (ID 131)\n");
    GARLIC_setChar(131, peix);

    // Most rajola peix
    GARLIC_printf("%0PROC 2: Most PEIX: \x83\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	GARLIC_printf("%0PROC 2: Barreja \x82 de \x83 figures!\n");
	
	// Retrassar el proces per perdre la CPU (simula un canvi de context)
	GARLIC_delay(0);
}

/* **** GROC **** */
void proc1_groc() {
    // Def rajola rombo en l'ID 130
    GARLIC_printf("%1PROC 1: Def ROMBO RAT (ID 128)\n");
    GARLIC_setChar(128, rombo_ratlles);

    // Most rajola rombo
    GARLIC_printf("%1PROC 1: Most ROMBO RAT: \x80\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	// Def rajola smile en l'ID 132
    GARLIC_printf("%1PROC 1: Def SMILE ROD (ID 129)\n");
    GARLIC_setChar(129, smile_rodo);

    // Most rajola smile
    GARLIC_printf("%1PROC 1: Most SMILE ROD: \x81\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	GARLIC_printf("%1PROC 1: Barreja \x80 de \x81 figures!\n");
	
	// Retrassar el proces per perdre la CPU (simula un canvi de context)
	GARLIC_delay(0);
}

void proc2_groc() {
    // Def rajola avion en l'ID 130
    GARLIC_printf("%1PROC 2: Def AVION RAT (ID 130)\n");
    GARLIC_setChar(130, avion_ratlles);

    // Most rajola avion
    GARLIC_printf("%1PROC 2: Most AVION RAT: \x82\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	// Def rajola peix en l'ID 132
    GARLIC_printf("%1PROC 2: Def PEIX RAT (ID 131)\n");
    GARLIC_setChar(131, peix_ratlles);

    // Most rajola peix
    GARLIC_printf("%1PROC 2: Most PEIX RAT: \x83\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	GARLIC_printf("%1PROC 2: Barreja \x82 de \x83 figures!\n");
	
	// Retrassar el proces per perdre la CPU (simula un canvi de context)
	GARLIC_delay(0);
}

/* **** VERD **** */
void proc1_verd() {
    // Def rajola rombo en l'ID 130
    GARLIC_printf("%2PROC 1: Def ROMBO (ID 132)\n");
    GARLIC_setChar(132, rombo);

    // Most rajola rombo
    GARLIC_printf("%2PROC 1: Most ROMBO: \x84\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	// Def rajola smile en l'ID 132
    GARLIC_printf("%2PROC 1: Def SMILE (ID 133)\n");
    GARLIC_setChar(133, smile);

    // Most rajola smile
    GARLIC_printf("%2PROC 1: Most SMILE: \x85\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	GARLIC_printf("%2PROC 1: Barreja \x84 de \x85 figures!\n");
	
	// Retrassar el proces per perdre la CPU (simula un canvi de context)
	GARLIC_delay(0);
}

void proc2_verd() {
    // Def rajola avion en l'ID 130
    GARLIC_printf("%2PROC 2: Def AVION (ID 134)\n");
    GARLIC_setChar(134, avion);

    // Most rajola avion
    GARLIC_printf("%2PROC 2: Most AVION: \x86\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	// Def rajola peix en l'ID 132
    GARLIC_printf("%2PROC 2: Def PEIX (ID 135)\n");
    GARLIC_setChar(135, peix);

    // Most rajola peix
    GARLIC_printf("%2PROC 2: Most PEIX: \x87\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	GARLIC_printf("%2PROC 2: Barreja \x86 de \x87 figures!\n");
	
	// Retrassar el proces per perdre la CPU (simula un canvi de context)
	GARLIC_delay(0);
}

/* **** ROIG **** */
void proc1_roig() {
    // Def rajola rombo en l'ID 130
    GARLIC_printf("%3PROC 1: Def ROMBO RAT (ID 132)\n");
    GARLIC_setChar(132, rombo_ratlles);

    // Most rajola rombo
    GARLIC_printf("%3PROC 1: Most ROMBO RAT: \x84\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	// Def rajola smile en l'ID 132
    GARLIC_printf("%3PROC 1: Def SMILE ROD (ID 133)\n");
    GARLIC_setChar(133, smile_rodo);

    // Most rajola smile
    GARLIC_printf("%3PROC 1: Most SMILE ROD: \x85\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	GARLIC_printf("%3PROC 1: Barreja \x84 de \x85 figures!\n");
	
	// Retrassar el proces per perdre la CPU (simula un canvi de context)
	GARLIC_delay(0);
}

void proc2_roig() {
    // Def rajola avion en l'ID 130
    GARLIC_printf("%3PROC 2: Def AVION RAT (ID 134)\n");
    GARLIC_setChar(134, avion_ratlles);

    // Most rajola avion
    GARLIC_printf("%3PROC 2: Most AVION RAT: \x86\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	// Def rajola peix en l'ID 132
    GARLIC_printf("%3PROC 2: Def PEIX RAT (ID 135)\n");
    GARLIC_setChar(135, peix_ratlles);

    // Most rajola peix
    GARLIC_printf("%3PROC 2: Most PEIX RAT: \x87\n");

    // Retrassar el proces per perdre la CPU (simula un canvi de context)
    GARLIC_delay(0);
	
	GARLIC_printf("%3PROC 2: Barreja \x86 de \x87 figures!\n");
	
	// Retrassar el proces per perdre la CPU (simula un canvi de context)
	GARLIC_delay(0);
}


// Joc de proves CUST
// arg = color (0..3)
int _start(int arg) {

	GARLIC_printf("* Inici TEST - CUST\n");
	GARLIC_printf("ARG: %d\n", arg);

	// Per cada arg, Most el text d'un color especific
	switch (arg)
	{
		case 0:
			proc1_blanc();	// Cridar proces 1
			proc2_blanc();	// Cridar proces 2
			break;
		case 1:
			proc1_groc();	// Cridar proces 1
			proc2_groc();	// Cridar proces 2
			break;
		case 2:
			proc1_verd();	// Cridar proces 1
			proc2_verd();	// Cridar proces 2
			break;
		case 3:
			proc1_roig();	// Cridar proces 1
			proc2_roig();	// Cridar proces 2
			break;
	}
	
	GARLIC_printf("* Fi TEST - CUST\n");

    return 0;
}