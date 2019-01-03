	AREA BOOT_RESET, CODE, READONLY
	DCD	0x20000800
	DCD boot_reset
	SPACE 0xc0-0x08
	AREA	|.text|, CODE, READONLY
boot_reset PROC
	EXPORT boot_reset
	IMPORT boot_main

	LDR r0, =boot_main
	BX r0
	ENDP
	END
