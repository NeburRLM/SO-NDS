@;==============================================================================
@;
@;	"garlic_itcm_graf.s":	c�digo de rutinas de soporte a la gesti�n de
@;							ventanas gr�ficas (versi�n 1.0)
@;
@;==============================================================================

NVENT	= 16					@; n�mero de ventanas totales
PPART	= 4					@; n�mero de ventanas horizontales
							@; (particiones de pantalla)
L2_PPART = 2				@; log base 2 de PPART

VCOLS	= 32				@; columnas y filas de cualquier ventana
VFILS	= 24
PCOLS	= VCOLS * PPART		@; n�mero de columnas totales (en pantalla)
PFILS	= VFILS * PPART		@; n�mero de filas totales (en pantalla)

WBUFS_LEN = 68				@; longitud de cada buffer de ventana (64+4)

.section .itcm,"ax",%progbits

	.arm
	.align 2


	.global _gg_escribirLinea
	@; Rutina para escribir toda una l�nea de caracteres almacenada en el
	@; buffer de la ventana especificada;
	@;Par�metros:
	@;	R0: ventana a actualizar (int v)
	@;	R1: fila actual (int f)
	@;	R2: n�mero de caracteres a escribir (int n)
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
	@; Rutina para desplazar una posici�n hacia arriba todas las filas de la
	@; ventana (v) y borrar el contenido de la �ltima fila;
	@;Par�metros:
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
	@; Par�metros:
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



	.global _gg_escribirLineaTabla
	@; escribe los campos b�sicos de una linea de la tabla correspondiente al
	@; z�calo indicado por par�metro con el color especificado; los campos
	@; son: n�mero de z�calo, PID, keyName y direcci�n inicial
	@;Par�metros:
	@;	R0 (z)		->	n�mero de z�calo
	@;	R1 (color)	->	n�mero de color (0..3)
_gg_escribirLineaTabla:
	push {lr}


	pop {pc}



	.global _gg_escribirCar
	@; escribe un car�cter (baldosa) en la posici�n de la ventana indicada,
	@; con un color concreto;
	@;Par�metros:
	@;	R0 (vx)		->	coordenada x de ventana (0..31)
	@;	R1 (vy)		->	coordenada y de ventana (0..23)
	@;	R2 (car)	->	c�digo del car�cter, como n�mero de baldosa (0..127)
	@;	R3 (color)	->	n�mero de color del texto (0..3)
	@; pila (vent)	->	n�mero de ventana (0..15)
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
		mov r4, #128
		mla r1, r3, r4, r2		@; R1 = color * 128 + car
	@; 4. Guardar caracter
		strh r1, [r0]
		
.LfiEscrCar:
	pop {r0-r6, pc}



	.global _gg_escribirMat
	@; escribe una matriz de 8x8 car�cteres a partir de una posici�n de la
	@; ventana indicada, con un color concreto;
	@;Par�metros:
	@;	R0 (vx)		->	coordenada x inicial de ventana (0..31)
	@;	R1 (vy)		->	coordenada y inicial de ventana (0..23)
	@;	R2 (m)		->	puntero a matriz 8x8 de c�digos ASCII (direcci�n)
	@;	R3 (color)	->	n�mero de color del texto (0..3)
	@; pila	(vent)	->	n�mero de ventana (0..15)
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
		mov r7, #128	@; R7 = Despl. color
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
	@; Rutina de Servicio de Interrupci�n (RSI) para actualizar la representa-
	@; ci�n del PC actual.
_gg_rsiTIMER2:
	push {lr}


	pop {pc}


.end

