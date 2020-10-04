include 'win32ax.inc'
.code
        main:
         invoke  MessageBox,NULL,'Hello, World!','Fasm message box:',MB_OK
         invoke  ExitProcess,0
.end main
