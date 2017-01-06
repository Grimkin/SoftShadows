#pragma once
#include "Types.h"
#include <math.h>

extern Quaternion QuaternionIdentity;
extern const float g_Pi;

// conversions
inline float2 make_float2( const int2& in ) {
	return float2( static_cast<float>( in.x ), static_cast<float>( in.y ) );
}
inline float3 make_float3( const int3& in ) {
	return float3( static_cast<float>( in.x ), static_cast<float>( in.y ), static_cast<float>( in.z ) );
}
inline float4 make_float4( const int4& in ) {
	return float4( static_cast<float>( in.x ), static_cast<float>( in.y ), static_cast<float>( in.z ), static_cast<float>( in.w ) );
}
inline float2 make_float2( const uint2& in ) {
	return float2( static_cast<float>( in.x ), static_cast<float>( in.y ) );
}
inline float3 make_float3( const uint3& in ) {
	return float3( static_cast<float>( in.x ), static_cast<float>( in.y ), static_cast<float>( in.z ) );
}
inline float4 make_float4( const uint4& in ) {
	return float4( static_cast<float>( in.x ), static_cast<float>( in.y ), static_cast<float>( in.z ), static_cast<float>( in.w ) );
}

inline int2 make_int2( const float2& in ) {
	return int2( static_cast<int>( in.x ), static_cast<int>( in.y ) );
}
inline int3 make_int3( const float3& in ) {
	return int3( static_cast<int>( in.x ), static_cast<int>( in.y ), static_cast<int>( in.z ) );
}
inline int4 make_int4( const float4& in ) {
	return int4( static_cast<int>( in.x ), static_cast<int>( in.y ), static_cast<int>( in.z ), static_cast<int>( in.w ) );
}
inline float2a make_float2a( const float2& in ) {
	return float2a( in.x, in.y );
}
inline float3a make_float3a( const float3& in ) {
	return float3a( in.x, in.y, in.z );
}
inline float4a make_float4a( const float4& in ) {
	return float4a( in.x, in.y, in.z, in.w );
}

