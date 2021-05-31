#ifndef CONFIG_READERS
# define DEFAULT_READERS 2
#else
# define DEFAULT_READERS (CONFIG_READERS)
#endif

#define REPEAT10(x) x;x;x;x;x;x;x;x;x;x

atomic_int x;

void *thread_writer(void *unused)
{
#ifndef MANUAL_UNROLL
	for (int i = 0u; i < 42; i++)
		x = i;
#else
	REPEAT10(x = 1);
	REPEAT10(x = 1);
	REPEAT10(x = 1);
	REPEAT10(x = 1);
	x = 1;
	x = 1;
#endif
	return NULL;
}

void *thread_reader(void *unused)
{
	int a = x;
	while (a != 0)
		a = x;
	return NULL;
}
