@;==============================================================================
@;
@;	"garlic_itcm_mem.s":	código de rutinas de soporte a la carga de
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
	_gm_zocMem:	.space NUM_FRANJAS	@; vector ocupación franjas

.section .itcm,"ax",%progbits

	.arm
	.align 2
	

	.global _gm_reubicar
	@; rutina de soporte a _gm_cargarPrograma(), que interpreta los 'relocs'
	@; de un fichero ELF, contenido en un buffer *fileBuf, y ajustar las
	@; direcciones de memoria correspondientes a las referencias de tipo
	@; R_ARM_ABS32, a partir de las direcciones de memoria destino de código
	@; (dest_code) y datos (dest_data), y según el valor de las direcciones de
	@; las referencias a reubicar y de las direcciones de inicio de los
	@; segmentos de código (pAddr_code) y datos (pAddr_data)
	@;Parámetros:
	@; R0: dirección inicial del buffer de fichero (char *fileBuf)
	@; R1: dirección de inicio de segmento de código (unsigned int pAddr_code)
	@; R2: dirección de destino en la memoria (unsigned int *dest_code)
	@; R3: dirección de inicio de segmento de datos (unsigned int pAddr_data)
	@; (pila): dirección de destino en la memoria (unsigned int *dest_data)
	@;Resultado:
	@; cambio de las direcciones de memoria que se tienen que ajustar
