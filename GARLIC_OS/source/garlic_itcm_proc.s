@;==============================================================================
@;
@;	"garlic_itcm_proc.s":	código de las rutinas de control de procesos (1.0)
@;						(ver "garlic_system.h" para descripción de rutinas)
@;
@;==============================================================================

.section .itcm,"ax",%progbits

	.arm
	.align 2
	
	.global _gp_WaitForVBlank
	@; rutina para pausar el procesador mientras no se produzca una interrupción
	@; de retroceso vertical (VBL); es un sustituto de la "swi #5" que evita
	@; la necesidad de cambiar a modo supervisor en los procesos GARLIC;
_gp_WaitForVBlank:
	push {r0-r1, lr}
	ldr r0, =__irq_flags
.Lwait_espera:
	mcr p15, 0, lr, c7, c0, 4	@; HALT (suspender hasta nueva interrupción)
	ldr r1, [r0]			@; R1 = [__irq_flags]
	tst r1, #1				@; comprobar flag IRQ_VBL
	beq .Lwait_espera		@; repetir bucle mientras no exista IRQ_VBL
	bic r1, #1
	str r1, [r0]			@; poner a cero el flag IRQ_VBL
	pop {r0-r1, pc}


	.global _gp_IntrMain
	@; Manejador principal de interrupciones del sistema Garlic;
