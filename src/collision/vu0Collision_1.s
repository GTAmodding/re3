QuitAndFail:
	NOP[E]                                   IADDIU VI01, VI00, 0
	NOP                                      NOP


QuitAndSucceed:
	NOP[E]                                   IADDIU VI01, VI00, 1
	NOP                                      NOP


; 20 -- unused
; VF12, VF13 xyz: sphere centers
; VF14, VF15 x: sphere radii
; out:
;  VI01: set when collision
;  VF01: supposed to be intersection point?
;  VF02: normal (pointing towards s1, not normalized)
.globl Vu0SphereToSphereCollision
Vu0SphereToSphereCollision:
	SUB.xyz VF02, VF13, VF12                 NOP                     ; dist of centers
	ADD.x VF04, VF14, VF15                   NOP                     ; s = sum of radii
	MUL.xyzw VF03, VF02, VF02                NOP                     ;
	MUL.x VF04, VF04, VF04                   DIV Q, VF14x, VF04x     ; square s
	NOP                                      NOP                     ;
	NOP                                      NOP                     ;
	MULAx.w ACC, VF00, VF03                  NOP                     ;
	MADDAy.w ACC, VF00, VF03                 NOP                     ;
	MADDz.w VF03, VF00, VF03                 NOP                     ; d = DistSq of centers
	NOP                                      NOP                     ;
	MULAw.xyz ACC, VF12, VF00                NOP                     ;
	MADDq.xyz VF01, VF02, Q                  NOP                     ; intersection, but wrong
	CLIPw.xyz VF04, VF03                     NOP                     ; compare s and d
	SUB.xyz VF02, VF00, VF02                 NOP                     ; compute normal
	NOP                                      NOP                     ;
	NOP                                      NOP                     ;
	NOP                                      FCAND VI01, 0x3         ; 0x2 cannot be set here
	NOP[E]                                   NOP                     ;
	NOP                                      NOP                     ;


; B8 -- unused
; VF12:
; VF13: radius
; VF14:
; VF15: box dimensions (?)
.globl Vu0SphereToAABBCollision
Vu0SphereToAABBCollision:
	SUB.xyz VF03, VF12, VF14                 LOI 0.5
	MULi.xyz VF15, VF15, I                   NOP
	MUL.x VF13, VF13, VF13                   NOP
	SUB.xyz VF04, VF03, VF15                 NOP
	ADD.xyz VF05, VF03, VF15                 MR32.xyzw VF16, VF15
	CLIPw.xyz VF03, VF16                     MR32.xyzw VF17, VF16
	MUL.xyz VF04, VF04, VF04                 NOP
	MUL.xyz VF05, VF05, VF05                 NOP
	CLIPw.xyz VF03, VF17                     MR32.xyzw VF16, VF17
	NOP                                      FCAND VI01, 0x1
	MINI.xyz VF04, VF04, VF05                MFIR.x VF09, VI01
	NOP                                      NOP
	CLIPw.xyz VF03, VF16                     FCAND VI01, 0x4
	NOP                                      MFIR.y VF09, VI01
	NOP                                      NOP
	MULAx.w ACC, VF00, VF00                  NOP
	ADD.xyz VF01, VF00, VF03                 FCAND VI01, 0x10
	NOP                                      MFIR.z VF09, VI01
	NOP                                      LOI 2
	NOP                                      FCAND VI01, 0x30
	SUBAw.xyz ACC, VF00, VF00                IADD VI04, VI00, VI01
	ITOF0.xyz VF09, VF09                     FCAND VI01, 0x300
	NOP                                      IADD VI03, VI00, VI01
	NOP                                      FCAND VI01, 0x3000
	NOP                                      IADD VI02, VI00, VI01
	MADDi.xyzw VF09, VF09, I                 NOP
	NOP                                      IBEQ VI04, VI00, IgnoreZValue
	NOP                                      NOP
	MADDAz.w ACC, VF00, VF04                 NOP
	MUL.z VF01, VF09, VF15                   NOP
