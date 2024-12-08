@;==============================================================================
@;
@;	"garlic_itcm_mem.s":	c�digo de rutinas de soporte a la carga de
@;							programas en memoria (version 1.0)
@;
@;==============================================================================

NUM_FRANJAS = 768
INI_MEM_PROC = 0x01002000

.section .dtcm,"wa",%progbits
		
		.align 2
		.global quo
	quo:	.word 0
		.global mod
	mod:	.word 0

				.global _gm_zocMem
	_gm_zocMem:	.space NUM_FRANJAS	@; vector ocupaci�n franjas

.section .itcm,"ax",%progbits

	.arm
	.align 2
	

	.global _gm_reubicar
	@; rutina de soporte a _gm_cargarPrograma(), que interpreta los 'relocs'
	@; de un fichero ELF, contenido en un buffer *fileBuf, y ajustar las
	@; direcciones de memoria correspondientes a las referencias de tipo
	@; R_ARM_ABS32, a partir de las direcciones de memoria destino de c�digo
	@; (dest_code) y datos (dest_data), y seg�n el valor de las direcciones de
	@; las referencias a reubicar y de las direcciones de inicio de los
	@; segmentos de c�digo (pAddr_code) y datos (pAddr_data)
	@;Par�metros:
	@; R0: direcci�n inicial del buffer de fichero (char *fileBuf)
	@; R1: direcci�n de inicio de segmento de c�digo (unsigned int pAddr_code)
	@; R2: direcci�n de destino en la memoria (unsigned int *dest_code)
	@; R3: direcci�n de inicio de segmento de datos (unsigned int pAddr_data)
	@; (pila): direcci�n de destino en la memoria (unsigned int *dest_data)
	@;Resultado:
	@; cambio de las direcciones de memoria que se tienen que ajustar
