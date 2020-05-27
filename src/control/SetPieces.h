#pragma once

#include "config.h"

class CVehicle;
class CCopPed;

enum eSetPieceType : uint8
{
	SETPIECE_NONE = 0,
	SETPIECE_TWOCOPCARSINALLEY,
	SETPIECE_CARBLOCKINGPLAYERFROMSIDE,
	SETPIECE_CARRAMMINGPLAYERFROMSIDE,
	SETPIECE_CREATECOPPERONFOOT,
	SETPIECE_CREATETWOCOPPERSONFOOT,
	SETPIECE_TWOCARSBLOCKINGPLAYERFROMSIDE,
	SETPIECE_TWOCARSRAMMINGPLAYERFROMSIDE
};

class CSetPiece
{
public:
	eSetPieceType m_nType;
	uint32 m_nLastTimeCreated;
	CVector2D m_vTriggerInf;
	CVector2D m_vTriggerSup;
	CVector2D m_vSpawn1;
	CVector2D m_vSpawn2;
	CVector2D m_vTarget1;
	CVector2D m_vTarget2;

	CVehicle* TryToGenerateCopCar(CVector2D, CVector2D);
	CCopPed* TryToGenerateCopPed(CVector2D);
	void Update(void);
};

class CSetPieces
{
	static bool bDebug;
	static uint32 NumSetPieces;
	static CSetPiece aSetPieces[NUM_SETPIECES];
public:
	static void Init(void);
	static void AddOne(uint8 type, CVector2D, CVector2D, CVector2D, CVector2D, CVector2D, CVector2D);
	static void Save(uint8*, uint32*);
	static void Load(uint8*, uint32);
	static void Update(void);
};
