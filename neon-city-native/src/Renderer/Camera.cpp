#include "Renderer/Camera.hpp"
#include <algorithm>

namespace NeonCity::Renderer
{
    Camera::Camera()
    {
        DirectX::XMStoreFloat4x4(&m_view, DirectX::XMMatrixIdentity());
        DirectX::XMStoreFloat4x4(&m_proj, DirectX::XMMatrixIdentity());
        SetLens(DirectX::XMConvertToRadians(65.0f), 16.0f / 9.0f, 0.1f, 1000.0f);
        UpdateViewMatrix();
    }

    void Camera::SetLens(float fovY, float aspect, float zn, float zf)
    {
        m_fovY = fovY;
        m_aspect = aspect;
        m_nearZ = zn;
        m_farZ = zf;

        DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
        DirectX::XMStoreFloat4x4(&m_proj, P);
    }

    void Camera::SetPosition(float x, float y, float z)
    {
        m_position = DirectX::XMFLOAT3(x, y, z);
    }

    void Camera::SetPosition(const DirectX::XMFLOAT3& pos)
    {
        m_position = pos;
    }

    DirectX::XMVECTOR Camera::GetPosition() const
    {
        return DirectX::XMLoadFloat3(&m_position);
    }

    DirectX::XMFLOAT3 Camera::GetPosition3f() const
    {
        return m_position;
    }

    DirectX::XMVECTOR Camera::GetRight() const
    {
        return DirectX::XMLoadFloat3(&m_right);
    }

    DirectX::XMFLOAT3 Camera::GetRight3f() const
    {
        return m_right;
    }

    DirectX::XMVECTOR Camera::GetUp() const
    {
        return DirectX::XMLoadFloat3(&m_up);
    }

    DirectX::XMFLOAT3 Camera::GetUp3f() const
    {
        return m_up;
    }

    DirectX::XMVECTOR Camera::GetLook() const
    {
        return DirectX::XMLoadFloat3(&m_look);
    }

    DirectX::XMFLOAT3 Camera::GetLook3f() const
    {
        return m_look;
    }

    DirectX::XMMATRIX Camera::GetView() const
    {
        return DirectX::XMLoadFloat4x4(&m_view);
    }

    DirectX::XMMATRIX Camera::GetProj() const
    {
        return DirectX::XMLoadFloat4x4(&m_proj);
    }

    DirectX::XMMATRIX Camera::GetViewProj() const
    {
        return DirectX::XMMatrixMultiply(GetView(), GetProj());
    }

    void Camera::Strafe(float d)
    {
        DirectX::XMVECTOR s = DirectX::XMVectorReplicate(d);
        DirectX::XMVECTOR r = DirectX::XMLoadFloat3(&m_right);
        DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&m_position);
        DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorMultiplyAdd(s, r, p));
    }

    void Camera::Walk(float d)
    {
        DirectX::XMVECTOR s = DirectX::XMVectorReplicate(d);
        DirectX::XMVECTOR l = DirectX::XMLoadFloat3(&m_look);
        DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&m_position);
        DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorMultiplyAdd(s, l, p));
    }

    void Camera::Elevate(float d)
    {
        m_position.y += d;
    }

    void Camera::Pitch(float angle)
    {
        // Clamp pitch between -89 and +89 degrees
        float maxPitch = DirectX::XMConvertToRadians(89.0f);
        float newPitch = std::clamp(m_currentPitch + angle, -maxPitch, maxPitch);
        float actualDelta = newPitch - m_currentPitch;
        m_currentPitch = newPitch;

        // Rotate up and look vector about right vector
        DirectX::XMMATRIX R = DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&m_right), actualDelta);
        DirectX::XMStoreFloat3(&m_up, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&m_up), R));
        DirectX::XMStoreFloat3(&m_look, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&m_look), R));
    }

    void Camera::RotateY(float angle)
    {
        // Rotate basis vectors about world y-axis
        DirectX::XMMATRIX R = DirectX::XMMatrixRotationY(angle);
        DirectX::XMStoreFloat3(&m_right, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&m_right), R));
        DirectX::XMStoreFloat3(&m_up, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&m_up), R));
        DirectX::XMStoreFloat3(&m_look, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&m_look), R));
    }

    void Camera::UpdateViewMatrix()
    {
        DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&m_right);
        DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&m_up);
        DirectX::XMVECTOR L = DirectX::XMLoadFloat3(&m_look);
        DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&m_position);

        // Keep camera axes orthonormal
        L = DirectX::XMVector3Normalize(L);
        U = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(L, R));
        R = DirectX::XMVector3Cross(U, L);

        DirectX::XMStoreFloat3(&m_right, R);
        DirectX::XMStoreFloat3(&m_up, U);
        DirectX::XMStoreFloat3(&m_look, L);

        float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, R));
        float y = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, U));
        float z = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, L));

        m_view(0, 0) = m_right.x;
        m_view(1, 0) = m_right.y;
        m_view(2, 0) = m_right.z;
        m_view(3, 0) = x;

        m_view(0, 1) = m_up.x;
        m_view(1, 1) = m_up.y;
        m_view(2, 1) = m_up.z;
        m_view(3, 1) = y;

        m_view(0, 2) = m_look.x;
        m_view(1, 2) = m_look.y;
        m_view(2, 2) = m_look.z;
        m_view(3, 2) = z;

        m_view(0, 3) = 0.0f;
        m_view(1, 3) = 0.0f;
        m_view(2, 3) = 0.0f;
        m_view(3, 3) = 1.0f;
    }
}