inline bool operator==( const float2& a, const float2& b ) {
	return a.x == b.x && a.y == b.y;
}
inline bool operator==( const float3& a, const float3& b ) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}
inline bool operator==( const float4& a, const float4& b ) {
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

inline bool operator!=( const float2& a, const float2& b ) {
	return !( a == b );
}
inline bool operator!=( const float3& a, const float3& b ) {
	return !( a == b );
}
inline bool operator!=( const float4& a, const float4& b ) {
	return !( a == b );
}

inline bool operator==( const uint2& a, const uint2& b ) {
	return a.x == b.x && a.y == b.y;
}
inline bool operator==( const uint3& a, const uint3& b ) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}
inline bool operator==( const uint4& a, const uint4& b ) {
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

inline bool operator!=( const uint2& a, const uint2& b ) {
	return !( a == b );
}
inline bool operator!=( const uint3& a, const uint3& b ) {
	return !( a == b );
}
inline bool operator!=( const uint4& a, const uint4& b ) {
	return !( a == b );
}

inline uint2 operator|( const uint2& a, const uint2& b ) {
	uint2 out;
	out.x = a.x | b.x;
	out.y = a.y | b.y;
	return out;
}

inline uint3 operator|( const uint3& a, const uint3& b ) {
	uint3 out;
	out.x = a.x | b.x;
	out.y = a.y | b.y;
	out.z = a.z | b.z;
	return out;
}

inline uint4 operator|( const uint4& a, const uint4& b ) {
	uint4 out;
	out.x = a.x | b.x;
	out.y = a.y | b.y;
	out.z = a.z | b.z;
	out.w = a.w | b.w;
	return out;
}
inline uint2 operator<<( const uint2& v, const uint32_t shift ) {
	uint2 out;
	out.x = v.x << shift;
	out.y = v.y << shift;
	return out;
}
inline uint3 operator<<( const uint3& v, const uint32_t shift ) {
	uint3 out;
	out.x = v.x << shift;
	out.y = v.y << shift;
	out.z = v.z << shift;
	return out;
}
inline uint4 operator<<( const uint4& v, const uint32_t shift ) {
	uint4 out;
	out.x = v.x << shift;
	out.y = v.y << shift;
	out.z = v.z << shift;
	out.w = v.w << shift;
	return out;
}
inline uint2 operator>>( const uint2& v, const uint32_t shift ) {
	uint2 out;
	out.x = v.x >> shift;
	out.y = v.y >> shift;
	return out;
}
inline uint3 operator>>( const uint3& v, const uint32_t shift ) {
	uint3 out;
	out.x = v.x >> shift;
	out.y = v.y >> shift;
	out.z = v.z >> shift;
	return out;
}
inline uint4 operator>>( const uint4& v, const uint32_t shift ) {
	uint4 out;
	out.x = v.x >> shift;
	out.y = v.y >> shift;
	out.z = v.z >> shift;
	out.w = v.w >> shift;
	return out;
}
inline uint2& operator<<=( uint2& v, const uint32_t shift ) {
	v.x <<= shift;
	v.y <<= shift;
	return v;
}inline uint3& operator<<=( uint3& v, const uint32_t shift ) {
	v.x <<= shift;
	v.y <<= shift;
	v.z <<= shift;
	return v;
}
inline uint4& operator<<=( uint4& v, const uint32_t shift ) {
	v.x <<= shift;
	v.y <<= shift;
	v.z <<= shift;
	v.w <<= shift;
	return v;
}
inline uint2& operator>>=( uint2& v, const uint32_t shift ) {
	v.x >>= shift;
	v.y >>= shift;
	return v;
}inline uint3& operator>>=( uint3& v, const uint32_t shift ) {
	v.x >>= shift;
	v.y >>= shift;
	v.z >>= shift;
	return v;
}
inline uint4& operator>>=( uint4& v, const uint32_t shift ) {
	v.x >>= shift;
	v.y >>= shift;
	v.z >>= shift;
	v.w >>= shift;
	return v;
}

inline bool any( const uint2& v ) {
	return v.x || v.y;
}

inline bool any( const uint3& v ) {
	return v.x || v.y || v.z;
}

inline bool any( const uint4& v ) {
	return v.x || v.y || v.z || v.w;
}

inline bool all( const uint2& v ) {
	return v.x && v.y;
}

inline bool all( const uint3& v ) {
	return v.x && v.y && v.z;
}

inline bool all( const uint4& v ) {
	return v.x && v.y && v.z && v.w;
}

inline float2 operator-( const float2 v ) {
	return float2( -v.x, -v.y );
}
inline float3 operator-( const float3 v ) {
	return float3( -v.x, -v.y, -v.z );
}
inline float4 operator-(const float4 v) {
	return float4( -v.x, -v.y, -v.z, -v.w );
}

// float vector multiplations
inline float2 operator*( const float2& a, const float2& b ) {
	return float2( a.x * b.x, a.y * b.y );
}

inline float2 operator*( const float2& v, float s ) {
	return float2( v.x * s, v.y * s );
}

inline float2 operator*( float s, const float2& v ) {
	return float2( v.x * s, v.y * s );
}

inline float3 operator*( const float3& a, const float3& b ) {
	return float3( a.x * b.x, a.y * b.y, a.z * b.z );
}

inline float3 operator*( const float3& v, float s ) {
	return float3( v.x * s, v.y * s, v.z * s );
}

inline float3 operator*( float s, const float3& v ) {
	return float3( v.x * s, v.y * s, v.z * s );
}

inline float4 operator*( const float4& a, const float4& b ) {
	return float4( a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w );
}

inline float4 operator*( const float4& v, float s ) {
	return float4( v.x * s, v.y * s, v.z * s, v.w * s );
}

inline float4 operator*( float s, const float4& v ) {
	return float4( v.x * s, v.y * s, v.z * s, v.w * s );
}

inline float2& operator*=( float2& a, const float2& b ) {
	a.x *= b.x;
	a.y *= b.y;
	return a;
}

inline float2& operator*=( float2& v, float s ) {
	v.x *= s;
	v.y *= s;
	return v;
}

inline float3& operator*=( float3& a, const float3& b ) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	return a;
}

