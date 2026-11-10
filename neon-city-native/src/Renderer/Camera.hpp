#pragma once
#include <DirectXMath.h>

namespace NeonCity::Renderer
{
    enum class CameraMode
    {
        FirstPerson,
        ThirdPerson
    };

    class Camera
    {
    public:
        Camera();

        void SetLens(float fovY, float aspect, float zn, float zf);
        void SetPosition(float x, float y, float z);
        void SetPosition(const DirectX::XMFLOAT3& pos);

        [[nodiscard]] DirectX::XMVECTOR GetPosition() const;
        [[nodiscard]] DirectX::XMFLOAT3 GetPosition3f() const;

        [[nodiscard]] DirectX::XMVECTOR GetRight() const;
        [[nodiscard]] DirectX::XMFLOAT3 GetRight3f() const;
        [[nodiscard]] DirectX::XMVECTOR GetUp() const;
        [[nodiscard]] DirectX::XMFLOAT3 GetUp3f() const;
        [[nodiscard]] DirectX::XMVECTOR GetLook() const;
        [[nodiscard]] DirectX::XMFLOAT3 GetLook3f() const;

        [[nodiscard]] float GetNearZ() const { return m_nearZ; }
        [[nodiscard]] float GetFarZ() const { return m_farZ; }
        [[nodiscard]] float GetFovY() const { return m_fovY; }
        [[nodiscard]] float GetAspectRatio() const { return m_aspect; }

        [[nodiscard]] DirectX::XMMATRIX GetView() const;
        [[nodiscard]] DirectX::XMMATRIX GetProj() const;
        [[nodiscard]] DirectX::XMMATRIX GetViewProj() const;

        // Camera movement & controls
        void Strafe(float d);
        void Walk(float d);
        void Elevate(float d);

        void Pitch(float angle);
        void RotateY(float angle);

        void UpdateViewMatrix();

        void SetMode(CameraMode mode) { m_mode = mode; }
        [[nodiscard]] CameraMode GetMode() const { return m_mode; }

    private:
        DirectX::XMFLOAT3 m_position{ 0.0f, 2.0f, -10.0f };
        DirectX::XMFLOAT3 m_right{ 1.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 m_up{ 0.0f, 1.0f, 0.0f };
        DirectX::XMFLOAT3 m_look{ 0.0f, 0.0f, 1.0f };

        float m_nearZ{ 0.1f };
        float m_farZ{ 1000.0f };
        float m_aspect{ 16.0f / 9.0f };
        float m_fovY{ DirectX::XMConvertToRadians(65.0f) };

        DirectX::XMFLOAT4X4 m_view;
        DirectX::XMFLOAT4X4 m_proj;

        CameraMode m_mode{ CameraMode::FirstPerson };
        float m_currentPitch{ 0.0f };
    };
}
