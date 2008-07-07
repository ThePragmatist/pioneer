#ifndef _MATRIX4X4_H
#define _MATRIX4X4_H

#include <math.h>
#include <stdio.h>
#include "vector3.h"

template <typename T>
class matrix4x4 {
	private:
	T cell[16];
	public:
	matrix4x4 () {}
	matrix4x4 (T val) {
		cell[0] = cell[1] = cell[2] = cell[3] = cell[4] = cell[5] = cell[6] = 
		cell[7] = cell[8] = cell[9] = cell[10] = cell[11] = cell[12] = cell[13] = 
		cell[14] = cell[15] = val;
	}
	// row-major 4x3 matrix
	void LoadFromOdeMatrix(const T *r) {
		cell[ 0] = r[ 0];cell[ 1] = r[ 4];cell[ 2] = r[ 8];cell[ 3] = 0;
		cell[ 4] = r[ 1];cell[ 5] = r[ 5];cell[ 6] = r[ 9];cell[ 7] = 0;
		cell[ 8] = r[ 2];cell[ 9] = r[ 6];cell[10] = r[10];cell[11] = 0;
		cell[12] =     0;cell[13] =     0;cell[14] =     0;cell[15] = 1;
	}
	// row-major 3x3 matrix
	void LoadFrom3x3Matrix(const T *r) {
		cell[0] = r[0]; cell[4] = r[1]; cell[8] = r[2]; cell[12] = 0;
		cell[1] = r[3]; cell[5] = r[4]; cell[9] = r[5]; cell[13] = 0;
		cell[2] = r[6]; cell[6] = r[7]; cell[10] = r[8]; cell[14] = 0;
		cell[3] = 0;    cell[7] = 0;    cell[11] = 0;    cell[15] = 1;
	}
	// row-major
	void SaveTo3x3Matrix(T *r) {
		r[0] = cell[0]; r[1] = cell[4]; r[2] = cell[8];
		r[3] = cell[1]; r[4] = cell[5]; r[5] = cell[9];
		r[6] = cell[2]; r[7] = cell[6]; r[8] = cell[10];
	}
	void SaveToOdeMatrix(T r[12]) {
		r[0] = cell[0]; r[1] = cell[4]; r[2] = cell[8]; r[3] = 0;
		r[4] = cell[1]; r[5] = cell[5]; r[6] = cell[9]; r[7] = 0;
		r[8] = cell[2]; r[9] = cell[6]; r[10] = cell[10]; r[11] = 0;
	}
	static matrix4x4 Identity () {
		matrix4x4 m = matrix4x4 (0);
		m.cell[0] = m.cell[5] = m.cell[10] = m.cell[15] = 1.0f;
		return m;
	}
	static matrix4x4 ScaleMatrix(T x, T y, T z) {
		matrix4x4 m;
		m[0] = x; m[1] = m[2] = m[3] = 0;
		m[5] = y; m[4] = m[6] = m[7] = 0;
		m[10] = z; m[8] = m[9] = m[11] = 0;
		m[12] = m[13] = m[14] = 0; m[15] = 1;
		return m;
	}
	void RotateZ (T radians) { *this = (*this) * RotateZMatrix (radians); }
	void RotateY (T radians) { *this = (*this) * RotateYMatrix (radians); }
	void RotateX (T radians) { *this = (*this) * RotateXMatrix (radians); }
	static matrix4x4 RotateXMatrix (T radians) {
		matrix4x4 m;
		T cos_r = cosf (radians);
		T sin_r = sinf (radians);
		m[0] = 1.0f;
		m[1] = 0;
		m[2] = 0;
		m[3] = 0;

		m[4] = 0;
		m[5] = cos_r;
		m[6] = -sin_r;
		m[7] = 0;
		
		m[8] = 0;
		m[9] = sin_r;
		m[10] = cos_r;
		m[11] = 0;
		
		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1.0f;
		return m;
	}
	static matrix4x4 RotateYMatrix (T radians) {
		matrix4x4 m;
		T cos_r = cosf (radians);
		T sin_r = sinf (radians);
		m[0] = cos_r;
		m[1] = 0;
		m[2] = sin_r;
		m[3] = 0;

		m[4] = 0;
		m[5] = 1;
		m[6] = 0;
		m[7] = 0;
		
		m[8] = -sin_r;
		m[9] = 0;
		m[10] = cos_r;
		m[11] = 0;

		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1.0f;
		return m;
	}
	static matrix4x4 RotateZMatrix (T radians) {
		matrix4x4 m;
		T cos_r = cosf (radians);
		T sin_r = sinf (radians);
		m[0] = cos_r;
		m[1] = -sin_r;
		m[2] = 0;
		m[3] = 0;
		
		m[4] = sin_r;
		m[5] = cos_r;
		m[6] = 0;
		m[7] = 0;
		
		m[8] = 0;
		m[9] = 0;
		m[10] = 1.0f;
		m[11] = 0;
		
		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1.0f;
		return m;
	}
	T& operator [] (const int i) { return cell[i]; }
	friend matrix4x4 operator+ (const matrix4x4 &a, const matrix4x4 &b) {
		matrix4x4 m;
		for (int i=0; i<16; i++) m.cell[i] = a.cell[i] + b.cell[i];
		return m;
	}
	friend matrix4x4 operator- (const matrix4x4 &a, const matrix4x4 &b) {
		matrix4x4 m;
		for (int i=0; i<16; i++) m.cell[i] = a.cell[i] - b.cell[i];
		return m;
	}
	friend matrix4x4 operator* (const matrix4x4 &a, const matrix4x4 &b) {
		matrix4x4 m;
		m.cell[0] = a.cell[0]*b.cell[0] + a.cell[4]*b.cell[1] + a.cell[8]*b.cell[2] + a.cell[12]*b.cell[3];
		m.cell[1] = a.cell[1]*b.cell[0] + a.cell[5]*b.cell[1] + a.cell[9]*b.cell[2] + a.cell[13]*b.cell[3];
		m.cell[2] = a.cell[2]*b.cell[0] + a.cell[6]*b.cell[1] + a.cell[10]*b.cell[2] + a.cell[14]*b.cell[3];
		m.cell[3] = a.cell[3]*b.cell[0] + a.cell[7]*b.cell[1] + a.cell[11]*b.cell[2] + a.cell[15]*b.cell[3];

		m.cell[4] = a.cell[0]*b.cell[4] + a.cell[4]*b.cell[5] + a.cell[8]*b.cell[6] + a.cell[12]*b.cell[7];
		m.cell[5] = a.cell[1]*b.cell[4] + a.cell[5]*b.cell[5] + a.cell[9]*b.cell[6] + a.cell[13]*b.cell[7];
		m.cell[6] = a.cell[2]*b.cell[4] + a.cell[6]*b.cell[5] + a.cell[10]*b.cell[6] + a.cell[14]*b.cell[7];
		m.cell[7] = a.cell[3]*b.cell[4] + a.cell[7]*b.cell[5] + a.cell[11]*b.cell[6] + a.cell[15]*b.cell[7];

		m.cell[8] = a.cell[0]*b.cell[8] + a.cell[4]*b.cell[9] + a.cell[8]*b.cell[10] + a.cell[12]*b.cell[11];
		m.cell[9] = a.cell[1]*b.cell[8] + a.cell[5]*b.cell[9] + a.cell[9]*b.cell[10] + a.cell[13]*b.cell[11];
		m.cell[10] = a.cell[2]*b.cell[8] + a.cell[6]*b.cell[9] + a.cell[10]*b.cell[10] + a.cell[14]*b.cell[11];
		m.cell[11] = a.cell[3]*b.cell[8] + a.cell[7]*b.cell[9] + a.cell[11]*b.cell[10] + a.cell[15]*b.cell[11];

		m.cell[12] = a.cell[0]*b.cell[12] + a.cell[4]*b.cell[13] + a.cell[8]*b.cell[14] + a.cell[12]*b.cell[15];
		m.cell[13] = a.cell[1]*b.cell[12] + a.cell[5]*b.cell[13] + a.cell[9]*b.cell[14] + a.cell[13]*b.cell[15];
		m.cell[14] = a.cell[2]*b.cell[12] + a.cell[6]*b.cell[13] + a.cell[10]*b.cell[14] + a.cell[14]*b.cell[15];
		m.cell[15] = a.cell[3]*b.cell[12] + a.cell[7]*b.cell[13] + a.cell[11]*b.cell[14] + a.cell[15]*b.cell[15];
		return m;
	}
	friend vector3<T> operator * (const matrix4x4 &a, const vector3<T> &v) {
		vector3<T> out;
		out.x = a.cell[0]*v.x + a.cell[4]*v.y + a.cell[8]*v.z + a.cell[12];
		out.y = a.cell[1]*v.x + a.cell[5]*v.y + a.cell[9]*v.z + a.cell[13];
		out.z = a.cell[2]*v.x + a.cell[6]*v.y + a.cell[10]*v.z + a.cell[14];
		return out;
	}
	vector3<T> ApplyRotationOnly (const vector3<T> &v) const {
		vector3<T> out;
		out.x = cell[0]*v.x + cell[4]*v.y + cell[8]*v.z;
		out.y = cell[1]*v.x + cell[5]*v.y + cell[9]*v.z;
		out.z = cell[2]*v.x + cell[6]*v.y + cell[10]*v.z;
		return out;
	}
	void Translatef (T x, T y, T z) {
		matrix4x4 m = Identity ();
		m[12] = x;
		m[13] = y;
		m[14] = z;
		*this = (*this) * m;
	}
	matrix4x4 InverseOf () const {
		matrix4x4 m;
		// this only works for matrices containing only rotation and transform
		m[0] = cell[0]; m[1] = cell[4]; m[2] = cell[8];
		m[4] = cell[1]; m[5] = cell[5]; m[6] = cell[9];
		m[8] = cell[2]; m[9] = cell[6]; m[10] = cell[10];
		m[12] = -(cell[0]*cell[12] + cell[1]*cell[13] + cell[2]*cell[14]);
		m[13] = -(cell[4]*cell[12] + cell[5]*cell[13] + cell[6]*cell[14]);
		m[14] = -(cell[8]*cell[12] + cell[9]*cell[13] + cell[10]*cell[14]);
		m[3] = m[7] = m[11] = 0;
		m[15] = 1.0f;

		return m;
	}
	void Print () const {
		for (int i=0; i<4; i++) {
			printf ("%.2f %.2f %.2f %.2f\n", cell[i], cell[i+4], cell[i+8], cell[i+12]);
		}
		printf ("\n");
	}
};

typedef matrix4x4<float> matrix4x4f;
typedef matrix4x4<double> matrix4x4d;

#endif /* _MATRIX4X4_H */