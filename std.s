ord:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	16(%rbp), %rax
	movzbl	(%rax), %eax
	popq	%rbp
	ret

print:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $48, %rsp
    movq    16(%rbp), %rbx
    jmp     .PL1
.PL0:
    movzbl   (%rbx), %ecx
	call	 putchar
    inc      %rbx
.PL1:
    cmpb    $0, (%rbx)
    jne     .PL0
    addq    $48, %rsp
    popq    %rbp
    ret

tgetchar:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	call	getchar
	movb	%al, -1(%rbp)
	movl	$1, %edx
	movl	$2, %ecx
	call	calloc
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rdx
	movzbl	-1(%rbp), %eax
	movb	%al, (%rdx)
	movq	-16(%rbp), %rax
	addq	$1, %rax
	movb	$0, (%rax)
	movq	-16(%rbp), %rax
	addq	$48, %rsp
	popq	%rbp
	ret

chr:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
	cmpl	$0, 16(%rbp)
	js	.CL4
	cmpl	$127, 16(%rbp)
	jle	.CL5
.CL4:
	movl	$1, %ecx
	call	exit
.CL5:
	movl	$1, %edx
	movl	$2, %ecx
	call	calloc
	movq	%rax, -8(%rbp)
	movl	16(%rbp), %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movb	%dl, (%rax)
	movq	-8(%rbp), %rax
	addq	$1, %rax
	movb	$0, (%rax)
	movq	-8(%rbp), %rax
	addq	$48, %rsp
	popq	%rbp
	ret

size:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	movq	16(%rbp), %rcx
	call	strlen
	addq	$32, %rsp
	popq	%rbp
	ret


substring:
	pushq	%rbp
	pushq	%rbx
	subq	$56, %rsp
	leaq	128(%rsp), %rbp
	movq	%rcx, -48(%rbp)
	movl	%edx, -40(%rbp)
	movl	%r8d, -32(%rbp)
	movl	-40(%rbp), %edx
	movl	-32(%rbp), %eax
	addl	%edx, %eax
	movslq	%eax, %rbx
	movq	-48(%rbp), %rcx
	call	strlen
	cmpq	%rax, %rbx
	jbe	.SL2
	movl	$1, %ecx
	call	exit
.SL2:
	movl	-32(%rbp), %eax
	addl	$1, %eax
	cltq
	movl	$1, %edx
	movq	%rax, %rcx
	call	calloc
	movq	%rax, -96(%rbp)
	movl	$0, -84(%rbp)
	jmp	.SL3
.SL4:
	movl	-40(%rbp), %edx
	movl	-84(%rbp), %eax
	addl	%edx, %eax
	cltq
	movq	-48(%rbp), %rdx
	addq	%rdx, %rax
	movl	-84(%rbp), %edx
	movslq	%edx, %rdx
	movq	-96(%rbp), %rcx
	addq	%rcx, %rdx
	movzbl	(%rax), %eax
	movb	%al, (%rdx)
	addl	$1, -84(%rbp)
.SL3:
	movl	-84(%rbp), %eax
	cmpl	-32(%rbp), %eax
	jl	.SL4
	movl	-32(%rbp), %eax
	cltq
	movq	-96(%rbp), %rdx
	addq	%rdx, %rax
	movb	$0, (%rax)
	movq	-96(%rbp), %rax
	addq	$56, %rsp
	popq	%rbx
	popq	%rbp
	ret
   
concat:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$64, %rsp
	movq	24(%rbp), %rax
	movq	%rax, %rcx
	call	strlen
	movl	%eax, -12(%rbp)
	movq	16(%rbp), %rcx
	call	strlen
	movl	%eax, -16(%rbp)
	movl	-12(%rbp), %edx
	movl	-16(%rbp), %eax
	addl	%edx, %eax
	addl	$1, %eax
	cltq
	movl	$1, %edx
	movq	%rax, %rcx
	call	calloc
	movq	%rax, -24(%rbp)
	movl	$0, -4(%rbp)
	jmp	.LO2
.LO3:
	movl	-4(%rbp), %eax
	cltq
	movq	24(%rbp), %rdx
	addq	%rdx, %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	movq	-24(%rbp), %rcx
	addq	%rcx, %rdx
	movzbl	(%rax), %eax
	movb	%al, (%rdx)
	addl	$1, -4(%rbp)
.LO2:
	movl	-4(%rbp), %eax
	cmpl	-12(%rbp), %eax
	jl	.LO3
	movl	$0, -8(%rbp)
	jmp	.LO4
.LO5:
	movl	-8(%rbp), %eax
	cltq
	movq	16(%rbp), %rdx
	addq	%rdx, %rax
	movl	-8(%rbp), %ecx
	movl	-12(%rbp), %edx
	addl	%ecx, %edx
	movslq	%edx, %rdx
	movq	-24(%rbp), %rcx
	addq	%rcx, %rdx
	movzbl	(%rax), %eax
	movb	%al, (%rdx)
	addl	$1, -8(%rbp)
.LO4:
	movl	-8(%rbp), %eax
	cmpl	-16(%rbp), %eax
	jl	.LO5
	movq	-24(%rbp), %rax
	addq	$64, %rsp
	popq	%rbp
	ret
