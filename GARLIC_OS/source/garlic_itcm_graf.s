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
	push {lr}
    
	@; 1. Calcular dir. base _gd_wbfs[ventana]
	ldr r4, =_gd_wbfs		@; Carregar dir. base del vector _gd_wbfs[]
	mov r5, #WBUFS_LEN		@; WBUFS_LEN = mida buffer de cada ventana
	mul r3, r0, r5			@; Calcular despl. de la finestra sobre el buffer
	add r5, r4, r3			@; Dir. memoria _gd_wbfs[ventana]
	add r6, r5, #4			@; Dir. incial _gd_wbfs[ventana].pChars (caracters a escriure)
	
	@; 2. Calcular dir. ini. finestra sobre el bitmap
	bl _calcPtrVentana	@; R0 = Dir. ini. finestra sobre el bitmap
	
	@; 3. Escriure caracters al fons 2
	@; TODO
	
	pop {pc}


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
	mov r3, r1
	mul r1, r3, r2				@; R1 = VFILS * (v / PPART) -> fila inicial de la ventana (0 o 24)
	
	@; 2. Calcular columna inicial (v % PPART)
	mov r2, #L2_PPART
	and r2, r0, r2		@; R2 = v % PPART
	mov r3, #VCOLS
	mov r4, r2
	mul r2, r4, r3		@; R2 = VCOLS * (v % PPART) -> columna inicial de la ventana (0 o 32)
	
	@; 3. Calcular posicio al bitmap (fons 2)
	mov r3, #PCOLS
	mov r4, r1
	mul r1, r4, r3	@; R1 = Fila inicial * PCOLS (accedir a la fila X del bitmap)
	add r1, r1, r2	@; R1 = Posicio especifica de la finestra al bitmap (fila + columna)
	
	@; 4. Obtindre direccio del bitmap (fons 2)
	ldr r2, =mapPtr_2
	ldrh r2, [r2]	@; Carregar dir. ini. fons 2 (mapPtr_2)
	mov r3, #2		@; Cada baldosa ocupa 2 bytes
	mov r4, r1
	mul r1, r4, r3	@; Fila inicial baldoses
	add r0, r2, r1	@; Dir. ini. finestra sobre el bitmap

	pop {r1-r4, pc}


.end

