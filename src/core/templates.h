#pragma once

template<typename T, int n>
class CStore
{
public:
	int allocPtr;
	T store[n];

	T *alloc(void){
		if(this->allocPtr >= n){
			printf("Size of this thing:%d needs increasing\n", n);
			assert(0);
		}
		return &this->store[this->allocPtr++];
	}
	void clear(void){
		this->allocPtr = 0;
	}
	int getIndex(T *item){
		assert(item >= &this->store[0]);
		assert(item < &this->store[n]);
		return item - this->store;
	}
	T *getItem(int index){
		assert(index >= 0);
		assert(index < n);
		return &this->store[index];
	}
};

template<typename T, typename U = T>
class CPool
{
	U     *m_entries;
	union Flags {
		struct {
			uint8 id   : 7;
			uint8 free : 1;
		};
			uint8 u;
	}     *m_flags;
	int    m_size;
	int    m_allocPtr;

public:
	CPool(int size){
		// TODO: use new here
		m_entries = (U*)new uint8[sizeof(U)*size];
		m_flags = (Flags*)new uint8[sizeof(Flags)*size];
		m_size = size;
		m_allocPtr = 0;
		for(int i = 0; i < size; i++){
			m_flags[i].id   = 0;
			m_flags[i].free = 1;
		}
	}

	~CPool() {
		Flush();
	}
	void Flush() {
		if (m_size > 0) {
			delete[] (uint8*)m_entries;
			delete[] (uint8*)m_flags;
			m_entries = nil;
			m_flags = nil;
			m_size = 0;
			m_allocPtr = 0;
		}
	}
	int GetSize(void) const { return m_size; }
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
		while(!m_flags[m_allocPtr].free);
		m_flags[m_allocPtr].free = 0;
		m_flags[m_allocPtr].id++;
		return (T*)&m_entries[m_allocPtr];
	}
	T *New(int handle){
		T *entry = (T*)&m_entries[handle>>8];
		SetNotFreeAt(handle);
		return entry;
	}
	void SetNotFreeAt(int handle){
		int idx = handle>>8;
		m_flags[idx].free = 0;
		m_flags[idx].id = handle & 0x7F;
		for(m_allocPtr = 0; m_allocPtr < m_size; m_allocPtr++)
			if(m_flags[m_allocPtr].free)
				return;
	}
	void Delete(T *entry){
		int i = GetJustIndex(entry);
		m_flags[i].free = 1;
		if(i < m_allocPtr)
			m_allocPtr = i;
	}
	T *GetSlot(int i){
		return m_flags[i].free ? nil : (T*)&m_entries[i];
	}
	T *GetAt(int handle){
#ifdef FIX_BUGS
		if (handle == -1)
			return nil;
#endif
		return m_flags[handle>>8].u == (handle & 0xFF) ?
		       (T*)&m_entries[handle >> 8] : nil;
	}
	int GetIndex(T *entry){
		int i = GetJustIndex_NoFreeAssert(entry);
		return m_flags[i].u + (i<<8);
	}
	int GetJustIndex(T *entry){
		int index = GetJustIndex_NoFreeAssert(entry);
		assert(!IsFreeSlot(index));
		return index;
	}
	int GetJustIndex_NoFreeAssert(T* entry){
		int index = ((U*)entry - m_entries);
		assert((U*)entry == (U*)&m_entries[index]); // cast is unsafe - check required
		return index;
	}
	int GetNoOfUsedSpaces(void) const{
		int i;
		int n = 0;
		for(i = 0; i < m_size; i++)
			if(!m_flags[i].free)
				n++;
		return n;
	}
	bool IsFreeSlot(int i) { return !!m_flags[i].free; }
	void ClearStorage(uint8 *&flags, U *&entries){
		delete[] (uint8*)flags;
		delete[] (uint8*)entries;
		flags = nil;
		entries = nil;
	}
	uint32 GetMaxEntrySize() const { return sizeof(U); }
	void CopyBack(uint8 *&flags, U *&entries){
		memcpy(m_flags, flags, sizeof(uint8)*m_size);
		memcpy(m_entries, entries, sizeof(U)*m_size);
		debug("Size copied:%d (%d)\n", sizeof(U)*m_size, sizeof(Flags)*m_size);
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
	int Count(void){
		int n = 0;
		CLink<T> *lnk;
		for(lnk = head.next; lnk != &tail; lnk = lnk->next)
			n++;
		return n;
	}
};
