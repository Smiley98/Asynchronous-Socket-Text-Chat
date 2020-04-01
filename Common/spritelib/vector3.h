#pragma once

namespace spritelib
{
	namespace math
	{
		class Vector3
		{
		public:
			double x, y, z;

			static Vector3 one;
			static Vector3 zero;

			static Vector3 right;
			static Vector3 left;
			static Vector3 up;
			static Vector3 down;
			static Vector3 forward;
			static Vector3 backward;

			Vector3();
			Vector3(double a_value);
			Vector3(double a_x, double a_y);
			Vector3(double a_x, double a_y, double a_z);

			Vector3 multiply(Vector3 a_other);
			Vector3 multiply(double a_other);

			Vector3 divide(Vector3 a_other);
			Vector3 divide(double a_other);

			Vector3 add(Vector3 a_other);
			Vector3 add(double a_other);

			Vector3 subtract(Vector3 a_other);
			Vector3 subtract(double a_other);

			Vector3 cross(Vector3 a_other);
			Vector3 normalize();
			
			double dot(Vector3 a_other);
			double magnitude();
		};
	}
}