IgnoreZValue:
	NOP                                      IBEQ VI03, VI00, IgnoreYValue
	NOP                                      NOP
	MADDAy.w ACC, VF00, VF04                 NOP
	MUL.y VF01, VF09, VF15                   NOP
IgnoreYValue:
	NOP                                      IBEQ VI02, VI00, IgnoreXValue
	NOP                                      NOP
	MADDAx.w ACC, VF00, VF04                 NOP
	MUL.x VF01, VF09, VF15                   NOP
IgnoreXValue:
	MADDx.w VF06, VF00, VF00                 NOP
	SUB.xyz VF02, VF03, VF01                 NOP
	ADD.xyz VF01, VF01, VF14                 NOP
	MULx.w VF01, VF00, VF00                  NOP
	CLIPw.xyz VF13, VF06                     NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x1
QuitMicrocode:
	NOP[E]                                   NOP
	NOP                                      NOP


; 240
.globl Vu0LineToSphereCollision
Vu0LineToSphereCollision:
	SUB.xyzw VF01, VF13, VF12                NOP
	SUB.xyzw VF02, VF14, VF12                NOP
	MUL.xyz VF03, VF01, VF02                 NOP
	MUL.xyz VF04, VF01, VF01                 NOP
	MUL.x VF15, VF15, VF15                   NOP
	MUL.xyz VF02, VF02, VF02                 NOP
	MULAx.w ACC, VF00, VF03                  NOP
	MADDAy.w ACC, VF00, VF03                 NOP
	MADDz.w VF03, VF00, VF03                 NOP
	MULAx.w ACC, VF00, VF04                  NOP
	MADDAy.w ACC, VF00, VF04                 NOP
	MADDz.w VF01, VF00, VF04                 NOP
	MULAx.w ACC, VF00, VF02                  NOP
	MADDAy.w ACC, VF00, VF02                 NOP
	MADDz.w VF02, VF00, VF02                 NOP
	MULA.w ACC, VF03, VF03                   NOP
	MADDAx.w ACC, VF01, VF15                 NOP
	MSUB.w VF05, VF01, VF02                  NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      IADDIU VI02, VI00, 0x10
	NOP                                      FMAND VI01, VI02
	NOP                                      IBNE VI01, VI00, QuitAndFail
	NOP                                      NOP
	CLIPw.xyz VF15, VF02                     SQRT Q, VF05w
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x1
	NOP                                      IBNE VI00, VI01, LineStartInsideSphere
	NOP                                      NOP
	SUBq.w VF05, VF03, Q                     NOP
	SUB.w VF05, VF05, VF01                   DIV Q, VF05w, VF01w
	NOP                                      FMAND VI01, VI02
	NOP                                      IBNE VI01, VI00, QuitAndFail
	NOP                                      NOP
	NOP                                      FMAND VI01, VI02
	NOP                                      IBEQ VI01, VI00, QuitAndFail
	NOP                                      NOP
	ADDA.xyz ACC, VF12, VF00                 NOP
	MADDq.xyz VF01, VF01, Q                  NOP
	MULx.w VF01, VF00, VF00                  NOP
	SUB.xyz VF02, VF01, VF14                 NOP
	NOP[E]                                   NOP
	NOP                                      NOP
LineStartInsideSphere:
	NOP                                      MOVE.xyzw VF01, VF12
	NOP[E]                                   IADDIU VI01, VI00, 0x1
	NOP                                      NOP


