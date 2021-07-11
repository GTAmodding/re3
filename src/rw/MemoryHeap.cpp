#include "common.h"
#include "main.h"
#include "FileMgr.h"
#include "Timer.h"
#include "ModelInfo.h"
#include "Streaming.h"
#include "FileLoader.h"
#include "MemoryHeap.h"

// TODO(MIAMI)

#ifdef USE_CUSTOM_ALLOCATOR

//#define MEMORYHEAP_ASSERT(cond) { if (!(cond)) { printf("ASSERT File:%s Line:%d\n", __FILE__, __LINE__); exit(1); } }
//#define MEMORYHEAP_ASSERT_MESSAGE(cond, message) { if (!(cond)) { printf("ASSERT File:%s Line:%d:\n\t%s\n", __FILE__, __LINE__, message); exit(1); } }

#define MEMORYHEAP_ASSERT(cond) assert(cond)
#define MEMORYHEAP_ASSERT_MESSAGE(cond, message) assert(cond)

// registered pointers that we keep track of
void **gPtrList[4000];
int32 numPtrs;
int32 gPosnInList;
// indices into the ptr list in here are free
CStack<int32, 4000> m_ptrListIndexStack;
// how much memory we've moved
uint32 memMoved;

CMemoryHeap gMainHeap;

void
CMemoryHeap::Init(uint32 total)
{
	MEMORYHEAP_ASSERT((total != 0xF) != 0);

	m_totalMemUsed = 0;
	m_memUsed = nil;
	m_currentMemID = MEMID_FREE;
	m_blocksUsed = nil;
	m_totalBlocksUsed = 0;
	m_unkMemId = -1;

	uint8 *mem = (uint8*)malloc(total);
	assert(((uintptr)mem & 0xF) == 0);
	m_start = (HeapBlockDesc*)mem;
	m_end = (HeapBlockDesc*)(mem + total - sizeof(HeapBlockDesc));
	m_start->m_memId = MEMID_FREE;
	m_start->m_size = total - 2*sizeof(HeapBlockDesc);
	m_end->m_memId = MEMID_GAME;
	m_end->m_size = 0;

	m_freeList.m_last.m_size = INT_MAX;
	m_freeList.Init();
	m_freeList.Insert(m_start);

	// TODO: figure out what these are and use sizeof
	m_fixedSize[0].Init(0x10);
	m_fixedSize[1].Init(0x20);
	m_fixedSize[2].Init(0xE0);
	m_fixedSize[3].Init(0x60);
	m_fixedSize[4].Init(0x1C0);
	m_fixedSize[5].Init(0x50);

	m_currentMemID = MEMID_FREE;	// disable registration
	m_memUsed = (uint32*)Malloc(NUM_MEMIDS * sizeof(uint32));
	m_blocksUsed = (uint32*)Malloc(NUM_MEMIDS * sizeof(uint32));
	RegisterMalloc(GetDescFromHeapPointer(m_memUsed));
	RegisterMalloc(GetDescFromHeapPointer(m_blocksUsed));

	m_currentMemID = MEMID_GAME;
	for(int i = 0; i < NUM_MEMIDS; i++){
		m_memUsed[i] = 0;
		m_blocksUsed[i] = 0;
	}
}

void
CMemoryHeap::RegisterMalloc(HeapBlockDesc *block)
{
	block->m_memId = m_currentMemID;
	if(m_currentMemID == MEMID_FREE)
		return;
	m_totalMemUsed += block->m_size + sizeof(HeapBlockDesc);
	m_memUsed[m_currentMemID] += block->m_size + sizeof(HeapBlockDesc);
	m_blocksUsed[m_currentMemID]++;
	m_totalBlocksUsed++;
}

void
CMemoryHeap::RegisterFree(HeapBlockDesc *block)
{
	if(block->m_memId == MEMID_FREE)
		return;
	m_totalMemUsed -= block->m_size + sizeof(HeapBlockDesc);
	m_memUsed[block->m_memId] -= block->m_size + sizeof(HeapBlockDesc);
	m_blocksUsed[block->m_memId]--;
	m_totalBlocksUsed--;
}

