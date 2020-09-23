#pragma clang diagnostic ignored "-Wimplicit-function-declaration"

/* Fail gracefully when an undeclared external function is called */

int main()
{
	my_function(42);
	return 0;
}