; 3C0
.globl Vu0LineToAABBCollision
Vu0LineToAABBCollision:
	SUB.xyzw VF08, VF13, VF12                LOI 0.5
	MULi.xyz VF15, VF15, I                   IADDIU VI08, VI00, 0x0
	SUB.xyzw VF12, VF12, VF14                NOP
	SUB.xyzw VF13, VF13, VF14                NOP
	NOP                                      DIV Q, VF00w, VF08x
	NOP                                      MR32.xyzw VF03, VF15
	SUB.xyz VF06, VF15, VF12                 NOP
	ADD.xyz VF07, VF15, VF12                 NOP
	NOP                                      NOP
	CLIPw.xyz VF12, VF03                     MR32.xyzw VF04, VF03
	NOP                                      NOP
	ADDq.x VF09, VF00, Q                     DIV Q, VF00w, VF08y
	NOP                                      NOP
	CLIPw.xyz VF12, VF04                     MR32.xyzw VF05, VF04
	SUB.xyz VF07, VF00, VF07                 IADDIU VI06, VI00, 0xCC
	NOP                                      IADDIU VI07, VI00, 0x30
	NOP                                      NOP
	CLIPw.xyz VF12, VF05                     FCGET VI02
	NOP                                      IAND VI02, VI02, VI06
	ADDq.y VF09, VF00, Q                     DIV Q, VF00w, VF08z
	SUB.xyz VF10, VF00, VF10                 NOP
	CLIPw.xyz VF13, VF03                     FCGET VI03
	CLIPw.xyz VF13, VF04                     IAND VI03, VI03, VI07
	CLIPw.xyz VF13, VF05                     FCAND VI01, 0x3330
	NOP                                      IBEQ VI01, VI00, StartPointInsideAABB
	NOP                                      NOP
	ADDq.z VF09, VF00, Q                     FCGET VI04
	NOP                                      FCGET VI05
	NOP                                      IAND VI04, VI04, VI06
	NOP                                      IAND VI05, VI05, VI07
	MULx.xyz VF17, VF08, VF09                NOP
	MULy.xyz VF18, VF08, VF09                IADDIU VI07, VI00, 0x80
	MULz.xyz VF19, VF08, VF09                IAND VI06, VI02, VI07
	MUL.w VF10, VF00, VF00                   IAND VI07, VI04, VI07
	NOP                                      NOP
	NOP                                      IBEQ VI06, VI07, CheckMaxXSide
	NOP                                      NOP
	MULAx.xyz ACC, VF17, VF07                NOP
	MADDw.xyz VF16, VF12, VF00               NOP
	MUL.x VF10, VF07, VF09                   NOP
	CLIPw.xyz VF16, VF04                     NOP
	CLIPw.xyz VF16, VF05                     NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x330
	NOP                                      IBNE VI01, VI00, CheckMaxXSide
	NOP                                      NOP
	MULx.w VF10, VF00, VF10                  IADDIU VI08, VI00, 0x1
	ADD.yz VF02, VF00, VF00                  MOVE.xyzw VF01, VF16
	SUBw.x VF02, VF00, VF00                  NOP
CheckMaxXSide:
	MULAx.xyz ACC, VF17, VF06                IADDIU VI07, VI00, 0x40
	MADDw.xyz VF16, VF12, VF00               IAND VI06, VI02, VI07
	MUL.x VF10, VF06, VF09                   IAND VI07, VI04, VI07
	NOP                                      NOP
	NOP                                      IBEQ VI06, VI07, CheckMinYSide
	NOP                                      NOP
	CLIPw.xyz VF16, VF04                     NOP
	CLIPw.xyz VF16, VF05                     NOP
	CLIPw.xyz VF10, VF10                     NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0xCC03
	NOP                                      IBNE VI01, VI00, CheckMinYSide
	NOP                                      NOP
	MULx.w VF10, VF00, VF10                  IADDIU VI08, VI00, 0x1
	ADD.yz VF02, VF00, VF00                  MOVE.xyzw VF01, VF16
	ADDw.x VF02, VF00, VF00                  NOP
CheckMinYSide:
	MULAy.xyz ACC, VF18, VF07                IADDIU VI07, VI00, 0x8
	MADDw.xyz VF16, VF12, VF00               IAND VI06, VI02, VI07
	MUL.y VF10, VF07, VF09                   IAND VI07, VI04, VI07
	NOP                                      NOP
	NOP                                      IBEQ VI06, VI07, CheckMaxYSide
	NOP                                      NOP
	CLIPw.xyz VF16, VF03                     NOP
	CLIPw.xyz VF16, VF05                     NOP
	CLIPw.xyz VF10, VF10                     NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x3C0C
	NOP                                      IBNE VI01, VI00, CheckMaxYSide
	NOP                                      NOP
	MULy.w VF10, VF00, VF10                  IADDIU VI08, VI00, 0x1
	ADD.xz VF02, VF00, VF00                  MOVE.xyzw VF01, VF16
	SUBw.y VF02, VF00, VF00                  NOP
