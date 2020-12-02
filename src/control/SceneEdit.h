#pragma once
#ifdef GTA_SCENE_EDIT
class CPed;
class CVehicle;

struct CMovieCommand
{
	int32 m_nCommandId;
	CVector m_vecPosition;
	CVector m_vecCamera;
	int16 m_nActorId;
	int16 m_nActor2Id;
	int16 m_nVehicleId;
	int16 m_nModelIndex;
};

class CSceneEdit
{
public:
	enum {
		MOVIE_DO_NOTHING = 0,
		MOVIE_NEW_ACTOR,
		MOVIE_MOVE_ACTOR,
		MOVIE_SELECT_ACTOR,
		MOVIE_DELETE_ACTOR,
		MOVIE_NEW_VEHICLE,
		MOVIE_MOVE_VEHICLE,
		MOVIE_SELECT_VEHICLE,
		MOVIE_DELETE_VEHICLE,
		MOVIE_GIVE_WEAPON,
		MOVIE_GOTO,
		MOVIE_GOTO_WAIT,
		MOVIE_GET_IN_CAR,
		MOVIE_GET_OUT_CAR,
		MOVIE_KILL,
		MOVIE_FLEE,
		MOVIE_WAIT,
		MOVIE_POSITION_CAMERA,
		MOVIE_SET_CAMERA_TARGET,
		MOVIE_SELECT_CAMERA_MODE,
		MOVIE_SAVE_MOVIE,
		MOVIE_LOAD_MOVIE,
		MOVIE_PLAY_MOVIE,
		MOVIE_END,
		MOVIE_TOTAL_COMMANDS
	};
	enum {
		NUM_ACTORS_IN_MOVIE = 5,
		NUM_VEHICLES_IN_MOVIE = 5,
		NUM_COMMANDS_IN_MOVIE = 20
	};
	static int32 m_bCameraFollowActor;
	static CVector m_vecCurrentPosition;
	static CVector m_vecCamHeading;
	static CVector m_vecGotoPosition;
	static int32 m_nVehicle;
	static int32 m_nVehicle2;
	static int32 m_nActor;
	static int32 m_nActor2;
	static int32 m_nVehiclemodelId;
	static int32 m_nPedmodelId;
	static int16 m_nCurrentMovieCommand;
	static int16 m_nCurrentCommand;
	static int16 m_nCurrentVehicle;
	static int16 m_nCurrentActor;
	static bool m_bEditOn;
	static bool m_bRecording;
	static bool m_bCommandActive;
	static bool m_bActorSelected;
	static bool m_bActor2Selected;
	static bool m_bVehicleSelected;
	static int16 m_nNumActors;
	static int16 m_nNumVehicles;
	static int16 m_nNumMovieCommands;
	static int16 m_nWeaponType;
	static CPed* pActors[NUM_ACTORS_IN_MOVIE];
	static CVehicle* pVehicles[NUM_VEHICLES_IN_MOVIE];
	static bool m_bDrawGotoArrow;
	static CMovieCommand Movie[NUM_COMMANDS_IN_MOVIE];

	static void LoadMovie(void);
	static void SaveMovie(void);
	static void Initialise(void);
	static void InitPlayback(void);
	static void ReInitialise(void);
	static void Update(void);
	static void Draw(void);
	static void ProcessCommand(void);
	static void PlayBack(void);
	static void ClearForNewCommand(void);
	static void SelectActor(void);
	static void SelectActor2(void);
	static void SelectVehicle(void);
	static bool SelectWeapon(void);
};
#endif
