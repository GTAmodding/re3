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
	case SURFACE_DIRT:		return ADHESIVE_LOOSE;
	case SURFACE_DIRTTRACK:		return ADHESIVE_HARD;
	case SURFACE_PAVEMENT:		return ADHESIVE_ROAD;
	case SURFACE_METAL6:		return ADHESIVE_HARD;
	case SURFACE_GLASS:		return ADHESIVE_HARD;
	case SURFACE_SCAFFOLD:		return ADHESIVE_HARD;
	case SURFACE_METAL_DOOR:	return ADHESIVE_HARD;
	case SURFACE_BILLBOARD:		return ADHESIVE_HARD;
	case SURFACE_STEEL:		return ADHESIVE_HARD;
	case SURFACE_METAL_POLE:	return ADHESIVE_HARD;
	case SURFACE_STREET_LIGHT:	return ADHESIVE_HARD;
	case SURFACE_METAL14:		return ADHESIVE_HARD;
	case SURFACE_METAL15:		return ADHESIVE_HARD;
	case SURFACE_METAL_FENCE:	return ADHESIVE_HARD;
	case SURFACE_FLESH:		return ADHESIVE_RUBBER;
	case SURFACE_SAND:		return ADHESIVE_SAND;
	case SURFACE_PUDDLE:		return ADHESIVE_WET;
	case SURFACE_WOOD:		return ADHESIVE_ROAD;
	case SURFACE_WOOD_BOX:		return ADHESIVE_ROAD;
	case SURFACE_WOOD_PLANK:	return ADHESIVE_ROAD;
	case SURFACE_TIRE:		return ADHESIVE_RUBBER;
	case SURFACE_HARD24:		return ADHESIVE_HARD;
	case SURFACE_HEDGE:		return ADHESIVE_LOOSE;
	case SURFACE_STONE:		return ADHESIVE_LOOSE;
	case SURFACE_METAL27:		return ADHESIVE_HARD;
	case SURFACE_METAL28:		return ADHESIVE_HARD;
	case SURFACE_RUBBER29:		return ADHESIVE_RUBBER;
	case SURFACE_LOOSE30:		return ADHESIVE_LOOSE;
	case SURFACE_BOLLARD:		return ADHESIVE_HARD;
	case SURFACE_GATE:		return ADHESIVE_HARD;
	case SURFACE_SAND33:		return ADHESIVE_SAND;
	case SURFACE_ROAD34:		return ADHESIVE_ROAD;
	default:			return ADHESIVE_ROAD;
	}
}

float
CSurfaceTable::GetWetMultiplier(uint8 surfaceType)
{
	switch(surfaceType){
	case SURFACE_DEFAULT:
	case SURFACE_TARMAC:
	case SURFACE_DIRTTRACK:
	case SURFACE_PAVEMENT:
	case SURFACE_SCAFFOLD:
	case SURFACE_WOOD:
	case SURFACE_WOOD_BOX:
	case SURFACE_WOOD_PLANK:
	case SURFACE_HEDGE:
	case SURFACE_LOOSE30:
	case SURFACE_BOLLARD:
	case SURFACE_ROAD34:
		return 1.0f - CWeather::WetRoads*0.25f;

	case SURFACE_GRASS:
	case SURFACE_METAL6:
	case SURFACE_GLASS:
	case SURFACE_METAL_DOOR:
	case SURFACE_BILLBOARD:
	case SURFACE_STEEL:
	case SURFACE_METAL_POLE:
	case SURFACE_STREET_LIGHT:
	case SURFACE_METAL14:
	case SURFACE_METAL15:
	case SURFACE_METAL_FENCE:
	case SURFACE_FLESH:
	case SURFACE_TIRE:
	case SURFACE_HARD24:
	case SURFACE_STONE:
	case SURFACE_METAL27:
	case SURFACE_METAL28:
	case SURFACE_RUBBER29:
	case SURFACE_GATE:
		return 1.0f - CWeather::WetRoads*0.4f;

	case SURFACE_SAND:
	case SURFACE_SAND33:
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