inline float3& operator*=( float3& v, float s ) {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

inline float4& operator*=( float4& a, const float4& b ) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= a.w;
	return a;
}

inline float4& operator*=( float4& v, float s ) {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	v.w *= s;
	return v;
}

// float vector divisions
inline float2 operator/( const float2& a, const float2& b ) {
	return float2( a.x / b.x, a.y / b.y );
}

inline float2 operator/( const float2& v, float s ) {
	return float2( v.x / s, v.y / s );
}

inline float2 operator/( float s, const float2& v ) {
	return float2( s / v.x, s / v.y );
}

inline float3 operator/( const float3& a, const float3& b ) {
	return float3( a.x / b.x, a.y / b.y, a.z / b.z );
}

inline float3 operator/( const float3& v, float s ) {
	return float3( v.x / s, v.y / s, v.z / s );
}

inline float3 operator/( float s, const float3& v ) {
	return float3( s / v.x, s / v.y, s / v.z );
}

inline float4 operator/( const float4& a, const float4& b ) {
	return float4( a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w );
}

inline float4 operator/( const float4& v, float s ) {
	return float4( v.x / s, v.y / s, v.z / s, v.w / s );
}

inline float4 operator/( float s, const float4& v ) {
	return float4( s / v.x, s / v.y, s / v.z, s / v.w );
}

inline float2& operator/=( float2& a, const float2& b ) {
	a.x /= b.x;
	a.y /= b.y;
	return a;
}

inline float2& operator/=( float2& v, float s ) {
	v.x /= s;
	v.y /= s;
	return v;
}

inline float3& operator/=( float3& a, const float3& b ) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	return a;
}

inline float3& operator/=( float3& v, float s ) {
	v.x /= s;
	v.y /= s;
	v.z /= s;
	return v;
}

inline float4& operator/=( float4& a, const float4& b ) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	a.w /= a.w;
	return a;
}

inline float4& operator/=( float4& v, float s ) {
	v.x /= s;
	v.y /= s;
	v.z /= s;
	v.w /= s;
	return v;
}

// float vector addition
inline float2 operator+( const float2& a, const float2& b ) {
	return float2( a.x + b.x, a.y + b.y );
}

inline float2 operator+( const float2& v, float s ) {
	return float2( v.x + s, v.y + s );
}

inline float2 operator+( float s, const float2& v ) {
	return float2( v.x + s, v.y + s );
}

inline float3 operator+( const float3& a, const float3& b ) {
	return float3( a.x + b.x, a.y + b.y, a.z + b.z );
}

inline float3 operator+( const float3& v, float s ) {
	return float3( v.x + s, v.y + s, v.z + s );
}

inline float3 operator+( float s, const float3& v ) {
	return float3( v.x + s, v.y + s, v.z + s );
}

inline float4 operator+( const float4& a, const float4& b ) {
	return float4( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w );
}

inline float4 operator+( const float4& v, float s ) {
	return float4( v.x + s, v.y + s, v.z + s, v.w + s );
}

inline float4 operator+( float s, const float4& v ) {
	return float4( v.x + s, v.y + s, v.z + s, v.w + s );
}

inline float2& operator+=( float2& a, const float2& b ) {
	a.x += b.x;
	a.y += b.y;
	return a;
}

inline float2& operator+=( float2& v, float s ) {
	v.x += s;
	v.y += s;
	return v;
}

inline float3& operator+=( float3& a, const float3& b ) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

