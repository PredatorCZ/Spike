// ------------------------------------------------------------
// Following implementation encapsulates header only library
// Copyright(c) 2018-2021 Jesse Yurkovich
// Copyright(c) 2021-2022 Lukas Cone
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#pragma once
#include "../../datas/internal/sc_architecture.hpp"
#include <array>
#include <nlohmann/json_fwd.hpp>
#include <unordered_map>

#ifdef GLTF_EXPORT
    #define GLTF_EXTERN ES_EXPORT
#elif defined(GLTF_IMPORT)
    #define GLTF_EXTERN ES_IMPORT
#else
    #define GLTF_EXTERN
#endif

#if (defined(__cplusplus) && __cplusplus >= 201703L) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L) && (_MSC_VER >= 1911))
    #define FX_GLTF_HAS_CPP_17
    #define FX_GLTF_NODISCARD [[nodiscard]]
    #include <string_view>

#else
    #define FX_GLTF_NODISCARD
#endif

namespace fx
{
namespace gltf
{
    namespace detail
    {
        constexpr uint32_t DefaultMaxBufferCount = 8;
        constexpr uint32_t DefaultMaxMemoryAllocation = 32 * 1024 * 1024;
    } // namespace detail

    namespace defaults
    {
        constexpr std::array<float, 16> IdentityMatrix{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        constexpr std::array<float, 4> IdentityRotation{ 0, 0, 0, 1 };
        constexpr std::array<float, 4> IdentityVec4{ 1, 1, 1, 1 };
        constexpr std::array<float, 3> IdentityVec3{ 1, 1, 1 };
        constexpr std::array<float, 3> NullVec3{ 0, 0, 0 };
        constexpr float IdentityScalar = 1;
        constexpr float FloatSentinel = 10000;

        constexpr bool AccessorNormalized = false;

        constexpr float MaterialAlphaCutoff = 0.5f;
        constexpr bool MaterialDoubleSided = false;
    } // namespace defaults

    using Attributes = std::unordered_map<std::string, uint32_t>;

    struct NeverEmpty
    {
        FX_GLTF_NODISCARD static bool empty() noexcept
        {
            return false;
        }
    };

    struct ExtensionsAndExtras
    {
        uint64_t rawData[2]{};
        ExtensionsAndExtras() = default;
        GLTF_EXTERN ExtensionsAndExtras(ExtensionsAndExtras &&);
        GLTF_EXTERN ExtensionsAndExtras(const ExtensionsAndExtras &);
        GLTF_EXTERN ExtensionsAndExtras & operator=(ExtensionsAndExtras &&);
        GLTF_EXTERN ExtensionsAndExtras & operator=(const ExtensionsAndExtras &);

        nlohmann::json & GetExtensionsAndExtras() noexcept
        {
            return reinterpret_cast<nlohmann::json &>(*rawData);
        }

        const nlohmann::json & GetExtensionsAndExtras() const noexcept
        {
            return reinterpret_cast<const nlohmann::json &>(*rawData);
        }

        GLTF_EXTERN ~ExtensionsAndExtras();
        FX_GLTF_NODISCARD bool emptyExtensions() const noexcept
        {
            return rawData[0] == 0 && rawData[1] == 0;
        }
    };

    struct Accessor : ExtensionsAndExtras
    {
        enum class ComponentType : uint16_t
        {
            None = 0,
            Byte = 5120,
            UnsignedByte = 5121,
            Short = 5122,
            UnsignedShort = 5123,
            UnsignedInt = 5125,
            Float = 5126
        };

        enum class Type : uint8_t
        {
            None,
            Scalar,
            Vec2,
            Vec3,
            Vec4,
            Mat2,
            Mat3,
            Mat4
        };

        struct Sparse : ExtensionsAndExtras
        {
            struct Indices : NeverEmpty, ExtensionsAndExtras
            {
                uint32_t bufferView{};
                uint32_t byteOffset{};
                ComponentType componentType{ ComponentType::None };
            };

            struct Values : NeverEmpty, ExtensionsAndExtras
            {
                uint32_t bufferView{};
                uint32_t byteOffset{};
            };

            int32_t count{};
            Indices indices{};
            Values values{};

            FX_GLTF_NODISCARD bool empty() const noexcept
            {
                return count == 0;
            }
        };

        int32_t bufferView{ -1 };
        uint32_t byteOffset{};
        uint32_t count{};
        bool normalized{ defaults::AccessorNormalized };

        ComponentType componentType{ ComponentType::None };
        Type type{ Type::None };
        Sparse sparse;

        std::string name;
        std::vector<float> max{};
        std::vector<float> min{};
    };

    struct Animation : ExtensionsAndExtras
    {
        struct Channel : ExtensionsAndExtras
        {
            struct Target : NeverEmpty, ExtensionsAndExtras
            {
                int32_t node{ -1 };
                std::string path{};
            };

