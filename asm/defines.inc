%define STDIN 0
%define STDOUT 1
%define STDERR 2

%define SYS_WRITE 1
%define SYS_EXIT 60

; =============================================================================
; Debug print to print the value that is stored in rax
; Before Usage rax should be safed if the value has to stay the same
; =============================================================================
debug_print:
    mov rcx, digitSpace
    mov rbx, 10 ; newline
    mov [rcx], rbx
    inc rcx
    mov [digitSpacePos], rcx

_printRAXLoop:
    mov rdx, 0 ; weird concat roule

    mov rbx, 10
    div rbx

    push rax
    add rdx, 48

    mov rcx, [digitSpacePos]
    mov [rcx], dl ; move current retrieved char into digitspace
    inc rcx
    mov [digitSpacePos], rcx

    pop rax
    cmp rax, 0
    jne _printRAXLoop
    
    ; print the digitspace backwards

_printRAXLoop2:
    mov rcx, [digitSpacePos]
    mov rax, 1
    mov rdi, 1
    mov rsi, rcx
    mov rdx, 1
    syscall     ; write a single byte

    mov rcx, [digitSpacePos]
    dec rcx
    mov [digitSpacePos], rcx

    ; keep printing until at the beginning of the digitspace
    cmp rcx, digitSpace
    jge _printRAXLoop2

    ret
; =============================================================================
; End Debug print
; =============================================================================
