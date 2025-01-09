@;==============================================================================
@;
@;	"garlic_itcm_api.s":	c�digo de las rutinas del API de GARLIC 2.0
@;							(ver "GARLIC_API.h" para descripci�n de las
@;							 funciones correspondientes)
@;
@;==============================================================================

.section .itcm,"ax",%progbits

	.arm
	.align 2


	.global _ga_pid
	@;Resultado:
	@; R0 = identificador del proceso actual
_ga_pid:
	push {r1, lr}
	ldr r0, =_gd_pidz
	ldr r1, [r0]			@; R1 = valor actual de PID + z�calo
	mov r0, r1, lsr #0x4	@; R0 = PID del proceso actual
	pop {r1, pc}


	.global _ga_random
	@;Resultado:
	@; R0 = valor aleatorio de 32 bits
_ga_random:
	push {r1-r5, lr}
	ldr r0, =_gd_seed
	ldr r1, [r0]			@; R1 = valor de semilla de n�meros aleatorios
	ldr r2, =0x0019660D
	ldr r3, =0x3C6EF35F
	umull r4, r5, r1, r2	@; R5:R4 = _gd_seed * 0x19660D
	add r4, r3				@; R4 += 0x3C6EF35F
	str r4, [r0]			@; guarda la nueva semilla (R4)
	mov r0, r5				@; devuelve por R0 el valor aleatorio (R5)
	pop {r1-r5, pc}


	.global _ga_divmod
	@;Par�metros
	@; R0: unsigned int num
	@; R1: unsigned int den
	@; R2: unsigned int * quo
	@; R3: unsigned int * mod
	@;Resultado
	@; R0: 0 si no hay problema, !=0 si hay error en la divisi�n
_ga_divmod:
	push {r4-r7, lr}
	cmp r1, #0				@; verificar si se est� intentando dividir por cero
	bne .Ldiv_ini
	mov r0, #1				@; c�digo de error
	b .Ldiv_fin2
.Ldiv_ini:
	mov r4, #0				@; R4 es el cociente (q)
	mov r5, #0				@; R5 es el resto (r)
	mov r6, #31				@; R6 es �ndice del bucle (de 31 a 0)
	mov r7, #0xff000000
.Ldiv_for1:
	tst r0, r7				@; comprobar si hay bits activos en una zona de 8
	bne .Ldiv_for2			@; bits del numerador, para evitar el rastreo bit a bit
	mov r7, r7, lsr #8
	sub r6, #8				@; 8 bits menos a buscar
	cmp r7, #0
	bne .Ldiv_for1
	b .Ldiv_fin1			@; caso especial (numerador = 0 -> q=0 y r=0)
.Ldiv_for2:
	mov r7, r0, lsr r6		@; R7 es variable de trabajo j;
	and r7, #1				@; j = bit i-�simo del numerador; 
	mov r5, r5, lsl #1		@; r = r << 1;
	orr r5, r7				@; r = r | j;
	mov r4, r4, lsl #1		@; q = q << 1;
	cmp r5, r1
	blo .Ldiv_cont			@; si (r >= divisor), activar bit en cociente
	sub r5, r1				@; r = r - divisor;
	orr r4, #1				@; q = q | 1;
 .Ldiv_cont:
	sub r6, #1				@; decrementar �ndice del bucle
	cmp r6, #0
	bge .Ldiv_for2			@; bucle for-2, mientras i >= 0
.Ldiv_fin1:
	str r4, [r2]
	str r5, [r3]			@; guardar resultados en memoria (por referencia)
	mov r0, #0				@; c�digo de OK
.Ldiv_fin2:
	pop {r4-r7, pc}


	.global _ga_divmodL
	@;Par�metros
	@; R0: long long * num
	@; R1: unsigned int * den
	@; R2: long long * quo
	@; R3: unsigned int * mod
	@;Resultado
	@; R0: 0 si no hay problema, !=0 si hay error en la divisi�n
_ga_divmodL:
	push {r4-r6, lr}
	ldr r4, [r1]			@; R4 = denominador
	cmp r4, #0				@; verificar si se est� intentando dividir por cero
	bne .LdivL_ini
	mov r0, #1				@; c�digo de error
	b .LdivL_fin
