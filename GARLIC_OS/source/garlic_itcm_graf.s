@;==============================================================================
@;
@;	"garlic_itcm_graf.s":	código de rutinas de soporte a la gestión de
@;							ventanas gráficas (versión 1.0)
@;
@;==============================================================================

NVENT	= 16					@; número de ventanas totales
PPART	= 4					@; número de ventanas horizontales
							@; (particiones de pantalla)
L2_PPART = 2				@; log base 2 de PPART

VCOLS	= 32				@; columnas y filas de cualquier ventana
VFILS	= 24
PCOLS	= VCOLS * PPART		@; número de columnas totales (en pantalla)
PFILS	= VFILS * PPART		@; número de filas totales (en pantalla)

WBUFS_LEN = 68				@; longitud de cada buffer de ventana (64+4)

.section .itcm,"ax",%progbits

	.arm
	.align 2


	.global _gg_escribirLinea
	@; Rutina para escribir toda una línea de caracteres almacenada en el
	@; buffer de la ventana especificada;
	@;Parámetros:
	@;	R0: ventana a actualizar (int v)
	@;	R1: fila actual (int f)
	@;	R2: número de caracteres a escribir (int n)
_gg_escribirLinea:
	push {r0-r7, lr}
	
	@; 1. Calcular dir. base _gd_wbfs[ventana]
	ldr r4, =_gd_wbfs		@; Carregar dir. base del vector _gd_wbfs[]
	mov r5, #WBUFS_LEN		@; WBUFS_LEN = mida buffer de cada ventana
	mul r3, r0, r5			@; Desplacament de la ventana a aplicar sobre el buffer
	add r3, r4, r3			@; Dir. ini _gd_wbfs[ventana]
	add r3, r3, #4			@; Dir. ini _gd_wbfs[ventana].pChars (caracters a escriure)
	
	@; 2. Calcular ptr bitmap per escriure els caracters del buffer
	bl _calcPtrVentana		@; R0 = Punter a mapPtr_2[fila + columna] -> Accedir a la finestra v sobre el bitmap
	mov r5, #PCOLS
	mov r5, r5, lsl #1		@; R5 = PCOLS x 2 a causa de les baldoses (2 bytes)
	mla r6, r1, r5, r0 		@; R6 = Dir. base + despl. fila (2 * PCOLS * fila)
	
	@; 3. Escriure caracters al fons 2
	mov r2, r2, lsl #1	@; charPndt * 2 ja que a la FASE 2 cada baldosa ocupa 2 bytes (char a short) 
	mov r4, #0	@; R4 = Index caracters buffer
	mov r5, #0	@; R5 = Index baldosa bitmap

.LreadCharPndt:
	cmp r4, r2		@; Mentre index (R4 -> [0,32]) != charPndt (R2 -> [0,32]) , escriure per pantalla 
	bge .LfiEscribir
	
	ldrh r7, [r3, r4]	@; R7 = Carregar valor pChars[i]
	strh r7, [r6, r5]	@; R7 = Guardar valor pChars[i] sobre el bitmap
	
	add r4, #2	@; R4 = i++ sobre buffer pChars
	add r5, #2	@; R5 = baldosa++ (2 bytes)
	b .LreadCharPndt
	
.LfiEscribir:
	pop {r0-r7, pc}


	.global _gg_desplazar
	@; Rutina para desplazar una posición hacia arriba todas las filas de la
	@; ventana (v) y borrar el contenido de la última fila;
	@;Parámetros:
	@;	R0: ventana a desplazar (int v)
_gg_desplazar:
	push {r0-r8, lr}
	
	@; 1. Accedir al bitmap
	mov r1, #1	@; Accedir a la fila 1
	bl _calcPtrVentana		@; R0 = Punter a mapPtr_2[fila + columna] -> Accedir a la finestra v sobre el bitmap
	
	@; 2. Desplacar cap amunt files
	mov r3, #PCOLS          @; Nombre de columnes totals
	mov r3, r3, lsl #1		@; R3 = PCOLS x 2 -> Cada baldosa ocupa 2 bytes
	mov r7, #VCOLS	
	mov r7, r7, lsl #1		@; R7 = VCOLS x 2 -> Cada baldosa ocupa 2 bytes
