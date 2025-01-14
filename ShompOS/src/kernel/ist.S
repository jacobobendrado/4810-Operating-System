# ist.S
# Table of Interrupt Service Routines
# Cedarville University 2024-25 OSDev Team

.include "src/kernel/isr.S"
.global isr_stub_table
isr_stub_table:
    .long isr_0 
    .long isr_stub_1
    .long isr_2
    .long isr_stub_3
    .long isr_4
    .long isr_5
    .long isr_stub_6
    .long isr_stub_7
    .long isr_8
    .long isr_stub_9
    .long isr_10
    .long isr_11
    .long isr_12
    .long isr_stub_13
    .long isr_stub_14
    .long isr_stub_15
    .long isr_16
    .long isr_stub_17
    .long isr_stub_18
    .long isr_19
    .long isr_stub_20
    .long isr_stub_21
    .long isr_stub_22
    .long isr_stub_23
    .long isr_stub_24
    .long isr_stub_25
    .long isr_stub_26
    .long isr_stub_27
    .long isr_stub_28
    .long isr_stub_29
    .long isr_stub_30
    .long isr_stub_31
