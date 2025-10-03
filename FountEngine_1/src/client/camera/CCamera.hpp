#pragma once
#include "../../math/CMath.hpp"

class CCamera {
public:
	CCamera();

	void Update(float flDeltaTime, const bool* pKeys);
	void OnMouseMove(int iDeltaX, int iDeltaY);

	void SetPosition(const CMath::Vector3_t& vecPosition);
	void SetRotation(float flYaw, float flPitch);

	const CMath::Matrix4x4_t& GetViewMatrix() const;
	const CMath::Vector3_t& GetPosition() const;
	const CMath::Vector3_t& GetForward() const;

private:
	void UpdateViewMatrix();

	CMath::Vector3_t m_vecPosition;
	CMath::Vector3_t m_vecForward;
	CMath::Vector3_t m_vecRight;
	CMath::Vector3_t m_vecUp;

	float m_flYaw;
	float m_flPitch;

	CMath::Matrix4x4_t m_mViewMatrix;
};