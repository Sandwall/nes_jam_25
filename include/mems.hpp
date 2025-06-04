#pragma once

#include <stdint.h>
#include <stddef.h>

namespace mems {
	// Linear allocator used to group together allocations
	// On Memory Arenas:
	// https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator
	struct Arena {
#define push_struct(ptr, struc) push_data(ptr, sizeof(struc))
		void alloc(uint64_t cap = 1000LL * 1000LL * 1000LL / 4LL);
		void dealloc();

		void clear();
		void clear_decommit();
		void* peek();
		void* push(size_t len);
		void* push_data(void* pData, size_t sizeData);
		void* push_zero(size_t len);
		void pop(size_t len);
		void pop_to(size_t newPos);

		void* data;
		size_t pos;
		size_t capacity;
	};

	// Helper struct meant to automatically handle temporary allocations
	// Constructor records the current pos of the arena, and the destructor pops to that old pos
	struct ArenaScope {
		ArenaScope(Arena& a);
		~ArenaScope();

	private:
		size_t startPos;
		Arena& arena;
	};

	void* load_file(Arena&, const char*, size_t&);

	void init();
	void close();
	Arena& get_scratch();
}

#ifdef MEMS_IMPLEMENTATION

#include <stdio.h>
#include <assert.h>

#if defined(_WIN32)
#define MEMS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define MEMS_UNIX
#error mems.cpp not implemented for Unix-type systems!
#else
#error mems.cpp not implemented for this platform!
#endif

//
// MEMORY ABSTRACTION IMPLEMENTATION
//

namespace mems {

	// 4096 is a common page size, but for it to actually be accurate,
	// we need to set this using information obtained from the OS in mems::init()
	uint64_t pageSize = 4096;
	uint64_t round_to_page_size(uint64_t size) {
		uint64_t rem = size % pageSize;
		size -= rem;
		return size + pageSize;
	}

#if defined(MEMS_WIN)
	inline uint64_t get_page_size() {
		SYSTEM_INFO si = { 0 };
		GetSystemInfo(&si);
		return si.dwPageSize;
	}

	inline void* _reserve(size_t cap) {
		// maybe round_to_page_size should be used here?
		return VirtualAlloc(nullptr, cap, MEM_RESERVE, PAGE_READWRITE);
	}

	inline void* _commit(void* start, size_t size) {
		return VirtualAlloc(start, size, MEM_COMMIT, PAGE_READWRITE);
	}

	inline bool _release(void* region) {
		return VirtualFree(region, 0, MEM_RELEASE);
	}

#pragma warning(disable : 6250)
	inline bool _decommit(void* region, size_t size) {
		return VirtualFree(region, size, MEM_DECOMMIT);
	}

#elif defined(MEMS_UNIX)
	inline uint64_t get_page_size() {
		return 4096;
	}

	inline void* _reserve(size_t cap) {
		return nullptr;
	}

	inline void* _commit(void* start, size_t size) {
		return nullptr;
	}

	inline bool _release(void* region) {
		return false;
	}

	inline bool _decommit(void* region, size_t size) {
		return false;
	}

#endif

	//
	// MEMORY STRUCTURES IMPLEMENTATION
	// NOTE: Platform specific code is above
	//

	// This function simply loads a file into an arena
	void* load_file(Arena& arena, const char* path, size_t& size) {
		::FILE* fp = fopen(path, "rb");
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		void* buffer = arena.push(size);
		fread(buffer, size, 1, fp);
		fclose(fp);
		return buffer;
	}

	Arena scratchArena;

	void init() {
		pageSize = get_page_size();
		scratchArena.alloc();
	}

	void close() {
		scratchArena.dealloc();
	}

	Arena& get_scratch() {
		// we're not clearing here anymore since anyone that calls this function
		// might not specifically want to clear the arena unnecessarily

		// scratchArena.clear();
		return scratchArena;
	}

	/* Got rid of end_scratch, because like, just call scratch.clear if you really need to */
	//void end_scratch(Arena& a) {
	//	a.clear();
	//}

	void Arena::alloc(uint64_t cap) {
		// we'll reserve 0.25GB for each Arena, which is a good spot between
		// "too little" and "holy balls that's too much" memory
		// when we encounter the problem of this actually being too little memory,
		// we can probably either raise this size, or start chaining Arenas
		capacity = round_to_page_size(cap);
		data = mems::_reserve(capacity);

		// We'll commit the first page of memory, so that we can initially make use of it
		mems::_commit(data, pageSize);
	}

	void Arena::dealloc() {
		clear_decommit(); // added this, not sure if it's really required
		// we can do profiling and testing for that

		mems::_release(data);
	}

	// This function is called "peek", and while it might make sense for it to be called that
	// It's important to remember that the context of this function is to obtain a memory address
	// to the next spot that can be allocated at. It's useful if you want to initialize an array
	// by appending to it (this is a less hacky way of doing "data = Arena::push(1); Arena::pop();")
	void* Arena::peek() {
		return static_cast<uint8_t*>(data) + pos;
	}

	// Returns a pointer to len bytes of memory
	void* Arena::push(size_t len) {
		assert(pos + len < capacity);
		_commit(static_cast<uint8_t*>(data) + pos, len);
		uint64_t prev = pos;
		pos += len;
		return static_cast<uint8_t*>(data) + prev;
	}

	// Copies pData into the arena and returns a pointer to it
	void* Arena::push_data(void* pData, size_t sizeData) {
		assert(pos + sizeData < capacity);
		_commit(static_cast<uint8_t*>(data) + pos, sizeData);
		uint64_t prev = pos;
		memcpy(static_cast<uint8_t*>(data) + pos, pData, sizeData);
		pos += sizeData;
		return static_cast<uint8_t*>(data) + prev;
	}

	// Returns a pointer to len zero-initialized bytes
	void* Arena::push_zero(size_t len) {
		assert(pos + len < capacity);
		_commit(static_cast<uint8_t*>(data) + pos, len);
		uint64_t prev = pos;
		memset(static_cast<uint8_t*>(data) + pos, 0, len);
		pos += len;
		return static_cast<uint8_t*>(data) + prev;
	}

	// Undoes the most recent len bytes of allocation
	void Arena::pop(size_t len) {
		if (len > pos) pos = 0;
		else pos -= len;
	}

	// Instead of deallocating the top x bytes in the arena,
	// we "cut" the allocated bytes to newPos
	void Arena::pop_to(size_t newPos) {
		if (newPos > pos) return;
		pos = newPos;
	}

	void Arena::clear() {
		pos = 0;
	}

	// Decommits all memory except the first page
	void Arena::clear_decommit() {
		_decommit(static_cast<uint8_t*>(data) + pageSize, pos - pageSize);
		clear();
	}

	/* ArenaScope is handy, and the following example might illustrate why:
	*
	*	void bingus(i32 someNumber, Arena& arena) {
	*		ArenaScope arenaScope(arena);
	*		uint8_t* bytes = arena.push(someNumber);
	*
	*		// do something with bytes...
	*
	*		// now, at the end of the scope, arenaScope.~ArenaScope() runs and bytes is freed!
	*	}
	*/
	ArenaScope::ArenaScope(Arena& a) : arena(a) {
		startPos = a.pos;
	}

	ArenaScope::~ArenaScope() {
		arena.pop_to(startPos);
	}

}

#endif