#include "mat4.h"
#include "error.h"


namespace vmath
{

bool mat4::RunUnitTests()
{
	bool success = true;

	// First we make sure that nearly_equals is correct.  We kinda need it!
	if (!nearly_equals(0.2f + 0.1f, 0.15f + 0.15f) ||
		!nearly_equals(12.0f/3.0f, 0.25f*16.0f))
	{
		print_error(ERRMARK, "nearly_equals unit test failed.");
		success = false;
	}

	// Start with some test inputs.
	mat4 A = rotation(quat::euler(45, 34, 23));
	A *= translation(-2, 77, 99);
	A *= perspective(20, 0.8f, 1, 12345);

	mat4 B = perspective(90, 1.25f, 0.1f, 100);
	B *= rotation(quat::euler(5, 32, -12));

	mat4 test1 = (A * B).inverted();
	mat4 test2 = B.inverted() * A.inverted();

	for (int i = 0; i < 16; ++i)
	{
		if (!nearly_equals(test1.data[i], test2.data[i], 0.01f))
		{
			print_error(ERRMARK, "mat4 unit test failed: Inv(A*B) == Inv(B)*Inv(A)\n");

			print_error(ERRMORE, '[', test1.at(0, 0), "][", test1.at(0, 1), "][", test1.at(0, 2), "][", test1.at(0, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(1, 0), "][", test1.at(1, 1), "][", test1.at(1, 2), "][", test1.at(1, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(2, 0), "][", test1.at(2, 1), "][", test1.at(2, 2), "][", test1.at(2, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(3, 0), "][", test1.at(3, 1), "][", test1.at(3, 2), "][", test1.at(3, 3), "]\n\n");
			
			print_error(ERRMORE, '[', test2.at(0, 0), "][", test2.at(0, 1), "][", test2.at(0, 2), "][", test2.at(0, 3), "]\n");
			print_error(ERRMORE, '[', test2.at(1, 0), "][", test2.at(1, 1), "][", test2.at(1, 2), "][", test2.at(1, 3), "]\n");
			print_error(ERRMORE, '[', test2.at(2, 0), "][", test2.at(2, 1), "][", test2.at(2, 2), "][", test2.at(2, 3), "]\n");
			print_error(ERRMORE, '[', test2.at(3, 0), "][", test2.at(3, 1), "][", test2.at(3, 2), "][", test2.at(3, 3), "]\n");


			success = false;
			break;
		}
	}

	//  AA^(-1)=I
	test1 = A * A.inverted();
	for (int i = 0; i < 16; ++i)
	{
		if (!nearly_equals(test1.data[i], MAT4_IDENTITY.data[i], 0.0001f))
		{
			print_error(ERRMARK, "mat4 unit test failed: A*Inv(A) == Identity\n");
			print_error(ERRMORE, '[', test1.at(0, 0), "][", test1.at(0, 1), "][", test1.at(0, 2), "][", test1.at(0, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(1, 0), "][", test1.at(1, 1), "][", test1.at(1, 2), "][", test1.at(1, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(2, 0), "][", test1.at(2, 1), "][", test1.at(2, 2), "][", test1.at(2, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(3, 0), "][", test1.at(3, 1), "][", test1.at(3, 2), "][", test1.at(3, 3), "]\n");
			success = false;
			break;
		}
	}

	mat4 C = A * B;
	test1 = A.inverted() * A * B;
	for (int i = 0; i < 16; ++i)
	{
		if (!nearly_equals(test1.data[i], B.data[i], 0.0001f))
		{
			print_error(ERRMARK, "mat4 unit test failed: B == Inv(A)*A*B\n");
			print_error(ERRMORE, '[', test1.at(0, 0), "][", test1.at(0, 1), "][", test1.at(0, 2), "][", test1.at(0, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(1, 0), "][", test1.at(1, 1), "][", test1.at(1, 2), "][", test1.at(1, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(2, 0), "][", test1.at(2, 1), "][", test1.at(2, 2), "][", test1.at(2, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(3, 0), "][", test1.at(3, 1), "][", test1.at(3, 2), "][", test1.at(3, 3), "]\n");
			success = false;
			break;
		}
	}

	test1 = A.inverted() * C;
	for (int i = 0; i < 16; ++i)
	{
		if (!nearly_equals(test1.data[i], B.data[i], 0.001f))
		{
			print_error(ERRMARK, "mat4 unit test failed: B == Inv(A)*C\n");
			print_error(ERRMORE, '[', test1.at(0, 0), "][", test1.at(0, 1), "][", test1.at(0, 2), "][", test1.at(0, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(1, 0), "][", test1.at(1, 1), "][", test1.at(1, 2), "][", test1.at(1, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(2, 0), "][", test1.at(2, 1), "][", test1.at(2, 2), "][", test1.at(2, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(3, 0), "][", test1.at(3, 1), "][", test1.at(3, 2), "][", test1.at(3, 3), "]\n");
			success = false;
			break;
		}
	}

	test1 = A * B * B.inverted();
	for (int i = 0; i < 16; ++i)
	{
		if (!nearly_equals(test1.data[i], A.data[i], 0.0001f))
		{
			print_error(ERRMARK, "mat4 unit test failed: A = A*B*Inv(B)");
			print_error(ERRMORE, '[', test1.at(0, 0), "][", test1.at(0, 1), "][", test1.at(0, 2), "][", test1.at(0, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(1, 0), "][", test1.at(1, 1), "][", test1.at(1, 2), "][", test1.at(1, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(2, 0), "][", test1.at(2, 1), "][", test1.at(2, 2), "][", test1.at(2, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(3, 0), "][", test1.at(3, 1), "][", test1.at(3, 2), "][", test1.at(3, 3), "]\n");
			success = false;
			break;
		}
	}

	test1 = C * B.inverted();
	for (int i = 0; i < 16; ++i)
	{
		if (!nearly_equals(test1.data[i], A.data[i], 0.0001f))
		{
			print_error(ERRMARK, "mat4 unit test failed: A == C*Inv(B)\n");
			print_error(ERRMORE, '[', test1.at(0, 0), "][", test1.at(0, 1), "][", test1.at(0, 2), "][", test1.at(0, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(1, 0), "][", test1.at(1, 1), "][", test1.at(1, 2), "][", test1.at(1, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(2, 0), "][", test1.at(2, 1), "][", test1.at(2, 2), "][", test1.at(2, 3), "]\n");
			print_error(ERRMORE, '[', test1.at(3, 0), "][", test1.at(3, 1), "][", test1.at(3, 2), "][", test1.at(3, 3), "]\n");
			success = false;
			break;
		}
	}

	return success;
}

}; // namespace vmath