inline float3& operator+=( float3& v, float s ) {
	v.x += s;
	v.y += s;
	v.z += s;
	return v;
}

inline float4& operator+=( float4& a, const float4& b ) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += a.w;
	return a;
}

inline float4& operator+=( float4& v, float s ) {
	v.x += s;
	v.y += s;
	v.z += s;
	v.w += s;
	return v;
}

// float vector addition
inline float2 operator-( const float2& a, const float2& b ) {
	return float2( a.x - b.x, a.y - b.y );
}

inline float2 operator-( const float2& v, float s ) {
	return float2( v.x - s, v.y - s );
}

inline float2 operator-( float s, const float2& v ) {
	return float2( v.x - s, v.y - s );
}

inline float3 operator-( const float3& a, const float3& b ) {
	return float3( a.x - b.x, a.y - b.y, a.z - b.z );
}

inline float3 operator-( const float3& v, float s ) {
	return float3( v.x - s, v.y - s, v.z - s );
}

inline float3 operator-( float s, const float3& v ) {
	return float3( v.x - s, v.y - s, v.z - s );
}

inline float4 operator-( const float4& a, const float4& b ) {
	return float4( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w );
}

inline float4 operator-( const float4& v, float s ) {
	return float4( v.x - s, v.y - s, v.z - s, v.w - s );
}

inline float4 operator-( float s, const float4& v ) {
	return float4( v.x - s, v.y - s, v.z - s, v.w - s );
}

inline float2& operator-=( float2& a, const float2& b ) {
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

inline float2& operator-=( float2& v, float s ) {
	v.x -= s;
	v.y -= s;
	return v;
}

inline float3& operator-=( float3& a, const float3& b ) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

inline float3& operator-=( float3& v, float s ) {
	v.x -= s;
	v.y -= s;
	v.z -= s;
	return v;
}

inline float4& operator-=( float4& a, const float4& b ) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= a.w;
	return a;
}

inline float4& operator-=( float4& v, float s ) {
	v.x -= s;
	v.y -= s;
	v.z -= s;
	v.w -= s;
	return v;
}

inline bool operator==( const int2& a, const int2& b ) {
	return a.x == b.x && a.y == b.y;
}
inline bool operator==( const int3& a, const int3& b ) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}
inline bool operator==( const int4& a, const int4& b ) {
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

inline bool operator!=( const int2& a, const int2& b ) {
	return !( a == b );
}
inline bool operator!=( const int3& a, const int3& b ) {
	return !( a == b );
}
inline bool operator!=( const int4& a, const int4& b ) {
	return !( a == b );
}


inline int2 operator-( const int2 v ) {
	return int2( -v.x, -v.y );
}
inline int3 operator-( const int3 v ) {
	return int3( -v.x, -v.y, -v.z );
}
inline int4 operator-( const int4 v ) {
	return int4( -v.x, -v.y, -v.z, -v.w );
}

// int vector multiplications
inline int2 operator*( const int2& a, const int2& b ) {
	return int2( a.x * b.x, a.y * b.y );
}

inline int2 operator*( const int2& v, int s ) {
	return int2( v.x * s, v.y * s );
}

inline int2 operator*( int s, const int2& v ) {
	return int2( v.x * s, v.y * s );
}

inline int3 operator*( const int3& a, const int3& b ) {
	return int3( a.x * b.x, a.y * b.y, a.z * b.z );
}

inline int3 operator*( const int3& v, int s ) {
	return int3( v.x * s, v.y * s, v.z * s );
}

inline int3 operator*( int s, const int3& v ) {
	return int3( v.x * s, v.y * s, v.z * s );
}

inline int4 operator*( const int4& a, const int4& b ) {
	return int4( a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w );
}

inline int4 operator*( const int4& v, int s ) {
	return int4( v.x * s, v.y * s, v.z * s, v.w * s );
}

inline int4 operator*( int s, const int4& v ) {
	return int4( v.x * s, v.y * s, v.z * s, v.w * s );
}

inline int2& operator*=( int2& a, const int2& b ) {
	a.x *= b.x;
	a.y *= b.y;
	return a;
}

inline int2& operator*=( int2& v, int s ) {
	v.x *= s;
	v.y *= s;
	return v;
}

inline int3& operator*=( int3& a, const int3& b ) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	return a;
}

