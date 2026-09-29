BITS 32

section .text

global context_switch

;
; void context_switch(
;     uint32_t *old_esp,
;     uint32_t new_esp
; );
;

context_switch:

    ;
    ; Get arguments BEFORE changing ESP.
    ;
    mov eax, [esp + 4]
    mov edx, [esp + 8]

    ;
    ; Save registers.
    ;
    pushad

    ;
    ; Save flags.
    ;
    pushfd

    ;
    ; Save current stack pointer.
    ;
    mov [eax], esp

    ;
    ; Switch to next process stack.
    ;
    mov esp, edx

    ;
    ; Restore flags.
    ;
    popfd

    ;
    ; Restore registers.
    ;
    popad

    ;
    ; Return into next process.
    ;
    ret
