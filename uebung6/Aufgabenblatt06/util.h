#ifndef util_h
#define util_h

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <fstream>
#include <cassert>
#include <vector>
#include <tuple>

static int state = {42};

//////////////////////////////// 2D & 3D vector classes ////////////////////////////////
/**
 * Basic 2D vector class.
 * Feel free to extend according to your needs.
 */
template<typename T>
class Vec2
{
public:
    Vec2() : x(0), y(0) {}
    Vec2(T xx) : x(xx), y(xx) {}
    Vec2(T xx, T yy) : x(xx), y(yy) {}

    Vec2 operator+(const Vec2 &v) const
    { return Vec2(x + v.x, y + v.y); }
    Vec2 operator-(const Vec2 &v) const
    { return Vec2(x - v.x, y - v.y); }
    Vec2 operator-() const
    { return Vec2(-x, -y); }
    Vec2 operator/(const T &r) const
    { return Vec2(x / r, y / r); }
    Vec2 operator*(const T &r) const
    { return Vec2(x * r, y * r); }
    Vec2& operator/=(const T &r)
    { x /= r, y /= r; return *this; }
    Vec2& operator*=(const T &r)
    { x *= r, y *= r; return *this; }
    Vec2& operator+=(const T &r)
    { x += r, y += r; return *this; }
    Vec2& operator-=(const T &r)
    { x -= r, y -= r; return *this; }

    friend Vec2 operator*(const T &l, const Vec2<T> &v)
    { return Vec2(v.x * l, v.y * l); }
    friend Vec2 operator/(const T &l, const Vec2<T> &v)
    { return Vec2(v.x * l, v.y * l); }
    friend bool operator==(const Vec2 &l, const Vec2 &r)
    { return (l.x == r.x) & (l.y == r.y); }
    friend bool operator!=(const Vec2 &l, const Vec2 &r)
    { return !(l == r); }

    friend std::ostream& operator<<(std::ostream &s, const Vec2<T> &v)
    { return s << '[' << v.x << ' ' << v.y << ']'; }

    T x, y;
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;


/**
 * Basic 3D vector class.
 * Feel free to extend according to your needs.
 */
template<typename T>
class Vec3
{
public:
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec3(T xx) : x(xx), y(xx), z(xx) {}
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}