.LdesplFiles:
	cmp r1, #VFILS-1	@; Fila actual es la ultima -> Borrar ultima fila
	beq .LborrarFila
	
	@; Calcular dir mem baldosa actual i baldosa actual fila superior
	mla r4, r1, r3, r0 		@; R4 = Dir. fila actual (fila * PCOLS)
	sub r6, r1, #1			@; R6 = Fila superior (fila actual - 1)
	mla r5, r6, r3, r0 		@; R5 = Dir. fila superior (fila superior * PCOLS)
	
	@; Desplacar baldoses fila actual a fila superior
	mov r2, #0			@; R2 = Index baldosa actual
	.LdesplBaldoses:
		cmp r2, r7	@; Mentre no  arribem a la ultima baldosa de la fila
		beq .LdesplSegFila	@; fila++
		
		@; Carregar valor baldosa actual i guardarla a la fila superior
		ldrh r8, [r4, r2]	@; Accedir valor baldosa fila actual
		strh r8, [r5, r2]	@; Guardar valor baldosa en fila superior
		add r2, #2			@; baldosa++
		b .LdesplBaldoses
		
		@; Continuar a la seguent fila si fila actual ja processada
	.LdesplSegFila:
		add r1, #1	@; fila++
		b .LdesplFiles	@; Repetir procediment seguent fila
	
	@; Cas ultima fila, borrar valor baldoses
.LborrarFila:
	mov r2, #0		@; R2 = Index baldosa actual
    mov r9, #0		@; R9 = Valor espai buit ASCII (32)
	
	@; Calcular dir mem baldosa actual i baldosa actual fila superior
	mla r4, r1, r3, r0 		@; R4 = Dir. fila actual (fila * PCOLS)
	sub r6, r1, #1			@; R6 = Fila superior (fila actual - 1)
	mla r5, r6, r3, r0 		@; R5 = Dir. fila superior (fila superior * PCOLS)
	
	@; Borrar baldoses ultima fila
	.LborrarBaldoses:
		cmp r2, r7	@; Mentre no arribem a la ultima baldosa de la fila
		beq .Lfidespl	@; Final programa
		
		@; Guardar valor baldosa a fila superior
		ldrh r8, [r4, r2]	@; Accedir valor baldosa fila actual
		strh r8, [r5, r2]	@; Guardar valor baldosa en fila superior
		
		@; Borrar valor baldosa fila actual
		strh r9, [r4, r2]	@; Guardar espai buit en la baldosa actual
		add r2, #2			@; baldosa++
		b .LborrarBaldoses
		
.Lfidespl:
	pop {r0-r8, pc}


	.global _calcPtrVentana
	@; Rutina auxiliar para calcular la direccio de memoria inicial de la ventana sobre el bitmap (fons 2)
	@; Parámetros:
	@;	R0: ventana (int v)
_calcPtrVentana:
	push {r1-r4, lr}
	
	@; 1. Calcular fila inicial (v / PPART)
	mov r1, r0, lsr #L2_PPART	@; R1 = v / PPART -> (0 o 1 (y = 24))
	mov r3, #VFILS
	mul r1, r3, r1				@; Calcular fila (0 o 24)
	
	@; 2. Calcular columna inicial (v % PPART)
	mov r4, #L2_PPART+1			@; 0b11
	and r2, r0, r4				@; R2 = v % PPART (0 o 1 (x = 32))
	mov r4, #VCOLS              @; Nombre de columnes totals
    mul r2, r4, r2              @; Calcular posicio especifica columna (0 o 32)	
    
    @; 3. Calcular desplacament bitmap
    mov r4, #PCOLS              @; Nombre de columnes totals
    mla r1, r4, r1, r2          @; Accedir a la posicio en el mapa especifica de la finestra -> R1 = Fila * PCOLS + columna
    mov r1, r1, lsl #1          @; Cada baldosa ocupa 2 bytes -> R1 = Pos ini ventana tenint en compte les baldoses
    
	@; 4. Obtindre ptr al bitmap (fons 2)
	ldr r2, =mapPtr_2	@; Carregar dir. ini. fons 2 (mapPtr_2)
	ldr r0, [r2]        @; VRAM_A_MAIN_BG_0x06000000
	add r0, r0, r1		@; R0 = Punter a mapPtr_2[fila + columna] -> Accedir a la finestra v sobre el bitmap
	
	pop {r1-r4, pc}

.global _escriureSeparador
	@; Funcio auxiliar per escriure els separadors de la taula de la funcio _gg_escribirLineaTabla
	@; S'usa per optimitzar la longitud del codi, ja que es extensa
	@;Parametros:
	@;	R0 (BASE_LINEA) = (64 x z) + BASE_TAULA (R2)
	@;  R1 (SEPARADOR (en color)) = '|' + 128 * color
