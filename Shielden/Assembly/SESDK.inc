; ****************************************************************************
; Safengine SDK
; ****************************************************************************

SE_UNPROTECT_START               equ <db 0EBh,009h,053h,045h,055h,050h,042h,045h,047h,04Eh,000h>

SE_UNPROTECT_END                 equ <db 0EBh,009h,053h,045h,055h,050h,045h,04Eh,044h,050h,000h>

SE_PROTECT_START                 equ <db 0EBh,007h,053h,045h,042h,045h,047h,04Eh,000h>

SE_PROTECT_START_MUTATION        equ <db 0EBh,007h,053h,045h,042h,045h,047h,04Eh,04Dh>

SE_PROTECT_START_VIRTUALIZATION  equ <db 0EBh,007h,053h,045h,042h,045h,047h,04Eh,056h>

SE_PROTECT_START_ULTRA           equ <db 0EBh,007h,053h,045h,042h,045h,047h,04Eh,055h>

SE_PROTECT_END                   equ <db 0EBh,007h,053h,045h,045h,04Eh,044h,050h,000h>