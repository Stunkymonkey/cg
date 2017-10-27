// CG Assignment 1 Exercise 2 WS 17/18

#ifndef Point3D_hpp
#define Point3D_hpp

struct Point3D
{
	// Implement the constructor
	Point3D(float x, float y, float z) {
		x_value = x;
		y_value = y;
		z_value = z;
	}

	// Implement access to the three coordinate values.
	float x() const {return x_value;}
	float y() const {return y_value;}
	float z() const {return z_value;}

private:
	// Use a suitable std container to store the three values of a point in 3d space.
	float x_value;
	float y_value;
	float z_value;
};

#endif // !Point3D_hpp