CheckMaxYSide:
	MULAy.xyz ACC, VF18, VF06                IADDIU VI07, VI00, 0x4
	MADDw.xyz VF16, VF12, VF00               IAND VI06, VI02, VI07
	MUL.y VF10, VF06, VF09                   IAND VI07, VI04, VI07
	NOP                                      NOP
	NOP                                      IBEQ VI06, VI07, CheckMinZSide
	NOP                                      NOP
	CLIPw.xyz VF16, VF03                     NOP
	CLIPw.xyz VF16, VF05                     NOP
	CLIPw.xyz VF10, VF10                     NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x3C0C
	NOP                                      IBNE VI01, VI00, CheckMinZSide
	NOP                                      NOP
	MULy.w VF10, VF00, VF10                  IADDIU VI08, VI00, 0x1
	ADD.xz VF02, VF00, VF00                  MOVE.xyzw VF01, VF16
	ADDw.y VF02, VF00, VF00                  NOP
CheckMinZSide:
	MULAz.xyz ACC, VF19, VF07                IADDIU VI07, VI00, 0x20
	MADDw.xyz VF16, VF12, VF00               IAND VI06, VI03, VI07
	MUL.z VF10, VF07, VF09                   IAND VI07, VI05, VI07
	NOP                                      NOP
	NOP                                      IBEQ VI06, VI07, CheckMaxZSide
	NOP                                      NOP
	CLIPw.xyz VF16, VF03                     NOP
	CLIPw.xyz VF16, VF04                     NOP
	CLIPw.xyz VF10, VF10                     NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x3330
	NOP                                      IBNE VI01, VI00, CheckMaxZSide
	NOP                                      NOP
	MULz.w VF10, VF00, VF10                  IADDIU VI08, VI00, 0x1
	ADD.xy VF02, VF00, VF00                  MOVE.xyzw VF01, VF16
	SUBw.z VF02, VF00, VF00                  NOP
CheckMaxZSide:
	MULAz.xyz ACC, VF19, VF06                IADDIU VI07, VI00, 0x10
	MADDw.xyz VF16, VF12, VF00               IAND VI06, VI03, VI07
	MUL.z VF10, VF06, VF09                   IAND VI07, VI05, VI07
	NOP                                      NOP
	NOP                                      IBEQ VI06, VI07, DoneAllChecks
	NOP                                      NOP
	CLIPw.xyz VF16, VF03                     NOP
	CLIPw.xyz VF16, VF04                     NOP
	CLIPw.xyz VF10, VF10                     NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x3330
	NOP                                      IBNE VI01, VI00, DoneAllChecks
	NOP                                      NOP
	MULz.w VF10, VF00, VF10                  IADDIU VI08, VI00, 0x1
	ADD.xy VF02, VF00, VF00                  MOVE.xyzw VF01, VF16
	ADDw.z VF02, VF00, VF00                  NOP
DoneAllChecks:
	ADD.xyz VF01, VF01, VF14                 IADD VI01, VI00, VI08
	NOP[E]                                   NOP
	NOP                                      NOP
StartPointInsideAABB:
	ADD.xyz VF01, VF12, VF14                 WAITQ
	NOP                                      IADDIU VI01, VI00, 0x1
	NOP[E]                                   NOP
	NOP                                      NOP


; 860
.globl Vu0LineToTriangleCollisionCompressedStart
Vu0LineToTriangleCollisionCompressedStart:
	ITOF0.xyzw VF17, VF17                    LOI 0.000244140625	; 1.0/4096.0
	ITOF0.xyzw VF14, VF14                    NOP
	ITOF0.xyzw VF15, VF15                    NOP
	ITOF0.xyzw VF16, VF16                    NOP
	MULi.xyz VF17, VF17, I                   LOI 0.0078125		; 1.0/128.0
	MULi.w VF17, VF17, I                     NOP
	MULi.xyzw VF14, VF14, I                  NOP
	MULi.xyzw VF15, VF15, I                  NOP
	MULi.xyzw VF16, VF16, I                  NOP
