#include "common.h"

#include "main.h"
#include "FileMgr.h"
#include "Weather.h"
#include "Collision.h"
#include "SurfaceTable.h"

float CSurfaceTable::ms_aAdhesiveLimitTable[NUMADHESIVEGROUPS][NUMADHESIVEGROUPS];

void
CSurfaceTable::Initialise(Const char *filename)
{
	int lineno, fieldno;
	char *line;
	char surfname[256];
	float adhesiveLimit;

	CFileMgr::SetDir("");
	CFileMgr::LoadFile(filename, work_buff, sizeof(work_buff), "r");

	line = (char*)work_buff;
	for(lineno = 0; lineno < NUMADHESIVEGROUPS; lineno++){
		// skip white space and comments
		while(*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r' || *line == ';'){
			if(*line == ';'){
				while(*line != '\n' && *line != '\r')
					line++;
			}else
				line++;
		}

		sscanf(line, "%s", surfname);
		// skip what we just read
		while(!(*line == ' ' || *line == '\t' || *line == ','))
			line++;

		for(fieldno = 0; fieldno <= lineno; fieldno++){
			// skip white space
			while(*line == ' ' || *line == '\t' || *line == ',')
				line++;
			adhesiveLimit = 0.0f;
			if(*line != '-')
				sscanf(line, "%f", &adhesiveLimit);
			// skip what we just read
			while(!(*line == ' ' || *line == '\t' || *line == ',' || *line == '\n'))
				line++;

			ms_aAdhesiveLimitTable[lineno][fieldno] = adhesiveLimit;
			ms_aAdhesiveLimitTable[fieldno][lineno] = adhesiveLimit;
		}
	}
}

int
CSurfaceTable::GetAdhesionGroup(uint8 surfaceType)
{
	switch(surfaceType){
	case SURFACE_DEFAULT:		return ADHESIVE_ROAD;
	case SURFACE_TARMAC:		return ADHESIVE_ROAD;
	case SURFACE_GRASS:		return ADHESIVE_LOOSE;
	case SURFACE_GRAVEL:		return ADHESIVE_LOOSE;
	case SURFACE_MUD_DRY:		return ADHESIVE_HARD;
	case SURFACE_PAVEMENT:		return ADHESIVE_ROAD;
	case SURFACE_CAR:		return ADHESIVE_HARD;
	case SURFACE_GLASS:		return ADHESIVE_HARD;
	case SURFACE_TRANSPARENT_CLOTH:	return ADHESIVE_HARD;
	case SURFACE_GARAGE_DOOR:	return ADHESIVE_HARD;
	case SURFACE_CAR_PANEL:		return ADHESIVE_HARD;
	case SURFACE_THICK_METAL_PLATE:	return ADHESIVE_HARD;
	case SURFACE_SCAFFOLD_POLE:	return ADHESIVE_HARD;
	case SURFACE_LAMP_POST:		return ADHESIVE_HARD;
	case SURFACE_FIRE_HYDRANT:	return ADHESIVE_HARD;
	case SURFACE_GIRDER:		return ADHESIVE_HARD;
	case SURFACE_METAL_CHAIN_FENCE:	return ADHESIVE_HARD;
	case SURFACE_PED:		return ADHESIVE_RUBBER;
	case SURFACE_SAND:		return ADHESIVE_SAND;
	case SURFACE_WATER:		return ADHESIVE_WET;
	case SURFACE_WOOD_CRATES:	return ADHESIVE_ROAD;
	case SURFACE_WOOD_BENCH:	return ADHESIVE_ROAD;
	case SURFACE_WOOD_SOLID:	return ADHESIVE_ROAD;
	case SURFACE_RUBBER:		return ADHESIVE_RUBBER;
	case SURFACE_PLASTIC:		return ADHESIVE_HARD;
	case SURFACE_HEDGE:		return ADHESIVE_LOOSE;
	case SURFACE_STEEP_CLIFF:	return ADHESIVE_LOOSE;
	case SURFACE_CONTAINER:		return ADHESIVE_HARD;
	case SURFACE_NEWS_VENDOR:	return ADHESIVE_HARD;
	case SURFACE_WHEELBASE:		return ADHESIVE_RUBBER;
	case SURFACE_CARDBOARDBOX:	return ADHESIVE_LOOSE;
	case SURFACE_TRANSPARENT_STONE:	return ADHESIVE_HARD;
	case SURFACE_METAL_GATE:	return ADHESIVE_HARD;
	case SURFACE_SAND_BEACH:	return ADHESIVE_SAND;
	case SURFACE_CONCRETE_BEACH:	return ADHESIVE_ROAD;
	default:			return ADHESIVE_ROAD;
	}
}

float
CSurfaceTable::GetWetMultiplier(uint8 surfaceType)
{
	switch(surfaceType){
	case SURFACE_DEFAULT:
	case SURFACE_TARMAC:
	case SURFACE_MUD_DRY:
	case SURFACE_PAVEMENT:
	case SURFACE_TRANSPARENT_CLOTH:
	case SURFACE_WOOD_CRATES:
	case SURFACE_WOOD_BENCH:
	case SURFACE_WOOD_SOLID:
	case SURFACE_HEDGE:
	case SURFACE_CARDBOARDBOX:
	case SURFACE_TRANSPARENT_STONE:
	case SURFACE_CONCRETE_BEACH:
		return 1.0f - CWeather::WetRoads*0.25f;

	case SURFACE_GRASS:
	case SURFACE_CAR:
	case SURFACE_GLASS:
	case SURFACE_GARAGE_DOOR:
	case SURFACE_CAR_PANEL:
	case SURFACE_THICK_METAL_PLATE:
	case SURFACE_SCAFFOLD_POLE:
	case SURFACE_LAMP_POST:
	case SURFACE_FIRE_HYDRANT:
	case SURFACE_GIRDER:
	case SURFACE_METAL_CHAIN_FENCE:
	case SURFACE_PED:
	case SURFACE_RUBBER:
	case SURFACE_PLASTIC:
	case SURFACE_STEEP_CLIFF:
	case SURFACE_CONTAINER:
	case SURFACE_NEWS_VENDOR:
	case SURFACE_WHEELBASE:
	case SURFACE_METAL_GATE:
		return 1.0f - CWeather::WetRoads*0.4f;

	case SURFACE_SAND:
	case SURFACE_SAND_BEACH:
		return 1.0f - CWeather::WetRoads*0.5f;

	default:
		return 1.0f;
	}
}

float
CSurfaceTable::GetAdhesiveLimit(CColPoint &colpoint)
{
	return ms_aAdhesiveLimitTable[GetAdhesionGroup(colpoint.surfaceB)][GetAdhesionGroup(colpoint.surfaceA)];
}
