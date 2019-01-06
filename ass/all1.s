	.file	1 "all.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 1
	.abicalls
	.rdata
    .align  2
$LC0:
    .ascii  "Hello World!\012\000"
    .align  2
$LC1:
    .ascii  "%d\000"
    .align  2
$LC2:
    .ascii  "a = %d\012\000"
    .align  2
$LC3:
    .ascii  "d = %d\012\000"
    .text
	.align	2
	.globl	main
	.ent	main
	.type	main, @function
main:
	.set	nomips16
	.frame	$30,72,$31
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro

	addiu	$29,$29,-72
    sw  $31,68($29)
	sw	$30,64($29)
	move	$30,$29
	lui	$28,%hi(__gnu_local_gp)
	addiu	$28,$28,%lo(__gnu_local_gp)
	.cprestore	16
	li	$2,1000
	sw	$2,32($30)
	li	$2,24
	sw	$2,28($30)
	lw	$3,32($30)
	lw	$2,28($30)
	addu	$2,$3,$2
	sw	$2,24($30)
	lui	$2,%hi($LC0)
	addiu	$4,$2,%lo($LC0)
	lw	$25,%call16(printf)($28)
	jalr	$25
	nop
	lw	$28,16($30)
    addiu   $3,$30,36
	lui	$2,%hi($LC1)
	addiu	$4,$2,%lo($LC1)
	move	$5,$3
	lw	$25,%call16(scanf)($28)
	jalr	$25
	nop
	lw	$28,16($30)
    lw  $3,36($30)
	lui	$2,%hi($LC2)
	addiu	$4,$2,%lo($LC2)
	move	$5,$3
	lw	$25,%call16(printf)($28)
	jalr	$25
	nop
	lw	$28,16($30)
    lw  $3,24($30)
	lui	$2,%hi($LC3)
	addiu	$4,$2,%lo($LC3)
	move	$5,$3
	lw	$25,%call16(printf)($28)
	jalr	$25
	nop
	lw	$28,16($30)
	move	$2,$0
	move	$sp,$30
	lw	$31,68($29)
	lw	$30,64($29)
	addiu	$29,$29,72
	j	$31
	nop
	.set	macro
	.set	reorder
	.end	main
	.ident	"GCC: (GNU) 4.3.6 20101004 (prerelease)"
