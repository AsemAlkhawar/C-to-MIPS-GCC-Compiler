.align 2
.data
_nl: .asciiz "\n"
.align 2
.text
# println: print out an integer followed by a newline
println:
li $v0, 1          # System call for print integer
syscall            # Execute syscall (prints the integer in $a0)

li $v0, 4          # System call for print string
la $a0, _nl        # Load address of newline character into $a0
syscall            # Execute syscall (prints newline)

jr $ra             # Return to caller
letterTree:
	addiu	$sp, $sp, -84
	sw	$31, 80($sp)
	sw	$fp, 76($sp)
	sw	$s0, 72($sp)	# Store $s0 at offset 72 from the stack pointer
	sw	$s1, 68($sp)	# Store $s1 at offset 68 from the stack pointer
	sw	$s2, 64($sp)	# Store $s2 at offset 64 from the stack pointer
	sw	$s3, 60($sp)	# Store $s3 at offset 60 from the stack pointer
	sw	$s4, 56($sp)	# Store $s4 at offset 56 from the stack pointer
	sw	$s5, 52($sp)	# Store $s5 at offset 52 from the stack pointer
	sw	$s6, 48($sp)	# Store $s6 at offset 48 from the stack pointer
	sw	$s7, 44($sp)	# Store $s7 at offset 44 from the stack pointer
	move	$fp, $sp
	sw	$4, 36($fp)
	li	$s0, 0
	sw	$s0, 32($fp)
	li	$s0, 0
	sw	$s0, 28($fp)
	lw	$s0, 28($fp)
	move	$4,$s0
	jal	getNextLetter
	move	$s0, $t9
	sw	$s0, 20($fp)
	b	$L2
$L3:
	li	$s0, 0
	sw	$s0, 24($fp)
	b	$L4
$L5:
	lw	$s0, 20($fp)
	move	$4,$s0
	jal	printc
	lw	$s0, 24($fp)
	li	$s1, 1
	addu	$s2,$s0,$s1
	sw	$s2, 24($fp)
$L4:
	lw	$s0, 32($fp)
	lw	$s1, 24($fp)
	sle	$s2, $s1, $s0
	li	$t8, 1
	beq	$s2, $t8, $L5
	jal	println
	lw	$s0, 32($fp)
	li	$s1, 1
	addu	$s2,$s0,$s1
	sw	$s2, 32($fp)
	lw	$s0, 28($fp)
	lw	$s1, 36($fp)
	addu	$s2,$s0,$s1
	sw	$s2, 28($fp)
	lw	$s0, 28($fp)
	move	$4,$s0
	jal	getNextLetter
	move	$s0, $t9
	sw	$s0, 20($fp)
$L2:
	li	$s0, 0
	lw	$s1, 20($fp)
	sne	$s2, $s1, $s0
	li	$t8, 1
	beq	$s2, $t8, $L3
	lw	$s0, 28($fp)
	move	$t9, $s0
	move	$sp, $fp
	lw	$31, 80($sp)
	lw	$fp, 76($sp)
	lw	$s0, 72($sp)	# Store $s0 at offset 72 from the stack pointer
	lw	$s1, 68($sp)	# Store $s1 at offset 68 from the stack pointer
	lw	$s2, 64($sp)	# Store $s2 at offset 64 from the stack pointer
	lw	$s3, 60($sp)	# Store $s3 at offset 60 from the stack pointer
	lw	$s4, 56($sp)	# Store $s4 at offset 56 from the stack pointer
	lw	$s5, 52($sp)	# Store $s5 at offset 52 from the stack pointer
	lw	$s6, 48($sp)	# Store $s6 at offset 48 from the stack pointer
	lw	$s7, 44($sp)	# Store $s7 at offset 44 from the stack pointer
	addiu	$sp, $sp, 84
	jr	$31
.data