_gp_IntrMain:
	mov	r12, #0x4000000
	add	r12, r12, #0x208	@; R12 = base registros de control de interrupciones	
	ldr	r2, [r12, #0x08]	@; R2 = REG_IE (máscara de bits con int. permitidas)
	ldr	r1, [r12, #0x0C]	@; R1 = REG_IF (máscara de bits con int. activas)
	and r1, r1, r2			@; filtrar int. activas con int. permitidas
	ldr	r2, =irqTable
.Lintr_find:				@; buscar manejadores de interrupciones específicos
	ldr r0, [r2, #4]		@; R0 = máscara de int. del manejador indexado
	cmp	r0, #0				@; si máscara = cero, fin de vector de manejadores
	beq	.Lintr_setflags		@; (abandonar bucle de búsqueda de manejador)
	ands r0, r0, r1			@; determinar si el manejador indexado atiende a una
	beq	.Lintr_cont1		@; de las interrupciones activas
	ldr	r3, [r2]			@; R3 = dirección de salto del manejador indexado
	cmp	r3, #0
	beq	.Lintr_ret			@; abandonar si dirección = 0
	mov r2, lr				@; guardar dirección de retorno
	blx	r3					@; invocar el manejador indexado
	mov lr, r2				@; recuperar dirección de retorno
	b .Lintr_ret			@; salir del bucle de búsqueda
.Lintr_cont1:	
	add	r2, r2, #8			@; pasar al siguiente índice del vector de
	b	.Lintr_find			@; manejadores de interrupciones específicas
.Lintr_ret:
	mov r1, r0				@; indica qué interrupción se ha servido
.Lintr_setflags:
	str	r1, [r12, #0x0C]	@; REG_IF = R1 (comunica interrupción servida)
	ldr	r0, =__irq_flags	@; R0 = dirección flags IRQ para gestión IntrWait
	ldr	r3, [r0]
	orr	r3, r3, r1			@; activar el flag correspondiente a la interrupción
	str	r3, [r0]			@; servida (todas si no se ha encontrado el maneja-
							@; dor correspondiente)
	mov	pc,lr				@; retornar al gestor de la excepción IRQ de la BIOS


	.global _gp_rsiVBL
	@; Manejador de interrupciones VBL (Vertical BLank) de Garlic:
	@; se encarga de actualizar los tics, intercambiar procesos, etc.;
_gp_rsiVBL:
	push {r4-r7, lr}
		@; incrementem el contador de tics
		ldr r4, =_gd_tickCount
		ldr r5, [r4]
		add r5, #1
		str r5, [r4]
		
		@; mirem si hi ha algun proces a la cua de ready
		ldr r4, =_gd_nReady
		ldr r5, [r4]
		cmp r5, #1
		blo .Lfi
		
		@; mirem si el proces és el del SO
		ldr r6, =_gd_pidz
		ldr r7, [r6]
		tst r7, #0xF
		beq .Lsalvar
		
		@; mirem si el PID es 0
		tst r7, #0xF0
		beq .Lrest
		
		@; salvar el context
		.Lsalvar:
			bl _gp_salvarProc
			str r5, [r4]
		@;restaurar el context
		.Lrest:
			bl _gp_restaurarProc
			str r5, [r4]
		
		.Lfi:
	pop {r4-r7, pc}


	@; Rutina para salvar el estado del proceso interrumpido en la entrada
	@; correspondiente del vector _gd_pcbs[];
	@;Parámetros
	@; R4: dirección _gd_nReady
	@; R5: número de procesos en READY
	@; R6: dirección _gd_pidz
	@;Resultado
	@; R5: nuevo número de procesos en READY (+1)
_gp_salvarProc:
	push {r8-r11, lr}
		@; guardem el numero de zocalo a la ultima posicio de la cua de ready
		ldr r8, [r6]	@; sabem que _gd_pidz => PID + num. zocalo
		and r8, #0xf	@; num. zocalo
		ldr r9, =_gd_qReady
		strb r8, [r9, r5]
		
		@; guardem el valor de R15 en el pcb del proces a desbancar
		ldr r9, =_gd_pcbs
		mov r10, #24
		mla r11, r10, r8, r9		@; direccio del pcb del proces
		ldr r10, [r13, #60]			@; en r13 esta el SP i r15 (PC) esta en la posicio més baixa (60)
		str r10, [r11, #4]
		
		@; guardem el CPSR del proces a desbancar
		mrs r10, SPSR				@; agafem el SPSR perque es on s'ha guardat el CPSR del proces a desbancar
		str r10, [r11, #12]
		
		@; canviem el mode d'execució a system
		mov r8, r13
		mrs r10, CPSR
		and r10, #0xFFFFFFE0		@; posem a 0 els ultims 5 bits que son els que indiquen el mode d'execució
		orr r10, #0x1F				@; posem aquest 5 bits a 1 per aplicar el nou mode d'execució
		msr CPSR, r10
		
		sub r13, #56
		
		@; apilem el valor dels registres
		@; agafem els valors de la pila del mode irq (r8 -> SP)
		@; els guardem agafan la direccio que hi ha a r13 (SP)
		ldr r10, [r8, #40]
		str r10, [r13]
		
		ldr r10, [r8, #44]
		str r10, [r13, #4]
		
		ldr r10, [r8, #48]
		str r10, [r13, #8]
		
		ldr r10, [r8, #52]
		str r10, [r13, #12]
		
		ldr r10, [r8, #20]
		str r10, [r13, #16]
		
		ldr r10, [r8, #24]
		str r10, [r13, #20]
		
		ldr r10, [r8, #28]
		str r10, [r13, #24]
		
		ldr r10, [r8, #32]
		str r10, [r13, #28]
		
		ldr r10, [r8]
		str r10, [r13, #32]
		
		ldr r10, [r8, #4]
		str r10, [r13, #36]
		
		ldr r10, [r8, #8]
		str r10, [r13, #40]
		
		ldr r10, [r8, #12]
		str r10, [r13, #44]
		
		ldr r10, [r8, #56]
		str r10, [r13, #48]
		
		str r14, [r13, #52]
		
		@; guardem el valor del registre r13 en el camp SP del PCB
		str r13, [r11, #8]
		
		@; tornem al mode IRQ
		mrs r9, CPSR
		and r9, #0xFFFFFFE0
		orr r9, #0x12
		msr CPSR, r9
		
		@; retornem la rutina
		add r5, #1
	pop {r8-r11, pc}


	@; Rutina para restaurar el estado del siguiente proceso en la cola de READY;
	@;Parámetros
	@; R4: dirección _gd_nReady
	@; R5: número de procesos en READY
	@; R6: dirección _gd_pidz
_gp_restaurarProc:
	push {r8-r11, lr}
		@; recuperem el numero de zocalo
		ldr r8, =_gd_qReady
		ldrb r9, [r8]
		@; com treiem el proces de la cua de ready hem de moure tots els altres
		mov r10, #1
		.LavanPos:
			ldrb r11, [r8, r10]
			sub r10, #1
			strb r11, [r8, r10]
			add r10, #2
			cmp r10, r5
			blo .LavanPos
		sub r5, #1
		
		@; guardem el PID i el numero de zocalo
		ldr r8, =_gd_pcbs
		mov r10, #24
		mla r11, r10, r9, r8
		ldr r10, [r11]
		mov r10, r10, lsl #4		@; movem 4 posicions a l'esquerra perque el pidz es (pid + zoc)
		orr r10, r9
		str r10, [r6]
		
		@; recuperem r15 del proces a restaurar
		@; sabem que r15 esta al camp PC del PCB
		ldr r10, [r11, #4]
		str r10, [r13, #60]
		
		@; recuperem el CPSR
		@; el CPSR es troba en el camp status
		ldr r10, [r11, #12]
		msr SPSR, r10
		
		@; canviem el mode d'execucio del proces a restaurar
		mov r10, r13
		mrs r9, CPSR
		and r9, #0xFFFFFFE0
		orr r9, #0x1F
		msr CPSR, r9
		
		@;recuperem el r13, que és el camp SP del PCB
		ldr r13, [r11, #8]
		
		@; copiem el registres guardats del proces i els copiem a la pila del mode IRQ
		ldr r9, [r13]
		str r9, [r10, #40]
		
		ldr r9, [r13, #4]
		str r9, [r10, #44]
		
		ldr r9, [r13, #8]
		str r9, [r10, #48]
		
		ldr r9, [r13, #12]
		str r9, [r10, #52]
		
		ldr r9, [r13, #16]
		str r9, [r10, #20]
		
		ldr r9, [r13, #20]
		str r9, [r10, #24]
		
		ldr r9, [r13, #24]
		str r9, [r10, #28]
		
		ldr r9, [r13, #28]
		str r9, [r10, #32]
		
		ldr r9, [r13, #32]
		str r9, [r10]
		
		ldr r9, [r13, #36]
		str r9, [r10, #4]
		
		ldr r9, [r13, #40]
		str r9, [r10, #8]
		
		ldr r9, [r13, #44]
		str r9, [r10, #12]
		
		ldr r9, [r13, #48]
		str r9, [r10, #56]
		
		ldr r14, [r13, #52]
		
		add r13, #56				
		
		@; tornem al mode IRQ
		mrs r9, CPSR
		and r9, #0xFFFFFFE0
		add r9, #0x12		
		msr CPSR, r9
	pop {r8-r11, pc}


	.global _gp_numProc
	@;Resultado
	@; R0: número de procesos total
_gp_numProc:
	push {r1, lr}
		mov r0, #1 @;proces que esta en run
		ldr r1, =_gd_nReady
		ldr r1, [r1]
		add r0, r1
	pop {r1, pc}


	.global _gp_crearProc
	@; prepara un proceso para ser ejecutado, creando su entorno de ejecución y
	@; colocándolo en la cola de READY;
	@;Parámetros
	@; R0: intFunc funcion
	@; R1: int zocalo
	@; R2: char *nombre
	@; R3: int arg
	@;Resultado
	@; R0: 0 si no hay problema, >0 si no se puede crear el proceso
_gp_crearProc:
	push {r4-r10, lr}
		@; mirem quin és el numero de zocalo per si el podem donar o no
		cmp r1, #0
		moveq r0, #1	@;farem servir 1 per indicar que l'error és que aquest zocalo es reservat
		beq .LfinalCP
		@; per mirar si el zocalo esta ocupat mirarem al vector de _gd_pcbs
		ldr r4, =_gd_pcbs
		mov r5, #24		@; 24 => 6 int's del registre * 4 (ocupació d'un int)
		mla r6, r5, r1, r4	@; multipliquem r5 i r1 per saber el desplaçament per arribar al PID del zocalo que volem veure i el sumem a r4 que es la direccio inicial del vector de pcbs
		ldr r5, [r6]
		@; si el PID és 0 significa que esta lliure, sino significa que esta ocupat
		cmp r5, #0
		movne r0, #2	@;farem servir 2 per indicar que l'error és perque el zocalo esta ocupat
		bne .LfinalCP
		
		@; obtenir un nou PID pel nou proces i guardar el nou valor
		ldr r5, =_gd_pidCount
		ldr r7, [r5]
		add r7, #1
		str r7, [r5]
		str r7, [r6]
		
		@; guardem la direccio de la rutina inicial del proces (r0)
		@; compensem el decrement
		add r0, #4
		str r0, [r6, #4]
		
		@; guardem els 4 primers caracters del nom (r2)
		@; 1 caracter son 8 bits, volem els 4 primers llavors -> 4*8 = 32 bits
		ldr r2, [r2]
		str r2, [r6, #16]
		
		@; calcul de la direccio base de la pila
		@; els vectors de les piles estan en _gd_stacks[15*128]
		ldr r7, =_gd_stacks
		@; cada pila ocupa 128*4 = 512B
		@; 15 piles (0-14)
		mov r8, r1
		sub r8, #1
		mov r9, #512
		mul r8, r9
		add r8, r9		@; desplaçament inici de la pila
		add r7, r8		@; direccio inici de la pila
		
		@; guardem en la pila del proces els registres
		@; primer hem de guardar els arguments que tingui el proces, en aquest cas r3
		str r3, [r7]
		@; omplim els registres r0-r12 amb 0
		mov r8, #0 @; valor pels registres
		mov r9, #4 @; desplaçament per la pila (en la primera posicio esta l'argument, r0)
		.LomplirRx:
			str r8, [r7, r9]
			add r9, #4
			cmp r9, #48
			ble .LomplirRx
		@; guardem en r14 la direccio de la rutina _gp_terminarProc
		ldr r8, =_gp_terminarProc
		str r8, [r7, #52]
		
		@; guardem en el vector de pcbs la direccio incial de la pila que es on estan guardats els registres
		str r7, [r6, #8]
		
		@; guardem el valor incial del CPSR en el camp Status del vector de pcbs
		mrs r8, CPSR
		str r8, [r6, #12]
		
		@; guardem el numero de zocalo a la cua de Ready
		ldr r8, =_gd_qReady
		ldr r9, =_gd_nReady
		ldr r10, [r9]
		strb r1, [r8, r10]
		@;incrementem la variable _gd_nReady
		add r10, #1
		str r10, [r9]
		
		@; inicialitzem altres variables del pcb (en aquest cas nomes queda workTicks)
		mov r8, #0
		str r8, [r6, #20]
		
		@; retornem amb codi OK
		mov r0, #0
		.LfinalCP:
		
	pop {r4-r10, pc}


	@; Rutina para terminar un proceso de usuario:
	@; pone a 0 el campo PID del PCB del zócalo actual, para indicar que esa
	@; entrada del vector _gd_pcbs[] está libre; también pone a 0 el PID de la
	@; variable _gd_pidz (sin modificar el número de zócalo), para que el código
	@; de multiplexación de procesos no salve el estado del proceso terminado.
_gp_terminarProc:
	ldr r0, =_gd_pidz
	ldr r1, [r0]			@; R1 = valor actual de PID + zócalo
	and r1, r1, #0xf		@; R1 = zócalo del proceso desbancado
	str r1, [r0]			@; guardar zócalo con PID = 0, para no salvar estado			
	ldr r2, =_gd_pcbs
	mov r10, #24
	mul r11, r1, r10
	add r2, r11				@; R2 = dirección base _gd_pcbs[zocalo]
	mov r3, #0
	str r3, [r2]			@; pone a 0 el campo PID del PCB del proceso
.LterminarProc_inf:
	bl _gp_WaitForVBlank	@; pausar procesador
	b .LterminarProc_inf	@; hasta asegurar el cambio de contexto
	
.end

