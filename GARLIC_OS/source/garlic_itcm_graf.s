@;==============================================================================
@;
@;	"garlic_itcm_graf.s":	c�digo de rutinas de soporte a la gesti�n de
@;							ventanas gr�ficas (versi�n 1.0)
@;
@;==============================================================================

NVENT	= 4					@; n�mero de ventanas totales
PPART	= 2					@; n�mero de ventanas horizontales
							@; (particiones de pantalla)
L2_PPART = 1				@; log base 2 de PPART

VCOLS	= 32				@; columnas y filas de cualquier ventana
VFILS	= 24
PCOLS	= VCOLS * PPART		@; n�mero de columnas totales (en pantalla)
PFILS	= VFILS * PPART		@; n�mero de filas totales (en pantalla)

WBUFS_LEN = 36				@; longitud de cada buffer de ventana (32+4)

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
	push {lr}


	pop {pc}


	.global _gg_desplazar
	@; Rutina para desplazar una posici�n hacia arriba todas las filas de la
	@; ventana (v) y borrar el contenido de la �ltima fila;
	@;Par�metros:
	@;	R0: ventana a desplazar (int v)
_gg_desplazar:
	push {lr}


	pop {pc}


.end

