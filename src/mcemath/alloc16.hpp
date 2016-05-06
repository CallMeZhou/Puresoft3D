/*

I'm sorry I don't wanna waste time to explain this in detail. I just tell you why this little freak is here.

In one word, MCE uses a lot of new processor instructions, like sse/sse2, to boost its performance. But
those instructions require data to be aligned at 16-bytes boundary in memory. Visual C++'s __declspec(align(16))
doesn't help for heap allocation while all STL's containers use heap. So, this STL style allocator works
for STL containers adopted in MCE using _align_malloc to allocate memory.

I admit I don't know pretty much about STL's allocator, nor did I have time to study it just for such a trivial
problem - alignment. Maybe I would do it later when I get nothing to do.

Reference: the code was copied from Stephan T. Lavavej's blog. URL:
http://blogs.msdn.com/b/vcblog/archive/2008/08/28/the-mallocator.aspx
MANY THANKS TO STEPHAN T. LAVAVEJ'S.
*/
#pragma once
#include <stdlib.h>

template <typename T, int ALIGN = 16>
class alloc16
{
public:
	// The following will be the same for virtually all allocators.
	typedef T * pointer;
	typedef const T * const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	T * address(T& r) const { return &r; }
	const T * address(const T& s) const { return &s; }

	size_t max_size() const
	{
		// The following has been carefully written to be independent of
		// the definition of size_t and to avoid signed/unsigned warnings.
		return (static_cast<size_t>(0) - static_cast<size_t>(1)) / sizeof(T);
	}

	// The following must be the same for all allocators.
	template <typename U> struct rebind
	{
		typedef alloc16<U> other;
	};

	bool operator!=(const alloc16& other) const
	{
		return !(*this == other);
	}

	void construct(T * const p, const T& t) const
	{
		void * const pv = static_cast<void *>(p);
		new (pv) T(t);
	}

	void destroy(T * const p) const; // Defined below.

									 // Returns true if and only if storage allocated from *this
									 // can be deallocated from other, and vice versa.
									 // Always returns true for stateless allocators.
	bool operator==(const alloc16& other) const
	{
		return true;
	}

	// Default constructor, copy constructor, rebinding constructor, and destructor.
	// Empty for stateless allocators.
	alloc16() { }
	alloc16(const alloc16&) { }
	template <typename U> alloc16(const alloc16<U>&) { }
	~alloc16() { }

	// The following will be different for each allocator.
	T * allocate(const size_t n) const
	{
		// The return value of allocate(0) is unspecified.
		// alloc16 returns NULL in order to avoid depending
		// on malloc(0)'s implementation-defined behavior
		// (the implementation can define malloc(0) to return NULL,
		// in which case the bad_alloc check below would fire).
		// All allocators can return NULL in this case.
		if (n == 0)
		{
			return NULL;
		}

		// All allocators should contain an integer overflow check.
		// The Standardization Committee recommends that std::length_error
		// be thrown in the case of integer overflow.
		if (n > max_size())
		{
			throw std::length_error("alloc16<T>::allocate() - Integer overflow.");
		}

		// alloc16 wraps malloc().
		//void * const pv = malloc(n * sizeof(T));
		void * const pv = _aligned_malloc(n * sizeof(T), ALIGN);

		// Allocators should throw std::bad_alloc in the case of memory allocation failure.
		if (pv == NULL)
		{
			throw std::bad_alloc();
		}

		return static_cast<T *>(pv);
	}

	void deallocate(T * const p, const size_t n) const
	{
		// alloc16 wraps free().
		//free(p);
		_aligned_free(p);
	}

	// The following will be the same for all allocators that ignore hints.
	template <typename U> T * allocate(const size_t n, const U * /* const hint */) const
	{
		return allocate(n);
	}

	// Allocators are not required to be assignable, so
	// all allocators should have a private unimplemented
	// assignment operator. Note that this will trigger the
	// off-by-default (enabled under /Wall) warning C4626
	// "assignment operator could not be generated because a
	// base class assignment operator is inaccessible" within
	// the STL headers, but that warning is useless.

private:
	alloc16& operator=(const alloc16&);

};

// A compiler bug causes it to believe that p->~T() doesn't reference p.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#endif

// The definition of destroy() must be the same for all allocators.
template <typename T, int ALIGN> void alloc16<T, ALIGN>::destroy(T * const p) const
{
	p->~T();
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