inline int3& operator*=( int3& v, int s ) {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

inline int4& operator*=( int4& a, const int4& b ) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= a.w;
	return a;
}

inline int4& operator*=( int4& v, int s ) {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	v.w *= s;
	return v;
}

// int vector multiplications
inline int2 operator/( const int2& a, const int2& b ) {
	return int2( a.x / b.x, a.y / b.y );
}

inline int2 operator/( const int2& v, int s ) {
	return int2( v.x / s, v.y / s );
}

inline int2 operator/( int s, const int2& v ) {
	return int2( s / v.x, s / v.y );
}

inline int3 operator/( const int3& a, const int3& b ) {
	return int3( a.x / b.x, a.y / b.y, a.z / b.z );
}

inline int3 operator/( const int3& v, int s ) {
	return int3( v.x / s, v.y / s, v.z / s );
}

inline int3 operator/( int s, const int3& v ) {
	return int3( s / v.x, s / v.y, s / v.z );
}

inline int4 operator/( const int4& a, const int4& b ) {
	return int4( a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w );
}

inline int4 operator/( const int4& v, int s ) {
	return int4( v.x / s, v.y / s, v.z / s, v.w / s );
}

inline int4 operator/( int s, const int4& v ) {
	return int4( s / v.x, s / v.y, s / v.z, s / v.w );
}

inline int2& operator/=( int2& a, const int2& b ) {
	a.x /= b.x;
	a.y /= b.y;
	return a;
}

inline int2& operator/=( int2& v, int s ) {
	v.x /= s;
	v.y /= s;
	return v;
}

inline int3& operator/=( int3& a, const int3& b ) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	return a;
}

inline int3& operator/=( int3& v, int s ) {
	v.x /= s;
	v.y /= s;
	v.z /= s;
	return v;
}

inline int4& operator/=( int4& a, const int4& b ) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	a.w /= a.w;
	return a;
}

inline int4& operator/=( int4& v, int s ) {
	v.x /= s;
	v.y /= s;
	v.z /= s;
	v.w /= s;
	return v;
}

// int vector additions
inline int2 operator+( const int2& a, const int2& b ) {
	return int2( a.x + b.x, a.y + b.y );
}

inline int2 operator+( const int2& v, int s ) {
	return int2( v.x + s, v.y + s );
}

inline int2 operator+( int s, const int2& v ) {
	return int2( v.x + s, v.y + s );
}

inline int3 operator+( const int3& a, const int3& b ) {
	return int3( a.x + b.x, a.y + b.y, a.z + b.z );
}

inline int3 operator+( const int3& v, int s ) {
	return int3( v.x + s, v.y + s, v.z + s );
}

inline int3 operator+( int s, const int3& v ) {
	return int3( v.x + s, v.y + s, v.z + s );
}

inline int4 operator+( const int4& a, const int4& b ) {
	return int4( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w );
}

inline int4 operator+( const int4& v, int s ) {
	return int4( v.x + s, v.y + s, v.z + s, v.w + s );
}

inline int4 operator+( int s, const int4& v ) {
	return int4( v.x + s, v.y + s, v.z + s, v.w + s );
}

inline int2& operator+=( int2& a, const int2& b ) {
	a.x += b.x;
	a.y += b.y;
	return a;
}

inline int2& operator+=( int2& v, int s ) {
	v.x += s;
	v.y += s;
	return v;
}

inline int3& operator+=( int3& a, const int3& b ) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

inline int3& operator+=( int3& v, int s ) {
	v.x += s;
	v.y += s;
	v.z += s;
	return v;
}

