@;==============================================================================
@;
@;	"garlic_dtcm.s":	zona de datos b�sicos del sistema GARLIC 2.0
@;						(ver "garlic_system.h" para descripci�n de variables)
@;
@;==============================================================================

.section .dtcm,"wa",%progbits

	.align 2

	.global _gd_pidz			@; Identificador de proceso + z�calo actual
_gd_pidz:	.word 0

	.global _gd_pidCount		@; Contador global de PIDs
_gd_pidCount:	.word 0

	.global _gd_tickCount		@; Contador global de tics
_gd_tickCount:	.word 0

	.global _gd_sincMain		@; Sincronismos con programa principal
_gd_sincMain:	.word 0

	.global _gd_seed			@; Semilla para generaci�n de n�meros aleatorios
_gd_seed:	.word 0xFFFFFFFF

	.global _gd_nReady			@; N�mero de procesos en la cola de READY
_gd_nReady:	.word 0

	.global _gd_qReady			@; Cola de READY (procesos preparados)
_gd_qReady:	.space 16

	.global _gd_nDelay			@; N�mero de procesos en la cola de DELAY
_gd_nDelay:	.word 0

	.global _gd_qDelay			@; Cola de DELAY (procesos retardados)
_gd_qDelay:	.space 16 * 4

	.global _gd_pcbs			@; Vector de PCBs de los procesos activos
_gd_pcbs:	.space 16 * 6 * 4

	.global _gd_wbfs			@; Vector de WBUFs de las ventanas disponibles
_gd_wbfs:	.space 16 * (4 + 64)

	.global _gd_stacks			@; Vector de pilas de los procesos de usuario
_gd_stacks:	.space 15 * 128 * 4

	.global _gd_sem				@; Vector de semaforos globales tipo mutex
_gd_sem: .space 8

	.global _gd_bufferZ			@; PROG_G - Buffer per poder convertir el zocalo Z de num a STR
_gd_bufferZ: .space 3			@; Objectiu: Poder escriure Z en la pantalla inferior de la NDS (_gg_escribirLineaTabla)

	.global _gd_bufferPID			@; PROG_G - Buffer per poder convertir el PID de num a STR
_gd_bufferPID: .space 3			@; Objectiu: Poder escriure PID en la pantalla inferior de la NDS (_gg_escribirLineaTabla)

	.global _gd_bufferPC			@; PROG_G - Buffer per poder convertir el PC de num a STR
_gd_bufferPC: .space 9			@; Objectiu: Poder escriure PC en la pantalla inferior de la NDS (_gg_escribirLineaTabla)

	.global _gd_perc				@; Variable para guardar el procentage en string
_gd_perc: .word 0

	.global _gd_quo					@; Variable para guardar el cuociente de la division
_gd_quo: .word 0

	.global _gd_mod					@; Variable para guardar el modulo de la division
_gd_mod: .word 0

	.global _gd_bloqSem
_gd_bloqSem: .space 8				@; Variable para guardar el zocalo bloqueado por el semaforo

.end
