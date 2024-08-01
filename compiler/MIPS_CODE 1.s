
.data
_nl: .asciiz "\n"
_ml: .asciiz "collatz("
_m2: .asciiz ") completed after "
_m3: .asciiz " calls to collatz_line().\n\n"
_space: .asciiz " "

.globl 	collatz_line
.globl 	collatz

.text
# println: print out an integer followed by a newline
print1:

addi $v0, $zero, 4   # System call for print string
la $a0, _ml        # Load address of newline character into $a0
syscall            # Execute syscall (prints newline)
jr $ra             # Return to caller

print2:

addi $v0, $zero, 4   # System call for print string
la $a0, _m2        # Load address of newline character into $a0
syscall            # Execute syscall (prints newline)
jr $ra             # Return to caller

print3:

addi $v0, $zero, 4   # System call for print string
la $a0, _m3        # Load address of newline character into $a0
syscall            # Execute syscall (prints newline)
jr $ra             # Return to caller

# println: print out an integer followed by a newline
println:

addi $v0, $zero, 4   # System call for print string
la $a0, _nl          # Load address of newline character into $a0
syscall              # Execute syscall (prints newline)
jr $ra               # Return to caller

# printSpace: print out a space
printSpace:

addi $v0, $zero, 4   # System call for print string
la $a0, _space       # Load address of space character into $a0
syscall              # Execute syscall (prints space)
jr $ra               # Return to caller

# printn: print out an integer
printn:

addi $v0, $zero, 1   # System call for print integer
syscall              # Execute syscall (prints the integer in $a0)
jr $ra               # Return to caller

printc:

addi $v0, $zero, 11   # System call for print string
syscall            # Execute syscall (prints newline)
jr $ra             # Return to caller



collatz_line:
	addiu $sp, $sp, -72
	sw $31, 68($sp)
	sw $fp, 64($sp)
	sw $s0, 60($sp)
	sw $s1, 56($sp)
	sw $s2, 52($sp)
	sw $s3, 48($sp)
	sw $s4, 44($sp)
	sw $s5, 40($sp)
	sw $s6, 36($sp)
	sw $s7, 32($sp)
	addu $fp, $sp, $zero
	sw $4, 24($fp)
	lw $s0, 24($fp)
	addi $s1, $zero, 2

	# Remainder operation
	div $s0, $s1
	mfhi $s2

	addi $s0, $zero, 1

	# Set equal operation
	subu $s1, $s2, $s0
	# Assume $s1 holds the value to be checked if it's zero
	# We need to set $s1 to 1 if it is zero, otherwise set it to 0

	addi $t0, $zero, 1    # Load 1 into $t0
	slt $s1, $zero, $s1   # Set $s1 to 1 if 0 < $s1 (i.e., if $s1 is not zero)
	subu $s1, $t0, $s1    # Subtract the result from 1, effectively inverting it
	# Now $s1 = 1 if it was originally 0, and $s1 = 0 if it was non-zero


	beq $s1, $zero, one
	lw $s0, 24($fp)
	addu $4, $s0, $zero
	lw $s0, 24($fp)
	addu $t9, $s0, $zero
one				:
	lw $s0, 24($fp)
	addu $4, $s0, $zero
	jal printn
	lw $s0, 24($fp)
	sw $s0, 20($fp)
	beq $zero, $zero, three
two:
	lw $s0, 20($fp)
	addi $s1, $zero, 2
	div $s0, $s1
	mflo $s2
	sw $s2, 20($fp)
	jal printSpace
	lw $s0, 20($fp)
	addu $4, $s0, $zero
	jal printn
three:
	lw $s0, 20($fp)
	addi $s1, $zero, 2

	# Remainder operation
	div $s0, $s1
	mfhi $s2

	addi $s0, $zero, 0

	# Set equal operation
	subu $s1, $s2, $s0
	# Assume $s1 holds the value to be checked if it's zero
	# We need to set $s1 to 1 if it is zero, otherwise set it to 0

	addi $t0, $zero, 1    # Load 1 into $t0
	slt $s1, $zero, $s1   # Set $s1 to 1 if 0 < $s1 (i.e., if $s1 is not zero)
	subu $s1, $t0, $s1    # Subtract the result from 1, effectively inverting it
	# Now $s1 = 1 if it was originally 0, and $s1 = 0 if it was non-zero


	addi $t8, $zero, 1
	beq $s1, $t8, two
	jal println
	lw $s0, 20($fp)
	addu $t9, $s0, $zero
	addu $sp, $fp, $zero
	lw $31, 68($sp)
	lw $fp, 64($sp)
	lw $s0, 60($sp)
	lw $s1, 56($sp)
	lw $s2, 52($sp)
	lw $s3, 48($sp)
	lw $s4, 44($sp)
	lw $s5, 40($sp)
	lw $s6, 36($sp)
	lw $s7, 32($sp)
	addiu $sp, $sp, 72
	jr $31


