@;==============================================================================
@;
@;	"garlic_itcm_mem.s":	c�digo de rutinas de soporte a la carga de
@;							programas en memoria (version 1.0)
@;
@;==============================================================================

.section .dtcm,"wa",%progbits
		
		.align 2
		.global quo
	quo:	.word 0
		.global mod
	mod:	.word 0

.section .itcm,"ax",%progbits

	.arm
	.align 2
	

	.global _gm_reubicar
	@; rutina para interpretar los 'relocs' de un fichero ELF y ajustar las
	@; direcciones de memoria correspondientes a las referencias de tipo
	@; R_ARM_ABS32, restando la direcci�n de inicio de segmento y sumando
	@; la direcci�n de destino en la memoria;
	@;Par�metros:
	@; R0: direcci�n inicial del buffer de fichero (char *fileBuf)
	@; R1: direcci�n de inicio de segmento (unsigned int pAddr)
	@; R2: direcci�n de destino en la memoria (unsigned int *dest)
	@;Resultado:
	@; cambio de las direcciones de memoria que se tienen que ajustar
_gm_reubicar:
	push {r0-r12,lr}

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
	@; s'emmagatzemen els registres r0 a r3 en la pila per preservar l'estat dels registres abans de realitzar la divis� per calcular el n�mero de reubicadors
	push {r0-r3}
	mov r6, r0														@; canviem el registre r0 per r6 per guardar el apuntador al buffer del fitxer .elf
	ldr r0, [r6, r3]												@; del buffer, obtenim el valor de sh_size
	add r3, #16														@; sumem 16 per accedir a la @ de sh_entsize, que ens indica el que ocupa en bytes cada reubicador
	ldr r1, [r6, r3]												@; del buffer obtenim el valor de sh_entsize
	ldr r2, =quo													@; carrguem la @ de memoria del quocient en el registre r2
	ldr r3, =mod													@; carrguem la @ de memoria del modul/residu en el registre r3
	bl _ga_divmod													@; cridem a la rutina _ga_divmod per calcular el n�mero de reubicadors, dividint el sh_size (el tamany de la secci�) entre sh_entsize (el que ocupa cada reubicador) 
	ldr r7, [r2]													@; carreguem el resultat del quocient en r7
	ldr r8, [r3]													@; carreguem el resultat del modul/residu en r8
	pop {r0-r3}														@; restaurem els registres r0 a r3 desde la pila, d'aquesta manera podem seguir utilitzant aquests registres amb els valors inicials passats per par�metre en aquesta rutina
	
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
	add r11, r2														@; obtenim la @ aboluta de dest� en la memoria, sumant el valor de la @ absoluta de reubicaci� amb la @ de dest� en memoria
	sub r11, r1														@; ajustem la @ absoluta de dest� restant la @ d'inici del segment
	str r11, [r9]													@; guardem la @ de reubicaci� en la memoria
	b .LContinua_recorregut_reubicadors

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


.end

