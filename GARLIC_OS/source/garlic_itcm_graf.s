@;==============================================================================
@;
@;	"garlic_itcm_graf.s":	código de rutinas de soporte a la gestión de
@;							ventanas gráficas (versión 1.0)
@;
@;==============================================================================

NVENT	= 4					@; número de ventanas totales
PPART	= 2					@; número de ventanas horizontales
							@; (particiones de pantalla)
L2_PPART = 1				@; log base 2 de PPART

VCOLS	= 32				@; columnas y filas de cualquier ventana
VFILS	= 24
PCOLS	= VCOLS * PPART		@; número de columnas totales (en pantalla)
PFILS	= VFILS * PPART		@; número de filas totales (en pantalla)

WBUFS_LEN = 36				@; longitud de cada buffer de ventana (32+4)

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
    
	@; TODO: ACABAR DE CALCULAR DIRECCIO MEMORIA SOBRE EL QUE S'ESCRIUEN ELS CHARS A LA FINESTRA
	
	@; 1. Calcular dir. base _gd_wbfs[ventana]
	ldr r4, =_gd_wbfs		@; Carregar dir. base del vector _gd_wbfs[]
	mov r5, #WBUFS_LEN		@; WBUFS_LEN = mida buffer de cada ventana
	mul r3, r0, r5			@; Desplacament de la ventana a aplicar sobre el buffer
	add r3, r4, r3			@; Dir. ini _gd_wbfs[ventana]
	add r3, r3, #4			@; Dir. ini _gd_wbfs[ventana].pChars (caracters a escriure)
	
	@; 2. Calcular ptr bitmap per escriure els caracters del buffer
	bl _calcPtrVentana		@; R0 = Punter a mapPtr_2[fila + columna]
	mov r5, #PCOLS          @; Nombre de columnes totals
	@;mov r5, r5, lsl #1		@; R5 = PCOLS * 2 -> Cada baldosa ocupa 2 btyes 
	mla r6, r1, r5, r0 		@; R6 = Dir. base + despl. fila (2 * PCOLS * fila)
	
	@; 3. Escriure caracters al fons 2
	mov r4, #0	@; R4 = Index caracters buffer
	mov r5, #0	@; R5 = Index baldosa bitmap
	@;mov r2, r2, lsl #1	@; Cada baldosa = 2 bytes, per tant, convertir chars buffer a bytes
	
.LreadCharPndt:
	cmp r4, r2		@; Mentre R4 < R2 (charPndt), escriure a la pantalla
	beq .Lfi
	
	ldrb r7, [r3, r4]	@; R7 = Carregar valor pChars[i++]
	strh r7, [r6, r5]	@; R7 = Guardar valor pChars[i] sobre el bitmap
	
	add r4, #1	@; R4 = i++ sobre buffer pChars
	add r5, #2	@; R5 = baldosa++ (2 bytes)
	b .LreadCharPndt
	
.Lfi:
	pop {r0-r7, pc}


	.global _gg_desplazar
	@; Rutina para desplazar una posición hacia arriba todas las filas de la
	@; ventana (v) y borrar el contenido de la última fila;
	@;Parámetros:
	@;	R0: ventana a desplazar (int v)
_gg_desplazar:
	push {lr}
	
	pop {pc}


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
	add r0, r0, r1		@; Punter a mapPtr_2[fila + columna]
	
	pop {r1-r4, pc}


.end

