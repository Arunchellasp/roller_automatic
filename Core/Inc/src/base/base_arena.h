
#ifndef BASE_ARENA_H
#define BASE_ARENA_H

typedef struct Arena Arena;
struct Arena
{
	u32 pos;
	u32 size;
};
#define ARENA_HEADER_SIZE 32
StaticAssert(sizeof(Arena) <= ARENA_HEADER_SIZE, arena_header_size_check);

typedef struct Temp Temp;
struct Temp
{
	Arena *arena;
	u32 pos;
};

#define push_array(arena, type, count) (type *)MemoryZero(arena_push(arena, sizeof(type) * count), sizeof(type) * count)

internal Arena *
arena_alloc(u32 size)
{
	Arena *arena = (Arena *)calloc(size, 1);
	Assert(arena);
	arena->size = size;
	arena->pos = ARENA_HEADER_SIZE;
	return arena;
};

internal void
arena_release(Arena *arena)
{
	free((void *)arena);
}

internal void *
arena_push(Arena *arena, u32 size)
{
	Assert((arena->pos + size) <= arena->size);
	void *result = (void *)(((u8 *)arena) + arena->pos);
	arena->pos += size;
	return result;
};

internal void 
arena_pop_to(Arena *arena, u32 pos)
{
	pos = ClampBot(ARENA_HEADER_SIZE, pos);
	arena->pos = pos;
}

internal Temp
temp_begin(Arena *arena)
{
	Temp result = {.arena = arena, .pos = arena->pos};
	return result;
}

internal void
temp_end(Temp temp)
{
	temp.arena->pos = temp.pos;
}

#endif //BASE_ARENA_H
