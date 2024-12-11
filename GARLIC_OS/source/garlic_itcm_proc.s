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
		@; incremento del contador de tics
		ldr r4, =_gd_tickCount
		ldr r5, [r4]
		add r5, #1
		str r5, [r4]
		
		@;incremento de workTicks
		ldr r4, =_gd_pidz
		ldr r4, [r4]
		mov r5, #0xF
		and r4, r4, r5			@;r4 = nº zocalo
		ldr r6, =_gd_pcbs		
		mov r5, #24
		mla r7, r5, r4, r6		@;r5 = inicio de la tabla de pcbs del zocalo
		ldr r4, [r7, #20]		@;accedemos al workTicks
		mov r6, r4, lsl #8
		mov r6, r6, lsr #8
		and r4, r4, #0xFF000000
		add r6, #1				@;incrementamos los workTicks
		orr r5, r4, r6
		str	r5, [r7, #20]

		bl _gp_actualizarDelay
		
		@; miramos si hay algun proceso en la cola de ready
		ldr r4, =_gd_nReady
		ldr r5, [r4]
		cmp r5, #1
		blo .Lfi
		
		@; miramos si el proceso es el del SO
		ldr r6, =_gd_pidz
		ldr r7, [r6]
		tst r7, #0xF
		beq .Lsalvar
		
		@; miramos si el PID es 0
		tst r7, #0xF0
		beq .Lrest
		
		@; salvamos el contexto
		.Lsalvar:
			bl _gp_salvarProc
			str r5, [r4]
		@;restauramos el contexto
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
		@; miramos si el proceso debe ir a la cola de Delay o a la de Ready
		ldr r8, [r6]
		mov r8, r8, lsl #1
		ands r9, r8, #1			@; si el bit no es 1 entonces lo trataremos como un proceso sin delay, pero si es 1 lo trataremos con delay
		movne r8, r8, lsr #1
		beq .LesReady
		mov r8, r8, lsl #8		@; ponemos en los 8 bits bajos el numero de zocalo
		and r8, #0xF			@; cogemos solo el numero de zocalo
		b .LsaltarR
		
		@; guardamos el numero de zocalo en la ultima posición de la cola de ready
		.LesReady:
			ldr r8, [r6]				@; sabemos que _gd_pidz => PID + num. zocalo
			and r8, #0xf				@; num. zocalo
			ldr r9, =_gd_qReady
			strb r8, [r9, r5]
		
		.LsaltarR:
		
		@; guardamos el valor de R15 en el pcb del proceso a desbancar
		ldr r9, =_gd_pcbs
		mov r10, #24
		mla r11, r10, r8, r9		@; direccion del pcb del proceso
		ldr r10, [r13, #60]			@; en r13 esta el SP i r15 (PC) esta en la posición más baja (60)
		str r10, [r11, #4]
		
		@; guardamos el CPSR del proceso a desbancar
		mrs r10, SPSR				@; cogemos el SPSR porque es donde se ha guardado el CPSR del proceso a desbancar
		str r10, [r11, #12]
		
		@; cambiamos el modo de ejecución a system
		mov r8, r13
		mrs r10, CPSR
		and r10, #0xFFFFFFE0		@; ponemos a 0 los ultimos 5 bits que son los que indican el modo de ejecución
		orr r10, #0x1F				@; ponemos estos 5 bits a 1 para aplicar el nuevo modo de ejecución
		msr CPSR, r10
		
		sub r13, #56				@; nos ponemos al inicio de la zona de la pila del usuario
		
		@; apilamos el valor de los registros
		@; cogemos los valores de la pila del modo irq (r8 -> SP)
		@; los guardamos cogiendo la dirección que hay en r13 (SP)
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
		
		@; guardamos el valor del registro r13 en el campo SP del PCB
		str r13, [r11, #8]
		
		@; volvemos al modo IRQ
		mrs r9, CPSR
		and r9, #0xFFFFFFE0
		orr r9, #0x12
		msr CPSR, r9
		
		@; devolvemos la rutina
		add r5, #1
	pop {r8-r11, pc}


	@; Rutina para restaurar el estado del siguiente proceso en la cola de READY;
	@;Parámetros
	@; R4: dirección _gd_nReady
	@; R5: número de procesos en READY
	@; R6: dirección _gd_pidz
_gp_restaurarProc:
	push {r8-r11, lr}
		@; recuperamos el numero de zocalo
		ldr r8, =_gd_qReady
		ldrb r9, [r8]
		@; como sacamos el proceso de la cola de ready tenemos que mover todos los otros
		mov r10, #1
		.LavanPos:
			ldrb r11, [r8, r10]
			sub r10, #1
			strb r11, [r8, r10]
			add r10, #2
			cmp r10, r5
			blo .LavanPos
		sub r5, #1
		
		@; guardamos el PID i el numero de zocalo
		ldr r8, =_gd_pcbs
		mov r10, #24
		mla r11, r10, r9, r8
		ldr r10, [r11]
		mov r10, r10, lsl #4		@; movemos 4 posiciones a la izquierda porque el pidz es (pid + zoc)
		orr r10, r9
		str r10, [r6]
		
		@; recuperamos r15 del proceso a restaurar
		@; sabemos que en r15 esta el campo PC del PCB
		ldr r10, [r11, #4]
		str r10, [r13, #60]
		
		@; recuperamos el CPSR
		@; el CPSR se encuentra en el campo status
		ldr r10, [r11, #12]
		msr SPSR, r10
		
		@; cambiamos el modo de ejecucion del proceso a restaurar
		mov r10, r13
		mrs r9, CPSR
		and r9, #0xFFFFFFE0
		orr r9, #0x1F
		msr CPSR, r9
		
		@;recuperamos el r13, que es el campo SP del PCB
		ldr r13, [r11, #8]
		
		@; copiamos los registros guardados del proceso y los copiamos en la pila del modo IRQ
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
		
		@; volvemos al modo IRQ
		mrs r9, CPSR
		and r9, #0xFFFFFFE0
		add r9, #0x12		
		msr CPSR, r9
	pop {r8-r11, pc}

	@; Rutina para actualizar la cola de procesos retardados, poniendo en
	@; cola de READY aquellos cuyo número de tics de retardo sea 0
_gp_actualizarDelay:
	push {r0-r11, lr}
	ldr r0, =_gd_qDelay
	ldr r1, =_gd_nDelay
	ldr r2, [r1]
	ldr r5, =_gd_qReady
	ldr r6, =_gd_nReady
	ldr r7, [r6]
	ldr r8, =_gd_pcbs
	mov r3, #0
	.Ldecrementar:
		cmp r3, r2
		beq .LfiDecrementar
		ldr r4, [r0, r3]
		ldr r11, =0xFFFFFF
		tst r4, r11				@; si tots els bits estan a 1 significa que el proces esta a la cua de delay per un bloqueig d'un semafor
		addne r3, #1
		bne .Ldecrementar
		sub r4, #1
		ldr r11, =0xFFFF
		tst r4, r11				@; mirem si ja ha arribat a 0
		beq .LaReady
		str r4, [r0, r3]		@; sino guardem amb el decrement
		add r3, #1
		b .Ldecrementar
	.LaReady:
		mov r4, r4, lsl #8		@; posem el zocalo als 4 bits baixos
		str r4, [r5, r6]		@; guardem el proces a la cua de ready
		add r7, #1
		str r7, [r6]			@; incrementem el numero de procesos a la cua de ready
		sub r2, #1				@; restem el proces a la cua de delay
		str r2, [r1]
		ldr r9, [r8, r4]
		mov r9, r9, lsl #1		@; canveim el bit mes alt del pid per posarlo a 0 i que ja no sigui ignorat
		mov r10, #0
		and r9, r9, r10
		mov r9, r9, lsr #1
		str r9, [r8, r4]
		mov r11, r3
		.LiniMovD:
			cmp r11, r2
			beq .Ldecrementar	@; quan ja els em mogut tots seguim decrementan
			add r11, #1
			ldr r4, [r0, r11]
			sub r11, #1
			str r4, [r0, r11]
			add r11, #1
			b .LiniMovD
	.LfiDecrementar:
	pop {r0-r11, pc}


	.global _gp_numProc
	@;Resultado
	@; R0: número de procesos total
_gp_numProc:
	push {r1-r2, lr}
	mov r0, #1				@; contar siempre 1 proceso en RUN
	ldr r1, =_gd_nReady
	ldr r2, [r1]			@; R2 = número de procesos en cola de READY
	add r0, r2				@; añadir procesos en READY
	ldr r1, =_gd_nDelay
	ldr r2, [r1]			@; R2 = número de procesos en cola de DELAY
	add r0, r2				@; añadir procesos retardados
	pop {r1-r2, pc}


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
		@; miramos cual es el numero de zocalo por si lo podemos dar o no
		cmp r1, #0
		moveq r0, #1	@; usaremos 1 para indicar que el error es que este zocalo es reservado
		beq .LfinalCP
		@; para mirar si el zocalo esta ocupado miraremos el vector de _gd_pcbs
		ldr r4, =_gd_pcbs
		mov r5, #24		@; 24 => 6 int's del registro * 4 (ocupación de un int)
		mla r6, r5, r1, r4	@; multiplicamos r5 i r1 para saber el desplazamiento para llegar al PID del zocalo que queremos ver y le sumamos r4 que es la direccion inicial del vector de pcbs
		ldr r5, [r6]
		@; si el PID es 0 significa que esta libre, sino significa que esta ocupado
		cmp r5, #0
		movne r0, #2	@; usaremos 2 para indicar que el error es porque el zocalo esta ocupado
		bne .LfinalCP
		
		@; obtener un nuevo PID para el nuevo proceso y guardar su nuevo valor
		@; inhibimos este fragmento de codigo porque es importante que si ya se obtiene un pid la variable pidCount aumente y que este pid se guarde en el campo pid para que ningun otro proceso use el mismo pid
		bl _gp_inhibirIRQs
		ldr r5, =_gd_pidCount
		ldr r7, [r5]
		add r7, #1
		str r7, [r5]
		str r7, [r6]
		bl _gp_desinhibirIRQs
		@; guardamos la dirección de la rutina inicial del proceso (r0)
		@; compensamos el decremento
		add r0, #4
		str r0, [r6, #4]
		
		@; guardamos los 4 primeros caracteres del nombre (r2)
		@; 1 caracter son 8 bits, queremos los 4 primeros entonces -> 4*8 = 32 bits
		ldr r2, [r2]
		str r2, [r6, #16]
		
		@; calculo de la direccion base de la pila
		@; los vectores de las pilas estan en _gd_stacks[15*128]
		ldr r7, =_gd_stacks
		@; cada pila ocupa 128*4 = 512B
		@; 15 pilas (0-14)
		mov r8, r1
		sub r8, #1
		mov r9, #512
		mul r8, r9
		add r8, r9		@; desplazamiento inicial de la pila
		add r7, r8		
		sub r7, #56		@; direccion inicial de la pila
		
		@; guardamos en la pila del proceso los registros
		@; primero tenemos que guardar los argumentos que tenga el proceso, en este caso r3
		str r3, [r7]
		@; llenamos los registros r0-r12 con 0
		mov r8, #0 @; valor para los registros
		mov r9, #4 @; desplazamiento por la pila (en la primera posicion esta el argumento, r0)
		.LomplirRx:
			str r8, [r7, r9]
			add r9, #4
			cmp r9, #48
			ble .LomplirRx
		@; guardamos en r14 la dirección de la rutina _gp_terminarProc
		ldr r8, =_gp_terminarProc
		str r8, [r7, #52]
		
		@; guardamos en el vector de pcbs la dirección incial de la pila que es donde estan guardados los registros
		str r7, [r6, #8]
		
		@; guardamos el valor incial del CPSR en el campo Status del vector de pcbs
		mrs r8, CPSR
		str r8, [r6, #12]
		
		@; guardamos el numero de zocalo en la cola de Ready
		ldr r8, =_gd_qReady
		ldr r9, =_gd_nReady
		ldr r10, [r9]
		@; es importante que si añadimos el proceso a la cola de ready nReady aumente en 1 para mantener la consistencia
		bl _gp_inhibirIRQs
		strb r1, [r8, r10]
		@;incrementamos la variable _gd_nReady
		add r10, #1
		str r10, [r9]
		bl _gp_desinhibirIRQs
		
		@; inicializamos otras variables del pcb (en este caso solo queda workTicks)
		mov r8, #0
		str r8, [r6, #20]
		
		@; devolvemos con codigo OK
		mov r0, #0
		.LfinalCP:
		
	pop {r4-r10, pc}


@; Rutina para terminar un proceso de usuario:
	@; pone a 0 el campo PID del PCB del zócalo actual, para indicar que esa
	@; entrada del vector _gd_pcbs está libre; también pone a 0 el PID de la
	@; variable _gd_pidz (sin modificar el número de zócalo), para que el código
	@; de multiplexación de procesos no salve el estado del proceso terminado.
_gp_terminarProc:
	ldr r0, =_gd_pidz
	ldr r1, [r0]			@; R1 = valor actual de PID + zócalo
	and r1, r1, #0xf		@; R1 = zócalo del proceso desbancado
	bl _gp_inhibirIRQs
	str r1, [r0]			@; guardar zócalo con PID = 0, para no salvar estado			
	ldr r2, =_gd_pcbs
	mov r10, #24
	mul r11, r1, r10
	add r2, r11				@; R2 = dirección base _gd_pcbs[zocalo]
	mov r3, #0
	str r3, [r2]			@; pone a 0 el campo PID del PCB del proceso
	str r3, [r2, #20]		@; borrar porcentaje de USO de la CPU
	ldr r0, =_gd_sincMain
	ldr r2, [r0]			@; R2 = valor actual de la variable de sincronismo
	mov r3, #1
	mov r3, r3, lsl r1		@; R3 = máscara con bit correspondiente al zócalo
	orr r2, r3
	str r2, [r0]			@; actualizar variable de sincronismo
	bl _gp_desinhibirIRQs
.LterminarProc_inf:
	bl _gp_WaitForVBlank	@; pausar procesador
	b .LterminarProc_inf	@; hasta asegurar el cambio de contexto

.global _gp_matarProc
	@; Rutina para destruir un proceso de usuario:
	@; borra el PID del PCB del zócalo referenciado por parámetro, para indicar
	@; que esa entrada del vector _gd_pcbs está libre; elimina el índice de
	@; zócalo de la cola de READY o de la cola de DELAY, esté donde esté;
	@; Parámetros:
	@;	R0:	zócalo del proceso a matar (entre 1 y 15).
_gp_matarProc:
	push {r1-r10, lr}
	ldr r1, =_gd_pcbs		@; cargamos la dirección del vector de pcbs
	mov r2, #24
	mla r3, r2, r0, r1		@; calculamos el desplazamiento
	mov r4, #0	
	@; inhibimos las IRQs porque si ya tenemos puesto el PID a 0 es importante asegurarse de que se ha eliminado el proceso de la cola correspondiente en el que se encuentre para que no haya incoherencias
	bl _gp_inhibirIRQs
	str r4, [r3]		@; ponemos el PID a 0
	
	ldr r5, =_gd_qReady
	ldr r6, =_gd_nReady
	ldr r7, [r6]
	.LforR:
		ldrb r8, [r6, r4]
		cmp r8, r0			@; miramos si coincide el zocalo
		beq .Ltreure
		add r4, #1
		cmp r4, r7
		blo .LforR			@; seguimos recorriendo la cola
		mov r4, #0
		ldr r5, =_gd_qDelay
		ldr r6, =_gd_nDelay
		ldr r7, [r6]
		mov r10, #0
		
	.LforD:
		ldr r8, [r6, r4]
		@; agafar els 8 bits alts per comprovar si es el num de zocalo
		cmp r9, r0
		beq .Ltreure
		add r4, #4
		add r10, #1
		cmp r4, r10
		blo .LforD
		b .LfiMP
	
	.Ltreure:
	sub r7, #1 				@; restamos 1 al numero de procesos en cola de Ready
	str r7, [r6]
		.Lmoure:
			add r4, #1
			ldr r1, [r5, r4]
			sub r4, #1
			str r1, [r5, r4]
			add r4, #1
			cmp r7, r4
			blo .Lmoure
			mov r7, #0
			str r7, [r5, r4]
			
	.LfiMP:
		bl _gp_desinhibirIRQs
	pop {r1-r10, pc}
	
.global _gp_retardarProc
	@; retarda la ejecución de un proceso durante cierto número de segundos,
	@; colocándolo en la cola de DELAY
	@;Parámetros
	@; R0: int nsec
_gp_retardarProc:
	push {r1-r7, lr}
	mov r1, #60			@; cada 1s se hacen aprox. 60 tics
	mul r2, r1, r0		@; calculamos el numero de tics segun los segundos
	mov r3, #0			@; registro donde se construira el word
	ldr r1, =_gd_pidz
	ldr r4, [r1]
	and r4, r4, #0xf	@; cogemos el numero de zocalo del proceso actual
	orr r3, r3, r4		@; añadimos el num. de zocalo
	mov r3, r3, lsr #8	@; ponemos el zocalo en los 8 bits altos
	orr r3, r3, r2		@; añadimos los tics
	ldr r5, =_gd_qDelay
	ldr r6, =_gd_nDelay
	ldr r7, [r6]
	bl _gp_inhibirIRQs	@; inhibimos las IRQ porque si añadimos el proceso en la cola de delay sera necesario tambien aumentar el numero de procesos en la cola de delay para que no haya incoherencias y indicamos en el pidz que el proceso esta en la cola de delay
	str r3, [r5, r7]	@; añadimos en la cola el word creado
	add r7, #1
	str r7, [r6]		@; incrementamos el numero de procesos en delay
	mov r4, r4, lsl #1
	mov r5, #1
	orr r4, r4, r5
	mov r4, r4, lsr #1	@; ponemos el bit mas alto a 1
	str r4, [r1]		@; guardamos el nuevo pidz
	bl _gp_desinhibirIRQs
	bl _gp_WaitForVBlank	@; invocamos a la nueva rutina
	pop {r1-r7, pc}			@; no retornará hasta que se haya agotado el retardo


	.global _gp_inihibirIRQs
	@; pone el bit IME (Interrupt Master Enable) a 0, para inhibir todas
	@; las IRQs y evitar así posibles problemas debidos al cambio de contexto
_gp_inhibirIRQs:
	push {r0-r1, lr}
	ldr r0, =0x4000208	@; cargamos la dirección del registro IME
	ldr r1, [r0]
	bic r1, #1
	str r1, [r0]
	pop {r0-r1, pc}
	
	.global _gp_desinihibirIRQs
	@; pone el bit IME (Interrupt Master Enable) a 1, para desinhibir todas
	@; las IRQs
_gp_desinhibirIRQs:
	push {r0-r1, lr}
	ldr r0, =0x4000208
	ldr r1, [r0]
	orr r1, #1
	str r1, [r0]
	pop {r0-r1, pc}
	
	.global _gp_rsiTIMER0
	@; Rutina de Servicio de Interrupción (RSI) para contabilizar los tics
	@; de trabajo de cada proceso: suma los tics de todos los procesos y calcula
	@; el porcentaje de uso de la CPU, que se guarda en los 8 bits altos de la
	@; entrada _gd_pcbs[z].workTicks de cada proceso (z) y, si el procesador
	@; gráfico secundario está correctamente configurado, se imprime en la
	@; columna correspondiente de la tabla de procesos.
_gp_rsiTIMER0:
	push {r0-r11, lr}
	ldr r9, =_gd_pcbs
	mov r10, #24
	mov r11, #0
	mov r5, #0
	.Lsuma:
		cmp r11, #15
		movgt r11, #0
		bgt .Lperc			@; comprovamos si hemos visto todas las posiciones del vector de pcbs
		mla r3, r10, r11, r9	@; calculamos la dirección inicial de cada pcb
		ldr r4, [r3]		@; accedemos al PID para ver si hay un proceso
		cmp r4, #0
		beq .LesSO
		.LaccWork:
			ldr r4, [r3, #20]	@; accedemos a los workticks
			add r5, r4			@; guardamos el total de los procesos
			add r11, #1
			b .Lsuma
		.LesSO:					@; miramos si el proceso es el del sistema operativo para tenerlo en cuenta
			cmp r11, #0
			addne r11, #1
			bne .Lsuma
			beq .LaccWork
	
	.Lperc:
		cmp r11, #15
		bgt .LfiRSI
		mla r3, r10, r11, r9
		ldr r4, [r3]
		cmp r4, #0
		beq .LesSO2
		.LsegPerc:
			ldr r4, [r3, #20]
			mov r7, #100
			mul r4, r7
			mov r0, r4
			mov r1, r5
			ldr r2, =_gd_quo
			ldr r3, =_gd_mod
			bl _ga_divmod				@; dividimos para obtener el porcentaje
			ldr r2, [r2]
			ldr r3, [r3]
			mov r8, #0
			and r4, r4, r8			@; ponemos a 0 los workticks
			mov r8, r2
			mov r8, r8, lsr #8		@; ponemos el porcentage en los 8 bits altos
			str r8, [r3, #20]		@; guardamos en workticks
			@; pasar el porcentaje a string
			ldr r0, =_gd_perc
			mov r1, #4
			@;mov r2, r6
			bl _gs_num2str_dec
			@; escribir en la pantalla el porcentage
			@; en r0 -> string acabado con centinela
			@; en r1 -> fila
			@; en r2 -> columna
			@; en r3 -> color
			ldr r0, =_gd_perc
			add r1, r11, #4
			mov r2, #28
			mov r3, #0
			bl _gs_escribirStringSub
			add r11, #1
			b .Lperc
		.LesSO2:				@; mirem si el proces es el del so per tindrel en comte
			cmp r11, #0
			addne r11, #1
			bne .Lperc
			beq .LsegPerc
	.LfiRSI:
		ldr r0, =_gd_sincMain
		ldr r1, [r0]
		mov r2, #1
		orr r3, r1, r2
		str r3, [r0]
	pop {r0-r11, pc}

.end