            int32_t sampler{ -1 };
            Target target{};
        };

        struct Sampler : ExtensionsAndExtras
        {
            enum class Type
            {
                Linear,
                Step,
                CubicSpline
            };

            int32_t input{ -1 };
            int32_t output{ -1 };

            Type interpolation{ Sampler::Type::Linear };
        };

        std::string name{};
        std::vector<Channel> channels{};
        std::vector<Sampler> samplers{};
    };

    struct Asset : NeverEmpty, ExtensionsAndExtras
    {
        std::string copyright{};
        std::string generator{};
        std::string minVersion{};
        std::string version{ "2.0" };
    };

    struct Buffer : ExtensionsAndExtras
    {
        uint32_t byteLength{};

        std::string name;
        std::string uri;

        std::vector<uint8_t> data{};

        FX_GLTF_NODISCARD bool GLTF_EXTERN IsEmbeddedResource() const noexcept;

        void GLTF_EXTERN SetEmbeddedResource();
    };

    struct BufferView : ExtensionsAndExtras
    {
        enum class TargetType : uint16_t
        {
            None = 0,
            ArrayBuffer = 34962,
            ElementArrayBuffer = 34963
        };

        std::string name;

        int32_t buffer{ -1 };
        uint32_t byteOffset{};
        uint32_t byteLength{};
        uint32_t byteStride{};

        TargetType target{ TargetType::None };
    };

    struct Camera : ExtensionsAndExtras
    {
        enum class Type
        {
            None,
            Orthographic,
            Perspective
        };

        struct Orthographic : NeverEmpty, ExtensionsAndExtras
        {
            float xmag{ defaults::FloatSentinel };
            float ymag{ defaults::FloatSentinel };
            float zfar{ -defaults::FloatSentinel };
            float znear{ -defaults::FloatSentinel };
        };

        struct Perspective : NeverEmpty, ExtensionsAndExtras
        {
            float aspectRatio{};
            float yfov{};
            float zfar{};
            float znear{};
        };

        std::string name{};
        Type type{ Type::None };

        Orthographic orthographic;
        Perspective perspective;
    };

    struct Image : ExtensionsAndExtras
    {
        int32_t bufferView{};

        std::string name;
        std::string uri;
        std::string mimeType;

        FX_GLTF_NODISCARD bool GLTF_EXTERN IsEmbeddedResource() const noexcept;

        void GLTF_EXTERN MaterializeData(std::vector<uint8_t> & data) const;
    };

    struct Material : ExtensionsAndExtras
    {
        enum class AlphaMode : uint8_t
        {
            Opaque,
            Mask,
            Blend
        };

        struct Texture : ExtensionsAndExtras
        {
            int32_t index{ -1 };
            int32_t texCoord{};

            FX_GLTF_NODISCARD bool empty() const noexcept
            {
                return index == -1;
            }
        };

        struct NormalTexture : Texture
        {
            float scale{ defaults::IdentityScalar };
        };

        struct OcclusionTexture : Texture
        {
            float strength{ defaults::IdentityScalar };
        };

        struct PBRMetallicRoughness : ExtensionsAndExtras
        {
            std::array<float, 4> baseColorFactor = defaults::IdentityVec4;
            Texture baseColorTexture;

            float roughnessFactor{ defaults::IdentityScalar };
            float metallicFactor{ defaults::IdentityScalar };
            Texture metallicRoughnessTexture;

            FX_GLTF_NODISCARD bool empty() const
            {
                return baseColorTexture.empty() && metallicRoughnessTexture.empty() && metallicFactor == 1.0f && roughnessFactor == 1.0f && baseColorFactor == defaults::IdentityVec4;
            }
        };

        float alphaCutoff{ defaults::MaterialAlphaCutoff };
        AlphaMode alphaMode{ AlphaMode::Opaque };

        bool doubleSided{ defaults::MaterialDoubleSided };

        NormalTexture normalTexture;
        OcclusionTexture occlusionTexture;
        PBRMetallicRoughness pbrMetallicRoughness;

        Texture emissiveTexture;
        std::array<float, 3> emissiveFactor = defaults::NullVec3;

        std::string name;
    };

    struct Primitive : ExtensionsAndExtras
    {
        enum class Mode : uint8_t
        {
            Points = 0,
            Lines = 1,
            LineLoop = 2,
            LineStrip = 3,
            Triangles = 4,
            TriangleStrip = 5,
            TriangleFan = 6
        };

        int32_t indices{ -1 };
        int32_t material{ -1 };

        Mode mode{ Mode::Triangles };

        Attributes attributes{};
        std::vector<Attributes> targets{};
    };

    struct Mesh : ExtensionsAndExtras
    {
        std::string name;