_gm_reubicar:
	push {r0-r12,lr}

	ldr  r6, [SP, #56]												@; R6 -> carreguem el cinquè paràmetre a partir de la suma 56 (4bytes per registre * 14registres pila)
	mov r12, r3														@; R12 -> guardem el valor de r3 passat per paràmetre a r12 per tal de mantenir el codi de la fase1
	ldr r3, [r0, #32]												@; R3 -> carreguem el valor del desplaçament de la taula de seccions a partir del buffer (e_shoff)
	add r3, #4														@; R3 -> incrementem a 4 r3 per situar-nos al tipus de seccio
	ldrh r4, [r0, #48]												@; R4 -> carreguem el número d'entrades de la taula de seccions a partir del buffer (e_shnum)
	
	
.LRecorregut_seccions:
	ldr r5, [r0, r3]												@; R5 -> carreguem el tipus de seccio en la taula de seccions a partir del buffer (sh_type) 
	cmp r5, #9														@; comprovem si el tipus de secció és de tipus 9 (SHT_REC -> secció de reubicadors)
	bne .LFi_seccio													@; si el tipus de secció no és de tipus 9, saltem a la següent secció
	b .LEs_Seccio_reubicadors										@; si el tipus de secció és de tipus 9, continuarem accedint a l'estructura de reubicadors


.LEs_Seccio_reubicadors:	
	add r3, #12														@; accedim a la @ de sh_offset
	ldr r5, [r0, r3]												@; carreguem el valor de sh_offset (estructura reubicadors)
	add r3, #4														@; sumem 4 per accedir a la @ de sh_size (tamany de la secció dins del fitxer)
	@; s'emmagatzemen els registres r0 a r3 i r6 en la pila per preservar l'estat dels registres abans de realitzar la divisó per calcular el número de reubicadors
	push {r0-r3, r6}
	mov r6, r0														@; canviem el registre r0 per r6 per guardar el apuntador al buffer del fitxer .elf
	ldr r0, [r6, r3]												@; del buffer, obtenim el valor de sh_size
	add r3, #16														@; sumem 16 per accedir a la @ de sh_entsize, que ens indica el que ocupa en bytes cada reubicador
	ldr r1, [r6, r3]												@; del buffer obtenim el valor de sh_entsize
	ldr r2, =quo													@; carreguem la @ de memoria del quocient en el registre r2
	ldr r3, =mod													@; carreguem la @ de memoria del modul/residu en el registre r3
	bl _ga_divmod													@; cridem a la rutina _ga_divmod per calcular el número de reubicadors, dividint el sh_size (el tamany de la secció) entre sh_entsize (el que ocupa cada reubicador) 
	ldr r7, [r2]													@; carreguem el resultat del quocient en r7
	ldr r8, [r3]													@; carreguem el resultat del modul/residu en r8
	pop {r0-r3, r6}													@; restaurem els registres r0 a r3 i r6 desde la pila, d'aquesta manera podem seguir utilitzant aquests registres amb els valors inicials passats per paràmetre en aquesta rutina
	
	add r3, #16 													@; tornem a fer la suma de r3 per tal no perdre la @ de memoria a la que apuntava r3 entre el push i el pop
	cmp r7, #0														@; comprovem si la divisió s'ha realitzat correctament
	beq .LContinua_recorregut_seccio								@; si ha hagut algun problema en la divisió (r7=0), passem a recorrer la següent secció si es que en queden per recórrer


.LRecorregut_reubicadors:
	ldr r9, [r0, r5]												@; del buffer obtenim el valor del primer reubicador, el offset de l'estructura de reubicadors (r_offset)
	add r5, #4														@; sumem 4 al primer reubicador (offset) per situar-nos en el tipus de reubicador (r_info)
	ldr r10, [r0, r5]												@; del buffer obtenim el valor de r_info
	and r10, #0xFF													@; apliquem màscara per quedar-nos amb els 8 bits més baixos de r_info que conté el tipus de reubicador que s'ha d'aplicar
	cmp r10, #2														@; comparem el tipus de reubicador amb 2 (R_ARM_ABS32)
	beq .LReubicar													@; si el reubicador és de tipus 2 (R_ARM_ABS32), passem a la fase de reubiucar
	b .LContinua_recorregut_reubicadors								@; si no és de tipus 2, passem a recórrer el següent reubicador si es que en queden per recórrer
	

.LReubicar:
	add r9, r2														@; suma el valor del offset del reubicador amb la @ de destí en memoria
	sub r9, r1														@; resta la @ d'inici del segment per obtenir la @ absoluta de reubicació en memoria
	ldr r11, [r9]													@; obtenim el contingut de la @ absoluta
	cmp r12, #0														@; comprovem si es tractarà el segment de codi a partir de r12 (valor passat pel tercer paràmetre)
	beq .LReubicar_segment_codi										@; si és així, tractem el segment de codi
	cmp r11, r12													@; sinò, comprovem que és cumpleixi que la seva @ de memòria sigui superior que la @ del segment de codi
	bge .LReubicar_segment_dades									@; si es compleix, tractarem el segment de dades

.LReubicar_segment_codi:		
	add r11, r2														@; obtenim la @ aboluta de destí del segment de codi en la memoria, sumant el valor de la @ absoluta de reubicació amb la @ de destí en memoria
	sub r11, r1														@; ajustem la @ absoluta de destí restant la @ d'inici del segment
	str r11, [r9]													@; guardem la @ de reubicació en la memoria
	b .LContinua_recorregut_reubicadors								@; continuem amb el recorregut dels reubicadors

.LReubicar_segment_dades:
	add r11, r6														@; obtenim la @ aboluta de destí del segment de dades en la memoria, sumant el valor de la @ absoluta de reubicació amb la @ de destí en memoria 
	sub r11, r12													@; ajustem la @ absoluta de destí restant la @ d'inici del segment
	str r11, [r9]													@; guardem la @ de reubicació en la memoria
	b .LContinua_recorregut_reubicadors								@; continuem amb el recorregut dels reubicadors

.LFi_seccio:	
	sub r4, #1														@; decrementem el número d'entrades de la taula de seccions per continuar amb la següent 
	cmp r4, #0
	ble .LFi														@; si r4 <= 0, ja haurem consultat totes les seccions i per tant, acabem amb el recorregut	
	b .LSalt_seguent_seccio											@; si segueix sent més gran 0, seguim amb el recorregut de les seccions

.LSalt_seguent_seccio:
	add r3, #40														@; com que la secció actual no es de tipus 9, saltem directament al valor de sh_type de la següent seccio 
	b .LRecorregut_seccions											@; continuem amb el recorregut de la següent secció
	
.LContinua_recorregut_reubicadors:
	sub r7, #1														@; decrementem el número de reubicadors per continuar amb la següent (contador)
	cmp r7, #0
	ble .LContinua_recorregut_seccio								@; si r7 <= 0, ja haurem consultat tots els reubicadors i per tant, acabem amb el recorregut	
	add r5, #4														@; si en cara queden per consultar, sumem 4 (sh_offset) per situar-nos a l'estructura de reubicadors del següent reubicador
	b .LRecorregut_reubicadors										@; continuem amb el recorregut de reubicadors

.LContinua_recorregut_seccio:
	sub r4, #1														@; decrementem el número d'entrades de la taula de seccions per continuar amb la següent 
	cmp r4, #0
	ble .LFi														@; si r4 <= 0, ja haurem consultat totes les seccions i, per tant, acabarem amb l'objectiu de reubicar d'aquesta rutina
	add r3, #8														@; si encara queden seccions per consultar, sumem 8 per accedir al tipus de la secció de la següent (sh_type) 
	b .LRecorregut_seccions
	
.LFi:

	pop {r0-r12,pc}


.global _gm_reservarMem
	@; Rutina para reservar un conjunto de franjas de memoria libres
	@; consecutivas que proporcionen un espacio suficiente para albergar
	@; el tamaño de un segmento de código o datos del proceso (según indique
	@; tipo_seg), asignado al número de zócalo que se pasa por parámetro;
	@; también se encargará de invocar a la rutina _gm_pintarFranjas(), para
	@; representar gráficamente la ocupación de la memoria de procesos;
	@; la rutina devuelve la primera dirección del espacio reservado; 
	@; en el caso de que no quede un espacio de memoria consecutivo del
	@; tamaño requerido, devuelve cero.
	@;Parámetros
	@;	R0: el número de zócalo que reserva la memoria
	@;	R1: el tamaño en bytes que se quiere reservar
	@;	R2: el tipo de segmento reservado (0 -> código, 1 -> datos)
	@;Resultado
	@;	R0: dirección inicial de memoria reservada (0 si no es posible)
_gm_reservarMem:
	push {r1-r12,lr}
	
	@; càlcul de les franjas que necessitem, dividint el tamany en bytes que es necessita reservar per 32 bytes de cada franja
	push {r0-r3}
	mov r0, r1
	mov r1, #32
	ldr r2, =quo
	ldr r3, =mod
	bl _ga_divmod
	ldr r4, [r2]													@; emmagatzemem en r4, el resultat de la divisió (número de franjas que necessitem reservar)
	ldr r5, [r3]													@; emmagatzemem en r5 el residu de la divisió
	pop {r0-r3}
	
	mov r6, #0														@; contador bucle franjas
	mov r7, #0														@; contador franjas seguides
	ldr r8, =_gm_zocMem												@; vector _gm_zocMem per gestionar memoria 
	ldr r9, =NUM_FRANJAS											@; número de posicions que conté el vector 
	
	cmp r5, #0														@; comparem si el residu és = 0
	beq .LBucle_franjas_ 											@; si ho és, comencem amb el tractament de les franjas
	add r4, #1 														@; si no ho és, incrementem una franja de més necessària  

.LBucle_franjas_:													@; tractament de les franjes del vector _gm_zocMem
	cmp r6, r9														@; comparem si s'ha acabat amb el recorregut del vector
	bge .LNo_reserva 												@; si el contador del bucle >= NUM_FRANJAS, haurà tractat totes les posicions del vector sense fer cap reserva de memòria
	cmp r7, r4														@; comprovem si ja es poden reservar les franjas necessàries
	bge .LFer_reserva												@; si el número de franjas requerides es poden reservar de manera consecutiva, podrem fer la reserva de memòria
	ldrb r10, [r8, r6]												@; guardem en r10, el valor de la posició del contador del bucle r6, del vector _gm_zocMem	
	cmp r10, #0 													@; si en aquesta posició es troba un 0
	beq .LFranja_lliure 											@; si es troba un 0, la franja actual estarà lliure
	bne .LFranja_ocupada											@; si es troba un valor != de 0, la franja actual estarà ocupada

.LFranja_lliure:													
	cmp r7, #0														@; primer, comprovarem si el contador de franjas seguidas és = 0 										
	beq .LGuarda_index												@; si és igual a 0, voldrà dir que es comença amb una nova seqüència de franjas seguides, per tal guardarem la posició on comença

.LContinua_franja_lliure:											@; una vegada guardada la posició actual, continuem										
	add r7, #1														@; incrementem el contador de franjas seguidas					 
	b .LContinuar_bucle_franjas										@; saltem per incrementar l'índex del bucle i continuar amb el tractament de les franjas

.LGuarda_index:														
	mov r11, r6														@; guarda l'índex inicial on comença la successió de franjas consecutives dins del vector _gm_zocMem													
	b .LContinua_franja_lliure 										@; continuem amb la gestió de la franja lliure

.LFranja_ocupada:		
	mov r7, #0														@; fiquem a 0 el contador de franjas consecutives per resetearlo
	b .LContinuar_bucle_franjas 									@; saltem per incrementar l'índex del bucle i continuar amb el tractament de les franjas

.LContinuar_bucle_franjas:
	add r6, #1 														@; incrementem el contador del bucle de les franjas
	b .LBucle_franjas_ 												@; continuem amb el bucle

.LFer_reserva:		 												@; quan contador de franjas consecutives és igual al número de requerides, farem la reserva de memòria												
	mov r6, r11														@; guardarem el valor de la posició on comença la seqüència de les franjes consecutives a r6 (índex actual de bucle)										
	add r10, r11, r4 												@; en r10, guardarem l'última franja del vector que es necessita al fer la reserva (anirà de la posició del valor r6 al de r10)
	
.LBucle_reserva: 													@; bucle per gestionar la reserva de les franjas del vector _gm_zocMem
	cmp r6, r10 	 												@; comparem el contador del bucle r6 amb la posició final de l'última franja		
	bge .LPintar_franjasReserva  									@; si r6 >= r10, ja s'haurà fet la reserva de totes les franjas i per tant, passarem a fer la representació d'aquesta reserva de franjas
	strb r0, [r8, r6]	 											@; si encara no s'ha arribat al final, guardarem en la posició de contador del bucle r6 del vector, el valor del zócalo a tractar passat per paràmetre	 
	add r6, #1 														@; incrementarem el contador del bucle de la reserva
	b .LBucle_reserva 												@; continuarem amb el bucle de la reserva

.LPintar_franjasReserva: 											@; si s'ha acabat amb la gestió de la reserva de franjas, farem la representació
	push {r1,r2}													@; s'emmagatzemen els registres r1 i r2 en la pila per preservar l'estat dels registres abans de modificar els valors dels registres esmentats per realitzar la crida a _gm_pintarFranjas
	@; r0(zócalo) i r3(tipus segment) ja tenen els seus valors corresponents
	mov r1, r11														@; guardem en r1 l'índex inicial de la franja
	mov r2, r7														@; guardem en r2 el número de franjas (consecutives) a pintar
	bl _gs_pintarFranjas 											@; cridem a la rutina _gs_pintarFranjas
	pop {r1,r2}														@; restaurem els registres r1 i r2 desde la pila
	@; retornarem a r0, el valor de la posició de memòria on s'han reservat les franjes consecutives
	mov r1, #32 													@; guardem en r1 el valor de 32 (32 bytes per franja)
	ldr r12, =INI_MEM_PROC 											@; guardem en r12 el valor inicial de la posició de la memòria on comencen els processos d'usuari emmagatzemats
	mla r0, r11, r1, r12 											@; calculem a r0 = (índex incial * 32 bytes) + INI_MEM_PROC
	b .LFi_reservarMem 												@; acabem amb la rutina _gm_reservarMem

.LNo_reserva: 														@; si no s'ha pogut fer la reserva (quan el contador del bucle inicial >= NUM_FRANJAS)
	mov r0, #0	 													@; guardem en r0 un 0 per retornar-lo, indicant que no s'ha pogut realitzar la reserva de memòria

.LFi_reservarMem:	 												@; finalitzem la rutina de _gm_reservarMem

	pop {r1-r12,pc}



	.global _gm_liberarMem
	@; Rutina para liberar todas las franjas de memoria asignadas al proceso
	@; del zócalo indicado por parámetro; también se encargará de invocar a la
	@; rutina _gm_pintarFranjas(), para actualizar la representación gráfica
	@; de la ocupación de la memoria de procesos.
	@;Parámetros
	@;	R0: el número de zócalo que libera la memoria
_gm_liberarMem:
	push {r0-r7,lr}
	
	ldr r1, =_gm_zocMem												@; carreguem el vector _gm_zocMem per gestionar memoria que ocupa cada segment		
	mov r2, #0														@; índex bucle per fer el recorregut pel vector
	ldr r5, =NUM_FRANJAS											@; carreguem el número de posicions que conté el vector
	mov r4, #0														@; contador del número de franjas a pintar
	
.LBucle_franjas:													@; bucle incial
	ldrb r3, [r1, r2]												@; carreguem el valor de la posició del vector r2 al registre r3
	mov r7, r2														@; r7 -> posició del índex inicial de la franja
	cmp r3, r0														@; comparem el valor carregat anteriorment del vector amb el zócalo
	beq .LContar_num_franjas										@; si el valor carregat del vector és igual al número del zócalo a tractar, comencem a contar les franjes consecutives
	bne .LComprovar_bucle_franjas 									@; sinó, fem una comprovació per seguir amb el recorregut del vector

.LContar_num_franjas:												@; bucle quan el es cumpleix la primera comparació de r3 == r0
	add r4, #1														@; contador del número de franjas a pintar ho incrementem a 1
	mov r6, #0														@; r6 = 0, per guardar un 0 a la posició de r2 != de 0 (mateix número del zocalo)
	strb r6, [r1, r2]												@; emmagatzemem el 0 anterior a la posició actual del vector a tractar per treure de la franja del vector el zócalo en ús
	b .LComprovar_bucle_num_franjas									@; seguirem amb el recorregut contant el número de franjas a pintar mentre sigui igual que el zócalo a tractar

.LContinuar_bucle_num_franjas:										@; continuació del bucle .LContar_num_franjas una vegada comprovat que l'índex del bucle és més petit que el número de franjas
	ldrb r3, [r1, r2] 												@; carreguem el valor de la posició del vector r2 al registre r3									
	cmp r3, r0														@; comparem el valor carregat anteriorment del vector amb el zócalo
	beq .LContar_num_franjas										@; si els valors són iguals, continuem amb el bucle per tal d'incrementar el contador del número de franjas a pintar						
	bne .LPintar_franjas											@; sinó, passem a preparar els registres per passar-li correctament els valors a la rutina _gm_pintarFranjas			

.LPintar_franjas:
	push {r0-r3}													@; s'emmagatzemen els registres r0 a r3 en la pila per preservar l'estat dels registres abans de modificar els valors dels registres esmentats per realitzar la crida a _gm_pintarFranjas
	mov r0, #0														@; número de zocalo				
	mov r1, r7														@; índex incial de la franja
	mov r2, r4														@; número de franjas a pintar
	mov r3, #0														@; tipus de segment (volem eliminar tant el de codi com el de dades, per tant podem passar-li 0 o 1)			
	bl _gs_pintarFranjas											@; fem un branch with link a la rutina _gs_pintarFranjas
	pop {r0-r3}														@; restaurem els registres r0 a r3 desde la pila, d'aquesta manera podem seguir utilitzant aquests registres amb els valors que tenien inicials 
	b .LComprovar_bucle_franjas										@; comprovarem si podem seguir amb el tractament del vector de franjas una vegada hem acabat de tractar el primer segment
	
.LComprovar_bucle_num_franjas:										@; comprovació per seguir amb el tractament quan r3 == r0
	add r2, #1														@; incrementem el valor de l'índex de la posició a tractar del vector
	cmp r2, r5														@; comparem aquest índex amb el valor de NUM_FRANJAS
	bge .LFiLiberarMem												@; si r2>=NUM_FRANJAS, finalitzem amb la rutina ja que ja haurem tractat tot el vector
	b .LContinuar_bucle_num_franjas									@; sinó, continuarem amb el tractament quan r3 == r0
	
.LComprovar_bucle_franjas:											@; comprovació per seguir amb el tractament quan r3 era diferent de r0
	add r2, #1														@; incrementem el valor de l'índex de la posició a tractar del vector 
	cmp r2, r5														@; comparem aquest índex amb el valor de NUM_FRANJAS
	bge .LFiLiberarMem												@; si r2>=NUM_FRANJAS, finalitzem amb la rutina ja que ja haurem tractat tot el vector
	b .LBucle_franjas												@; sinó, continuarem amb el tractament quan r3 ha sigut diferent de r0

.LFiLiberarMem:														@; finalitzem rutina de _gm_liberarMem	

	pop {r0-r7,pc}



	.global _gm_rsiTIMER1
	@; Rutina de Servicio de Interrupción (RSI) para actualizar la representa-
	@; ción de la pila y el estado de los procesos activos.
_gm_rsiTIMER1:
	push {lr}


	pop {pc}

.end