.LdivL_ini:
	ldrd r0, [r0]			@; R1:R0 = numerador
	mov r5, r2				@; R5 apunta a quo
	mov r6, r3				@; R6 apunta a mod
	mov r2, r4
	mov r3, #0				@; R3:R2 = denominador
	bl __aeabi_ldivmod
	strd r0, [r5]
	str r2, [r6]			@; guardar resultados en memoria (por referencia)			
	mov r0, #0				@; c�digo de OK
.LdivL_fin:
	pop {r4-r6, pc}


	.global _ga_printf
	@;Par�metros
	@; R0: char * format
	@; R1: unsigned int val1 (opcional)
	@; R2: unsigned int val2 (opcional)
_ga_printf:
	push {r4, lr}
	ldr r4, =_gd_pidz		@; R4 = direcci�n _gd_pidz
	ldr r3, [r4]

	and r3, #0xf			@; R3 = ventana de salida (z�calo actual MOD 16)
	bl _gg_escribir			@; llamada a la funci�n definida en "garlic_graf.c"

	pop {r4, pc}
	
    .global _ga_setchar
	@;Par�metros
	@; R0: n (numero de caracter ASCII Extended, entre 128 i 135)
	@; R1: buffer (punter a matriu de 8x8 bytes)
_ga_setchar:
	push {r4, lr}             @; Guardar registros usados

	@; Comprovar si R0 esta dins del rang (128-135)
	cmp r0, #128
	blt .Lend_setChar
	cmp r0, #135
    bgt .Lend_setChar

	bl _gg_setChar	@; Si n correcte, processar buffer
.Lend_setChar:
	pop {r4, pc}

	.global _ga_wait
_ga_wait:
	push {r1-r11, lr}
	ldr r1, =_gd_sem
	ldrb r2, [r1, r0]		@; cogemos el valor del semaforo
	cmp r2, #1				@; si ya esta bloqueado indicamos que no se puede bloquear porque el semaforo ya esta siendo usado por otro proceso
	bne .Lwait_noBloq
	
	mov r3, #0
	strb r3, [r1, r0]		@; guardamos el valor bloqueado (0)
	ldr r4, =_gd_qReady
	ldr r5, =_gd_nReady
	ldr r6, [r5]
	ldr r9, =_gd_pidz
	ldr r9, [r9]
	mov r7, #0xf
	and r9, r9, r7			@; numero de zocalo
	mov r7, #0
	.LsacarDeReady:
		cmp r7, r6
		beq .LfiSacarDeReady
		ldr r8, [r4, r7]
		cmp r8, r9
		beq .LmovReady
		add r7, #1
		b .LsacarDeReady
	.LmovReady:
		mov r10, #0
		mov r11, #0xffffff
		orr r10, r10, r9		@;a�adimos el numero de zocalo
		mov r10, r10, lsr #8
		orr r10, r10, r11		@; ponemos los 24 bits restantes a 1
		ldr r11, =_gd_qDelay
		str r10, [r11]			@; guardamos el proceso bloqueado en la cola de Delay
		mov r10, r7
		.LiniMov:
			cmp r6, r7
			beq .LfiIniMov
			add r7, #1
			ldr r10, [r4, r7]
			sub r7, #1
			str r10, [r4, r7]
			add r7, #1
			b .LiniMov
	.LfiIniMov:
		sub r6, #1
		str r6, [r5]			@; restamos un proceso a nReady
		ldr r10, =_gd_nDelay
		ldr r11, [r10]
		add r11, #1
		str r11, [r10]			@; a�adimos un proceso a nDelay
	
	.LfiSacarDeReady:
	mov r0, #1				@; retornamos codigo de bloqueo correcto
	b .Lwait_fi
	
	.Lwait_noBloq:
		mov r0, #0			@; retornamos codigo de que no puede ser bloqueado por ese semaforo
	
	.Lwait_fi:
	pop {r1-r11, pc}

	.global _ga_signal