; fall through

; 8A8
; VF12: point0
; VF13: point1
; VF14-16: verts
; VF17: plane
; out:
;  VF01: intersection point
;  VF02: triangle normal
;  VF03 x: intersection parameter
.globl Vu0LineToTriangleCollisionStart
Vu0LineToTriangleCollisionStart:
	MUL.xyz VF10, VF17, VF12                 LOI 0.5
	MUL.xyz VF11, VF17, VF13                 NOP
	SUB.xyz VF02, VF13, VF12                 NOP                            ; line dist
	ADD.xyz VF17, VF17, VF00                 NOP
	MULi.w VF03, VF00, I                     NOP
	MULAx.w ACC, VF00, VF10                  NOP
	MADDAy.w ACC, VF00, VF10                 IADDIU VI06, VI00, 0xE0
	MADDz.w VF10, VF00, VF10                 FMAND VI05, VI06               ; -- normal sign flags, unused
	MULAx.w ACC, VF00, VF11                  NOP
	MADDAy.w ACC, VF00, VF11                 NOP
	MADDz.w VF11, VF00, VF11                 NOP
	SUB.w VF09, VF17, VF10                   NOP                            ; plane-pos 0
	CLIPw.xyz VF17, VF03                     NOP                            ; compare normal against 0.5 to figure out which in which dimension to compare
	NOP                                      IADDIU VI02, VI00, 0x10        ; Sw flag
	SUBA.w ACC, VF17, VF11                   NOP                            ; plane-pos 1
	SUB.w VF08, VF11, VF10                   FMAND VI01, VI02
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FMAND VI02, VI02
	NOP                                      IBEQ VI01, VI02, QuitAndFail   ; if on same side, no collision
	NOP                                      NOP
	NOP                                      DIV Q, VF09w, VF08w            ; parameter of intersection
	NOP                                      FCAND VI01, 0x3                ; check x direction
	NOP                                      IADDIU VI02, VI01, 0x7F
	NOP                                      IADDIU VI06, VI00, 0x80
	NOP                                      IAND VI02, VI02, VI06          ; Sx flag
	NOP                                      FCAND VI01, 0xC                ; check y direction
	NOP                                      IADDIU VI03, VI01, 0x3F
	MULAw.xyz ACC, VF12, VF00                IADDIU VI06, VI00, 0x40
	MADDq.xyz VF01, VF02, Q                  IAND VI03, VI03, VI06          ; point of intersection -- Sy flag
	MULx.w VF01, VF00, VF00                  FCAND VI01, 0x30               ; -- check z direction
	ADDq.x VF03, VF00, Q                     IADDIU VI04, VI01, 0x1F        ; output parameter
	SUB.xyz VF05, VF15, VF14                 IADDIU VI06, VI00, 0x20        ; edge vectors
	SUB.xyz VF08, VF01, VF14                 IAND VI04, VI04, VI06          ; edge vectors -- Sz flag
	SUB.xyz VF06, VF16, VF15                 IADD VI06, VI02, VI03          ; edge vectors
	SUB.xyz VF09, VF01, VF15                 IADD VI06, VI06, VI04          ; edge vectors -- combine flags
	SUB.xyz VF07, VF14, VF16                 NOP                            ; edge vectors
	SUB.xyz VF10, VF01, VF16                 NOP                            ; edge vectors
	OPMULA.xyz ACC, VF08, VF05               NOP
	OPMSUB.xyz VF18, VF05, VF08              NOP                            ; cross1
	OPMULA.xyz ACC, VF09, VF06               NOP
	OPMSUB.xyz VF19, VF06, VF09              NOP                            ; cross2
	OPMULA.xyz ACC, VF10, VF07               NOP
	OPMSUB.xyz VF20, VF07, VF10              FMAND VI02, VI06               ; cross3
	NOP                                      NOP
	NOP                                      FMAND VI03, VI06
	NOP                                      NOP
	NOP                                      FMAND VI04, VI06
	NOP                                      NOP
	NOP                                      IBNE VI03, VI02, QuitAndFail   ; point has to lie on the same side of all edges (i.e. inside)
	NOP                                      NOP
	NOP                                      IBNE VI04, VI02, QuitAndFail
	NOP                                      NOP
	MULw.xyz VF02, VF17, VF00                IADDIU VI01, VI00, 0x1         ; success
	NOP[E]                                   NOP
	NOP                                      NOP


