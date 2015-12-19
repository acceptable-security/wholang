.global blah
blah:
PUSH %EBP
MOVLL %ESP, %EBP
MOV 8(%EBP), %EAX
MOV %EAX, -4(%EBP)
POP %EBP
RET
.global _main
_main:
PUSH %EBP
MOVL %ESP, %EBP
PUSH $3
CALL blah
ADD $4, %ESP
MOV %EAX, -8(%EBP)
MOV $0, %EAX
POP %EBP
RET
