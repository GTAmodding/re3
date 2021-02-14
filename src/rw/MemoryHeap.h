#pragma once

// some windows shit
#ifdef MoveMemory
#undef MoveMemory
#endif

#ifdef USE_CUSTOM_ALLOCATOR
#define PUSH_MEMID(id) gMainHeap.PushMemId(id)
#define POP_MEMID() gMainHeap.PopMemId()
#define REGISTER_MEMPTR(ptr) gMainHeap.RegisterMemPointer(ptr)
#else
#define PUSH_MEMID(id)
#define POP_MEMID()
#define REGISTER_MEMPTR(ptr)
#endif

enum {
	MEMID_FREE,
	MEMID_GAME = 1,	// "Game"
	MEMID_WORLD = 2,	// "World"
	MEMID_ANIMATION = 3,	// "Animation"
	MEMID_POOLS = 4,	// "Pools"
	MEMID_DEF_MODELS = 5,	// "Default Models"
	MEMID_STREAM = 6,	// "Streaming"
	MEMID_STREAM_MODELS = 7,	// "Streamed Models" (instance)
	MEMID_STREAM_TEXUTRES = 8,	// "Streamed Textures"
	MEMID_TEXTURES = 9,	// "Textures"
	MEMID_COLLISION = 10,	// "Collision"
	MEMID_RENDERLIST = 11,	// ?
	MEMID_GAME_PROCESS = 12,	// "Game Process"
	MEMID_SCRIPT = 13,	// "Script"
	MEMID_CARS = 14,	// "Cars"
	MEMID_RENDER = 15,	// "Render"
	MEMID_FRONTEND = 17,	// ?

	NUM_MEMIDS,

	NUM_FIXED_MEMBLOCKS = 6
};

template<typename T, uint32 N>
class CStack
{
public:
	T values[N];
	uint32 sp;

	CStack() : sp(0) {}
	void push(const T& val) { values[sp++] = val; }
	T& pop() { return values[--sp]; }
};


struct HeapBlockDesc
{
	uint32 m_size;
	int16 m_memId;
	int16 m_ptrListIndex;
	HeapBlockDesc *m_next;
	HeapBlockDesc *m_prev;

	HeapBlockDesc *GetNextConsecutive(void)
	{
		return (HeapBlockDesc*)((uintptr)this + sizeof(HeapBlockDesc) + m_size);
	}

	void *GetDataPointer(void)
	{
		return (void*)((uintptr)this + sizeof(HeapBlockDesc));
	}

	void RemoveHeapFreeBlock(void)
	{
		m_next->m_prev = m_prev;
		m_prev->m_next = m_next;
	}

	// after node
	void InsertHeapFreeBlock(HeapBlockDesc *node)
	{
		m_next = node->m_next;
		node->m_next->m_prev = this;
		m_prev = node;
		node->m_next = this;
	}

	HeapBlockDesc *FindSmallestFreeBlock(uint32 size)
	{
		HeapBlockDesc *b;
		for(b = m_next; b->m_size < size; b = b->m_next);
		return b;
	}
};

#ifdef USE_CUSTOM_ALLOCATOR
// TODO: figure something out for 64 bit pointers
static_assert(sizeof(HeapBlockDesc) == 0x10, "HeapBlockDesc must have 0x10 size otherwise most of assumptions don't make sense");
#endif

struct HeapBlockList
{
	HeapBlockDesc m_first;
	HeapBlockDesc m_last;

	void Init(void)
	{
		m_first.m_next = &m_last;
		m_last.m_prev = &m_first;
	}

	void Insert(HeapBlockDesc *node)
	{
		node->InsertHeapFreeBlock(&m_first);
	}
};

struct CommonSize
{
	HeapBlockList m_freeList;
	uint32 m_size;
	uint32 m_failed;
	uint32 m_remaining;

	void Init(uint32 size);
	void Free(HeapBlockDesc *node)
	{
		m_freeList.Insert(node);
		m_remaining++;
	}
	HeapBlockDesc *Malloc(void)
	{
		if(m_freeList.m_first.m_next == &m_freeList.m_last){
			m_failed++;
			return nil;
		}
		HeapBlockDesc *block = m_freeList.m_first.m_next;
		m_remaining--;
		block->RemoveHeapFreeBlock();
		block->m_ptrListIndex = -1;
		return block;
	}
};

class CMemoryHeap
{
public:
	HeapBlockDesc *m_start;
	HeapBlockDesc *m_end;
	HeapBlockList m_freeList;
	CommonSize m_fixedSize[NUM_FIXED_MEMBLOCKS];
	uint32 m_totalMemUsed;
	CStack<int32, 16> m_idStack;
	uint32 m_currentMemID;
	uint32 *m_memUsed;
	uint32 m_totalBlocksUsed;
	uint32 *m_blocksUsed;
	uint32 m_unkMemId;

	CMemoryHeap(void) : m_start(nil) {}
	void Init(uint32 total);
	void RegisterMalloc(HeapBlockDesc *block);
	void RegisterFree(HeapBlockDesc *block);
	void *Malloc(uint32 size);
	void *Realloc(void *ptr, uint32 size);
	void Free(void *ptr);
	void FillInBlockData(HeapBlockDesc *block, HeapBlockDesc *end, uint32 size);
	uint32 CombineFreeBlocks(HeapBlockDesc *block);
	void *MoveMemory(void *ptr);
	HeapBlockDesc *WhereShouldMemoryMove(void *ptr);
	void *MoveHeapBlock(HeapBlockDesc *dst, HeapBlockDesc *src);
	void PopMemId(void);
	void PushMemId(int32 id);
	void RegisterMemPointer(void *ptr);
	void TidyHeap(void);
	uint32 GetMemoryUsed(int32 id);
	uint32 GetBlocksUsed(int32 id);
	int32 GetLargestFreeBlock(void) { return m_freeList.m_last.m_prev->m_size; }

	void ParseHeap(void);

	HeapBlockDesc *GetDescFromHeapPointer(void *block)
	{
		return (HeapBlockDesc*)((uintptr)block - sizeof(HeapBlockDesc));
	}
	uint32 GetSizeBetweenBlocks(HeapBlockDesc *first, HeapBlockDesc *second)
	{
		return (uintptr)second - (uintptr)first - sizeof(HeapBlockDesc);
	}
	void FreeBlock(HeapBlockDesc *block){
		for(int i = 0; i < NUM_FIXED_MEMBLOCKS; i++){
			if(m_fixedSize[i].m_size == block->m_size){
				m_fixedSize[i].Free(block);
				return;
			}
		}
		HeapBlockDesc *b = m_freeList.m_first.FindSmallestFreeBlock(block->m_size);
		block->InsertHeapFreeBlock(b->m_prev);
	}
};

extern CMemoryHeap gMainHeap;