; A68
; VF12: center
; VF14: line origin
; VF15: line vector to other point
; out: VF16 xyz: nearest point on line; w: distance to that point
DistanceBetweenSphereAndLine:
	SUB.xyz VF20, VF12, VF14                 NOP
	MUL.xyz VF21, VF15, VF15                 NOP
	ADDA.xyz ACC, VF14, VF15                 NOP
	MSUBw.xyz VF25, VF12, VF00               NOP                                        ; VF25 = VF12 - (VF14+VF15)
	MUL.xyz VF22, VF20, VF20                 NOP
	MUL.xyz VF23, VF20, VF15                 NOP
	MULAx.w ACC, VF00, VF21                  NOP
	MADDAy.w ACC, VF00, VF21                 NOP
	MADDz.w VF21, VF00, VF21                 NOP	                                    ; MagSq VF15 (line length)
	MULAx.w ACC, VF00, VF23                  NOP
	MADDAy.w ACC, VF00, VF23                 NOP
	MADDz.w VF23, VF00, VF23                 NOP                                        ; dot(VF12-VF14, VF15)
	MULAx.w ACC, VF00, VF22                  NOP
	MADDAy.w ACC, VF00, VF22                 NOP
	MADDz.w VF22, VF00, VF22                 IADDIU VI08, VI00, 0x10                    ; MagSq VF12-VF14 -- Sw bit
	MUL.xyz VF25, VF25, VF25                 FMAND VI08, VI08
	NOP                                      DIV Q, VF23w, VF21w
	NOP                                      IBNE VI00, VI08, NegativeRatio
	NOP                                      NOP
	ADDA.xyz ACC, VF00, VF14                 NOP
	MADDq.xyz VF16, VF15, Q                  WAITQ                                     ; nearest point on infinte line
	ADDq.x VF24, VF00, Q                     NOP                                       ; ratio
	NOP                                      NOP
	NOP                                      NOP
	SUB.xyz VF26, VF16, VF12                 NOP
	CLIPw.xyz VF24, VF00                     NOP                                       ; compare ratio to 1.0
	NOP                                      NOP
	NOP                                      NOP
	MUL.xyz VF26, VF26, VF26                 NOP
	NOP                                      FCAND VI01, 0x1
	NOP                                      IBNE VI00, VI01, RatioGreaterThanOne
	NOP                                      NOP
	MULAx.w ACC, VF00, VF26                  NOP
	MADDAy.w ACC, VF00, VF26                 NOP
	MADDz.w VF16, VF00, VF26                 NOP                                      ; distance
	NOP                                      JR VI15
	NOP                                      NOP
NegativeRatio:
	ADD.xyz VF16, VF00, VF14                 NOP                 ; return line origin
	MUL.w VF16, VF00, VF22                   NOP                 ; and DistSq to it
	NOP                                      JR VI15
	NOP                                      NOP
RatioGreaterThanOne:
	MULAx.w ACC, VF00, VF25                  NOP
	MADDAy.w ACC, VF00, VF25                 NOP
	MADDz.w VF16, VF00, VF25                 NOP
	ADD.xyz VF16, VF14, VF15                 NOP                ; return toerh line point
	NOP                                      JR VI15
	NOP                                      NOP


