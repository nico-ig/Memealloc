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
  pushq %r15
  
  movq %rdi, %r12                 #%r12 has the size to be alloc

  call current_brk
  movq %rax, %r13                 #%r13 has the current brk
  movq original_brk(%rip), %r14   #%r14 has the start address

  _VERIFY_ADDR:                   #compare the current address with current brk,
    cmpq %r13, %r14               #verify if is necessary to alloc more space in heap
    jge _ALLOC_HEAP

    cmpq $1, (%r14)               #verify if current address is allocated
    je _NEXT_ADDR                 #current address is already allocated
                                  
                                  #current address is free
    cmp %r12, +8(%r14)            #verify if current block is big enough
    jl _NEXT_ADDR                 #cannot fit in current block

    movq +8(%r14), %r15           #%r15 has the current block size
    subq $17, %r15                #min size so it can be split in two
    cmp %r12, %r15                #verify if the remaining size is enough
    jl _DONT_SPLIT                #block cannot be split in two
    
                                  #block will be split in two    
    movq %r14, %r15               #%r15 has a copy of current address
    addq %r12, %r15               #%r15 is the start of remaining block after split
    movq $0, (%r15)               #flag remaining block as free

    movq +8(%r14), %r13           #%r13 has the total block size
    subq %r12, %r13               #%r13 has the total block size - requested size
    subq $16, %r13                #%r13 has the remaining size after split and block header
    movq %r13, +8(%r15)           #save the size for the remainig block

    jmp _ALLOC_END                #save requested block

    _DONT_SPLIT:
      movq +8(%r14), %r12         #requested block will have the same size as free block
      jmp _ALLOC_END              #save requested block

    _NEXT_ADDR:
      addq +8(%r14), %r14           
      addq $16, %r14
      jmp _VERIFY_ADDR

    _ALLOC_HEAP:                  #current address is at the end of current heap
      addq %r12, %r13             #%r13 has the current brk + the size to be alloc
      addq $16, %r13              #%r13 has the current brk + total size

      movq %r13, %rdi             #alloc the new size in heap
      movq $12, %rax
      syscall

  _ALLOC_END:
    movq $1, (%r14)               #flag the block as being used 
    movq %r12, 8(%r14)            #save the size 
    addq $16, %r14                #return address is after the block header
    movq %r14, %rax               #return the address 
 
  popq %r15                       #restore preserved registers
  popq %r14
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
