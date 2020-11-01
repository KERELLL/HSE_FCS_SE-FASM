format PE Console

entry start
                    
include 'win32a.inc'

;-----------------------------------------------------
; Выпонил: Рычков Кирилл Павлович
; Группа: 191
;
; Вариант 21:
; Разработать программу вычисления корня
; квадратного по итерационной формуле
; Герона Александрийского с точностью не
; хуже 0,05% (использовать FPU)
;-----------------------------------------------------

section '.data' data readable writeable

x         dq ?;Введённое пользователем значение:
res1      dq ?
res2      dq ?
const2    dq 2.0;константа равная 2
buf       db 256 dup(0);для чтения числа типа double
inxct     dd 0.0005;Точность 0.05%
msg1      db 'f=sqrt(x), x>=0',10,'Enter x: ',0
wrng      db 'Wrong x!',13,10,0
outDouble db '%lf',0
geronStr  db 'Geron sqrt(x) = %lg',13,10,0
sqrtStr   db 'sqrt(x) = %lg',13,10,0
inxctStr  db 'Inexactness: %lg'

section '.code' code readable executable
start:
        ccall [printf],msg1                 ;просим пользователя ввести число
        ccall [gets],buf                    ;считываем строку с числом
        ccall [sscanf],buf,outDouble,x     ;преобразуем втроку в double
        
        ;проверяем удалось ли преобразовать строку в double
        cmp eax,1               
        jz main

        
wrongMsg:;выводим сообщение об ошибке
        ccall [printf],wrng
        jmp start                 ;просим пользователя ввести число еще раз

main:     ;проверяем корректность введенного числа
        fld [x]          ;введенное пользователем число
        fldz              ;0
        fcompp            ;сравниваем 0 с введенным числом
        fstsw   ax        ;получаем флгаи
        sahf              ;переносим их в флаги процессора
        ja wrongMsg       ;0>x, то просим пользователя ввести x еще раз

        ;передаем в стек x
        fld qword [x]
        sub esp, 8
        fstp qword [esp]
        call sqrt         ;вычислить точное значение sqrt(x)
        add esp, 8        ;очищаем стек от переданных параметров

        fst [res1]        ;сохраняем результат вычислений в res1

        ;выводим результат sqrt(x)
        sub esp, 8
        fstp qword [esp]
        push sqrtStr      ;Формат сообщения
        call [printf]     ;выводим результат
        add esp, 12       ;очищаем стек от переданных параметров

        ;передаем в стек точность вычисления
        fld [inxct]
        sub esp, 8
        fstp qword [esp]
        ;передаем в стек значение x
        fld qword [x]
        sub esp, 8
        fstp qword [esp]
        call geronSqrt    ;вычисляет значение sqrt по формуле Герона
        add esp, 16       ;очищаем стек от переданных параметров

        fst [res2]        ;сохраняем результат вычилений в res2

        ;выводим резултат вычислений в консоль
        sub esp, 8
        fstp qword [esp]
        push geronStr     ;формат сообщения
        call [printf]
        add esp, 12       ;очищаем стек от переданных параметров

        ;находим погрешность
        fld [res1]        ;получаем значение res1
        fld [res2]        ;получаем значение res2
        fsubp st1, st     ;вычисляем их разность
        fabs              ;получаем модуль их разности

        ;выводим погрешность
        sub esp, 8
        fstp qword [esp]
        push inxctStr     ;Формат сообщения
        call [printf]     ;выводим погрешность
        add esp, 12       ;очищаем стек от переданных параметров

        ccall [_getch]    ;ожидание нажатия любой клавиши
        
        stdcall [ExitProcess], 0  ;завершение работы программы


;----------Описание-GeronSqrt----------------------------------------
; Вычисляет значение sqrt(x) с погрешностью inxct и сохраняет его
; в st(0)
; Параметры функции:
  xl      equ ebp+8   ;значение x
  inxctl  equ ebp+16  ;погрешность
;
;-----------GeronSqrt(double x, double inxct)------------------------

;Объявление локальных переменных:
xi      equ ebp-8   ;значение x_i
xp      equ ebp-16  ;значение x_(i-1

geronSqrt:
        push ebp                ;сохраняем значение регистра ebp
        mov ebp,esp
        sub esp,16              ;создаем месо под локальные переменные

;Вычисленное значение
        fldz
        fstp qword [xp]         ;xp = 0
        fld1
        fstp qword [xi]         ;xi = 1

geronLoop:
        fld qword [xl]          ;грузим x
        fdiv qword [xi]         ;x/xi
        fadd qword [xi]         ;x/xi + xi
        fdiv [const2]           ;(x/xi + xi)/2
        fst qword [xi]          ;xi = (x/xi + xi)/2
        fld qword [xp]
        fsubp st1, st           ;xi - xp
        fabs                    ;|xi - xp|
        fld qword [xi]
        fstp qword [xp]         ;записываем xi в xp
        fcomp qword [inxctl]    ;сравнить |xi - xp| c eps
        fstsw ax;               ;перенести флаги сравнения в ах
        sahf;                   ;занести ax в флаги процессора
        jnb geronLoop;          ;eсли |xi - xp|>=inxctl, продолжить цикл

        fld qword [xi]
        leave
ret
;--------------------------------------------------------------------

;-----------Описание-Sqrt--------------------------------------------
; Вычисляет точное значение sqrt(x) и сохраняет его в st(0)
;-----------Sqrt(double x)-------------------------------------------
sqrt:
        push ebp                 ;сохраняем значение регистра ebp
        mov ebp,esp
        fld qword [ebp+8];x
        fsqrt                    ;sqrt(1-x^2)
        pop ebp                  ;возвращаем значение регистра ebp
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