; BE0
.globl Vu0SphereToTriangleCollisionCompressedStart
Vu0SphereToTriangleCollisionCompressedStart:
	ITOF0.xyzw VF17, VF17                    LOI 0.000244140625	; 1.0/4096.0
	ITOF0.xyzw VF14, VF14                    NOP
	ITOF0.xyzw VF15, VF15                    NOP
	ITOF0.xyzw VF16, VF16                    NOP
	MULi.xyz VF17, VF17, I                   LOI 0.0078125		; 1.0/128.0
	MULi.w VF17, VF17, I                     NOP
	MULi.xyzw VF14, VF14, I                  NOP
	MULi.xyzw VF15, VF15, I                  NOP
	MULi.xyzw VF16, VF16, I                  NOP
; fall through

; C28
; VF12: sphere
; VF14-16: verts
; VF17: plane
; out:
;  VF01: intersection point
;  VF02: triangle normal
;  VF03 x: intersection parameter
.globl Vu0SphereToTriangleCollisionStart
Vu0SphereToTriangleCollisionStart:
	MUL.xyz VF02, VF12, VF17                 LOI 0.1
	ADD.xyz VF17, VF17, VF00                 NOP
	ADDw.x VF13, VF00, VF12                  NOP
	NOP                                      NOP
	MULAx.w ACC, VF00, VF02                  IADDIU VI06, VI00, 0xE0
	MADDAy.w ACC, VF00, VF02                 FMAND VI05, VI06                  ; normal sign flags
	MADDAz.w ACC, VF00, VF02                 NOP
	MSUB.w VF02, VF00, VF17                  NOP                               ; center plane pos
	MULi.w VF03, VF00, I                     MOVE.xyzw VF04, VF03
	NOP                                      NOP
	NOP                                      NOP
	CLIPw.xyz VF13, VF02                     NOP                               ; compare dist and radius
	CLIPw.xyz VF17, VF03                     NOP
	MULAw.xyz ACC, VF12, VF00                IADDIU VI07, VI00, 0x0            ; -- clear test case
	MSUBw.xyz VF01, VF17, VF02               NOP
	MULx.w VF01, VF00, VF00                  FCAND VI01, 0x3                   ; projected center on plane
	ABS.w VF02, VF02                         IBEQ VI00, VI01, QuitAndFail      ; no intersection
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x3                   ; -- check x direction
	SUB.xyz VF02, VF12, VF01                 IADDIU VI02, VI01, 0x7F
	NOP                                      IADDIU VI06, VI00, 0x80
	SUB.xyz VF05, VF15, VF14                 IAND VI02, VI02, VI06
	SUB.xyz VF08, VF01, VF14                 FCAND VI01, 0xC                   ; -- check y direction
	SUB.xyz VF06, VF16, VF15                 IADDIU VI03, VI01, 0x3F
	SUB.xyz VF09, VF01, VF15                 IADDIU VI06, VI00, 0x40
	SUB.xyz VF07, VF14, VF16                 IAND VI03, VI03, VI06
	SUB.xyz VF10, VF01, VF16                 FCAND VI01, 0x30                  ; -- check z direction
	MUL.xyz VF03, VF02, VF02                 IADDIU VI04, VI01, 0x1F
	OPMULA.xyz ACC, VF08, VF05               IADDIU VI06, VI00, 0x20
	OPMSUB.xyz VF18, VF05, VF08              IAND VI04, VI04, VI06
	OPMULA.xyz ACC, VF09, VF06               NOP
	OPMSUB.xyz VF19, VF06, VF09              IADD VI06, VI02, VI03
	OPMULA.xyz ACC, VF10, VF07               IADD VI06, VI06, VI04             ; -- combine flags
	OPMSUB.xyz VF20, VF07, VF10              FMAND VI02, VI06                  ; -- cross 1 flags
	MULAx.w ACC, VF00, VF03                  IAND VI05, VI05, VI06
	MADDAy.w ACC, VF00, VF03                 FMAND VI03, VI06                  ; -- cross 2 flags
	MADDz.w VF03, VF00, VF03                 IADDIU VI08, VI00, 0x3
	NOP                                      FMAND VI04, VI06                  ; -- cross 3 flags
	NOP                                      NOP
	NOP                                      IBNE VI02, VI05, CheckSide2
	NOP                                      RSQRT Q, VF00w, VF03w
	ADD.xyz VF04, VF00, VF16                 IADDIU VI07, VI07, 0x1                          ; inside side 1