inline int4& operator+=( int4& a, const int4& b ) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += a.w;
	return a;
}

inline int4& operator+=( int4& v, int s ) {
	v.x += s;
	v.y += s;
	v.z += s;
	v.w += s;
	return v;
}

// int vector substractions
inline int2 operator-( const int2& a, const int2& b ) {
	return int2( a.x - b.x, a.y - b.y );
}

inline int2 operator-( const int2& v, int s ) {
	return int2( v.x - s, v.y - s );
}

inline int2 operator-( int s, const int2& v ) {
	return int2( v.x - s, v.y - s );
}

inline int3 operator-( const int3& a, const int3& b ) {
	return int3( a.x - b.x, a.y - b.y, a.z - b.z );
}

inline int3 operator-( const int3& v, int s ) {
	return int3( v.x - s, v.y - s, v.z - s );
}

inline int3 operator-( int s, const int3& v ) {
	return int3( v.x - s, v.y - s, v.z - s );
}

inline int4 operator-( const int4& a, const int4& b ) {
	return int4( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w );
}

inline int4 operator-( const int4& v, int s ) {
	return int4( v.x - s, v.y - s, v.z - s, v.w - s );
}

inline int4 operator-( int s, const int4& v ) {
	return int4( v.x - s, v.y - s, v.z - s, v.w - s );
}

inline int2& operator-=( int2& a, const int2& b ) {
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

inline int2& operator-=( int2& v, int s ) {
	v.x -= s;
	v.y -= s;
	return v;
}

inline int3& operator-=( int3& a, const int3& b ) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

inline int3& operator-=( int3& v, int s ) {
	v.x -= s;
	v.y -= s;
	v.z -= s;
	return v;
}

inline int4& operator-=( int4& a, const int4& b ) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= a.w;
	return a;
}

inline int4& operator-=( int4& v, int s ) {
	v.x -= s;
	v.y -= s;
	v.z -= s;
	v.w -= s;
	return v;
}

// uint vector additions
inline uint2 operator+( const uint2& a, const uint2& b ) {
	return uint2( a.x + b.x, a.y + b.y );
}

inline uint2 operator+( const uint2& v, uint32_t s ) {
	return uint2( v.x + s, v.y + s );
}

inline uint2 operator+( uint32_t s, const uint2& v ) {
	return uint2( v.x + s, v.y + s );
}

inline uint3 operator+( const uint3& a, const uint3& b ) {
	return uint3( a.x + b.x, a.y + b.y, a.z + b.z );
}

inline uint3 operator+( const uint3& v, uint32_t s ) {
	return uint3( v.x + s, v.y + s, v.z + s );
}

inline uint3 operator+( uint32_t s, const uint3& v ) {
	return uint3( v.x + s, v.y + s, v.z + s );
}

inline uint4 operator+( const uint4& a, const uint4& b ) {
	return uint4( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w );
}

inline uint4 operator+( const uint4& v, uint32_t s ) {
	return uint4( v.x + s, v.y + s, v.z + s, v.w + s );
}

inline uint4 operator+( uint32_t s, const uint4& v ) {
	return uint4( v.x + s, v.y + s, v.z + s, v.w + s );
}

inline uint2& operator+=( uint2& a, const uint2& b ) {
	a.x += b.x;
	a.y += b.y;
	return a;
}

inline uint2& operator+=( uint2& v, uint32_t s ) {
	v.x += s;
	v.y += s;
	return v;
}

inline uint3& operator+=( uint3& a, const uint3& b ) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

inline uint3& operator+=( uint3& v, uint32_t s ) {
	v.x += s;
	v.y += s;
	v.z += s;
	return v;
}

inline uint4& operator+=( uint4& a, const uint4& b ) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += a.w;
	return a;
}

inline uint4& operator+=( uint4& v, uint32_t s ) {
	v.x += s;
	v.y += s;
	v.z += s;
	v.w += s;
	return v;
}

