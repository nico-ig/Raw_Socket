
# s0 = indice inicial
# s1 = indice final
# s2 = tam

.data
	vetor: .space 24 					# Espaço em bits para 6 int, cada int tem 4 bits
	espaco: .ascii " "					# É um espaco
	
.text
	main:
		li $s2, 6					# Tamanho do vetor
		li $s0, 0					# Indice inicial do vetor					
		addi $s1, $s2, -1			# s1 é o indice final, s1 = tam - 1
	
		move $a1, $s2				# passa o tam como paramentro
		jal leVetor					# le um vetor
		
								# Chama o selection Sort
		move $a1, $s0					# ai = indice inicial
		move $a2, $s1					# a2 = indice final
		jal selectionSort
		
		move $a1, $s0					# a1 = indice	
		move $a2, $s2					# a2 = tamanho
		jal imprimeVetor

								# Termina o programa
		li $v0, 10
		syscall


								# Função que lê um vetor, não retorna nada
	leVetor:
	
								# Configura a pilha
		addiu $sp, $sp, -12				# Aloca 12 bytes na pilha (a, b e ra)
		sw $ra, 0($sp)					# 1 posicao da pilha = ra
		sw $s0, 4($sp)					# 2 posicao da pilha = a
		sw $s1, 8($sp)					# 3 posicao da pilha = b
		
		li $t0, 0					# t0 é o índice, inicia em 0
		li $t1, 0					# t1 é o contador, t1 = 0
		move $t2, $a1				# t2 = tam
									# lê s2 elementos
		le_loop: 
			beq $t1, $t2, endLe 			# Se cont = tam sai do loop
			
			li $v0, 5				# Instrução 5 lê um int
			syscall
			sw $v0, vetor($t0)			# vetor[0] = $v0
	
			addi $t0, $t0, 4 			# Atualiza o indice, 4 pq um int tem 4 bits
			addi $t1, $t1, 1			# Atualiza o contador
				
			j le_loop				# Volta para o inicio do loop
		endLe:
		
								# Recupera os valores
		lw $ra, 0($sp) 					# Recupera o ra
		lw $s0, 4($sp)					# Recupera o a
		lw $s1, 8($sp)					# Recupera o b
		addiu $sp, $sp, 12				# Volta o sp
		
		jr $ra						# Retorna para onde a função foi chamada

				
								# Função que imprime um vetor, não retorna nada
	imprimeVetor:
	
								# Configura a pilha
		addiu $sp, $sp, -16				# Aloca 16 bytes na pilha (tam, a, b e ra)
		sw $ra, 0($sp)					# 1 posicao da pilha = ra
		sw $s0, 4($sp)					# 2 posicao da pilha = a
		sw $s1, 8($sp)					# 3 posicao da pilha = b
		sw $s2, 12($sp)					# 4 posica da pilha = tam
	
		li $t0, 0					# t0 é o contador, t1 = 0
		li $t1, 0					# t1 é o índice, inicia em 0
		
								# imprime t0 elementos
		imprime_loop: 
			beq $t0, $a2, endImprime		# Se cont = tam sai do loop
			
			lw $a0, vetor($t1)			# a0 = vetor[t1]
			li $v0, 1				# instrução 1 imprime a0
			syscall	
				
			la $a0, espaco 				# Imprime um espaço
			li $v0, 4				# instrução 1 imprime a0
			syscall
				
			addi $t0, $t0, 1			# Atualiza o contador
			addi $t1, $t1, 4 			# Atualiza o indice, 4 pq um int tem 4 bits
				
			j imprime_loop				# Volta para o inicio do loop
		endImprime: 
		
								# Recupera os valores
		lw $ra, 0($sp) 					# Recupera o ra
		lw $s0, 4($sp)					# Recupera o a
		lw $s1, 8($sp)					# Recupera o b
		lw $s2, 12($sp)					# Recupera o tam
		addiu $sp, $sp, 16				# Volta o sp
		
		jr $ra						# Retorna para onde a função foi chamada
		
	
								# Ordena em ordem crescente
	selectionSort:
		
								# Configura a pilha
		addiu $sp, $sp, -16				# Aloca 12 bytes na pilha (a, b e ra)
		sw $ra, 0($sp)					# 1 posicao da pilha = ra
		sw $s0, 4($sp)					# 2 posicao da pilha = a
		sw $s1, 8($sp)					# 3 posicao da pilha = b
		sw $s2, 12($sp)					# 4 posica da pilha = tam
		
		move $t0, $a1					# Libera os registradores de parametro
		move $t1, $a2
		bge $t0, $t1, fimSelection			# se a >= b sai da recursão
																				
		jal minimo					# Acha o menor valor
								# Faz a troca
		move $a1, $v0					# a3 = min (v0)
		move $a2, $t0					# a2 = a
		jal troca				
								# chama a recursão para o prox elem	
		addi $t0, $t0, 1 				# a++				
		jal selectionSort					
			
		fimSelection:
		
								# Recupera os valores
		lw $ra, 0($sp) 					# Recupera o ra
		lw $s0, 4($sp)					# Recupera o a
		lw $s1, 8($sp)					# Recupera o b
		lw $s2, 12($sp)					# Recupera o tam
		addiu $sp, $sp, 16				# Volta o sp
		
	jr $ra							# Retorna para onde a função foi chamada
		
									
	minimo:							# Acha o minimo do vetor
								# Configura a pilha
		addiu $sp, $sp, -16				# Aloca 16 bytes na pilha (tam, a, b e ra)
		sw $ra, 0($sp)					# 1 posicao da pilha = ra
		sw $s0, 4($sp)					# 2 posicao da pilha = a
		sw $s1, 8($sp)					# 3 posicao da pilha = b
		sw $s2, 12($sp)					# 4 posicao da pilha = tam
		
		move $t0, $a1 					# t0 é o índice
		sll $t0, $t0, 2					# t0 = t0*4 
		lw $t1, vetor($t0)				# t1 = vetor[t0*4]
		li $t2, 0					# t2 é o contador, t2 = 0	
		
		min_loop:					# Acha o menor valor
		
		beq $t2, $a2, fimMin				# Se cont == b sai do loop
			
		addi $t0, $t0, 4				# Atualiza o indice, um int tem 4 bits
		addi $t2, $t2, 1				# Atualiza o contador, contador++
				
		lw $t3, vetor($t0) 				# t3 = vetor[t0], comeca no 2º elem do vetor
		li $t4, 0					# indice do menor valor, comeca em 0
		bgt $t3, $t1, fimMin_if 			# se t3 > t1 sai do if
			move $t1, $t3				# se t3 < t1 t0 = t3 (achou o min)
			move $t4, $t0
		fimMin_if:
				
		j min_loop					# repete para o prox elem
			
		fimMin:
	
								# Recupera os valores
		lw $ra, 0($sp) 					# Recupera o ra
		lw $s0, 4($sp)					# Recupera o a
		lw $s1, 8($sp)					# Recupera o b
		lw $s2, 12($sp)					# Recupera o tam
		addiu $sp, $sp, 16				# Volta o sp
		
		move $v0, $t4 					# retorna o indice do menor valor (t0), v0 = t0
		jr $ra						# Retorna para onde a função foi chamada
		
								# Troca dois elementos
	troca: 
								# Configura a pilha
		addiu $sp, $sp, -16				# Aloca 16 bytes na pilha (tam, a, b e ra)
		sw $ra, 0($sp)					# 1 posicao da pilha = ra
		sw $s0, 4($sp)					# 2 posicao da pilha = a
		sw $s1, 8($sp)					# 3 posicao da pilha = b
		sw $s2, 12($sp)					# 4 posicao da pilha = tam
		
								# Guarda os valores
		lw $t0, vetor($a1)				# t0 = vetor[a1]
		lw $t1, vetor($a2)				# t1 = vetor[a2]
		
								# Troca os valores
		sw $t1, vetor($a1)				# vetor[a3] = t1
		sw $t0, vetor($t2)				# vetor[a1] = t0

								# Recupera os valores
		lw $ra, 0($sp) 					# Recupera o ra
		lw $s0, 4($sp)					# Recupera o a
		lw $s1, 8($sp)					# Recupera o b
		lw $s2, 12($sp)					# Recupera o tam
		addiu $sp, $sp, 16				# Volta o sp
		
		jr $ra						# Retorna para onde a função foi chamada
	
# fazer para mais de 6 elementos
# ler com espaço ao inves de enter