void*
CMemoryHeap::Malloc(uint32 size)
{
	static int recursion = 0;

	// weird way to round up
	if((size & 0xF) != 0)
		size = (size&~0xF) + 0x10;

	recursion++;

	// See if we can allocate from one of the fixed-size lists
	for(int i = 0; i < NUM_FIXED_MEMBLOCKS; i++){
		CommonSize *list = &m_fixedSize[i];
		if(m_fixedSize[i].m_size == size){
			HeapBlockDesc *block = list->Malloc();
			if(block){
				RegisterMalloc(block);
				recursion--;
				return block->GetDataPointer();
			}
			break;
		}
	}

	// now try the normal free list
	HeapBlockDesc *next;
	for(HeapBlockDesc *block = m_freeList.m_first.m_next;
	    block != &m_freeList.m_last;
	    block = next){
		MEMORYHEAP_ASSERT(block->m_memId == MEMID_FREE);
		MEMORYHEAP_ASSERT_MESSAGE(block >= m_start && block <= m_end, "Block outside of memory");

		// make sure block has maximum size
		uint32 initialsize = block->m_size;
		uint32 blocksize = CombineFreeBlocks(block);
#ifdef FIX_BUGS
		// has to be done here because block can be moved
		next = block->m_next;
#endif
		if(initialsize != blocksize){
			block->RemoveHeapFreeBlock();
			HeapBlockDesc *pos = block->m_prev->FindSmallestFreeBlock(block->m_size);
			block->InsertHeapFreeBlock(pos->m_prev);
		}
		if(block->m_size >= size){
			// got space to allocate from!
			block->RemoveHeapFreeBlock();
			FillInBlockData(block, block->GetNextConsecutive(), size);
			recursion--;
			return block->GetDataPointer();
		}
#ifndef FIX_BUGS
		next = block->m_next;
#endif
	}

	// oh no, we're losing, try to free some stuff
	static bool removeCollision = false;
	static bool removeIslands = false;
	static bool removeBigBuildings = false;
	size_t initialMemoryUsed = CStreaming::ms_memoryUsed;
	CStreaming::MakeSpaceFor(0xCFE800 - CStreaming::ms_memoryUsed);
	if (recursion > 10)
		CGame::TidyUpMemory(true, false);
	else if (recursion > 6)
		CGame::TidyUpMemory(false, true);
	if (initialMemoryUsed == CStreaming::ms_memoryUsed && recursion > 11) {
		if (!removeCollision && !CGame::playingIntro) {
			CModelInfo::RemoveColModelsFromOtherLevels(LEVEL_GENERIC);
			removeCollision = true;
		}
		else if (!removeIslands && !CGame::playingIntro) {
			CStreaming::RemoveIslandsNotUsed(LEVEL_INDUSTRIAL);
			CStreaming::RemoveIslandsNotUsed(LEVEL_COMMERCIAL);
			CStreaming::RemoveIslandsNotUsed(LEVEL_SUBURBAN);
			removeIslands = true;
		}
		else if (!removeBigBuildings) {
			CStreaming::RemoveBigBuildings(LEVEL_INDUSTRIAL);
			CStreaming::RemoveBigBuildings(LEVEL_COMMERCIAL);
			CStreaming::RemoveBigBuildings(LEVEL_SUBURBAN);
		}
		else {
			LoadingScreen("NO MORE MEMORY", nil, nil);
			LoadingScreen("NO MORE MEMORY", nil, nil);
		}
		CGame::TidyUpMemory(true, false);
	}
	void *mem = Malloc(size);
	if (removeCollision) {
		CTimer::Stop();
		// TODO: different on PS2
		CFileLoader::LoadCollisionFromDatFile(CCollision::ms_collisionInMemory);
		removeCollision = false;
		CTimer::Update();
	}
	if (removeBigBuildings || removeIslands) {
		CTimer::Stop();
		if (!CGame::playingIntro)
			CStreaming::RequestBigBuildings(CGame::currLevel);
		CStreaming::LoadAllRequestedModels(true);
		removeBigBuildings = false;
		removeIslands = false;
		CTimer::Update();
	}
	recursion--;
	return mem;
}

