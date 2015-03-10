// Intraprocedural/Scoping 1

void foo()
{
	int y=55;
	{
		int y=3;
		y++;
	}
	static_assert(y==55);

}