collatz:
	addiu   $sp, $sp, -76
	sw  $31, 72($sp)
	sw  $fp, 68($sp)
	sw  $s0, 64($sp)
	sw  $s1, 60($sp)
	sw  $s2, 56($sp)
	sw  $s3, 52($sp)
	sw  $s4, 48($sp)
	sw  $s5, 44($sp)
	sw  $s6, 40($sp)
	sw  $s7, 36($sp)
	addu    $fp, $sp, $zero
	sw  $4, 28($fp)
	lw  $s0, 28($fp)
	sw  $s0, 24($fp)
	addi    $s0, $zero, 0
	sw  $s0, 20($fp)
	lw  $s0, 24($fp)
	addu    $4, $s0, $zero
	jal collatz_line
	addu    $s0, $t9, $zero
	sw  $s0, 24($fp)
	beq $zero, $zero, loop_start
loop_body:
	addi    $s0, $zero, 3
	lw  $s1, 24($fp)
	mul $s2, $s0, $s1
	addi    $s0, $zero, 1
	addu    $s1, $s2, $s0
	sw  $s1, 24($fp)
	lw  $s0, 24($fp)
	addu    $4, $s0, $zero
	jal collatz_line
	addu    $s0, $t9, $zero
	sw  $s0, 24($fp)
	lw  $s0, 20($fp)
	addi    $s1, $zero, 1
	addu    $s2, $s0, $s1
	sw  $s2, 20($fp)
loop_start:
	addi    $s0, $zero, 1
	lw  $s1, 24($fp)
	subu    $t0, $s1, $s0
	addi $t1, $zero, 1  
	slt $s2, $t0, $t1  
	addi    $t8, $zero, 1
	bne $s2, $t8, loop_body
	jal print1
	lw  $s0, 28($fp)
	addu    $4, $s0, $zero
	jal printn
	jal print2
	lw  $s0, 20($fp)
	addu    $4, $s0, $zero
	jal printn
	jal print3
	addu    $sp, $fp, $zero
	lw  $31, 72($sp)
	lw  $fp, 68($sp)
	lw  $s0, 64($sp)
	lw  $s1, 60($sp)
	lw  $s2, 56($sp)
	lw  $s3, 52($sp)
	lw  $s4, 48($sp)
	lw  $s5, 44($sp)
	lw  $s6, 40($sp)
	lw  $s7, 36($sp)
	addiu   $sp, $sp, 76
	jr  $31
	
	
	
main:
	# fill the sX registers (and fp) with junk.  Each testcase will use a different
	# set of values.
	lui   $fp,      0x1111
	ori   $fp, $fp, 0x1111
	lui   $s0,      0x2222
	ori   $s0, $s0, 0x2222
	lui   $s1,      0x3333
	ori   $s1, $s1, 0x3333
	lui   $s2,      0x4444
	ori   $s2, $s2, 0x4444
	lui   $s3,      0x5555
	ori   $s3, $s3, 0x5555
	lui   $s4,      0x6666
	ori   $s4, $s4, 0x6666
	lui   $s5,      0x7777
	ori   $s5, $s5, 0x7777
	lui   $s6,      0x8888
	ori   $s6, $s6, 0x8888
	lui   $s7,      0x9999
	ori   $s7, $s7, 0x9999

	# instead of explicitly dumping the stack pointer, I'll push a dummy
	# variable onto the stack.  Some students are reporting different
	# stack values in their output.
	lui   $t0,     0xaaaa
	ori   $t0, $t0,0xaaaa
	addiu $sp, $sp,-4
	sw    $t0, 0($sp)



	# collatz_line(1)
	addi    $a0, $zero, 9            # arg1
        jal     collatz_line
        add     $t0, $v0,$zero

.data
TESTCASE_MSG1:		.asciiz "collatzLine() returned: "
.text
	addi    $v0, $zero,4              # print_str(MSG1)
	la      $a0, TESTCASE_MSG1
	syscall

	addi    $v0, $zero,1              # print_int(retval)
	add     $a0, $t0,$zero
	syscall

	addi    $v0, $zero,11             # print_str('\n');
	addi    $a0, $zero,0xa
	syscall



	# dump out all of the registers.