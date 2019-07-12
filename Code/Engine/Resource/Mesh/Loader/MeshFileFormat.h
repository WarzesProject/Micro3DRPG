#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/StringId.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4127)	// warning C4127: conditional expression is constant
	PRAGMA_WARNING_DISABLE_MSVC(4201)	// warning C4201: nonstandard extension used: nameless struct/union
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	PRAGMA_WARNING_DISABLE_MSVC(4324)	// warning C4324: '<x>': structure was padded due to alignment specifier
	#include <glm/glm.hpp>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId AssetId;	///< Asset identifier, internally just a POD "uint32_t", string ID scheme is "<project name>/<asset directory>/<asset name>"


	// Mesh file format content:
	// - File format header
	// - Mesh header
	// - Vertex and index buffer data
	// - Vertex array attribute definitions
	// - Sub-meshes
	namespace v1Mesh
	{


		//[-------------------------------------------------------]
		//[ Definitions                                           ]
		//[-------------------------------------------------------]
		static constexpr uint32_t FORMAT_TYPE	 = STRING_ID("Mesh");
		static constexpr uint32_t FORMAT_VERSION = 7;

		#pragma pack(push)
		#pragma pack(1)
			struct MeshHeader final
			{
				// Bounding, the bounding sphere radius is enclosing the bounding box (don't use the inner bounding box radius)
				glm::vec3 minimumBoundingBoxPosition;
				glm::vec3 maximumBoundingBoxPosition;
				glm::vec3 boundingSpherePosition;
				float	  boundingSphereRadius;
				// Vertex and index data
				uint8_t  numberOfBytesPerVertex;
				uint32_t numberOfVertices;
				uint8_t  indexBufferFormat;
				uint32_t numberOfIndices;
				uint8_t  numberOfVertexAttributes;
				// Sub-meshes
				uint16_t numberOfSubMeshes;
				// Optional skeleton
				uint8_t  numberOfBones;
			};

			struct SubMesh final
			{
				AssetId  materialAssetId;
				uint32_t startIndexLocation;
				uint32_t numberOfIndices;
			};
		#pragma pack(pop)


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
	} // v1Mesh
} // RendererRuntime
