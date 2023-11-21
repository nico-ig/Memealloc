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
  
  movq %rdi, %r12                 #%r12 has the biggest size found
  movq %rdi, %r15                 #%r15 has the requested size
  movq $0,  %r13                  #%r13 has the start of biggest block so far

  call current_brk                #%rax has the current brk
  movq original_brk(%rip), %r14   #%r14 has the start address

  _VERIFY_ADDR:                   #compare the current address with current brk,
    cmpq %rax, %r14               #verify if reached end of heap
    jge _END_OF_HEAP

    cmpq $1, (%r14)               #verify if current address is allocated
    je _NEXT_ADDR                 #current address is already allocated
                                  
                                  #current address is free
    cmpq %r12, +8(%r14)           #compare if current block size is bigger
    jle _NEXT_ADDR                #current block is not bigger

                                  #update biggest block so far, %r12 is the new
                                  #size we are trying to find a bigger block
    movq +8(%r14), %r12           #update the biggest size 
    movq %r14, %r13               #update the start of the biggest block
                                  #always check the next addr
    _NEXT_ADDR:
      addq +8(%r14), %r14           
      addq $16, %r14
      jmp _VERIFY_ADDR

    _END_OF_HEAP:                 #end of heap reached
      cmpq $0, %r13               #verify if no block was found
      jne _BLOCK_FOUND              

                                  #no block found, alloc new block at the end of heap
      movq %r14, %r13             #save the end of heap, it's the start of new block
      addq %r15, %rax             #%rax has the current brk + the size to be alloc
      addq $16, %rax              #%rax has the current brk + total size

      movq %rax, %rdi             #alloc the new size in heap
      movq $12, %rax
      syscall
      jmp _ALLOC_END

      _BLOCK_FOUND:
        subq $17, %r12            #min size so it can be split in two
        cmp %r15, %r12            #verify if the remaining size is enough
        jl _DONT_SPLIT            #block cannot be split in two

                                  #block will be split in two    
        movq %r14, %rbx           #%rbx has a copy of current address
        addq $16, %rbx                
        addq %r15, %rbx           #%rbx is the start of remaining block after split
        movq $0, (%rbx)           #flag remaining block as free

        movq +8(%r14), %r12       #%r13 has the total block size
        subq %r13, %r12           #%r13 has the total block size - requested size
        subq $16, %r12            #%r13 has the remaining size after split and block header
        movq %r12, +8(%rbx)       #save the size for the remainig block

        jmp _ALLOC_END            #save requested block

      _DONT_SPLIT:
        movq +8(%r13), %r15       #requested block will have the same size as free block
        jmp _ALLOC_END            #save requested block

  _ALLOC_END:
    movq $1, (%r13)               #flag the block as being used 
    movq %r15, +8(%r13)           #save the size 
    addq $16, %r13                #return address is after the block header
    movq %r13, %rax               #return the address 
 
  popq %r15                       #restore preserved register
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
  pushq %r13
 
  movq %rax, %r12               #r12 has the address to be free
  movq original_brk(%rip), %r9
  addq $16, %r9                 #address should be after the first block header

  movq $0, %r13                 #%r13 will be the return value
  cmp %r9, %r12                 #verify if address is after original brk
  jl _FREE_END                  #cant free address

  call current_brk              #%rax has current brk
  cmp %rax, %r12                #verify if address is before current brk
  jg _FREE_END                  #cant free address
  
  subq $16, %r12                #flag is 16 bytes before received address
  movq $0, (%r12)               #set flag to 0
  movq $1, %r13

  _FREE_END:
    movq %r13, %rax             #return value
    popq %r13                   #restore preserved register
    popq %r12
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