void*
CMemoryHeap::Realloc(void *ptr, uint32 size)
{
	if(ptr == nil)
		return Malloc(size);

	// weird way to round up
	if((size & 0xF) != 0)
		size = (size&~0xF) + 0x10;

	HeapBlockDesc *block = GetDescFromHeapPointer(ptr);

#ifdef FIX_BUGS
	// better handling of size < block->m_size
	if(size == 0){
		Free(ptr);
		return nil;
	}
	if(block->m_size >= size){
		// shrink allocated block
		RegisterFree(block);
		PushMemId(block->m_memId);
		FillInBlockData(block, block->GetNextConsecutive(), size);
		PopMemId();
		return ptr;
	}
#else
	// not growing. just returning here is a bit cheap though
	if(block->m_size >= size)
		return ptr;
#endif

	// have to grow allocated block
	HeapBlockDesc *next = block->GetNextConsecutive();
	MEMORYHEAP_ASSERT_MESSAGE(next >= m_start && next <= m_end, "Block outside of memory");
	if(next->m_memId == MEMID_FREE){
		// try to grow the current block
		// make sure the next free block has maximum size
		uint32 freespace = CombineFreeBlocks(next);
		HeapBlockDesc *end = next->GetNextConsecutive();
		MEMORYHEAP_ASSERT_MESSAGE(end >= m_start && end <= m_end, "Block outside of memory");
		// why the sizeof here?
		if(block->m_size + next->m_size + sizeof(HeapBlockDesc) >= size){
			// enough space to grow
			next->RemoveHeapFreeBlock();
			RegisterFree(block);
			PushMemId(block->m_memId);
			FillInBlockData(block, next->GetNextConsecutive(), size);
			PopMemId();
			return ptr;
		}
	}

	// can't grow the existing block, have to get a new one and copy
	PushMemId(block->m_memId);
	void *dst = Malloc(size);
	PopMemId();
	memcpy(dst, ptr, block->m_size);
	Free(ptr);
	return dst;
}

void
CMemoryHeap::Free(void *ptr)
{
	HeapBlockDesc *block = GetDescFromHeapPointer(ptr);
	MEMORYHEAP_ASSERT_MESSAGE(block->m_memId != MEMID_FREE, "MemoryHeap corrupt");
	MEMORYHEAP_ASSERT(m_unkMemId == -1 || m_unkMemId == block->m_memId);

	RegisterFree(block);
	block->m_memId = MEMID_FREE;
	CombineFreeBlocks(block);
	FreeBlock(block);
	if(block->m_ptrListIndex != -1){
		int32 idx = block->m_ptrListIndex;
		gPtrList[idx] = nil;
		m_ptrListIndexStack.push(idx);
	}
	block->m_ptrListIndex = -1;
}

// allocate 'size' bytes from 'block'
void
CMemoryHeap::FillInBlockData(HeapBlockDesc *block, HeapBlockDesc *end, uint32 size)
{
	block->m_size = size;
	block->m_ptrListIndex = -1;
	HeapBlockDesc *remainder = block->GetNextConsecutive();
	MEMORYHEAP_ASSERT(remainder <= end);

	if(remainder < end-1){
		RegisterMalloc(block);

		// can fit another block in the remaining space
		remainder->m_size = GetSizeBetweenBlocks(remainder, end);
		remainder->m_memId = MEMID_FREE;
		MEMORYHEAP_ASSERT(remainder->m_size != 0);
		FreeBlock(remainder);
	}else{
		// fully allocate this one
		if(remainder < end)
			// no gaps allowed
			block->m_size = GetSizeBetweenBlocks(block, end);
		RegisterMalloc(block);
	}
}

// Make sure free block has no other free blocks after it
uint32
CMemoryHeap::CombineFreeBlocks(HeapBlockDesc *block)
{
	HeapBlockDesc *next = block->GetNextConsecutive();
	if(next->m_memId != MEMID_FREE)
		return block->m_size;
	// get rid of free blocks after this one and adjust size
	for(; next->m_memId == MEMID_FREE; next = next->GetNextConsecutive())
		next->RemoveHeapFreeBlock();
	block->m_size = GetSizeBetweenBlocks(block, next);
	return block->m_size;
}

// Try to move all registered memory blocks into more optimal location
void
CMemoryHeap::TidyHeap(void)
{
	for(int i = 0; i < numPtrs; i++){
		if(gPtrList[i] == nil || *gPtrList[i] == nil)
			continue;
		HeapBlockDesc *newblock = WhereShouldMemoryMove(*gPtrList[i]);
		if(newblock)
			*gPtrList[i] = MoveHeapBlock(newblock, GetDescFromHeapPointer(*gPtrList[i]));
	}
}

