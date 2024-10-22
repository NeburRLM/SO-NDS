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
    
	@; 1. Calcular dir. base _gd_wbfs[ventana]
	ldr r4, =_gd_wbfs		@; Carregar dir. base del vector _gd_wbfs[]
	mov r5, #WBUFS_LEN		@; WBUFS_LEN = mida buffer de cada ventana
	mul r3, r0, r5			@; Calcular despl. de la finestra sobre el buffer
	add r3, r3, r4			@; Dir. memoria _gd_wbfs[ventana]
	add r5, r3, #4			@; Dir. incial _gd_wbfs[ventana].pChars (caracters a escriure)
	
	@; 2. Calcular ptr bitmap per escriure els caracters del buffer
	mov r5, r0				@; Guardar ventana
	bl _calcPtrVentana		@; Calcular posicio inicial al fons per la finestra
	mov r6, #PCOLS
	mov r6, r6, lsl #1		@; Cada baldosa es de 2 btyes 
	mla r7, r6, r1, r0 		@; R7 = Dir. base + despl. fila (2 * PCOLS * fila)
	
	@; 3. Escriure caracters al fons 2
	mov r3, #0	@; Index caracters buffer
	mov r5, #0	@; Index baldosa bitmap
	mov r2, r2, lsl #1	@; Cada baldosa = 2 bytes, per tant, convertir chars buffer a bytes
	
.LreadCharPndt:
	cmp r3, r2		@; Mentre charPndt != index char buffer, escriure a la pantalla
	beq .Lfi
	
	ldrh r6, [r4, r3]	@; Carregar valor pChars[i++]
	strh r6, [r7, r5]	@; Guardar valor pChars[i] al fons 2 (mapPtr_2)
	
	add r3, #2	@; Moure al seguent caracter del buffer (_gd_wbfs[ventana].pChars[i++])
	add r5, #2	@; Moure a la seguent baldosa del fons 2
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
	mov r1, r0, lsr #L2_PPART	@; R1 = v / PPART -> (2^L2_PPART = 2^1 = 2)
	mov r2, #VFILS
	
	@; 2. Calcular columna inicial (v % PPART)
	mov r3, #L2_PPART
	and r3, r0, r3		@; R3 = v % PPART
	
	@; Calcular desplacament al bitmap (fons 2)
	mul r1, r2, r1				@; R1 = VFILS * (v / PPART) -> fila inicial de la ventana (0 o 24)
	
	mov r4, #PPART
	mla r1, r4, r1, r3			@; R1 = Fila + columna -> VFILS * (v / PPART) * PPART + (v % PPART)
	
	mov r4, #VCOLS
	mov r4, r4, lsl #1 			@; R4 = VCOLS * 2 ja que cada baldosa ocupa 2 bytes
	mul r1, r4, r1				@; R1 = Ptr inicial ventana v -> (VCOLS * 2) * (VFILS * (v / PPART) * PPART + (v % PPART))

	@; 4. Obtindre ptr al bitmap (fons 2)
	ldr r2, =mapPtr_2	@; Carregar dir. ini. fons 2 (mapPtr_2)
	ldr r0, [r2]
	add r0, r0, r1		@; Punter a mapPtr_2[fila + columna]
	
	pop {r1-r4, pc}


.end

