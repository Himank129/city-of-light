#pragma once
#include <DirectXMath.h>
#include <vector>
#include <cstdint>

namespace NeonCity::Renderer
{
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 texCoord;
        DirectX::XMFLOAT4 color;
    };

    class GeometryGenerator
    {
    public:
        // Generates an expansive city grid floor (1200x1200m) with asphalt markings and road grid
        static void CreateGrid(float width, float depth, uint32_t m, uint32_t n,
                               std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices)
        {
            outVertices.clear();
            outIndices.clear();

            uint32_t vertexCount = m * n;
            uint32_t faceCount = (m - 1) * (n - 1) * 2;

            float halfWidth = 0.5f * width;
            float halfDepth = 0.5f * depth;

            float dx = width / (n - 1);
            float dz = depth / (m - 1);

            float du = 1.0f / (n - 1);
            float dv = 1.0f / (m - 1);

            outVertices.resize(vertexCount);
            for (uint32_t i = 0; i < m; ++i)
            {
                float z = halfDepth - i * dz;
                for (uint32_t j = 0; j < n; ++j)
                {
                    float x = -halfWidth + j * dx;

                    // Subtle asphalt variation and neon cyber grid lines
                    bool isRoadLine = (static_cast<int>(std::abs(x)) % 40 < 2) || (static_cast<int>(std::abs(z)) % 40 < 2);
                    DirectX::XMFLOAT4 col = isRoadLine ? 
                        DirectX::XMFLOAT4(0.0f, 0.94f, 1.0f, 1.0f) :  // Cyan road grid
                        DirectX::XMFLOAT4(0.12f, 0.14f, 0.18f, 1.0f); // Dark asphalt

                    uint32_t index = i * n + j;
                    outVertices[index].position = DirectX::XMFLOAT3(x, 0.0f, z);
                    outVertices[index].normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
                    outVertices[index].texCoord = DirectX::XMFLOAT2(j * du * 10.0f, i * dv * 10.0f);
                    outVertices[index].color = col;
                }
            }

            outIndices.resize(faceCount * 3);
            uint32_t k = 0;
            for (uint32_t i = 0; i < m - 1; ++i)
            {
                for (uint32_t j = 0; j < n - 1; ++j)
                {
                    outIndices[k]     = i * n + j;
                    outIndices[k + 1] = i * n + j + 1;
                    outIndices[k + 2] = (i + 1) * n + j;

                    outIndices[k + 3] = (i + 1) * n + j;
                    outIndices[k + 4] = i * n + j + 1;
                    outIndices[k + 5] = (i + 1) * n + j + 1;

                    k += 6;
                }
            }
        }

        // Generates a 3D box / building block with clean per-face normals
        static void CreateBox(float width, float height, float depth, const DirectX::XMFLOAT4& color,
                              std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices)
        {
            outVertices.clear();
            outIndices.clear();

            float w2 = 0.5f * width;
            float h2 = 0.5f * height;
            float d2 = 0.5f * depth;

            // 24 vertices (4 per face) for hard normals
            outVertices = {
                // Front (+Z)
                { {-w2, -h2,  d2}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, color },
                { {-w2,  h2,  d2}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, color },
                { { w2,  h2,  d2}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, color },
                { { w2, -h2,  d2}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, color },

                // Back (-Z)
                { {-w2, -h2, -d2}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, color },
                { { w2, -h2, -d2}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, color },
                { { w2,  h2, -d2}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, color },
                { {-w2,  h2, -d2}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, color },

                // Top (+Y)
                { {-w2,  h2, -d2}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, color },
                { {-w2,  h2,  d2}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, color },
                { { w2,  h2,  d2}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, color },
                { { w2,  h2, -d2}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, color },

                // Bottom (-Y)
                { {-w2, -h2, -d2}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, color },
                { { w2, -h2, -d2}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, color },
                { { w2, -h2,  d2}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, color },
                { {-w2, -h2,  d2}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, color },

                // Left (-X)
                { {-w2, -h2, -d2}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, color },
                { {-w2,  h2, -d2}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, color },
                { {-w2,  h2,  d2}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, color },
                { {-w2, -h2,  d2}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, color },

                // Right (+X)
                { { w2, -h2, -d2}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, color },
                { { w2, -h2,  d2}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, color },
                { { w2,  h2,  d2}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, color },
                { { w2,  h2, -d2}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, color }
            };

            outIndices = {
                0,  1,  2,      0,  2,  3,       // Front
                4,  5,  6,      4,  6,  7,       // Back
                8,  9,  10,     8,  10, 11,      // Top
                12, 13, 14,     12, 14, 15,      // Bottom
                16, 17, 18,     16, 18, 19,      // Left
                20, 21, 22,     20, 22, 23       // Right
            };
        }
    };
}
