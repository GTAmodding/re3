#include "common.h"

#include "main.h"
#include "Antennas.h"

CAntenna CAntennas::aAntennas[NUMANTENNAS];

void
CAntennas::Init(void)
{
	int i;
	for(i = 0; i < NUMANTENNAS; i++){
		aAntennas[i].active = false;
		aAntennas[i].updatedLastFrame = false;
	}
}

// Free antennas that aren't used anymore
void
CAntennas::Update(void)
{
	int i;

	for(i = 0; i < NUMANTENNAS; i++){
		if(aAntennas[i].active && !aAntennas[i].updatedLastFrame)
			aAntennas[i].active = false;
		aAntennas[i].updatedLastFrame = false;
	}
}

// Add a new one or update an old one
void
CAntennas::RegisterOne(uint32 id, CVector dir, CVector position, float length)
{
	int i, j;

	for(i = 0; i < NUMANTENNAS; i++)
		if(aAntennas[i].active && aAntennas[i].id == id)
			break;

	if(i >= NUMANTENNAS){
		// not found, register new one

		// find empty slot
		for(i = 0; i < NUMANTENNAS; i++)
			if(!aAntennas[i].active)
				break;

		// there is space
		if(i < NUMANTENNAS){
			aAntennas[i].active = true;
			aAntennas[i].updatedLastFrame = true;
			aAntennas[i].id = id;
			aAntennas[i].segmentLength = length/6.0f;
			for(j = 0; j < 6; j++){
				aAntennas[i].pos[j] = position + dir*j*aAntennas[i].segmentLength;
				aAntennas[i].speed[j] = CVector(0.0f, 0.0f, 0.0f);
			}
		}
	}else{
		// found, update
		aAntennas[i].Update(dir, position);
		aAntennas[i].updatedLastFrame = true;
	}
}

static RwIm3DVertex vertexbufferA[2];

void
CAntennas::Render(void)
{
	int i, j;

	PUSH_RENDERGROUP("CAntennas::Render");
	for(i = 0; i < NUMANTENNAS; i++){
		if(!aAntennas[i].active)
			continue;

		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

		for(j = 0; j < 5; j++){
			RwIm3DVertexSetRGBA(&vertexbufferA[0], 200, 200, 200, 100);
			RwIm3DVertexSetPos(&vertexbufferA[0],
				aAntennas[i].pos[j].x,
				aAntennas[i].pos[j].y,
				aAntennas[i].pos[j].z);
			RwIm3DVertexSetRGBA(&vertexbufferA[1], 200, 200, 200, 100);
			RwIm3DVertexSetPos(&vertexbufferA[1],
				aAntennas[i].pos[j+1].x,
				aAntennas[i].pos[j+1].y,
				aAntennas[i].pos[j+1].z);

			// LittleTest();
			if(RwIm3DTransform(vertexbufferA, 2, nil, 0)){
				RwIm3DRenderLine(0, 1);
				RwIm3DEnd();
			}
		}
	}

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);

	POP_RENDERGROUP();
}

void
CAntenna::Update(CVector dir, CVector basepos)
{
	int i;

	pos[0] = basepos;
	pos[1] = basepos + dir*segmentLength;

	for(i = 2; i < 6; i++){
		CVector basedir = pos[i-1] - pos[i-2];
		CVector newdir = pos[i] - pos[i-1] +	// drag along
			dir*0.1f +	// also drag up a bit for stiffness
			speed[i];	// and keep moving
		newdir.Normalise();
		newdir *= segmentLength;
		CVector newpos = pos[i-1] + (basedir + newdir)/2.0f;
		speed[i] = (newpos - pos[i])*0.9f;
		pos[i] = newpos;
	}
}
