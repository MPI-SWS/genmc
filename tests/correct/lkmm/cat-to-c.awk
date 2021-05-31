#!/usr/bin/awk -f


BEGIN {
	if (ARGC != 2) {
		print "Usage: modify_source.awk <input.litmus>" > "/dev/stderr";
		exit_invoked = 1;
		exit 1;
	}

	input_file = ARGV[1];
	output_file = ARGV[3];
	line_count = 0;
	in_comment = 0;

	header = "#include <stdlib.h>\n#include <lkmm.h>\n#include <pthread.h>\n#include <assert.h>\n"
	program = "";
	num_threads = 0;

	print "--- Modifying file:", ARGV[1] > "/dev/stderr";
}

{
	++line_count;

	## Do not collect inits
	if (line_count == 1 || match($0, "{}") != 0)
		next;

	## Comment out "exists", "forall" and friends
	r = "(exists|forall|locations|filter)(.*)"
	if (match($0, r, a)) {
		sub(r, "/* " a[0] " */");
	}

	## Remove derefence from ONCEs and collect variables
	r = "([READ|WRITE]_ONCE)\\(\\*(\\w+)(.*;)"
	if (match($0, r, a)) {
		++global_variables[a[2]];
		sub(r, a[1] "(" a[2] a[3]);
	}

	## Remove derefence from rcu_dereference and rcu_assign_pointer
	r = "(.*rcu_(dereference|assign_pointer))\\(\\*(\\w+)(.*;)"
	if (match($0, r, a)) {
		++pointer_variables[a[3]];
		sub(r, a[1] "(" a[3] a[4]);
	}

	## Remove comments
	r = "(.*)(\\(\\*)(.*)"
	if (match($0, r, a))
		sub(r, a[1] "/*" a[3]);
	r = "(.*)(\\*\\))(.*)";
	if (match($0, r, a))
		sub(r, a[1] "*/" a[3]);

	## Collect variables from acquires, releases
	r = "(smp_store_release|smp_load_acquire)\\((\\w+)(.*;)"
	if (match($0, r, a)) {
		++global_variables[a[2]];
		sub(r, a[1] "(\\&" a[2] a[3]);
	}

	## Collect atomics
	r = "(atomic_dec_and_test|atomic_inc|atomic_read|atomic_set|atomic_add|atomic_sub)\\((\\w+)(.*;)"
	if (match($0, r, a)) {
		++atomic_variables[a[2]];
		sub(r, a[1] "(\\&" a[2] a[3]);
	}

	## Collect variables from spinlocks
	r = "(spin_lock|spin_unlock|spin_trylock|spin_is_locked)\\((\\w+)(.*;)"
	if (match($0, r, a)) {
		++spinlocks[a[2]];
		sub(r, a[1] "(\\&" a[2] a[3]);
	}

	## Change the way threads are printed
	r = "(P[0-9]+)\\(.+\\)";
	if (match($0, r, a ) != 0) {
		++num_threads;
		sub(r, "void *" a[1] "(void *unused)");
	}

	## Collect line
	program = program "\n" $0;
}

END {
	if (exit_invoked)
		exit 1;

	## Print header
	printf header "\n";

	## variables and spinlocks
	for (v in global_variables)
		print "int " v ";"
	for (v in pointer_variables)
		print "int *" v ";"
	for (v in atomic_variables)
		print "atomic_t " v ";"
	for (s in spinlocks)
		print "spinlock_t " s ";"

	## print threads
	print program "\n";

	## print main
	printf("int main()\n{\n\tpthread_t ");
	for (i = 0; i < num_threads - 1; i++)
		printf("t%d, ", i);
	print "t" (num_threads - 1) ";\n"

	for (i = 0; i < num_threads; i++)
		print "\tpthread_create(&t" i ", NULL, P" i ", NULL);";

	print "\n\treturn 0;\n}"
}
