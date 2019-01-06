	.file 	1	"all.c"
	.rdata
	.align	2

$LC0:
	.ascii	"Hello World!\000"
	.align	2
	
$LC1:
	.ascii	"%d\000"
	.align	2
$LC2:
	.ascii	"a = %d\012\000"
	.align	2
$LC3:
	.ascii	"d = %d\012\000"
	.align	2
$LC4:
	.ascii	"e = %d\012\000"
	.text
	.align	2
	.globl	main
	.ent	main
	.type	main, @function
main:
	.frame	$30,56,$31		
	.mask	0xc0000000,-4
	.fmask	0x00000000,0	
	addiu	$29,$29,-56
	sw	$31,52($29)
	sw	$30,48($29)
	move	$30,$29
	lui	$28,%hi(__gnu_local_gp) #7ff0
	addiu	$28,$28,%lo(__gnu_local_gp)
	.cprestore	16
	#	sw  $gp, 16($sp)
	li	$2,1000			
	sw	$2,36($30)
	li	$2,24			
	sw	$2,32($30)
	lw	$3,36($30)
	lw	$2,32($30)
	addu	$2,$3,$2
	sw	$2,28($30)
	lw	$2,32($30)
	sll	$2,$2,1
	sw	$2,24($30)
	lui	$2,%hi($LC0)
	addiu	$4,$2,%lo($LC0)
	lw	$25,%call16(puts)($28)
	jalr	$25
	nop
	lw	$28,16($30)
	addiu	$3,$30,40
	lui	$2,%hi($LC1)
	addiu	$4,$2,%lo($LC1)
	move	$5,$3
	lw	$25,%call16(scanf)($28)
	jalr	$25
	nop
	lw	$28,16($30)
	lw	$3,40($30)
	lui	$2,%hi($LC2)
	addiu	$4,$2,%lo($LC2)
	move	$5,$3
	lw	$25,%call16(printf)($28)
	jalr	$25
	nop
	lw	$28,16($30)
	lui	$2,%hi($LC3)
	addiu	$4,$2,%lo($LC3)
	lw	$5,28($30)
	lw	$25,%call16(printf)($28)
	jalr	$25
	nop
	lw	$28,16($30)
#	lui	$2,%hi(268533760)
	lui	$2,%hi($LC4)
	addiu	$4,$2,%lo($LC4)
#	addiu 	$4,$2,%lo(268533760)
	lw	$5,24($30)
	lw	$25,%call16(printf)($28)
	jalr	$25
	nop
	lw	$28,16($30)
	move	$2,$0
	move	$29,$30
	lw	$31,52($29)
	lw	$30,48($29)
	addiu	$29,$29,56
	j	$31
	nop
	.end	main

