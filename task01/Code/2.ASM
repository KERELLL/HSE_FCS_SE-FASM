include 'win32ax.inc'

.code

  start:

        invoke  MessageBox,HWND_DESKTOP,"Want to know about me?",invoke GetCommandLine,MB_YESNO

        .if eax = IDYES
                invoke  MessageBox,HWND_DESKTOP,"I am the example program on assembler","Info",MB_OK
        .endif

        invoke  ExitProcess,0

.end start