_escriureSeparador:
	push {r2, lr}
	
	mov r2, #0			@; R2 = Index despl.
	strh r1, [r0, r2]	@; Escriure separador incial '|'

    add r2, #6			@; Seguent pos separador
    strh r1, [r0, r2]	@; Escriure separador 'Z|' 

    add r2, #10
	strh r1, [r0, r2]		@; Escriure separador'PID|'
    
	add r2, #10
	strh r1, [r0, r2]		@; Escriure separador 'Prog|'
	
	add r2, #18
	strh r1, [r0, r2]		@; Escriure separador 'PCactual|'
    
	add r2, #6
	strh r1, [r0, r2]		@; Escriure separador 'Pi|'

	add r2, #4
	strh r1, [r0, r2]		@; Escriure separador 'E|'
	
	add r2, #8
	strh r1, [r0, r2]		@; Escriure separador 'Uso|'
	
	pop {r2, pc}


	.global _gg_escribirLineaTabla
	@; escribe los campos básicos de una linea de la tabla correspondiente al
	@; zócalo indicado por parámetro con el color especificado; los campos
	@; son: número de zócalo, PID, keyName y dirección inicial
	@;Parámetros:
	@;	R0 (z)		->	número de zócalo
	@;	R1 (color)	->	número de color (0..3)
_gg_escribirLineaTabla:
	push {r0-r12, lr}

@; 0. Guardar dades
	mov r12, r0	@; R12 = z
	mov r11, r1	@; R11 = color
	
@; 1. Calcular posicio base de la linea en la taula
	mov r2, #0x06200000		@; R2 (BASE_MAPA) = base del mapa de caracters de la pantalla inferior
    mov r5, #64				@; R5 = VCOLS * 2
	@; Despl. inicial (4 files mes avall)
	mov r6, #4
	mla r6, r5, r6, r2		@; R6 = BASE_MAPA + (VCOLS * 2 * 4)
	@; Despl. segons Z
	mla r0, r5, r0, r6		@; R0 = BASE_MAPA_ACTUAL + (VCOLS * 2 * Z)
	mov r10, r0				@; R10 = Copia BASE_MAPA_ACTUAL + (VCOLS * 2 * Z)
	
@; 2. Generar color para els separadors (codi 104)
    mov r5, #128			@; Multiplicador per al color
    mov r6, #104			@; Codi separador '|'
    mla r1, r5, r1, r6		@; R1 = '|' + 128 * color
	
@; 3. Escriure els separadors en la posicio corresponent de la taula
	bl _escriureSeparador
	
@; 4. Calcular PCB[z]
	ldr r9, =_gd_pcbs		@; Dir base vector de PCBs
	mov r8, #24				@; Accedir a dir. mem. PCB[0] (ocupa 24 B cada entrada)
	mla r9, r8, r12, r9		@; R9 = Dir. mem. PCB[z]

@; 5. Obtenir i escriure en la pantalla inferior NDS els camps z, PCB[z].PID i PCB[z].keyName
	
	@; Camp Zocalo (Z)
	ldr r0, =_gd_bufferZ	@; R0 = Punter buffer Z per poder convertir el valor numeric a str (_gs_num2str_dec)
	mov r1, #3				@; R1 = Longitud buffer
	mov r2, r12				@; R2 = Z
	bl _gs_num2str_dec		@; Convertir valor numeric a str per poder escriure'l a la pantalla inferior de la NDS
	
	cmp r0, #0				@; R0 != 0 si error
	bne .LerrorEscripturaBuffer
	
	ldr r0, =_gd_bufferZ		@; R0 = Recarregar buffer, ara ja amb el valor convertit a string
	mov r1, r12					@; R1 = Fila
	add r1, #4					@; R1 = Fila + 4 (despl. a la primera fila de la taula)
	mov r2, #1					@; R2 = Columna
	mov r3, r11					@; R3 = Color
	bl _gs_escribirStringSub	@; Escriure string a pantalla inferior NDS
	
	@; Camp PID
	ldr r0, =_gd_bufferPID	@; R0 = Punter buffer PID per poder convertir el valor numeric a str (_gs_num2str_dec)
	mov r1, #3				@; R1 = Longitud buffer
	ldr r2, [r9]			@; R2 = PCB[z].PID
	cmp r2, #0				@; Solament escriure si PID != 0
	beq .LerrorEscripturaBuffer
	
	bl _gs_num2str_dec		@; Convertir valor numeric a str per poder escriure'l a la pantalla inferior de la NDS
	
	cmp r0, #0				@; R0 != 0 si error
	bne .LerrorEscripturaBuffer
	
	ldr r0, =_gd_bufferPID		@; R0 = Recarregar buffer, ara ja amb el valor convertit a string
	mov r1, r12					@; R1 = Fila
	add r1, #4					@; R1 = Fila + 4 (despl. a la primera fila de la taula)
	mov r2, #6					@; R2 = Columna
	mov r3, r11					@; R3 = Color
	bl _gs_escribirStringSub	@; Escriure string a pantalla inferior NDS
	
	@; Camp keyName
	mov r0, r9
	add r0, #16					@; R0 = Dir. mem. PCB[z].keyName
	mov r1, r12					@; R1 = Fila
	add r1, #4					@; R1 = Fila + 4 (despl. a la primera fila de la taula)
	mov r2, #9					@; R2 = Columna
	mov r3, r11					@; R3 = Color
	bl _gs_escribirStringSub	@; Escriure string a pantalla inferior NDS
	
	@; Reescriure 'Prog|', ja que keyName sobreescriu aquest valor mentre el proces corre
	mov r5, #128			@; Multiplicador per al color
    mov r6, #104			@; Codi separador '|'
    mla r11, r5, r11, r6	@; R1 = '|' + 128 * color
	add r10, #26		@; Despl. columna fins 'Prog|'
	strh r11, [r10]		@; Escriure separador 'Prog|'
	
