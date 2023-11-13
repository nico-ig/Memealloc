.section .bss
.global original_brk
.lcomm original_brk, 8

.section .text

.global setup_brk
setup_brk:
  pushq %rbp
  movq %rsp, %rbp

  movq $0, %rdi
  movq $12, %rax
  syscall
  movq %rax, original_brk(%rip)

  popq %rbp
  ret
  
.global dismiss_brk
dismiss_brk:
  pushq %rbp
  movq %rsp, %rbp

  movq original_brk(%rip), %rdi
  movq $12, %rax
  syscall
  
  popq %rbp
  ret

.global memory_alloc
memory_alloc:
  pushq %rbp
  movq %rsp, %rbp
  
  movq %rdi, %r8                  #%r8 has the size to be alloc

  movq $0, %rdi
  movq $12, %rax
  syscall
  movq %rax, %r9                  #%r9 has the current brk
  movq original_brk(%rip), %r10   #%r10 has the start address

                                  #compare the current address with current brk,
  cmp %r9, %r10                   #verify if is necessary to alloc more space in heap
  je _ALLOC_HEAP

  _ALLOC_HEAP:                    #current address is at the end of current heap
    addq %r8, %r9                 #%r9 has the current brk + the size to be alloc
    addq $16, %r9                 #%r9 has the current brk + total size

    movq %r9, %rdi                #alloc the new size in heap
    movq $12, %rax
    syscall

    movq $1, (%r10)               #flag the block as being used 
    movq %r8, 8(%r10)             #save the size 
    addq $16, %r10                #return address is after the block header
    movq %r10, %rax               #return the address 
 
  popq %rbp
  ret

.global memory_free
memory_free:
  pushq %rbp
  movq %rsp, %rbp
  
  popq %rbp
  ret
