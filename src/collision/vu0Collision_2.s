QuitAndFail2:
	NOP[E]                                   IADDIU VI01, VI00, 0x0
	NOP                                      NOP


QuitAndSucceed2:
	NOP[E]                                   IADDIU VI01, VI00, 0x1
	NOP                                      NOP


; 20
GetBBVertices:
	MULw.xy VF02, VF01, VF00                 NOP
	MUL.z VF02, VF01, VF11                   NOP
	MULw.xz VF03, VF01, VF00                 NOP
	MUL.y VF03, VF01, VF11                   NOP
	MULw.x VF04, VF01, VF00                  NOP
	MUL.yz VF04, VF01, VF11                  NOP
	NOP                                      JR VI15
	NOP                                      NOP


; 60
Vu0OBBToOBBCollision:
	SUBw.xyz VF11, VF00, VF00                LOI 0.5
	MULi.xyz VF12, VF12, I                   NOP
	MULi.xyz VF13, VF13, I                   NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      MOVE.xyz VF01, VF12
	NOP                                      BAL VI15, GetBBVertices
	NOP                                      NOP
	MULAx.xyz ACC, VF14, VF01                NOP
	MADDAy.xyz ACC, VF15, VF01               NOP
	MADDz.xyz VF01, VF16, VF01               NOP
	MULAx.xyz ACC, VF14, VF02                NOP
	MADDAy.xyz ACC, VF15, VF02               NOP
	MADDz.xyz VF02, VF16, VF02               NOP
	MULAx.xyz ACC, VF14, VF03                NOP
	MADDAy.xyz ACC, VF15, VF03               NOP
	MADDz.xyz VF03, VF16, VF03               NOP
	MULAx.xyz ACC, VF14, VF04                NOP
	MADDAy.xyz ACC, VF15, VF04               NOP
	MADDz.xyz VF04, VF16, VF04               NOP
	ABS.xyz VF05, VF01                       NOP
	ABS.xyz VF06, VF02                       NOP
	ABS.xyz VF07, VF03                       NOP
	ABS.xyz VF08, VF04                       NOP
	NOP                                      NOP
	MAX.xyz VF05, VF05, VF06                 NOP
	NOP                                      NOP
	MAX.xyz VF07, VF07, VF08                 NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	MAX.xyz VF05, VF05, VF07                 NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	ADD.xyz VF09, VF05, VF13                 NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	MULx.w VF05, VF00, VF09                  NOP
	MULy.w VF06, VF00, VF09                  NOP
	MULz.w VF07, VF00, VF09                  NOP
	CLIPw.xyz VF17, VF05                     NOP
	CLIPw.xyz VF17, VF06                     NOP
	CLIPw.xyz VF17, VF07                     MOVE.xyz VF01, VF13
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x3330
	NOP                                      IBNE VI01, VI00, QuitAndFail2
	NOP                                      NOP
	NOP                                      BAL VI15, GetBBVertices
	NOP                                      NOP
	MULAx.xyz ACC, VF18, VF01                NOP
	MADDAy.xyz ACC, VF19, VF01               NOP
	MADDz.xyz VF01, VF20, VF01               NOP
	MULAx.xyz ACC, VF18, VF02                NOP
	MADDAy.xyz ACC, VF19, VF02               NOP
	MADDz.xyz VF02, VF20, VF02               NOP
	MULAx.xyz ACC, VF18, VF03                NOP
	MADDAy.xyz ACC, VF19, VF03               NOP
	MADDz.xyz VF03, VF20, VF03               NOP
	MULAx.xyz ACC, VF18, VF04                NOP
	MADDAy.xyz ACC, VF19, VF04               NOP
	MADDz.xyz VF04, VF20, VF04               NOP
	ABS.xyz VF05, VF01                       NOP
	ABS.xyz VF06, VF02                       NOP
	ABS.xyz VF07, VF03                       NOP
	ABS.xyz VF08, VF04                       NOP
	NOP                                      NOP
	MAX.xyz VF05, VF05, VF06                 NOP
	NOP                                      NOP
	MAX.xyz VF07, VF07, VF08                 NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	MAX.xyz VF05, VF05, VF07                 NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	ADD.xyz VF09, VF05, VF12                 NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	MULx.w VF05, VF00, VF09                  NOP
	MULy.w VF06, VF00, VF09                  NOP
	MULz.w VF07, VF00, VF09                  NOP
	CLIPw.xyz VF21, VF05                     NOP
	CLIPw.xyz VF21, VF06                     NOP
	CLIPw.xyz VF21, VF07                     NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      NOP
	NOP                                      FCAND VI01, 0x3330
	NOP                                      IBNE VI01, VI00, QuitAndFail2
	NOP                                      NOP
	SUB.xyz VF06, VF02, VF01                 NOP
	SUB.xyz VF07, VF03, VF01                 NOP
	ADD.xyz VF08, VF04, VF01                 NOP
	ADD.x VF09, VF00, VF12                   NOP
	ADD.yz VF09, VF00, VF00                  NOP
	ADD.y VF10, VF00, VF12                   NOP
	ADD.xz VF10, VF00, VF00                  NOP
	ADD.z VF11, VF00, VF12                   IADDI VI04, VI00, 0x0
	ADD.xy VF11, VF00, VF00                  IADD VI02, VI00, VI00
	OPMULA.xyz ACC, VF06, VF09               NOP
	OPMSUB.xyz VF01, VF09, VF06              NOP
	OPMULA.xyz ACC, VF06, VF10               NOP
	OPMSUB.xyz VF02, VF10, VF06              NOP
	OPMULA.xyz ACC, VF06, VF11               NOP
	OPMSUB.xyz VF03, VF11, VF06              SQI.xyzw VF01, (VI02++)
	OPMULA.xyz ACC, VF07, VF09               NOP
	OPMSUB.xyz VF01, VF09, VF07              SQI.xyzw VF02, (VI02++)
	OPMULA.xyz ACC, VF07, VF10               NOP
	OPMSUB.xyz VF02, VF10, VF07              SQI.xyzw VF03, (VI02++)
	OPMULA.xyz ACC, VF07, VF11               NOP
	OPMSUB.xyz VF03, VF11, VF07              SQI.xyzw VF01, (VI02++)
	OPMULA.xyz ACC, VF08, VF09               NOP
	OPMSUB.xyz VF01, VF09, VF08              SQI.xyzw VF02, (VI02++)
	OPMULA.xyz ACC, VF08, VF10               NOP
	OPMSUB.xyz VF02, VF10, VF08              SQI.xyzw VF03, (VI02++)
	OPMULA.xyz ACC, VF08, VF11               LOI 0.5
	OPMSUB.xyz VF01, VF11, VF08              SQI.xyzw VF01, (VI02++)
	MULi.xyz VF06, VF06, I                   NOP
	MULi.xyz VF07, VF07, I                   SQI.xyzw VF02, (VI02++)
	MULi.xyz VF08, VF08, I                   NOP
	MUL.xyz VF02, VF21, VF01                 NOP
	MUL.xyz VF03, VF12, VF01                 NOP
	MUL.xyz VF09, VF06, VF01                 NOP
	MUL.xyz VF10, VF07, VF01                 NOP
	MUL.xyz VF11, VF08, VF01                 NOP
	ABS.xyz VF03, VF03                       NOP
	ADDy.x VF05, VF09, VF09                  NOP
	ADDx.y VF05, VF10, VF10                  NOP
	ADDx.z VF05, VF11, VF11                  NOP
	NOP                                      NOP
