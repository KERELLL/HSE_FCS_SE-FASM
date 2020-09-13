include 'win32ax.inc'
 
.data
 buf db 'TEST'
 bufsize = $ - buf
 byteswritten dd ?
.code
 main:
 invoke  CreateFile, 'test.txt', GENERIC_WRITE, 0, 0, 4, FILE_ATTRIBUTE_NORMAL, 0
 invoke  WriteFile, eax, buf, bufsize, byteswritten, 0
 invoke  ExitProcess, 0
 
.end main