CheckSide2:
	NOP                                      IBNE VI03, VI05, CheckSide3
	NOP                                      NOP
	ADD.xyz VF04, VF00, VF14                 IADDIU VI07, VI07, 0x1                          ; inside side 2
CheckSide3:
	NOP                                      IBNE VI04, VI05, FinishCheckingSides
	NOP                                      NOP
	ADD.xyz VF04, VF00, VF15                 IADDIU VI07, VI07, 0x1                          ; inside side 3
	NOP                                      NOP
	NOP                                      IBEQ VI07, VI08, TotallyInsideTriangle
	NOP                                      NOP
FinishCheckingSides:
	MUL.x VF13, VF13, VF13                   IADDIU VI08, VI00, 0x2
	MULq.xyz VF02, VF02, Q                   WAITQ
	NOP                                      IBNE VI07, VI08, IntersectionOutsideTwoSides
	NOP                                      NOP
	NOP                                      IBEQ VI02, VI05, CheckDistanceSide2
	NOP                                      NOP
	NOP                                      MOVE.xyzw VF15, VF05
	NOP                                      BAL VI15, DistanceBetweenSphereAndLine
	NOP                                      NOP
	NOP                                      B ProcessLineResult
	NOP                                      NOP
CheckDistanceSide2:
	NOP                                      IBEQ VI03, VI05, CheckDistanceSide3
	NOP                                      NOP
	NOP                                      MOVE.xyzw VF14, VF15
	NOP                                      MOVE.xyzw VF15, VF06
	NOP                                      BAL VI15, DistanceBetweenSphereAndLine
	NOP                                      NOP
	NOP                                      B ProcessLineResult
	NOP                                      NOP
CheckDistanceSide3:
	NOP                                      MOVE.xyzw VF14, VF16
	NOP                                      MOVE.xyzw VF15, VF07
	NOP                                      BAL VI15, DistanceBetweenSphereAndLine
	NOP                                      NOP
	NOP                                      B ProcessLineResult
	NOP                                      NOP
IntersectionOutsideTwoSides:
	SUB.xyz VF05, VF04, VF12                 NOP
	ADD.xyz VF01, VF00, VF04                 NOP                                ; col point
	SUB.xyz VF02, VF12, VF04                 NOP
	NOP                                      NOP
	MUL.xyz VF05, VF05, VF05                 NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	MULAx.w ACC, VF00, VF05                  NOP
	MADDAy.w ACC, VF00, VF05                 NOP
	MADDz.w VF05, VF00, VF05                 NOP                                ; distSq to vertex
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	CLIPw.xyz VF13, VF05                     SQRT Q, VF05w                      ; compare radiusSq and distSq
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x1
	ADDq.x VF03, VF00, Q                     WAITQ                              ; dist to vertex
	NOP                                      IBEQ VI00, VI01, QuitAndFail       ; too far
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      DIV Q, VF00w, VF03x
	MULq.xyz VF02, VF02, Q                   WAITQ                              ; col normal
	NOP[E]                                   NOP
	NOP                                      NOP
TotallyInsideTriangle:
	ADDw.x VF03, VF00, VF02                  WAITQ
	MULq.xyz VF02, VF02, Q                   NOP
	NOP[E]                                   IADDIU VI01, VI00, 0x1
	NOP                                      NOP
ProcessLineResult:
	CLIPw.xyz VF13, VF16                     SQRT Q, VF16w
	ADD.xyz VF01, VF00, VF16                 NOP
	SUB.xyz VF02, VF12, VF16                 NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x1
	ADDq.x VF03, VF00, Q                     WAITQ
	NOP                                      IBEQ VI00, VI01, QuitAndFail
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      DIV Q, VF00w, VF03x
	MULq.xyz VF02, VF02, Q                   WAITQ
	NOP[E]                                   NOP
	NOP                                      NOP

EndOfMicrocode:
