/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#pragma once
#include <string>
#include <glm/common.hpp>
#include "glm/geometric.hpp"
#include "API/Texture.h"
#include "glm/mat4x4.hpp"
#include "Data/HostDeviceData.h"
#include "Utils/Gui.h"
#include "Graphics/Model/Model.h"
#include "Graphics/Transform.h"

namespace Falcor
{
    class ConstantBuffer;
    class Gui;

    /** Base class for light sources. All light sources should inherit from this.
    */
    class Light : public Transform, std::enable_shared_from_this<Light>
    {
    public:
        using SharedPtr = std::shared_ptr<Light>;
        using SharedConstPtr = std::shared_ptr<const Light>;

        virtual ~Light() = default;

        /** Set the light parameters into a program. To use this you need to include/import 'ShaderCommon' inside your shader.
            \param[in] pBuffer The constant buffer to set the parameters into.
            \param[in] varName The name of the light variable in the program.
        */
        virtual void setIntoConstantBuffer(ConstantBuffer* pBuffer, const std::string& varName);

        /** Set the light parameters into a program. To use this you need to include/import 'ShaderCommon' inside your shader.
            \param[in] pBuffer The constant buffer to set the parameters into.
            \param[in] offset Byte offset into the constant buffer to set data to.
        */
        virtual void setIntoConstantBuffer(ConstantBuffer* pBuffer, size_t offset);

        /** Render UI elements for this light.
            \param[in] pGui The GUI to create the elements with
            \param[in] group Optional. If specified, creates a UI group to display elements within
        */
        virtual void renderUI(Gui* pGui, const char* group = nullptr);

        /** Prepare GPU data
        */
        virtual void prepareGPUData() = 0;

        /** Unload GPU data
        */
        virtual void unloadGPUData() = 0;

        /** Set intensity of the light. Units are in lumens for point and spot lights. Otherwise it is just a brightness multiplier.
        */
        void setIntensity(float intensity) { mIntensity = intensity; updateLightColor(); }

        /** Set the modulation color of the light.
        */
        void setColor(const vec3& color) { mColor = color; updateLightColor(); }

        /** Set the light direction
        */
        void setDirection(const vec3& dir) { mData.dirW = dir; }

        /** Get the intensity of the light
        */
        float getIntensity() const { return mIntensity; }

        /** Get the modulation color of the light
        */
        const vec3& getColor() const { return mColor; }

        /** Get the light's world-space direction
        */
        const vec3& getDirection() const { return mData.dirW; }

        /** Get the light type
        */
        uint32_t getType() const { return mData.type; }

        /** Get light data
        */
        inline const LightData& getData() const { return mData; }

        /** Name the light
        */
        const void setName(const std::string& Name) { mName = Name; }

        /** Get the light's name
        */
        const std::string& getName() const { return mName; }

        /** Gets the size of a single light data struct in bytes
        */
        static uint32_t getShaderStructSize() { return kDataSize; }

    protected:
        Light() = default;

        std::string mName;
        vec3 mColor = vec3(1.0f);
        float mIntensity = 1.0f;

        static const size_t kDataSize = sizeof(LightData); //TODO(tfoley) HACK:SPIRE - sizeof(MaterialData);
        LightData mData;

    private:
        void updateLightColor() { mData.lightColor = mColor * mIntensity; }
    };


    /** Directional light source.
    */
    class DirectionalLight : public Light, public std::enable_shared_from_this<DirectionalLight>
    {
    public:
        using SharedPtr = std::shared_ptr<DirectionalLight>;
        using SharedConstPtr = std::shared_ptr<const DirectionalLight>;

        /** Create a directional light
        */
        static SharedPtr create();

        virtual ~DirectionalLight() = default;

        /** Prepare GPU data
        */
        void prepareGPUData() override {}

        /** Unload GPU data
        */
        void unloadGPUData() override {}

        /** IMovableObject interface
        */
        void move(const vec3& position, const vec3& target, const vec3& up) override;

    private:
        DirectionalLight();
    };


    /** Simple infinitely-small point light with quadratic attenuation
    */
    class PointLight : public Light, public std::enable_shared_from_this<PointLight>
    {
    public:
        using SharedPtr = std::shared_ptr<PointLight>;
        using SharedConstPtr = std::shared_ptr<const PointLight>;

        /** Create a point light
        */
        static SharedPtr create();

        virtual ~PointLight() = default;

        /** Prepare GPU data
        */
        void prepareGPUData() override {}

        /** Unload GPU data
        */
        void unloadGPUData() override {}

        /** Render UI elements for this light.
            \param[in] pGui The GUI to create the elements with
            \param[in] group Optional. If specified, creates a UI group to display elements within
        */
        virtual void renderUI(Gui* pGui, const char* group = nullptr);

        /** Set radius of light's influence.
        */
        void setAttenuationRadius(float radius) { mData.attenuationRadius = radius; }

        /** Set the light position
        */
        void setPosition(const vec3& pos) { mData.posW = pos; }

        /** Set the up direction of the light's orientation. Used to determine orientation of tube lights.
        */
        void setUpVector(const vec3& up) { mData.upW = up; }

        /** Set radius of light source shape. Used simulate sphere/tube lights.
        */
        void setSourceRadius(float radius) { mData.sourceRadius = radius; }

        /** Set length of light source shape. Used to simulate tube lights.
        */
        void setSourceLength(float length) { mData.sourceLength = length; }

        /** Get the radius of the light's influence
        */
        float getAttenuationRadius() const { return mData.attenuationRadius; }

        /** Get the light's world-space position
        */
        const vec3& getPosition() const { return mData.posW; }

        /** Get the light's world-space up vector orientation
        */
        const vec3& getUpVector() const { return mData.upW; }

        /** Get the light's source radius
        */
        float getSourceRadius() const { return mData.sourceRadius; }

        /** Get the light's source length
        */
        float getSourceLength() const { return mData.sourceLength; }

        /** IMovableObject interface
        */
        void move(const vec3& position, const vec3& target, const vec3& up) override;

    protected:
        PointLight();
    };

    class SpotLight : public PointLight, public std::enable_shared_from_this<SpotLight>
    {
    public:
        using SharedPtr = std::shared_ptr<SpotLight>;
        using SharedConstPtr = std::shared_ptr<const SpotLight>;

        /** Create a spot light
        */
        static SharedPtr create();

        virtual ~SpotLight() = default;

        /** Render UI elements for this light.
            \param[in] pGui The GUI to create the elements with
            \param[in] group Optional. If specified, creates a UI group to display elements within
        */
        void renderUI(Gui* pGui, const char* group = nullptr) override;

        /** Set angle of inner cone in degrees. Measured from center of cone to edge
        */
        void setInnerConeAngle(float angle);

        /** Set angle of outer cone in degrees. Measured from center of cone to edge
        */
        void setOuterConeAngle(float angle);

        /** Get the inner cone angle in degrees
        */
        float getInnerConeAngle() const { return mInnerConeAngle; }

        /** Get the outer cone angle in degrees
        */
        float getOuterConeAngle() const { return mOuterConeAngle; }

    private:
        SpotLight();
        void updateConeProperties();

        // Angles in degrees
        float mInnerConeAngle = 0.0f;
        float mOuterConeAngle = 45.0f;
    };

#if 0
    /**
        Area light source

        This class is used to simulate area light sources. All emissive
        materials are treated as area light sources.
    */
    class AreaLight : public Light, public std::enable_shared_from_this<AreaLight>
    {
    public:
        using SharedPtr = std::shared_ptr<AreaLight>;
        using SharedConstPtr = std::shared_ptr<const AreaLight>;

        static SharedPtr create();

        AreaLight();
        ~AreaLight();
        
        /** Get total light power (needed for light picking)
        */
        float getPower() override;

        /** Set the light parameters into a program. To use this you need to include/import 'ShaderCommon' inside your shader.
            \param[in] pBuffer The constant buffer to set the parameters into.
            \param[in] varName The name of the light variable in the program.
        */
        void setIntoConstantBuffer(ConstantBuffer* pBuffer, const std::string& varName) override;