_gm_reubicar:
	push {r0-r12,lr}

	ldr  r6, [SP, #56]												@; R6 -> carreguem el cinqu� par�metre a partir de la suma 56 (4bytes per registre * 14registres pila)
	mov r12, r3														@; R12 -> guardem el valor de r3 passat per par�metre a r12 per tal de mantenir el codi de la fase1
	ldr r3, [r0, #32]												@; R3 -> carreguem el valor del despla�ament de la taula de seccions a partir del buffer (e_shoff)
	add r3, #4														@; R3 -> incrementem a 4 r3 per situar-nos al tipus de seccio
	ldrh r4, [r0, #48]												@; R4 -> carreguem el n�mero d'entrades de la taula de seccions a partir del buffer (e_shnum)
	
	
.LRecorregut_seccions:
	ldr r5, [r0, r3]												@; R5 -> carreguem el tipus de seccio en la taula de seccions a partir del buffer (sh_type) 
	cmp r5, #9														@; comprovem si el tipus de secci� �s de tipus 9 (SHT_REC -> secci� de reubicadors)
	bne .LFi_seccio													@; si el tipus de secci� no �s de tipus 9, saltem a la seg�ent secci�
	b .LEs_Seccio_reubicadors										@; si el tipus de secci� �s de tipus 9, continuarem accedint a l'estructura de reubicadors


.LEs_Seccio_reubicadors:	
	add r3, #12														@; accedim a la @ de sh_offset
	ldr r5, [r0, r3]												@; carreguem el valor de sh_offset (estructura reubicadors)
	add r3, #4														@; sumem 4 per accedir a la @ de sh_size (tamany de la secci� dins del fitxer)
	@; s'emmagatzemen els registres r0 a r3 i r6 en la pila per preservar l'estat dels registres abans de realitzar la divis� per calcular el n�mero de reubicadors
	push {r0-r3, r6}
	mov r6, r0														@; canviem el registre r0 per r6 per guardar el apuntador al buffer del fitxer .elf
	ldr r0, [r6, r3]												@; del buffer, obtenim el valor de sh_size
	add r3, #16														@; sumem 16 per accedir a la @ de sh_entsize, que ens indica el que ocupa en bytes cada reubicador
	ldr r1, [r6, r3]												@; del buffer obtenim el valor de sh_entsize
	ldr r2, =quo													@; carreguem la @ de memoria del quocient en el registre r2
	ldr r3, =mod													@; carreguem la @ de memoria del modul/residu en el registre r3
	bl _ga_divmod													@; cridem a la rutina _ga_divmod per calcular el n�mero de reubicadors, dividint el sh_size (el tamany de la secci�) entre sh_entsize (el que ocupa cada reubicador) 
	ldr r7, [r2]													@; carreguem el resultat del quocient en r7
	ldr r8, [r3]													@; carreguem el resultat del modul/residu en r8
	pop {r0-r3, r6}													@; restaurem els registres r0 a r3 i r6 desde la pila, d'aquesta manera podem seguir utilitzant aquests registres amb els valors inicials passats per par�metre en aquesta rutina
	
	add r3, #16 													@; tornem a fer la suma de r3 per tal no perdre la @ de memoria a la que apuntava r3 entre el push i el pop
	cmp r7, #0														@; comprovem si la divisi� s'ha realitzat correctament
	beq .LContinua_recorregut_seccio								@; si ha hagut algun problema en la divisi� (r7=0), passem a recorrer la seg�ent secci� si es que en queden per rec�rrer


.LRecorregut_reubicadors:
	ldr r9, [r0, r5]												@; del buffer obtenim el valor del primer reubicador, el offset de l'estructura de reubicadors (r_offset)
	add r5, #4														@; sumem 4 al primer reubicador (offset) per situar-nos en el tipus de reubicador (r_info)
	ldr r10, [r0, r5]												@; del buffer obtenim el valor de r_info
	and r10, #0xFF													@; apliquem m�scara per quedar-nos amb els 8 bits m�s baixos de r_info que cont� el tipus de reubicador que s'ha d'aplicar
	cmp r10, #2														@; comparem el tipus de reubicador amb 2 (R_ARM_ABS32)
	beq .LReubicar													@; si el reubicador �s de tipus 2 (R_ARM_ABS32), passem a la fase de reubiucar
	b .LContinua_recorregut_reubicadors								@; si no �s de tipus 2, passem a rec�rrer el seg�ent reubicador si es que en queden per rec�rrer
	

.LReubicar:
	add r9, r2														@; suma el valor del offset del reubicador amb la @ de dest� en memoria
	sub r9, r1														@; resta la @ d'inici del segment per obtenir la @ absoluta de reubicaci� en memoria
	ldr r11, [r9]													@; obtenim el contingut de la @ absoluta
	cmp r12, #0														@; comprovem si es tractar� el segment de codi a partir de r12 (valor passat pel tercer par�metre)
	beq .LReubicar_segment_codi										@; si �s aix�, tractem el segment de codi
	cmp r11, r12													@; sin�, comprovem que �s cumpleixi que la seva @ de mem�ria sigui superior que la @ del segment de codi
	bge .LReubicar_segment_dades									@; si es compleix, tractarem el segment de dades

.LReubicar_segment_codi:		
	add r11, r2														@; obtenim la @ aboluta de dest� del segment de codi en la memoria, sumant el valor de la @ absoluta de reubicaci� amb la @ de dest� en memoria
	sub r11, r1														@; ajustem la @ absoluta de dest� restant la @ d'inici del segment
	str r11, [r9]													@; guardem la @ de reubicaci� en la memoria
	b .LContinua_recorregut_reubicadors								@; continuem amb el recorregut dels reubicadors

.LReubicar_segment_dades:
	add r11, r6														@; obtenim la @ aboluta de dest� del segment de dades en la memoria, sumant el valor de la @ absoluta de reubicaci� amb la @ de dest� en memoria 
	sub r11, r12													@; ajustem la @ absoluta de dest� restant la @ d'inici del segment
	str r11, [r9]													@; guardem la @ de reubicaci� en la memoria
	b .LContinua_recorregut_reubicadors								@; continuem amb el recorregut dels reubicadors

.LFi_seccio:	
	sub r4, #1														@; decrementem el n�mero d'entrades de la taula de seccions per continuar amb la seg�ent 
	cmp r4, #0
	ble .LFi														@; si r4 <= 0, ja haurem consultat totes les seccions i per tant, acabem amb el recorregut	
	b .LSalt_seguent_seccio											@; si segueix sent m�s gran 0, seguim amb el recorregut de les seccions

.LSalt_seguent_seccio:
	add r3, #40														@; com que la secci� actual no es de tipus 9, saltem directament al valor de sh_type de la seg�ent seccio 
	b .LRecorregut_seccions											@; continuem amb el recorregut de la seg�ent secci�
	
.LContinua_recorregut_reubicadors:
	sub r7, #1														@; decrementem el n�mero de reubicadors per continuar amb la seg�ent (contador)
	cmp r7, #0
	ble .LContinua_recorregut_seccio								@; si r7 <= 0, ja haurem consultat tots els reubicadors i per tant, acabem amb el recorregut	
	add r5, #4														@; si en cara queden per consultar, sumem 4 (sh_offset) per situar-nos a l'estructura de reubicadors del seg�ent reubicador
	b .LRecorregut_reubicadors										@; continuem amb el recorregut de reubicadors

.LContinua_recorregut_seccio:
	sub r4, #1														@; decrementem el n�mero d'entrades de la taula de seccions per continuar amb la seg�ent 
	cmp r4, #0
	ble .LFi														@; si r4 <= 0, ja haurem consultat totes les seccions i, per tant, acabarem amb l'objectiu de reubicar d'aquesta rutina
	add r3, #8														@; si encara queden seccions per consultar, sumem 8 per accedir al tipus de la secci� de la seg�ent (sh_type) 
	b .LRecorregut_seccions
	
.LFi:

	pop {r0-r12,pc}


.global _gm_reservarMem
	@; Rutina para reservar un conjunto de franjas de memoria libres
	@; consecutivas que proporcionen un espacio suficiente para albergar
	@; el tama�o de un segmento de c�digo o datos del proceso (seg�n indique
	@; tipo_seg), asignado al n�mero de z�calo que se pasa por par�metro;
	@; tambi�n se encargar� de invocar a la rutina _gm_pintarFranjas(), para
	@; representar gr�ficamente la ocupaci�n de la memoria de procesos;
	@; la rutina devuelve la primera direcci�n del espacio reservado; 
	@; en el caso de que no quede un espacio de memoria consecutivo del
	@; tama�o requerido, devuelve cero.
	@;Par�metros
	@;	R0: el n�mero de z�calo que reserva la memoria
	@;	R1: el tama�o en bytes que se quiere reservar
	@;	R2: el tipo de segmento reservado (0 -> c�digo, 1 -> datos)
	@;Resultado
	@;	R0: direcci�n inicial de memoria reservada (0 si no es posible)
_gm_reservarMem:
	push {r0-r12, lr}
	
	@; c�lcul de les franjas que necessitem
	push {r0-r3}
	mov r0, r1
	mov r1, #32
	ldr r2, =quo
	ldr r3, =mod
	bl _ga_divmod
	ldr r4, [r2]
	ldr r5, [r3]
	pop {r0-r3}
	
	mov r6, #0	@; contador bucle franjas
	mov r7, #0	@; contador franjas seguides
	ldr r8, =_gm_zocMem
	ldr r9, =NUM_FRANJAS
	
	cmp r5, #0	@; comparem si el residu �s = 0
	beq .LBucle_franjas_ 	@; si ho �s, comencem amb el tractament de les franjas
	add r4, #1 	@; si no ho �s, incrementem una franja de m�s necess�ria  

.LBucle_franjas_:
	cmp r6, r9	@; comparem si s'ha acabat amb el recorregut del vector
	bge .LNo_reserva
	cmp r7, r4
	bge .LFer_reserva	
	ldrb r10, [r8, r6]	
	cmp r10, #0 			
	beq .LFranja_lliure 
	bne .LFranja_ocupada

.LFranja_lliure:
	cmp r7, #0														
	beq .LGuarda_index

.LContinua_franja_lliure:											
	add r7, #1								 
	b .LContinuar_bucle_franjas

.LGuarda_index:														
	mov r11, r6														
	b .LContinua_franja_lliure

.LFranja_ocupada:		
	mov r7, #0	
	b .LContinuar_bucle_franjas

.LContinuar_bucle_franjas:
	add r6, #1
	b .LBucle_franjas_

.LFer_reserva:														
	mov r6, r11														
	add r10, r11, r4
	
.LBucle_reserva:
	cmp r6, r10 			
	bge .LPintar_franjasReserva 
	strb r0, [r8, r6]		 
	add r6, #1 
	b .LBucle_reserva 

.LPintar_franjasReserva:
	push {r1,r2}	@;r0(z�calo) i r3(tipus segment) ja tenen els seus valors corresponents
	mov r1, r11	@; �ndex inicial de la franja
	mov r2, r7	@; n�mero de franjas a pintar
	bl _gs_pintarFranjas
	pop {r1,r2}
	
	mov r1, #32
	ldr r12, =INI_MEM_PROC
	mla r0, r11, r1, r12
	b .LFi_reservarMem

.LNo_reserva:
	mov r0, #0	

.LFi_reservarMem:

	pop {r0-r12, pc}



	.global _gm_liberarMem
	@; Rutina para liberar todas las franjas de memoria asignadas al proceso
	@; del z�calo indicado por par�metro; tambi�n se encargar� de invocar a la
	@; rutina _gm_pintarFranjas(), para actualizar la representaci�n gr�fica
	@; de la ocupaci�n de la memoria de procesos.
	@;Par�metros
	@;	R0: el n�mero de z�calo que libera la memoria
_gm_liberarMem:
	push {lr}


	pop {pc}



	.global _gm_rsiTIMER1
	@; Rutina de Servicio de Interrupci�n (RSI) para actualizar la representa-
	@; ci�n de la pila y el estado de los procesos activos.
_gm_rsiTIMER1:
	push {lr}


	pop {pc}

.end

