#ifndef VEC_H
#define VEC_H

#include <math.h>

class Vec2 {
public:
  float x, y;

  Vec2(): x(0), y(0) {}
  Vec2(float nX, float nY): x(nX), y(nY) {}
  Vec2(const Vec2& other): x(other.x), y(other.y) {}

  Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
  Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
  Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
  Vec2& operator=(const Vec2& other) { x = other.x; y = other.y; return *this; }
  float dot(const Vec2& other) const { return (x * other.y) - (y * other.x); }
  float mag() const { return sqrt(x*x + y*y); }
  float magSquared() const { return x*x + y*y; }

  Vec2 normalized() {
    float m = mag();
    return Vec2(x/m, y/m);
  }
};

class Vec3 {
public:
  float x, y, z;

  Vec3(float nX, float nY, float nZ): x(nX), y(nY), z(nZ) {}
  Vec3(const Vec3& other): x(other.x), y(other.y), z(other.z) {}

  Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
  Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
  Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
  Vec3& operator=(const Vec3& other) { x = other.x; y = other.y; z = other.z; return *this; }

  float mag() const { return sqrt(x*x + y*y + z*z); }
  float magSquared() const { return x*x + y*y + z*z; }

  Vec3 normalized() {
    float m = mag();
    return Vec3(x/m, y/m, z/m);
  }
};

#endif
