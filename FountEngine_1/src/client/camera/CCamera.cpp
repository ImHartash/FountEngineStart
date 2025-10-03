#include "CCamera.hpp"
#include <cmath>

CCamera::CCamera()
	: m_vecPosition(0.0f, 0.0f, 0.0f)
	, m_vecForward(0.0f, 0.0f, 1.0f)
	, m_vecRight(1.0f, 0.0f, 0.0f)
	, m_vecUp(0.0f, 1.0f, 0.0f)
	, m_flYaw(0.0f)
	, m_flPitch(0.0f)
{
	UpdateViewMatrix();
}

void CCamera::Update(float flDeltaTime, const bool* pKeys) {
	float flMoveSpeed = 1.0f * flDeltaTime;

	if (pKeys['W']) {
		m_vecPosition += m_vecForward * flMoveSpeed;
	}
	if (pKeys['S']) {
		m_vecPosition -= m_vecForward * flMoveSpeed;
	}
	if (pKeys['A']) {
		m_vecPosition -= m_vecRight * flMoveSpeed;
	}
	if (pKeys['D']) {
		m_vecPosition += m_vecRight * flMoveSpeed;
	}

	if (pKeys[0x20]) { // Space Control (VK_SPACE = 0x20)
		m_vecPosition += m_vecUp * flMoveSpeed;
	}

	if (pKeys[0x11]) { // CTRL Control (VK_CONTROL = 0x11)
		m_vecPosition -= m_vecUp * flMoveSpeed;
	}

	UpdateViewMatrix();
}

void CCamera::OnMouseMove(int iDeltaX, int iDeltaY) {
	float flSensitivity = 0.001f;
	m_flYaw += iDeltaX * flSensitivity;
	m_flPitch += iDeltaY * flSensitivity;

	float flMaxPitch = M_PI / 2.0f - 0.1f;
	if (m_flPitch > flMaxPitch) m_flPitch = flMaxPitch;
	if (m_flPitch < -flMaxPitch) m_flPitch = -flMaxPitch;

	m_vecForward = CMath::Vector3_t(
		cosf(m_flPitch) * sinf(m_flYaw),
		sinf(m_flPitch),
		cosf(m_flPitch) * cosf(m_flYaw)
	).Normalize();

	m_vecRight = CMath::Vector3_t(
		cosf(m_flYaw),
		0.0f,
		-sinf(m_flYaw)
	).Normalize();

	m_vecUp = m_vecForward.Cross(m_vecRight).Normalize();

	UpdateViewMatrix();
}

void CCamera::SetPosition(const CMath::Vector3_t& vecPosition) {
	m_vecPosition = vecPosition;
	UpdateViewMatrix();
}

void CCamera::SetRotation(float flYaw, float flPitch) {
	m_flYaw = flYaw;
	m_flPitch = flPitch;
	UpdateViewMatrix();
}

const CMath::Matrix4x4_t& CCamera::GetViewMatrix() const {
	return m_mViewMatrix;
}

const CMath::Vector3_t& CCamera::GetPosition() const {
	return m_vecPosition;
}

const CMath::Vector3_t& CCamera::GetForward() const {
	return m_vecForward;
}

void CCamera::UpdateViewMatrix() {
	CMath::Vector3_t vecTarget = m_vecPosition + m_vecForward;
	m_mViewMatrix = CMath::Matrix4x4_t::CreateLookAt(m_vecPosition, vecTarget, m_vecUp);
}