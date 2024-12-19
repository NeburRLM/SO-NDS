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
	and r2, r0, #L2_PPART		@; R2 = v % PPART (0 o 1 (x = 32))
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
	@; escribe los campos básicos de una linea de la tabla correspondiente al
	@; zócalo indicado por parámetro con el color especificado; los campos
	@; son: número de zócalo, PID, keyName y dirección inicial
	@;Parámetros:
	@;	R0 (z)		->	número de zócalo
	@;	R1 (color)	->	número de color (0..3)
_gg_escribirLineaTabla:
	push {lr}


	pop {pc}



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
	push {lr}
	

	pop {pc}



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
	push {lr}
	

	pop {pc}



	.global _gg_rsiTIMER2
	@; Rutina de Servicio de Interrupción (RSI) para actualizar la representa-
	@; ción del PC actual.
_gg_rsiTIMER2:
	push {lr}


	pop {pc}


.end