        std::vector<float> weights{};
        std::vector<Primitive> primitives{};
    };

    struct Node : ExtensionsAndExtras
    {
        std::string name;

        int32_t camera{ -1 };
        int32_t mesh{ -1 };
        int32_t skin{ -1 };

        std::array<float, 16> matrix = defaults::IdentityMatrix;
        std::array<float, 4> rotation = defaults::IdentityRotation;
        std::array<float, 3> scale = defaults::IdentityVec3;
        std::array<float, 3> translation = defaults::NullVec3;

        std::vector<int32_t> children{};
        std::vector<float> weights{};
    };

    struct Sampler : ExtensionsAndExtras
    {
        enum class MagFilter : uint16_t
        {
            None,
            Nearest = 9728,
            Linear = 9729
        };

        enum class MinFilter : uint16_t
        {
            None,
            Nearest = 9728,
            Linear = 9729,
            NearestMipMapNearest = 9984,
            LinearMipMapNearest = 9985,
            NearestMipMapLinear = 9986,
            LinearMipMapLinear = 9987
        };

        enum class WrappingMode : uint16_t
        {
            ClampToEdge = 33071,
            MirroredRepeat = 33648,
            Repeat = 10497
        };

        std::string name;

        MagFilter magFilter{ MagFilter::None };
        MinFilter minFilter{ MinFilter::None };

        WrappingMode wrapS{ WrappingMode::Repeat };
        WrappingMode wrapT{ WrappingMode::Repeat };

        FX_GLTF_NODISCARD bool empty() const noexcept
        {
            return name.empty() && magFilter == MagFilter::None && minFilter == MinFilter::None && wrapS == WrappingMode::Repeat && wrapT == WrappingMode::Repeat && emptyExtensions();
        }
    };

    struct Scene : ExtensionsAndExtras
    {
        std::string name;

        std::vector<uint32_t> nodes{};
    };

    struct Skin : ExtensionsAndExtras
    {
        int32_t inverseBindMatrices{ -1 };
        int32_t skeleton{ -1 };

        std::string name;
        std::vector<uint32_t> joints{};
    };

    struct Texture : ExtensionsAndExtras
    {
        std::string name;

        int32_t sampler{ -1 };
        int32_t source{ -1 };
    };

    struct Document : ExtensionsAndExtras
    {
        Asset asset;

        std::vector<Accessor> accessors{};
        std::vector<Animation> animations{};
        std::vector<Buffer> buffers{};
        std::vector<BufferView> bufferViews{};
        std::vector<Camera> cameras{};
        std::vector<Image> images{};
        std::vector<Material> materials{};
        std::vector<Mesh> meshes{};
        std::vector<Node> nodes{};
        std::vector<Sampler> samplers{};
        std::vector<Scene> scenes{};
        std::vector<Skin> skins{};
        std::vector<Texture> textures{};

        int32_t scene{ -1 };
        std::vector<std::string> extensionsUsed{};
        std::vector<std::string> extensionsRequired{};
    };

    struct ReadQuotas
    {
        uint32_t MaxBufferCount{ detail::DefaultMaxBufferCount };
        uint32_t MaxFileSize{ detail::DefaultMaxMemoryAllocation };
        uint32_t MaxBufferByteLength{ detail::DefaultMaxMemoryAllocation };
    };

    Document GLTF_EXTERN LoadFromText(std::istream & input, std::string const & documentRootPath, ReadQuotas const & readQuotas = {});

    Document GLTF_EXTERN LoadFromText(std::string const & documentFilePath, ReadQuotas const & readQuotas = {});

    Document GLTF_EXTERN LoadFromBinary(std::istream & input, std::string const & documentRootPath, ReadQuotas const & readQuotas = {});

    Document GLTF_EXTERN LoadFromBinary(std::string const & documentFilePath, ReadQuotas const & readQuotas = {});

    void GLTF_EXTERN Save(Document const & document, std::ostream & output, std::string const & documentRootPath, bool useBinaryFormat);

    void GLTF_EXTERN Save(Document const & document, std::string const & documentFilePath, bool useBinaryFormat);

    struct StreamState
    {
        size_t headerBegin;
        size_t binaryBegin;
        size_t binaryPadding;
        size_t jsonSize;
    };

    void GLTF_EXTERN StreamBinaryFull(Document & document, std::istream & input, size_t inputSize, std::ostream & output, const std::string & documentRootPath);

    FX_GLTF_NODISCARD StreamState GLTF_EXTERN StreamBinaryHeaders(Document & document, std::ostream & output, size_t inputSize);
    void GLTF_EXTERN StreamBinaryFinish(const Document & document, const StreamState & state, std::ostream & output, const std::string & documentRootPath);

} // namespace gltf
} // namespace fx
