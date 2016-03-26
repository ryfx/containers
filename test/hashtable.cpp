#include "jc_test.h"
#include "hashtable_rh.h"


#include <stdlib.h>

//https://attractivechaos.wordpress.com/2008/10/07/another-look-at-my-old-benchmark/

// tables to test
// EASTL: hash_map.h, fixed_hash_map.h
// STL: map, unordered_map
// Foundation: hashmap.h, hashtable.h   https://github.com/rampantpixels/foundation_lib
// ..//ProDBG/src/external/bgfx/3rdparty/glsl-optimizer/src/util/hash_table.h
// ..//ProDBG/src/external/foundation_lib/foundation/hashtable.h
// Bullet: bullet-2.82-r2704/src/LinearMath/btHashMap.h
// boost: hash_map, flat_map
// khash

const uint32_t STRESS_COUNT = 2000000;
const uint32_t PERCENT = 90;
const uint64_t EMPTY_KEY = 0xBAADC0D3F00DF17E;

#include <map>
#include <unordered_map>

struct SPod
{
	int a;
	int b;
	
	bool operator== (const SPod& rhs) const
	{
		return memcmp(this, &rhs, sizeof(SPod)) == 0;
	}
};

typedef jc::HashTable<uint64_t, SPod> TestHT64;

typedef struct SCtx
{
	uint32_t 	count;
	uint32_t 	memorysize;
	void* 		memory;
	TestHT64	ht;
} SCtx;

static SCtx* hashtable_main_setup()
{
	return reinterpret_cast<SCtx*>( malloc( sizeof(SCtx) ) );
}

static void hashtable_main_teardown(SCtx* ctx)
{
	free(ctx);
}

static void test_setup(SCtx* ctx)
{
	ctx->count = 10;
	//ctx->memorysize = TestHT64::CalcSize(1023, ctx->count);
	ctx->memorysize = TestHT64::CalcSize(ctx->count);
	ctx->memory = malloc( ctx->memorysize );
	//ctx->ht.Create(1023, ctx->count, ctx->memory);
	ctx->ht.Create(ctx->count, EMPTY_KEY, ctx->memory);
}

static void test_teardown(SCtx* ctx)
{
	free(ctx->memory);
}

static void hashtable_create(SCtx* ctx)
{
	//TestHT64 ht(1023, ctx->count, ctx->memory);
	TestHT64 ht(ctx->count, EMPTY_KEY, ctx->memory);
	
	ASSERT_TRUE( ht.Empty() );
	
	//ctx->ht.Create(1023, ctx->count, ctx->memory);
	ctx->ht.Create(ctx->count, EMPTY_KEY, ctx->memory);

	ASSERT_TRUE( ctx->ht.Empty() );
}

static void hashtable_insert_remove(SCtx* ctx)
{
	TestHT64& ht = ctx->ht;

	ASSERT_TRUE( ht.Empty() );
	ASSERT_EQ( 0, ht.Size() );
	
	for( uint32_t i = 0; i < ctx->count; ++i )
	{
		SPod v = { 1, 2 };
		ht.Put( 1234, v );
		
		ASSERT_EQ( 1, ht.Size() );

		const SPod* value = ht.Get(1234);
		ASSERT_TRUE( value != 0 );
		ASSERT_EQ( v, *value );

		ht.Erase(1234);
		
		ASSERT_TRUE( ht.Empty() );
		ASSERT_EQ( 0, ht.Size() );
	}
	
	// Reinsert it
	SPod v2 = { 2, 3 };
	ht.Put( 1234, v2 );
	ASSERT_EQ( 1, ht.Size() );
	const SPod* value = ht.Get(1234);
	ASSERT_TRUE( value != 0 );
	ASSERT_EQ( v2, *value );
	
	// Change it
	SPod v3 = { 3, 4 };
	ASSERT_EQ( 1, ht.Size() );
	ht.Put( 1234, v3 );
	ASSERT_EQ( 1, ht.Size() );
	value = ht.Get(1234);
	ASSERT_TRUE( value != 0 );
	ASSERT_EQ( v3, *value );

}