_ga_signal:
	push {r1-r11, lr}
	ldr r1, =_gd_sem
	ldrb r2, [r1, r0]		@; cogemos el valor del semaforo
	cmp r2, #0
	bne .Lsig_noBloq
	
	mov r3, #1
	strb r3, [r1, r0]		@; si esta bloqueado lo desbloqueamos (+1)
	@; treure de la cua de Delay i posar en la cua de Ready
	ldr r4, =_gd_qDelay
	ldr r5, =_gd_nDelay
	ldr r6, [r5]
	mov r7, #0
	mov r9, #0xf
	.LbuscD:
		cmp r6, r7
		beq .LfiBuscD
		ldr r8, [r4, r7]
		mov r8, r8, lsl #8	@; ponemos el zocalo en los 8 bits bajos
		and r8, r8, r9		@; obtenemos el numero de zocalo
		cmp r8, r0
		bne .LbuscD
		ldr r9, =_gd_qReady
		ldr r10, =_gd_nReady
		ldr r11, [r10]
		str r8, [r9]		@; guardamos el proceso en la cola de Ready
		add r11, #1
		str r11, [r10]
		.LmovD:				@; eliminamos el proceso que ha pasado a Ready y ajustamos la cola de Delay
			cmp r7, r6
			beq .LfiMovD
			add r7, #1
			ldr r10, [r4, r7]
			sub r7, #1
			str r10, [r4, r7]
			add r7, #1
			b .LmovD
		.LfiMovD:
			sub r6, #1
			str r6, [r5]	@; restamos un n a Delay
	.LfiBuscD:
	mov r0, #1				@; devolvemos codigo de que hemos desbloqueado un semaforo
	b .Lsig_fi
	
	.Lsig_noBloq:
		mov r0, #0			@; en caso de que no este bloqueado devolvemos codigo de que no estaba bloqueado
		
	.Lsig_fi:
	pop {r1-r11, pc}


	.global _ga_fopen
_ga_fopen:
	push {lr}
	bl _gm_fopen
	pop {pc}
	
	
	.global _ga_fread
_ga_fread:
	push {lr}
	bl _gm_fread
	pop {pc}
	
	
	.global _ga_fclose
_ga_fclose:
	push {lr}
	bl _gm_fclose
	pop {pc}


	.global _ga_printchar
	@;ParÃ¡metros
	@; R0: int vx
	@; R1: int vy
	@; R2: char c
	@; R3: int color
_ga_printchar:
	push {r4-r8, lr}
	mov r6, r0
	mov r7, r1
	mov r8, r2
	ldr r5, =_gd_pidz		@; R5 = direcciÃ³n _gd_pidz
	ldr r4, [r5]
	and r4, #0xf			@; R4 = ventana de salida (zÃ³calo actual)
	push {r4}				@; pasar 4Âº parÃ¡metro (nÃºm. ventana) por la pila
	bl _gg_escribirCar
	add sp, #4				@; eliminar 4Âº parÃ¡metro de la pila
	pop {r4-r8, pc}


	.align 2
	.global _ga_printmat
	@;Parámetros
	@; R0: int vx
	@; R1: int vy
	@; R2: char *m[]
	@; R3: int color
_ga_printmat:
	push {r4-r5, lr}
	ldr r5, =_gd_pidz		@; R5 = direcciÃ³n _gd_pidz
	ldr r4, [r5]
	and r4, #0xf			@; R4 = ventana de salida (zÃ³calo actual)
	push {r4}				@; pasar 4Âº parÃ¡metro (nÃºm. ventana) por la pila
	bl _gg_escribirMat
	add sp, #4				@; eliminar 4Âº parÃ¡metro de la pila
	pop {r4-r5, pc}


	.global _ga_delay
	@;Par�metros
	@; R0: int nsec
_ga_delay:
	push {r2-r3, lr}
	ldr r3, =_gd_pidz		@; R3 = direcciï¿½n _gd_pidz
	ldr r2, [r3]
	and r2, #0xf			@; R2 = zï¿½calo actual
	cmp r0, #0
	bhi .Ldelay1
	bl _gp_WaitForVBlank	@; si nsec = 0, solo desbanca el proceso
	b .Ldelay2				@; y salta al final de la rutina
.Ldelay1:
	cmp r0, #600
	movhi r0, #600			@; limitar el numero de segundos a 600 (10 minutos)
	bl _gp_retardarProc
.Ldelay2:
	pop {r2-r3, pc}


	.global _ga_clear
_ga_clear:
	push {r0-r1, lr}
	ldr r1, =_gd_pidz
	ldr r0, [r1]
	and r0, #0xf			@; R0 = zï¿½calo actual
	mov r1, #1				@; R1 = 1 -> 16 ventanas
	bl _gs_borrarVentana
	pop {r0-r1, pc}


.global _ga_fwrite
_ga_fwrite:
	push {lr}
	bl _gm_fwrite
	pop {pc}

.end