    Vec3 operator+(const Vec3 &v) const
    { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3 &v) const
    { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator-() const
    { return Vec3(-x, -y, -z); }
    Vec3 operator*(const T &r) const
    { return Vec3(x * r, y * r, z * r); }
    Vec3 operator*(const Vec3 &v) const
    { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3& operator/=(const T &r)
    { x /= r, y /= r, z /= r; return *this; }
    Vec3& operator*=(const T &r)
    { x *= r, y *= r, z *= r; return *this; }
    Vec3& operator+=(const Vec3 &r)
    { x += r.x, y += r.y, z += r.z; return *this; }
    Vec3& operator-=(const Vec3 &r)
    { x -= r.x, y -= r.y, z -= r.z; return *this; }

    friend Vec3 operator*(const T &l, const Vec3 &v)
    { return Vec3<T>(v.x * l, v.y * l, v.z * l); }
    friend Vec3 operator/(const T &l, const Vec3 &v)
    { return Vec3<T>(l / v.x, l / v.y, l / v.z); }
    friend bool operator==(const Vec3 &l, const Vec3 &r)
    { return (l.x == r.x) & (l.y == r.y) & (l.z == r.z); }
    friend bool operator!=(const Vec3 &l, const Vec3 &r)
    { return !(l == r); }

    inline Vec3& normalize()
    {
        T n = norm();
        if (n > 0)
        {
            T factor = 1 / sqrt(n);
            x *= factor, y *= factor, z *= factor;
        }
        return *this;
    }
    inline T dot(const Vec3<T> &v) const
    { return x * v.x + y * v.y + z * v.z; }
    inline Vec3 cross(const Vec3<T> &v) const
    { return Vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    inline T norm() const
    { return x * x + y * y + z * z; }
    inline T length() const
    { return sqrt(norm()); }

    friend std::ostream& operator<<(std::ostream &s, const Vec3<T> &v)
    { return s << '[' << v.x << ' ' << v.y << ' ' << v.z << ']'; }

    static inline Vec3 mix(const Vec3 &a, const Vec3& b, const float &mixValue)
    { return a * (1 - mixValue) + b * mixValue; }
    static inline Vec3 clamp(const float &lo, const float &hi, const Vec3 &v)
    { return Vec3(std::max(lo, std::min(hi, v.x)),
                  std::max(lo, std::min(hi, v.y)),
                  std::max(lo, std::min(hi, v.z))); }

    T x, y, z;
};

typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;


/**
 * @brief Simple Ray class. A ray is defined with an origin and a direction.
 */
class Ray
{
public:
    Ray() : origin(Vec3f()), dir(Vec3f()), depth(0) {}
    ~Ray() {}

    Vec3f origin;   //< Orinin of the ray
    Vec3f dir;      //< Direction of the ray.
	int depth;		//< Recursive depth of the ray.
};

static float dot(Vec3f const& v, Vec3f const& w)
{
	return v.x*w.x + v.y*w.y + v.z*w.z;
}

//////////
// TODO 1:
// Compute reflection of vector v with normal n.
//
static Vec3f reflect(Vec3f const& v, Vec3f const& n)
{
	return (((2 * n) * (n * v)) - v);
	/*TODO: replace*/ return v; /*TODO: replace*/
}

//////////////////////////////// Random number generation ////////////////////////////////
/**
 * @brief Own rand() implementation for cross platform deterministc 'randomness'.
 * @return pseudo random number
 */
static int cpRand()
{
    int const a = 1103515245;
    int const c = 12345;
    state = a * state + c;
    return (state >> 16) & 0x7FFF;
}

/**
 * @brief Generate a pseudorandom number in range [0,1] using the Mersenne Twister.
 * @return A random number in range [0,1] from a uniform distribution.
 */
static float getRand()
{
    std::mt19937 mtGen(cpRand());
    std::uniform_real_distribution<> distribNorm(0.0, 1.0);
    return distribNorm(mtGen);
}


/////////////////////////////////// PPM Image handling ///////////////////////////////////
/**
 * @brief Compare two PPM images pixelwise.
 *        This is a convenient method for testing your results.
 *        Note that only rudamentory error checking/assertions are implemented!
 *
 * @param referenceFileName The reference file to compare against.
 * @param testName The name of the test (here: ray generation resp. sphere intersection)
 * @param framebuffer The framebuffer containing the rendered pixel data.
 */
static void comparePPM(const std::string referenceFileName, const std::string testName,
                       const std::vector<Vec3f> &framebuffer)
{
    std::ifstream file(referenceFileName.c_str(), std::ios::in | std::ios::binary);
    if(file.is_open())
    {
        size_t width = 0;
        size_t height = 0;
        std::string magicNumber = "";
        std::string maxValue = "";
        // read header
        file >> magicNumber >> width >> height >> maxValue;
        assert(width*height == framebuffer.size());
        assert(magicNumber == "P6");
        assert(maxValue == "255");

        std::vector<char> pixel_data;
        pixel_data.resize(width * height * 3);

        file.ignore(256, '\n');
        file.read(reinterpret_cast<char*>(pixel_data.data()), pixel_data.size());

        size_t cnt = 0;
        for (int i = 0; i < (int)framebuffer.size(); ++i)
        {
            Vec3f color = Vec3<float>::clamp(0.f, 255.f, framebuffer.at(i));
            if ((pixel_data.at(i*3+0) != (char)(255 * color.x)) +
                (pixel_data.at(i*3+1) != (char)(255 * color.y)) +
                (pixel_data.at(i*3+2) != (char)(255 * color.z)))
                ++cnt;
        }
        if (cnt > 0.001*framebuffer.size()) // 0.1% tolerance
        {
            std::cout << cnt*100/framebuffer.size()
                      << "% of the pixel values did not match the reference in the "
                      << testName << "." << std::endl;
        }
        else
        {
            std::cout << "Looks like a perfect match for the " << testName << "!" << std::endl;
        }
        file.close();
    }
    else
    {
        std::cerr << "Somethiong went wrong during comparison to the reference image "
                  << referenceFileName << std::endl;
    }
}

/**
 * @brief Save an array of color values as a PPM image.
 * @param name The file name of the ppm file.
 * @param viewport The size of the viewport.
 * @param framebuffer Framebuffer containing the color values.
 */
static void saveAsPPM(const std::string name, const Vec2i viewport,
                      const std::vector<Vec3f> &framebuffer)
{
    if(framebuffer.size() != size_t(viewport.x*viewport.y))
    {
        std::cerr << "Invalid framebuffer size, could not write out image." << std::endl;
        return;
    }

    std::ofstream os(name, std::ios::out | std::ios::binary);
    os << "P6\n" << viewport.x << " " << viewport.y << "\n255\n";
    for (int j = viewport.y-1; j >= 0; --j)
    {
		for (int i = 0; i < viewport.x; ++i)
		{
			Vec3f color = Vec3<float>::clamp(0.f, 1.0f, framebuffer.at( i + j*viewport.x));
			char r = (char)(255 * color.x);
			char g = (char)(255 * color.y);
			char b = (char)(255 * color.z);
			os << r << g << b;
		}
    }
    os.close();
}


#endif // !util_h
