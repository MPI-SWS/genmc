#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>

/* A type whose size is a power of two must be naturally aligned to be operated
 * on atomically, however weak its own alignment requirement is. */
struct Unaligned {
	char b[8];
};

/* char followed by int leaves padding bits, which a compare-exchange has to
 * ignore. */
struct Padded {
	char c;
	int i;
};

static_assert(alignof(Unaligned) < 8, "expected a weakly aligned type");
static_assert(std::atomic_ref<Unaligned>::required_alignment == 8, "");
static_assert(std::atomic_ref<std::uint32_t>::required_alignment == 4, "");
static_assert(alignof(Padded) == 4 && sizeof(Padded) == 8, "expected padding and weak alignment");
static_assert(std::atomic_ref<Padded>::required_alignment == 8, "");

/* Padded asks for more alignment than the type itself declares, so say so
 * rather than relying on where the linker happens to put it. */
alignas(std::atomic_ref<Padded>::required_alignment) Padded obj = {1, 2};

int main()
{
	std::atomic_ref<Padded> r{obj};
	Padded expected;
	unsigned char *bytes = reinterpret_cast<unsigned char *>(&expected);

	r.store(Padded{3, 4});

	/* Give `expected` padding that differs from the object's. */
	for (unsigned k = 0; k < sizeof(Padded); ++k)
		bytes[k] = 0xFF;
	expected.c = 3;
	expected.i = 4;

	assert(r.compare_exchange_strong(expected, Padded{5, 6}));

	Padded got = r.load();
	assert(got.c == 5 && got.i == 6);

	/* On failure the caller's expected takes the value that was read. */
	expected.c = 0;
	expected.i = 0;
	assert(!r.compare_exchange_strong(expected, Padded{7, 8}));
	assert(expected.c == 5 && expected.i == 6);

	return 0;
}