.LerrorEscripturaBuffer:
	pop {r0-r12, pc}



	.global _gg_escribirCar
	@; escribe un carácter (baldosa) en la posición de la ventana indicada,
	@; con un color concreto;
	@;Parámetros:
	@;	R0 (vx)		->	coordenada x de ventana (0..31)
	@;	R1 (vy)		->	coordenada y de ventana (0..23)
	@;	R2 (car)	->	código del carácter, como número de baldosa (0..127)
	@;	R3 (color)	->	número de color del texto (0..3)
	@; pila (vent)	->	número de ventana (0..15)
_gg_escribirCar:
	push {r0-r6, lr}
@; 1. Comprovar valors correctes
	@; R0 = vx
	cmp r0, #0
	blo .LfiEscrCar
	cmp r0, #31
	bhi .LfiEscrCar
	@; R1 = vy - No es comprova pq sempre te valor 4097 (no 0 a 23)
	@; R2 = car
	cmp r2, #0
	blo .LfiEscrCar
	cmp r2, #127
	bhi .LfiEscrCar
	@; R3 = color
	cmp r3, #0
	blo .LfiEscrCar
	cmp r3, #3
	bhi .LfiEscrCar
	@; R4 = vent
	@; Cada registre en la pila ocupa 4 bytes
	@; En total utilitzem 7 registres de dades (R0-R6) = 7*4 = 28 bytes
	@; Tambe s'ha de contar el registre LR (4B)
	@; Total despl. per poder llegir el valor correcte de vent en la pila = 28 + 4 = 32 bytes
	ldr r4, [sp, #32]
	cmp r4, #0
	blo .LfiEscrCar
	cmp r4, #15
	bhi .LfiEscrCar
		
@; 2. Calcular posicio
	mov r5, r0				@; R5 = vx
	mov r0, r4				@; _calcPtrVentana necessita R0 = ventana
	bl _calcPtrVentana 		@; R0 = Punter a mapPtr_2[fila + columna] -> Accedir a la finestra v sobre el bitmap
	mov r4, #PCOLS
	mov r4, r4, lsl #1		@; R4 = PCOLS x 2 a causa de les baldoses (2 bytes)
	mov r5, r5, lsl #1		@; R5 = vx x 2 a causa de les baldoses (2 bytes)
	mla r6, r1, r4, r5 		@; R6 = vy * PCOLS + vx
	add r0, r6				@; R0 = Posicio (vx,vy)
		
@; 3. Calcular color
	mov r4, #128			@; Multiplicador per al color
	mla r1, r3, r4, r2		@; R1 = color * 128 + car
@; 4. Guardar caracter
	strh r1, [r0]
.LfiEscrCar:
	pop {r0-r6, pc}



	.global _gg_escribirMat
	@; escribe una matriz de 8x8 carácteres a partir de una posición de la
	@; ventana indicada, con un color concreto;
	@;Parámetros:
	@;	R0 (vx)		->	coordenada x inicial de ventana (0..31)
	@;	R1 (vy)		->	coordenada y inicial de ventana (0..23)
	@;	R2 (m)		->	puntero a matriz 8x8 de códigos ASCII (dirección)
	@;	R3 (color)	->	número de color del texto (0..3)
	@; pila	(vent)	->	número de ventana (0..15)
_gg_escribirMat:
	push {r0-r9, lr}
@; 1. Comprovar valors correctes
	@; R0 = vx
	cmp r0, #0
	blo .LFiMat
	cmp r0, #31
	bhi .LFiMat
	@; R1 = vy - No es comprova pq sempre te valor 4097 (no 0 a 23)
	@; R3 = color
	cmp r3, #0
	blo .LFiMat
	cmp r3, #3
	bhi .LFiMat
	@; R4 = vent
	@; Cada registre en la pila ocupa 4 bytes
	@; En total utilitzem 10 registres de dades (R0-R9) = 10*4 = 40 bytes
	@; Tambe s'ha de contar el registre LR (4B)
	@; Total despl. per poder llegir el valor correcte de vent en la pila = 40 + 4 = 44 bytes
	ldr r4, [sp, #44]
	cmp r4, #0
	blo .LFiMat
	cmp r4, #15
	bhi .LFiMat
		
@; 2. Calcular posicio
	mov r5, r0				@; R5 = vx
	mov r0, r4				@; _calcPtrVentana necessita R0 = ventana
	bl _calcPtrVentana 		@; R0 = Punter a mapPtr_2[fila + columna] -> Accedir a la finestra v sobre el bitmap
	mov r4, #PCOLS
	mov r4, r4, lsl #1		@; R4 = PCOLS x 2 a causa de les baldoses (2 bytes)
	mov r5, r5, lsl #1		@; R5 = vx x 2 a causa de les baldoses (2 bytes)
	mla r6, r1, r4, r5 		@; R6 = vy * PCOLS + vx
	add r0, r6				@; R0 = Posicio (vx,vy)
		
@; 3. Recorrer matriu 8x8
	mov r1, #0	@; R1 = Index despl. mat[i][j]
	mov r8, r4	@; R8 = PCOLS * 2 (baldoses 2B)
	mov r4, #0	@; R4 = Control vy
	mov r5, #0	@; R5 = Control vx
	mov r7, #128	@; R7 = Multiplicador per al color
.LY:
	cmp r4, #8	@; vy < 8
	bhs .LFiMat
.LX:
	cmp r5, #16	@; vx < 16 (pq baldoses ocupen 2B)
	addhs r4, #1	@; vy++
	movhs r5, #0	@; Reiniciar valor vx
	addhs r0, r8	@; Despl. fila inferior mapa
	bhs .LY

	ldrb r6, [r2, r1]	@; Carregar car mat[i][j]
	cmp r6, #32			@; car < 32 son caracters no imprimibles
	blo .LIter
	sub r6, #32			@; Aconseguir valor correcte car
	
	mla r9, r3, r7, r6 	@; R9 = color * 128 + car
	strh r9, [r0, r5]	@; Guardar car mat[i][j]
.LIter:
	add r1, #1	@; mat++
	add r5, #2	@; i++
	b .LX
	
.LFiMat:		
	pop {r0-r9, pc}



	.global _gg_rsiTIMER2
	@; Rutina de Servicio de Interrupción (RSI) para actualizar la representación del PC actual.
_gg_rsiTIMER2:
	push {r0-r7, lr}
	
	ldr r0, =_gd_pcbs		@; R0 = Dir base vector de PCBs
    mov r1, #0				@; R1 = Index de Z (inicial z = 0)
    mov r2, #16				@; R2 = Num total de Z (16)
    mov r3, #24				@; R3 = Mida de cada PCB (24 bytes per proces)

.IterZ:
    cmp r1, r2
    beq .FiRSI				@; Si ja s'han processat tots els Z sortir
	
	mov r7, r1				@; R7 = Copia zocalo Z
    mla r4, r1, r3, r0		@; R4 = Dir. base del PCB del zocalo actual (PCB[z])

    ldr r5, [r4, #0]		@; R5 = PCB[z].PID
    cmp r5, #0
    beq .SeguentZ			@; Si PID es 0 (no actiu), passar al seg. zocalo

    ldr r6, [r4, #4]		@; R6 = PCB[z].PC
    ldr r0, =_gd_bufferPC	@; R0 = Dir. del buffer per convertir el PC a str
    mov r1, #9				@; R1 = Mida bufferPC
    mov r2, r6				@; R2 = PCB[z].PC
    bl _gs_num2str_hex		@; Convertir valor numeric a str per poder escriure'l a la pantalla inferior de la NDS

    mov r0, r7					@; R0 = Index zocalo actual
    add r0, #4					@; R0 = Index fila taula (Z+4)
    mov r1, #20					@; R1 = Columna taula de PCactual
    ldr r2, =_gd_bufferPC		@; R2 = Dir. mem. string PCactual convertit a str
    mov r3, #3					@; R3 = Color
    bl _gs_escribirStringSub	@; Escriure string a pantalla inferior NDS
	
.SeguentZ:
    add r1, #1	@; z++
    b .IterZ
	
.FiRSI:

	pop {r0-r7, pc}


.end