static void hashtable_iterate(SCtx* ctx)
{
	(void)ctx;
	uint32_t count = 200;
	uint32_t memorysize = TestHT64::CalcSize(count);
	void* memory = malloc( memorysize );
	
	TestHT64 ht2;
	ht2.Create(count, EMPTY_KEY, memory);
	
	srand(0);
	
	std::map<uint64_t, SPod> comparison;
	
	// insert
	for( uint32_t i = 0; i < count; ++i )
	{
		uint64_t key = static_cast<uint64_t>( rand() );
		int v1 = rand();
		int v2 = rand();
		SPod pod = {v1, v2};

		ht2.Put( key, pod );
		
		comparison[key] = pod;
	}
	ASSERT_EQ( count, ht2.Size() );

	TestHT64::Iterator it = ht2.Begin();
	TestHT64::Iterator itend = ht2.End();
	for( ; it != itend; ++it )
	{
		const uint64_t* key = it.GetKey();
		const SPod* value = it.GetValue();
		
		std::map<uint64_t, SPod>::const_iterator compit = comparison.find(*key);
		ASSERT_TRUE( compit != comparison.end() );
		ASSERT_EQ( compit->first, *key );
		ASSERT_EQ( compit->second, *value );
	}
	
	free(memory);
}

static void hashtable_stress(SCtx* ctx)
{
	(void)ctx;
	uint32_t count = 10;
	uint32_t capacity = count;
	//uint32_t memorysize = TestHT64::CalcSize(table_size, capacity);
	uint32_t memorysize = TestHT64::CalcSize(capacity);
	void* memory = malloc( memorysize );
	
	TestHT64 ht;
	//ht.Create(table_size, capacity, memory);
	ht.Create(capacity, EMPTY_KEY, memory);
	
	srand(0);
	
	std::map<uint64_t, SPod> comparison;
	
	// insert
	for( uint32_t i = 0; i < (count*PERCENT)/100; ++i )
	{
		uint32_t key = static_cast<uint32_t>( rand() );
		int v1 = rand();
		int v2 = rand();
		SPod pod = {v1, v2};

		ht.Put( static_cast<uint64_t>(key), pod );
		
		comparison[key] = pod;
		
		//printf("PUT Key %d: %u\n", i, key);
	}
	ASSERT_EQ( (count*PERCENT)/100, ht.Size() );

	TestHT64::Iterator testit = ht.Begin();
	
	//printf("\ntest\n");
	
	TestHT64::Iterator testitend = ht.End();
	for( ; testit != testitend; ++testit )
	{
		uint64_t key = *testit.GetKey();
		//printf("Key: %u\n", key);
		
		std::map<uint64_t, SPod>::const_iterator compit = comparison.find(key);
		ASSERT_TRUE( compit != comparison.end() );
		ASSERT_EQ( compit->first, *testit.GetKey() );
		ASSERT_EQ( compit->second, *testit.GetValue() );
	}


	std::map<uint64_t, SPod>::const_iterator it = comparison.begin();
	for( ; it != comparison.end(); ++it )
	{
		const uint64_t key = it->first;
		const SPod& p = it->second;
		
		const SPod* value = ht.Get(key);
		ASSERT_TRUE( value != 0 );
		ASSERT_EQ( p, *value );

		ht.Erase(key);
	}

	ASSERT_EQ( 0, ht.Size() );
	
	free(memory);
}


TEST_BEGIN(hashtable_test, hashtable_main_setup, hashtable_main_teardown, test_setup, test_teardown)
    TEST(hashtable_create)
    TEST(hashtable_insert_remove)
    TEST(hashtable_iterate)
    TEST(hashtable_stress)
TEST_END(hashtable_test)

