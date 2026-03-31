# the float is passed as a parameter, and its convert to integer through the instruction "cvtss2si"

    .section .text
    .globl float_to_int
    .type float_to_int,

float_to_int:

cvtss2si %rax, %rdi  

ret

     .size float_to_int, .-float_to_int
