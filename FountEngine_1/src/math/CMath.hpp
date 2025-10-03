#pragma once
#include <cmath>

#define M_PI 3.1415926535f

namespace CMath {
	struct Vector2_t {
		float x, y;

		Vector2_t() : x(0), y(0) {}
		Vector2_t(float x, float y) : x(x), y(y) {}
	};

	struct Vector3_t {
		float x, y, z;

		Vector3_t() : x(0), y(0), z(0) {}
		Vector3_t(float x, float y, float z) : x(x), y(y), z(z) {}

		Vector3_t operator+(const Vector3_t& other) const {
			return Vector3_t(x + other.x, y + other.y, z + other.z);
		}

		Vector3_t operator-(const Vector3_t& other) const {
			return Vector3_t(x - other.x, y - other.y, z - other.z);
		}

		Vector3_t operator*(float flScalar) const {
			return Vector3_t(x * flScalar, y * flScalar, z * flScalar);
		}

		Vector3_t operator/(float flScalar) const {
			return Vector3_t(x / flScalar, y / flScalar, z / flScalar);
		}

		Vector3_t& operator+=(const Vector3_t& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Vector3_t& operator-=(const Vector3_t& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		float Dot(const Vector3_t& other) const {
			return x * other.x + y * other.y + z * other.z;
		}

		Vector3_t Cross(const Vector3_t& other) const {
			return Vector3_t(
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x
			);
		}

		float Length() const {
			return sqrtf(x * x + y * y + z * z);
		}

		Vector3_t Normalize() const {
			float flLength = Length();
			if (flLength > 0) {
				return *this / flLength;
			}
			return *this;
		}
	};

	struct Vector4_t {
		float x, y, z, w;

		Vector4_t() : x(0), y(0), z(0), w(0) {}
		Vector4_t(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		Vector4_t(const Vector3_t& vector3, float w) : x(vector3.x), y(vector3.y), z(vector3.z), w(w) {}
	};

	struct Matrix4x4_t {
		float m[4][4];

		Matrix4x4_t() {
			Identity();
		}

		void Identity() {
			memset(m, 0, sizeof(m));
			m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
		}

		static Matrix4x4_t CreateTranslation(float x, float y, float z) {
			Matrix4x4_t mResult;
			mResult.m[3][0] = x;
			mResult.m[3][1] = y;
			mResult.m[3][2] = z;
			return mResult;
		}

		static Matrix4x4_t CreateRotationX(float flRadians) {
			Matrix4x4_t mResult;
			float cosR = cosf(flRadians);
			float sinR = sinf(flRadians);

			mResult.m[1][1] = cosR;
			mResult.m[1][2] = sinR;
			mResult.m[2][1] = -sinR;
			mResult.m[2][2] = cosR;

			return mResult;
		}

		static Matrix4x4_t CreateRotationY(float flRadians) {
			Matrix4x4_t mResult;
			float cosR = cosf(flRadians);
			float sinR = sinf(flRadians);

			mResult.m[0][0] = cosR;
			mResult.m[0][2] = sinR;
			mResult.m[2][0] = -sinR;
			mResult.m[2][2] = cosR;

			return mResult;
		}

		static Matrix4x4_t CreateRotationZ(float flRadians) {
			Matrix4x4_t mResult;
			float cosR = cosf(flRadians);
			float sinR = sinf(flRadians);

			mResult.m[0][0] = cosR;
			mResult.m[0][1] = sinR;
			mResult.m[1][0] = -sinR;
			mResult.m[1][1] = cosR;

			return mResult;
		}

		static Matrix4x4_t CreatePerspectiveFieldOfView(float flFov, float flAspectRatio, float flNearPlane, float flFarPlane) {
			Matrix4x4_t mResult;
			float yScale = 1.0f / tanf(flFov * 0.5f);
			float xScale = yScale / flAspectRatio;
			float flRange = flFarPlane / (flFarPlane - flNearPlane);

			mResult.m[0][0] = xScale;
			mResult.m[1][1] = yScale;
			mResult.m[2][2] = flRange;
			mResult.m[2][3] = 1.0f;
			mResult.m[3][2] = -flRange * flNearPlane;
			mResult.m[3][3] = 0.0f;

			return mResult;
		}

		static Matrix4x4_t CreateLookAt(const Vector3_t& vecEye, const Vector3_t& vecTarget, const Vector3_t& vecUp) {
			Vector3_t zAxis = (vecTarget - vecEye).Normalize();
			Vector3_t xAxis = vecUp.Cross(zAxis).Normalize();
			Vector3_t yAxis = zAxis.Cross(xAxis);

			Matrix4x4_t mResult;
			mResult.m[0][0] = xAxis.x;
			mResult.m[0][1] = yAxis.x;
			mResult.m[0][2] = zAxis.x;
			mResult.m[0][3] = 0.0f;

			mResult.m[1][0] = xAxis.y;
			mResult.m[1][1] = yAxis.y;
			mResult.m[1][2] = zAxis.y;
			mResult.m[1][3] = 0.0f;

			mResult.m[2][0] = xAxis.z;
			mResult.m[2][1] = yAxis.z;
			mResult.m[2][2] = zAxis.z;
			mResult.m[2][3] = 0.0f;

			mResult.m[3][0] = -xAxis.Dot(vecEye);
			mResult.m[3][1] = -yAxis.Dot(vecEye);
			mResult.m[3][2] = -zAxis.Dot(vecEye);
			mResult.m[3][3] = 1.0f;

			return mResult;
		}

		Matrix4x4_t operator*(const Matrix4x4_t& mOther) {
			Matrix4x4_t mResult;
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					mResult.m[i][j] = 0;
					for (int k = 0; k < 4; ++k) {
						mResult.m[i][j] += m[i][k] * mOther.m[k][j];
					}
				}
			}

			return mResult;
		}
	};
}