#pragma once 

namespace math
{
	struct Matrix
	{
		float* operator[](int index) { return matrix[index]; }
		const float* operator[](int index) const { return matrix[index]; }

		float matrix[4][4];
	};
}

typedef math::Matrix Matrix;