        /** Render UI elements for this light.
            \param[in] pGui The GUI to create the elements with
            \param[in] group Optional. If specified, creates a UI group to display elements within
        */
        void renderUI(Gui* pGui, const char* group = nullptr) override;

        /** Prepare GPU data
        */
        void prepareGPUData() override;

        /** Unload GPU data
        */
        void unloadGPUData() override;

        /** Set the geometry mesh for this light
            \param[in] pModel Model that contains the geometry mesh for this light
            \param[in] meshId Geometry mesh id within the model
            \param[in] instanceId Geometry mesh instance id
        */
        void setMeshData(const Model::MeshInstance::SharedPtr& pMeshInstance);

        /** Obtain the geometry mesh for this light
            \return Mesh instance for this light
        */
        const Model::MeshInstance::SharedPtr& getMeshData() const { return mpMeshInstance; }

        /** Compute surface area of the mesh
        */
        void computeSurfaceArea();

        /** Get surface area of the mesh
        */
        float getSurfaceArea() const { return mSurfaceArea; }

        /** Get the probability distribution of the mesh
        */
        const std::vector<float>& getMeshCDF() const { return mMeshCDF; }

        /** Set the index buffer
            \param[in] indexBuf Buffer containing mesh indices
        */
        void setIndexBuffer(const Buffer::SharedPtr& indexBuf) { mIndexBuf = indexBuf; }

        /** Get the index buffer.
        */
        const Buffer::SharedPtr& getIndexBuffer() const { return mIndexBuf; }

        /** Set the vertex buffer.
            \param[in] vertexBuf Buffer containing mesh vertices
        */
        void setPositionsBuffer(const Buffer::SharedPtr& vertexBuf) { mVertexBuf = vertexBuf; }

        /** Get the vertex buffer.
        */
        const Buffer::SharedPtr& getPositionsBuffer() const { return mVertexBuf; }

        /** Set the texture coordinate/UV buffer.
            \param[in] texCoordBuf Buffer containing texture coordinates
        */
        void setTexCoordBuffer(const Buffer::SharedPtr& texCoordBuf) { mTexCoordBuf = texCoordBuf; }

        /** Get texture coordinate buffer.
        */
        const Buffer::SharedPtr& getTexCoordBuffer() const { return mTexCoordBuf; }

        /** Set the mesh CDF buffer.
            \param[in] meshCDF Buffer containing mesh CDF data
        */
        void setMeshCDFBuffer(const Buffer::SharedPtr& meshCDFBuf) { mMeshCDFBuf = meshCDFBuf; }

        /** Get the mesh CDF buffer
        */
        const Buffer::SharedPtr& getMeshCDFBuffer() const { return mMeshCDFBuf; }

        /**
            IMovableObject interface
        */
        void move(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) override;

        /** Creates area lights automatically from meshes with emissive materials in a model.
            \param[in] pModel Model
            \param[out] areaLights Array of area lights created from the model
        */
        static void createAreaLightsForModel(const Model::SharedPtr& pModel, std::vector<Light::SharedPtr>& areaLights);

    private:

        /** This is a utility function that creates an area light for the geometry mesh.
            \param[in] pMeshInstance Instance of geometry mesh
        */
        static Light::SharedPtr createAreaLight(const Model::MeshInstance::SharedPtr& pMeshInstance);

        Model::MeshInstance::SharedPtr mpMeshInstance; ///< Geometry mesh data
        Buffer::SharedPtr mIndexBuf;    ///< Buffer id for indices
        Buffer::SharedPtr mVertexBuf;   ///< Buffer id for vertices
        Buffer::SharedPtr mTexCoordBuf; ///< Buffer id for texcoord
        Buffer::SharedPtr mMeshCDFBuf;  ///< Buffer id for mesh Cumulative distribution function (CDF)

        float mSurfaceArea;          ///< Surface area of the mesh
        vec3 mTangent;               ///< Unnormalized tangent vector of the light
        vec3 mBitangent;             ///< Unnormalized bitangent vector of the light
        std::vector<float> mMeshCDF; ///< CDF function for importance sampling a triangle mesh
    };

#endif // 0
}
