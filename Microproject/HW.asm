format PE Console

entry start
                    
include 'win32a.inc'

;-----------------------------------------------------
; �������: ������ ������ ��������
; ������: 191
;
; ������� 21:
; ����������� ��������� ���������� �����
; ����������� �� ������������ �������
; ������ ���������������� � ��������� ��
; ���� 0,05% (������������ FPU)
;-----------------------------------------------------

section '.data' data readable writeable

x         dq ?;�������� ������������� ��������:
res1      dq ?
res2      dq ?
const2    dq 2.0;��������� ������ 2
buf       db 256 dup(0);��� ������ ����� ���� double
inxct     dd 0.0005;�������� 0.05%
msg1      db 'f=sqrt(x), x>=0',10,'Enter x: ',0
wrng      db 'Wrong x!',13,10,0
outDouble db '%lf',0
geronStr  db 'Geron sqrt(x) = %lg',13,10,0
sqrtStr   db 'sqrt(x) = %lg',13,10,0
inxctStr  db 'Inexactness: %lg'

section '.code' code readable executable
start:
        ccall [printf],msg1                 ;������ ������������ ������ �����
        ccall [gets],buf                    ;��������� ������ � ������
        ccall [sscanf],buf,outDouble,x     ;����������� ������ � double
        
        ;��������� ������� �� ������������� ������ � double
        cmp eax,1               
        jz main

        
wrongMsg:;������� ��������� �� ������
        ccall [printf],wrng
        jmp start                 ;������ ������������ ������ ����� ��� ���

main:     ;��������� ������������ ���������� �����
        fld [x]          ;��������� ������������� �����
        fldz              ;0
        fcompp            ;���������� 0 � ��������� ������
        fstsw   ax        ;�������� �����
        sahf              ;��������� �� � ����� ����������
        ja wrongMsg       ;0>x, �� ������ ������������ ������ x ��� ���

        ;�������� � ���� x
        fld qword [x]
        sub esp, 8
        fstp qword [esp]
        call sqrt         ;��������� ������ �������� sqrt(x)
        add esp, 8        ;������� ���� �� ���������� ����������

        fst [res1]        ;��������� ��������� ���������� � res1

        ;������� ��������� sqrt(x)
        sub esp, 8
        fstp qword [esp]
        push sqrtStr      ;������ ���������
        call [printf]     ;������� ���������
        add esp, 12       ;������� ���� �� ���������� ����������

        ;�������� � ���� �������� ����������
        fld [inxct]
        sub esp, 8
        fstp qword [esp]
        ;�������� � ���� �������� x
        fld qword [x]
        sub esp, 8
        fstp qword [esp]
        call geronSqrt    ;��������� �������� sqrt �� ������� ������
        add esp, 16       ;������� ���� �� ���������� ����������

        fst [res2]        ;��������� ��������� ��������� � res2

        ;������� �������� ���������� � �������
        sub esp, 8
        fstp qword [esp]
        push geronStr     ;������ ���������
        call [printf]
        add esp, 12       ;������� ���� �� ���������� ����������

        ;������� �����������
        fld [res1]        ;�������� �������� res1
        fld [res2]        ;�������� �������� res2
        fsubp st1, st     ;��������� �� ��������
        fabs              ;�������� ������ �� ��������

        ;������� �����������
        sub esp, 8
        fstp qword [esp]
        push inxctStr     ;������ ���������
        call [printf]     ;������� �����������
        add esp, 12       ;������� ���� �� ���������� ����������

        ccall [_getch]    ;�������� ������� ����� �������
        
        stdcall [ExitProcess], 0  ;���������� ������ ���������


;----------��������-GeronSqrt----------------------------------------
; ��������� �������� sqrt(x) � ������������ inxct � ��������� ���
; � st(0)
; ��������� �������:
  xl      equ ebp+8   ;�������� x
  inxctl  equ ebp+16  ;�����������
;
;-----------GeronSqrt(double x, double inxct)------------------------

;���������� ��������� ����������:
xi      equ ebp-8   ;�������� x_i
xp      equ ebp-16  ;�������� x_(i-1

geronSqrt:
        push ebp                ;��������� �������� �������� ebp
        mov ebp,esp
        sub esp,16              ;������� ���� ��� ��������� ����������

;����������� ��������
        fldz
        fstp qword [xp]         ;xp = 0
        fld1
        fstp qword [xi]         ;xi = 1

geronLoop:
        fld qword [xl]          ;������ x
        fdiv qword [xi]         ;x/xi
        fadd qword [xi]         ;x/xi + xi
        fdiv [const2]           ;(x/xi + xi)/2
        fst qword [xi]          ;xi = (x/xi + xi)/2
        fld qword [xp]
        fsubp st1, st           ;xi - xp
        fabs                    ;|xi - xp|
        fld qword [xi]
        fstp qword [xp]         ;���������� xi � xp
        fcomp qword [inxctl]    ;�������� |xi - xp| c eps
        fstsw ax;               ;��������� ����� ��������� � ��
        sahf;                   ;������� ax � ����� ����������
        jnb geronLoop;          ;e��� |xi - xp|>=inxctl, ���������� ����

        fld qword [xi]
        leave
ret
;--------------------------------------------------------------------

;-----------��������-Sqrt--------------------------------------------
; ��������� ������ �������� sqrt(x) � ��������� ��� � st(0)
;-----------Sqrt(double x)-------------------------------------------
sqrt:
        push ebp                 ;��������� �������� �������� ebp
        mov ebp,esp
        fld qword [ebp+8];x
        fsqrt                    ;sqrt(1-x^2)
        pop ebp                  ;���������� �������� �������� ebp
ret
;--------------------------------------------------------------------

section '.idata' import data readable

library kernel,'kernel32.dll',\
        user,'user32.dll',\
        msvcrt,'msvcrt.dll'

import  kernel,\
        ExitProcess,'ExitProcess'

import  msvcrt,\
        sscanf,'sscanf',\
        gets,'gets',\
        _getch,'_getch',\
        printf,'printf'