// uint vector substractions
inline uint2 operator-( const uint2& a, const uint2& b ) {
	return uint2( a.x - b.x, a.y - b.y );
}

inline uint2 operator-( const uint2& v, uint32_t s ) {
	return uint2( v.x - s, v.y - s );
}

inline uint2 operator-( uint32_t s, const uint2& v ) {
	return uint2( v.x - s, v.y - s );
}

inline uint3 operator-( const uint3& a, const uint3& b ) {
	return uint3( a.x - b.x, a.y - b.y, a.z - b.z );
}

inline uint3 operator-( const uint3& v, uint32_t s ) {
	return uint3( v.x - s, v.y - s, v.z - s );
}

inline uint3 operator-( uint32_t s, const uint3& v ) {
	return uint3( v.x - s, v.y - s, v.z - s );
}

inline uint4 operator-( const uint4& a, const uint4& b ) {
	return uint4( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w );
}

inline uint4 operator-( const uint4& v, uint32_t s ) {
	return uint4( v.x - s, v.y - s, v.z - s, v.w - s );
}

inline uint4 operator-( uint32_t s, const uint4& v ) {
	return uint4( v.x - s, v.y - s, v.z - s, v.w - s );
}

inline uint2& operator-=( uint2& a, const uint2& b ) {
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

inline uint2& operator-=( uint2& v, uint32_t s ) {
	v.x -= s;
	v.y -= s;
	return v;
}

inline uint3& operator-=( uint3& a, const uint3& b ) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

inline uint3& operator-=( uint3& v, uint32_t s ) {
	v.x -= s;
	v.y -= s;
	v.z -= s;
	return v;
}

inline uint4& operator-=( uint4& a, const uint4& b ) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= a.w;
	return a;
}

inline uint4& operator-=( uint4& v, uint32_t s ) {
	v.x -= s;
	v.y -= s;
	v.z -= s;
	v.w -= s;
	return v;
}

inline constexpr float Deg2Rad( float angle ) {
	return 3.14159265359f / 180.0f * angle;
}

inline constexpr float Rad2Deg( float angle ) {
	return 180.0f / 3.14159265359f * angle;
}

inline Quaternion QuaternionFromEuler( float3 angles ) {
	using namespace DirectX;
	Quaternion ret;
	XMStoreFloat4( &ret, XMQuaternionNormalize( XMQuaternionRotationRollPitchYawFromVector( DirectX::XMLoadFloat3( &angles ) ) ) );
	return ret;
}

inline Quaternion QuaternionMultiply( Quaternion q1, Quaternion q2 ) {
	using namespace DirectX;
	Quaternion ret;
	XMStoreFloat4( &ret, XMQuaternionNormalize( XMQuaternionMultiply( XMLoadFloat4( &q1 ), XMLoadFloat4( &q2 ) ) ) );
	return ret;
}

inline Quaternion QuaternionInvMultiply( Quaternion q1, Quaternion q2_inv ) {
	Quaternion ret;
	DirectX::XMStoreFloat4( &ret, DirectX::XMQuaternionMultiply( DirectX::XMLoadFloat4( &q1 ), DirectX::XMQuaternionConjugate( DirectX::XMLoadFloat4( &q2_inv ) ) ) );
	return ret;
}

inline Matrix MatrixMultiply( const Matrix& mat1, const Matrix& mat2 ) {
	using namespace DirectX;
	Matrix ret;
	XMStoreFloat4x4( &ret, XMMatrixMultiply( XMLoadFloat4x4( &mat1 ), XMLoadFloat4x4( &mat2 ) ) );
	return ret;
}

inline Matrix BuildTransformationMatrix( const float3& position, const Quaternion& rotation, const float3& scale ) {
	using namespace DirectX;
	Matrix ret;
	XMStoreFloat4x4( &ret, XMMatrixTransformation( g_XMIdentityR3, g_XMIdentityR3, XMLoadFloat3( &scale ), g_XMIdentityR3, XMLoadFloat4( &rotation ), XMLoadFloat3( &position ) ) );
	return ret;
}