// MIAMI: this is empty
void
CMemoryHeap::RegisterMemPointer(void *ptr)
{
	HeapBlockDesc *block = GetDescFromHeapPointer(*(void**)ptr);

	if(block->m_ptrListIndex != -1)
		return;		// already registered

	int index;
	if(m_ptrListIndexStack.sp > 0){
		// re-use a previously free'd index
		index = m_ptrListIndexStack.pop();
	}else{
		// have to find a new index
		index = gPosnInList;

		void **pp = gPtrList[index];
		// we're replacing an old pointer here??
		if(pp && *pp && *pp != (void*)0xDDDDDDDD)
			GetDescFromHeapPointer(*pp)->m_ptrListIndex = -1;

		gPosnInList++;
		if(gPosnInList == 4000)
			gPosnInList = 0;
		if(numPtrs < 4000)
			numPtrs++;
	}
	gPtrList[index] = (void**)ptr;
	block->m_ptrListIndex = index;
}

void*
CMemoryHeap::MoveMemory(void *ptr)
{
	HeapBlockDesc *newblock = WhereShouldMemoryMove(ptr);
	if(newblock)
		return MoveHeapBlock(newblock, GetDescFromHeapPointer(ptr));
	else
		return ptr;
}

HeapBlockDesc*
CMemoryHeap::WhereShouldMemoryMove(void *ptr)
{
	HeapBlockDesc *block = GetDescFromHeapPointer(ptr);
	MEMORYHEAP_ASSERT(block->m_memId != MEMID_FREE);

	HeapBlockDesc *next = block->GetNextConsecutive();
	if(next->m_memId != MEMID_FREE)
		return nil;

	// we want to move the block into another block
	// such that the free space between this and the next block can be minimized
	HeapBlockDesc *newblock = m_freeList.m_first.FindSmallestFreeBlock(block->m_size);
	// size of free space wouldn't decrease, so return
	if(newblock->m_size >= block->m_size + next->m_size)
		return nil;
	// size of free space wouldn't decrease enough
	if(newblock->m_size >= 16 + 1.125f*block->m_size)	// what are 16 and 1.125 here? sizeof(HeapBlockDesc)?
		return nil;
	return newblock;
}

void*
CMemoryHeap::MoveHeapBlock(HeapBlockDesc *dst, HeapBlockDesc *src)
{
	PushMemId(src->m_memId);
	dst->RemoveHeapFreeBlock();
	FillInBlockData(dst, dst->GetNextConsecutive(), src->m_size);
	PopMemId();
	memcpy(dst->GetDataPointer(), src->GetDataPointer(), src->m_size);
	memMoved += src->m_size;
	dst->m_ptrListIndex = src->m_ptrListIndex;
	src->m_ptrListIndex = -1;
	Free(src->GetDataPointer());
	return dst->GetDataPointer();
}

uint32
CMemoryHeap::GetMemoryUsed(int32 id)
{
	return m_memUsed[id];
}

uint32
CMemoryHeap::GetBlocksUsed(int32 id)
{
	return m_blocksUsed[id];
}

void
CMemoryHeap::PopMemId(void)
{
	assert(m_idStack.sp > 0);
	m_currentMemID = m_idStack.pop();
	assert(m_currentMemID != MEMID_FREE);
}

void
CMemoryHeap::PushMemId(int32 id)
{
	MEMORYHEAP_ASSERT(id != MEMID_FREE);
	assert(m_idStack.sp < 16);
	m_idStack.push(m_currentMemID);
	m_currentMemID = id;
}

void
CMemoryHeap::ParseHeap(void)
{
	char tmp[16];
	int fd = CFileMgr::OpenFileForWriting("heap.txt");
	CTimer::Stop();

	// CMemoryHeap::IntegrityCheck();

	uint32 addrQW = 0;
	for(HeapBlockDesc *block = m_start; block < m_end; block = block->GetNextConsecutive()){
		char chr = '*';	// free
		if(block->m_memId != MEMID_FREE)
			chr = block->m_memId-1 + 'A';
		int numQW = block->m_size>>4;

		if((addrQW & 0x3F) == 0){
			sprintf(tmp, "\n%5dK:", addrQW>>6);
			CFileMgr::Write(fd, tmp, 8);
		}
		CFileMgr::Write(fd, "#", 1);	// the descriptor, has to be 16 bytes!!!!
		addrQW++;

		while(numQW--){
			if((addrQW & 0x3F) == 0){
				sprintf(tmp, "\n%5dK:", addrQW>>6);
				CFileMgr::Write(fd, tmp, 8);
			}
			CFileMgr::Write(fd, &chr, 1);
			addrQW++;
		}
	}

	CTimer::Update();
	CFileMgr::CloseFile(fd);
}


void
CommonSize::Init(uint32 size)
{
	m_freeList.Init();
	m_size = size;
	m_failed = 0;
	m_remaining = 0;
}

#endif
