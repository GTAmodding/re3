#pragma once

template<typename T, int32 n>
class CStore
{
public:
	int32 allocPtr;
	T store[n];

	T *Alloc(void){
		if(allocPtr >= n){
			printf("Size of this thing:%d needs increasing\n", n);
			assert(0);
		}
		return &store[allocPtr++];
	}
	void Clear(void){
		allocPtr = 0;
	}
	int32 GetIndex(T *item){
		assert(item >= &store[0]);
		assert(item < &store[n]);
		return item - store;
	}
	T *GetItem(int32 index){
		assert(index >= 0);
		assert(index < n);
		return &store[index];
	}
};

#define POOLFLAG_ID     0x7f
#define POOLFLAG_ISFREE 0x80

template<typename T, typename U = T>
class CPool
{
	U     *m_entries;
	uint8 *m_flags;
	int32  m_size;
	int32  m_allocPtr;

public:
	CPool(int32 size){
		m_entries = (U*)new uint8[sizeof(U)*size];
		m_flags = new uint8[size];
		m_size = size;
		m_allocPtr = 0;
		for(int i = 0; i < size; i++){
			SetId(i, 0);
			SetIsFree(i, true);
		}
	}

	int GetId(int i) const
	{
		return m_flags[i] & POOLFLAG_ID;
	}

	bool GetIsFree(int i) const
	{
		return !!(m_flags[i] & POOLFLAG_ISFREE);
	}

	void SetId(int i, int id)
	{
		m_flags[i] = (m_flags[i] & POOLFLAG_ISFREE) | (id & POOLFLAG_ID);
	}

	void SetIsFree(int i, bool isFree)
	{
		if (isFree)
			m_flags[i] |= POOLFLAG_ISFREE;
		else
			m_flags[i] &= ~POOLFLAG_ISFREE;
	}

	~CPool() {
		Flush();
	}
	void Flush() {
		if (m_size > 0) {
			delete[] (uint8*)m_entries;
			delete[] m_flags;
			m_entries = nil;
			m_flags = nil;
			m_size = 0;
			m_allocPtr = 0;
		}
	}
	int32 GetSize(void) const { return m_size; }
	T *New(void){
		bool wrapped = false;
		do
#ifdef FIX_BUGS
			if (++m_allocPtr >= m_size) {
				m_allocPtr = 0;
				if (wrapped)
					return nil;
				wrapped = true;
			}
#else
			if(++m_allocPtr == m_size){
				if(wrapped)
					return nil;
				wrapped = true;
				m_allocPtr = 0;
			}
#endif
		while(!GetIsFree(m_allocPtr));
		SetIsFree(m_allocPtr, false);
		SetId(m_allocPtr, GetId(m_allocPtr)+1);
		return (T*)&m_entries[m_allocPtr];
	}
	T *New(int32 handle){
		T *entry = (T*)&m_entries[handle>>8];
		SetNotFreeAt(handle);
		return entry;
	}
	void SetNotFreeAt(int32 handle){
		int idx = handle>>8;
		SetIsFree(idx, false);
		SetId(idx, handle & POOLFLAG_ID);
		for(m_allocPtr = 0; m_allocPtr < m_size; m_allocPtr++)
			if(GetIsFree(m_allocPtr))
				return;
	}
	void Delete(T *entry){
		int i = GetJustIndex(entry);
		SetIsFree(i, true);
		if(i < m_allocPtr)
			m_allocPtr = i;
	}
	T *GetSlot(int i){
		return GetIsFree(i) ? nil : (T*)&m_entries[i];
	}
	T *GetAt(int handle){
#ifdef FIX_BUGS
		if (handle == -1)
			return nil;
#endif
		return m_flags[handle>>8] == (handle & 0xFF) ?
		       (T*)&m_entries[handle >> 8] : nil;
	}
	int32 GetIndex(T *entry){
		int i = GetJustIndex_NoFreeAssert(entry);
		return m_flags[i] + (i<<8);
	}
	int32 GetJustIndex(T *entry){
		int index = GetJustIndex_NoFreeAssert(entry);
		assert(!GetIsFree(index));
		return index;
	}
	int32 GetJustIndex_NoFreeAssert(T* entry){
		int index = ((U*)entry - m_entries);
		assert((U*)entry == (U*)&m_entries[index]); // cast is unsafe - check required
		return index;
	}
	int32 GetNoOfUsedSpaces(void) const{
		int i;
		int n = 0;
		for(i = 0; i < m_size; i++)
			if(!GetIsFree(i))
				n++;
		return n;
	}
	void ClearStorage(uint8 *&flags, U *&entries){
		delete[] flags;
		delete[] (uint8*)entries;
		flags = nil;
		entries = nil;
	}
	uint32 GetMaxEntrySize() const { return sizeof(U); }
	void CopyBack(uint8 *&flags, U *&entries){
		memcpy(m_flags, flags, sizeof(uint8)*m_size);
		memcpy(m_entries, entries, sizeof(U)*m_size);
		debug("Size copied:%d (%d)\n", sizeof(U)*m_size, m_size);
		m_allocPtr = 0;
		ClearStorage(flags, entries);
		debug("CopyBack:%d (/%d)\n", GetNoOfUsedSpaces(), m_size); /* Assumed inlining */
	}
	void Store(uint8 *&flags, U *&entries){
		flags = (uint8*)new uint8[sizeof(uint8)*m_size];
		entries = (U*)new uint8[sizeof(U)*m_size];
		memcpy(flags, m_flags, sizeof(uint8)*m_size);
		memcpy(entries, m_entries, sizeof(U)*m_size);
		debug("Stored:%d (/%d)\n", GetNoOfUsedSpaces(), m_size); /* Assumed inlining */
	}
};

template<typename T>
class CLink
{
public:
	T item;
	CLink<T> *prev;
	CLink<T> *next;

	void Insert(CLink<T> *link){
		link->next = this->next;
		this->next->prev = link;
		link->prev = this;
		this->next = link;
	}
	void Remove(void){
		this->prev->next = this->next;
		this->next->prev = this->prev;
	}
};

template<typename T>
class CLinkList
{
public:
	CLink<T> head, tail;
	CLink<T> freeHead, freeTail;
	CLink<T> *links;

	void Init(int n){
		links = new CLink<T>[n];
		head.next = &tail;
		tail.prev = &head;
		freeHead.next = &freeTail;
		freeTail.prev = &freeHead;
		while(n--)
			freeHead.Insert(&links[n]);
	}
	void Shutdown(void){
		delete[] links;
		links = nil;
	}
	void Clear(void){
		while(head.next != &tail)
			Remove(head.next);
	}
	CLink<T> *Insert(T const &item){
		CLink<T> *node = freeHead.next;
		if(node == &freeTail)
			return nil;
		node->item = item;
		node->Remove();		// remove from free list
		head.Insert(node);
		return node;
	}
	CLink<T> *InsertSorted(T const &item){
		CLink<T> *sort;
		for(sort = head.next; sort != &tail; sort = sort->next)
			if(sort->item.sort >= item.sort)
				break;
		CLink<T> *node = freeHead.next;
		if(node == &freeTail)
			return nil;
		node->item = item;
		node->Remove();		// remove from free list
		sort->prev->Insert(node);
		return node;
	}
	void Remove(CLink<T> *link){
		link->Remove();		// remove from list
		freeHead.Insert(link);	// insert into free list
	}
	int32 Count(void){
		int n = 0;
		CLink<T> *lnk;
		for(lnk = head.next; lnk != &tail; lnk = lnk->next)
			n++;
		return n;
	}
};