inline Matrix BuildPerspectiveMatrix( float FoV, float AspectRatio, float nearPlane, float farPlane ) {
	using namespace DirectX;
	Matrix ret;
	XMStoreFloat4x4( &ret, XMMatrixPerspectiveFovLH( FoV, AspectRatio, nearPlane, farPlane ) );
	return ret;
}

inline Matrix BuildOrthographicMatrix( float width, float height, float nearPlane, float farPlane ) {
	using namespace DirectX;
	Matrix ret;
	XMStoreFloat4x4( &ret, XMMatrixOrthographicLH( width, height, nearPlane, farPlane ) );
	return ret;
}

inline Matrix BuildViewMatrix( const float3& position, const float3& lookTo, const float3& up ) {
	using namespace DirectX;
	Matrix ret;
	XMStoreFloat4x4( &ret, XMMatrixLookToLH( XMLoadFloat3( &position ), XMLoadFloat3( &lookTo ), XMLoadFloat3( &up ) ) );
	return ret;
}

inline Matrix BuildViewMatrix( const float3& position, const Quaternion& rotation ) {
	using namespace DirectX;
	Matrix ret;
	XMVECTOR lookTo = XMVector3Rotate( g_XMIdentityR0, XMLoadFloat4( &rotation ) );
	XMVECTOR up = XMVector3Rotate( g_XMIdentityR2, XMLoadFloat4( &rotation ) );
	XMStoreFloat4x4( &ret, XMMatrixLookToLH( XMLoadFloat3( &position ), lookTo, up ) );
	return ret;
}

inline float3 Rotate( const float3& vec, const Quaternion& rot ) {
	using namespace DirectX;
	XMVECTOR vecV = XMLoadFloat3( &vec );
	XMVECTOR rotV = XMLoadFloat4( &rot );
	float3 out;
	XMStoreFloat3( &out, XMVector3Normalize( XMVector3Rotate( vecV, rotV ) ) );
	return out;
}

inline float2 Normalize( const float2& v ) {
	using namespace DirectX;
	float2 out;
	XMStoreFloat2( &out, XMVector2Normalize( XMLoadFloat2( &v ) ) );
	return out;
}
inline float3 Normalize( const float3& v ) {
	using namespace DirectX;
	float3 out;
	XMStoreFloat3( &out, XMVector3Normalize( XMLoadFloat3( &v ) ) );
	return out;
}
inline float4 Normalize( const float4& v ) {
	using namespace DirectX;
	float4 out;
	XMStoreFloat4( &out, XMVector4Normalize( XMLoadFloat4( &v ) ) );
	return out;
}

inline float3 Cross( const float3& a, const float3& b ) {
	float3 out;
	out.x = ( a.y * b.z ) - ( a.z * b.y );
	out.y = ( a.z * b.x ) - ( a.x * b.z );
	out.z = ( a.x * b.y ) - ( a.y * b.x );
	return out;
}

inline float Dot( const float2& a, const float2& b ) {
	return a.x * b.x + a.y * b.y;
}
inline float Dot( const float3& a, const float3& b ) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline float Dot( const float4& a, const float4& b ) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;	
}

inline Matrix InvertMatrix( const Matrix& matrix ) {
	using namespace DirectX;

	Matrix res;
	XMStoreFloat4x4( &res, XMMatrixInverse( nullptr, XMLoadFloat4x4( &matrix ) ) );
	return res;
}

inline float4 Mult( const Matrix& matrix, const float4& vec ) {
	using namespace DirectX;

	float4 res;
	XMStoreFloat4( &res, XMVector4Transform(  XMLoadFloat4( &vec ), XMLoadFloat4x4( &matrix ) ) );
	return res;
}