EdgePairLoop:
	ADDz.x VF05, VF05, VF09                  NOP
	ADDz.y VF05, VF05, VF10                  NOP
	ADDy.z VF05, VF05, VF11                  NOP
	MULAx.w ACC, VF00, VF02                  IADD VI03, VI02, VI00
	MADDAy.w ACC, VF00, VF02                 LQD.xyzw VF01, (--VI02)
	MADDz.w VF02, VF00, VF02                 NOP
	ABS.xyz VF05, VF05                       NOP
	MULAx.w ACC, VF00, VF03                  NOP
	MADDAy.w ACC, VF00, VF03                 NOP
	MADDAz.w ACC, VF00, VF03                 NOP
	MADDAx.w ACC, VF00, VF05                 NOP
	MADDAy.w ACC, VF00, VF05                 NOP
	MADDz.w VF03, VF00, VF05                 NOP
	ADDw.x VF04, VF00, VF02                  NOP
	MUL.xyz VF02, VF21, VF01                 NOP
	MUL.xyz VF03, VF12, VF01                 NOP
	MUL.xyz VF09, VF06, VF01                 NOP
	CLIPw.xyz VF04, VF03                     NOP
	MUL.xyz VF10, VF07, VF01                 NOP
	MUL.xyz VF11, VF08, VF01                 NOP
	ABS.xyz VF03, VF03                       NOP
	ADDy.x VF05, VF09, VF09                  FCAND VI01, 0x3
	ADDx.y VF05, VF10, VF10                  IBNE VI01, VI00, QuitAndFail2
	ADDx.z VF05, VF11, VF11                  NOP
	NOP                                      IBNE VI03, VI00, EdgePairLoop
	NOP                                      NOP
	NOP[E]                                   IADDIU VI01, VI00, 0x1
	NOP                                      NOP

EndOfMicrocode2:
