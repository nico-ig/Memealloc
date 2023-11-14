.section .bss
.global original_brk
.lcomm original_brk, 8

.section .text

.global setup_brk
setup_brk:
  pushq %rbp
  movq %rsp, %rbp

  call current_brk
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
  
  pushq %r12                      #save preserved registers
  pushq %r13
  pushq %r14
  
  movq %rdi, %r12                 #%r12 has the size to be alloc

  call current_brk
  movq %rax, %r13                 #%r13 has the current brk
  movq original_brk(%rip), %r14   #%r14 has the start address

                                  #compare the current address with current brk,
  cmp %r13, %r14                  #verify if is necessary to alloc more space in heap
  je _ALLOC_HEAP

  _ALLOC_HEAP:                    #current address is at the end of current heap
    addq %r12, %r13               #%r13 has the current brk + the size to be alloc
    addq $16, %r13                #%r13 has the current brk + total size

    movq %r13, %rdi               #alloc the new size in heap
    movq $12, %rax
    syscall

    movq $1, (%r14)               #flag the block as being used 
    movq %r12, 8(%r14)            #save the size 
    addq $16, %r14                #return address is after the block header
    movq %r14, %rax               #return the address 
 
  popq %r14                       #restore preserved registers
  popq %r13
  popq %r12

  popq %rbp
  ret

.global memory_free
memory_free:
  pushq %rbp
  movq %rsp, %rbp
  pushq %r12                    #save preserved register
 
  movq %rax, %r12               #r12 has the address to be free
  movq original_brk(%rip), %r9
  addq $16, %r9                 #address should be after the first block header

  cmp %r9, %r12                 #verify if address is after original brk
  jl _FREE_END                  #cant free address

  call current_brk              #%rax has current brk
  cmp %rax, %r12                #verify if address is before current brk
  jg _FREE_END                  #cant free address
  
  subq $16, %r12                #flag is 16 bytes before received address
  movq $0, (%r12)               #set flag to 0

  _FREE_END:
    popq %r12                   #restore preserved register
    popq %rbp
    ret

.global current_brk
current_brk:
  pushq %rbp
  movq %rsp, %rbp
  
  movq $0, %rdi
  movq $12, %rax
  syscall

  popq %rbp
  ret
