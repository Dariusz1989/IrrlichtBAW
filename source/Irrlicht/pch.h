#ifndef __PCH_H_INCLUDED__
#define __PCH_H_INCLUDED__

#include "aabbox3d.h"
#include "CConcurrentObjectCache.h"
#include "CFinalBoneHierarchy.h"
#include "CObjectCache.h"
#include "COpenGLStateManager.h"
#include "coreutil.h"
#include "dimension2d.h"
#include "driverChoice.h"
#include "ECullingTypes.h"
#include "EDebugSceneTypes.h"
#include "EDeviceTypes.h"
#include "EDriverTypes.h"
#include "EMaterialFlags.h"
#include "EMaterialTypes.h"
#include "EMeshWriterEnums.h"
#include "ESceneNodeAnimatorTypes.h"
#include "ESceneNodeTypes.h"
#include "IAnimatedMesh.h"
#include "IAnimatedMeshSceneNode.h"
#include "IBillboardSceneNode.h"
#include "ICameraSceneNode.h"
#include "ICommandBuffer.h"
#include "ICursorControl.h"
#include "IDriver.h"
#include "IDriverFence.h"
#include "IDriverMemoryAllocation.h"
#include "IDriverMemoryBacked.h"
#include "IDummyTransformationSceneNode.h"
#include "IEventReceiver.h"
#include "IFileArchive.h"
#include "IFileList.h"
#include "IFileSystem.h"
#include "IFrameBuffer.h"
#include "IGPUBuffer.h"
#include "IGPUProgrammingServices.h"
#include "IGPUTimestampQuery.h"
#include "IImage.h"
#include "IImageLoader.h"
#include "IImageWriter.h"
#include "ILogger.h"
#include "IMaterialRenderer.h"
#include "IMaterialRendererServices.h"
#include "IMeshSceneNode.h"
#include "IMeshSceneNodeInstanced.h"
#include "IMultisampleTexture.h"
#include "IOSOperator.h"
#include "IQueryObject.h"
#include "IReadFile.h"
#include "IrrlichtDevice.h"
#include "ISceneManager.h"
#include "ISceneNode.h"
#include "ISceneNodeAnimator.h"
#include "ISceneNodeAnimatorCameraFPS.h"
#include "ISceneNodeAnimatorCameraMaya.h"
#include "IShaderConstantSetCallBack.h"
#include "ISkinnedMeshSceneNode.h"
#include "ISkinningStateManager.h"
#include "ITexture.h"
#include "ITextureBufferObject.h"
#include "ITimer.h"
#include "ITransformFeedback.h"
#include "IVideoCapabilityReporter.h"
#include "IVideoDriver.h"
#include "IVideoModeList.h"
#include "IVirtualTexture.h"
#include "IWriteFile.h"
#include "Keycodes.h"
#include "line2d.h"
#include "line3d.h"
#include "matrix3x4SIMD.h"
#include "matrix4SIMD.h"
#include "matrix4x3.h"
#include "matrixutil.h"
#include "path.h"
#include "position2d.h"
#include "quaternion.h"
#include "rect.h"
#include "SAABoxCollider.h"
#include "SceneParameters.h"
#include "SCollisionEngine.h"
#include "SColor.h"
#include "SCompoundCollider.h"
#include "SEllipsoidCollider.h"
#include "SExposedVideoData.h"
#include "SIMDswizzle.h"
#include "SIrrCreationParameters.h"
#include "SKeyMap.h"
#include "SMaterial.h"
#include "SMaterialLayer.h"
#include "splines.h"
#include "STextureSamplingParams.h"
#include "STriangleMeshCollider.h"
#include "SVertexManipulator.h"
#include "SViewFrustum.h"
#include "vector2d.h"
#include "vector3d.h"
#include "vectorSIMD.h"
#include "irr/macros.h"
#include "irr/static_if.h"
#include "irr/switch_constexpr.h"
#include "irr/type_traits.h"
#include "irr/void_t.h"
#include "irr/asset/CCPUSkinnedMesh.h"
#include "irr/asset/CForsythVertexCacheOptimizer.h"
#include "irr/asset/CImageData.h"
#include "irr/asset/EFormat.h"
#include "irr/asset/IAsset.h"
#include "irr/asset/IAssetLoader.h"
#include "irr/asset/IAssetManager.h"
#include "irr/asset/IAssetWriter.h"
#include "irr/asset/ICPUBuffer.h"
#include "irr/asset/ICPUMesh.h"
#include "irr/asset/ICPUMeshBuffer.h"
#include "irr/asset/ICPUShader.h"
#include "irr/asset/ICPUSkinnedMesh.h"
#include "irr/asset/ICPUSkinnedMeshBuffer.h"
#include "irr/asset/ICPUSpecializedShader.h"
#include "irr/asset/ICPUTexture.h"
#include "irr/asset/IGeometryCreator.h"
#include "irr/asset/IGLSLCompiler.h"
#include "irr/asset/IMesh.h"
#include "irr/asset/IMeshManipulator.h"
#include "irr/asset/ISPIR_VProgram.h"
#include "irr/asset/SCPUMesh.h"
#include "irr/asset/ShaderCommons.h"
#include "irr/asset/ShaderRes.h"
#include "irr/core/BaseClasses.h"
#include "irr/core/EventDeferredHandler.h"
#include "irr/core/IBuffer.h"
#include "irr/core/IReferenceCounted.h"
#include "irr/core/irrString.h"
#include "irr/core/IThreadBound.h"
#include "irr/core/Types.h"
#include "irr/core/unlock_guard.h"
#include "irr/core/alloc/AddressAllocatorBase.h"
#include "irr/core/alloc/AddressAllocatorConcurrencyAdaptors.h"
#include "irr/core/alloc/address_allocator_traits.h"
#include "irr/core/alloc/AlignedBase.h"
#include "irr/core/alloc/aligned_allocator.h"
#include "irr/core/alloc/aligned_allocator_adaptor.h"
#include "irr/core/alloc/AllocatorTrivialBases.h"
#include "irr/core/alloc/ContiguousPoolAddressAllocator.h"
#include "irr/core/alloc/GeneralpurposeAddressAllocator.h"
#include "irr/core/alloc/HeterogenousMemoryAddressAllocatorAdaptor.h"
#include "irr/core/alloc/IAddressAllocator.h"
#include "irr/core/alloc/IAllocator.h"
#include "irr/core/alloc/LinearAddressAllocator.h"
#include "irr/core/alloc/MultiBufferingAllocatorBase.h"
#include "irr/core/alloc/null_allocator.h"
#include "irr/core/alloc/PoolAddressAllocator.h"
#include "irr/core/alloc/ResizableHeterogenousMemoryAllocator.h"
#include "irr/core/alloc/StackAddressAllocator.h"
#include "irr/core/math/irrMath.h"
#include "irr/core/math/plane3dSIMD.h"
#include "irr/core/memory/memory.h"
#include "irr/core/memory/new_delete.h"
#include "irr/video/asset_traits.h"
#include "irr/video/convertColor.h"
#include "irr/video/decodePixels.h"
#include "irr/video/encodePixels.h"
#include "irr/video/GPUMemoryAllocatorBase.h"
#include "irr/video/HostDeviceMirrorBufferAllocator.h"
#include "irr/video/IGPUMesh.h"
#include "irr/video/IGPUMeshBuffer.h"
#include "irr/video/IGPUObjectFromAssetConverter.h"
#include "irr/video/IGPUShader.h"
#include "irr/video/IGPUSkinnedMesh.h"
#include "irr/video/IGPUSpecializedShader.h"
#include "irr/video/ResizableBufferingAllocator.h"
#include "irr/video/SGPUMesh.h"
#include "irr/video/SimpleGPUBufferAllocator.h"
#include "irr/video/StreamingGPUBufferAllocator.h"
#include "irr/video/StreamingTransientDataBuffer.h"
#include "irr/video/SubAllocatedDataBuffer.h"
#include "BuiltInFont.h"
#include "CBillboardSceneNode.h"
#include "CBlit.h"
#include "CCameraSceneNode.h"
#include "CColorConverter.h"
#include "CCubeSceneNode.h"
#include "CDepthBuffer.h"
#include "CFileList.h"
#include "CFileSystem.h"
#include "CFPSCounter.h"
#include "CImage.h"
#include "CIrrDeviceConsole.h"
#include "CIrrDeviceLinux.h"
#include "CIrrDeviceSDL.h"
#include "CIrrDeviceStub.h"
#include "CIrrDeviceWin32.h"
#include "CLimitReadFile.h"
#include "CLogger.h"
#include "CMemoryFile.h"
#include "CMeshSceneNode.h"
#include "CMeshSceneNodeInstanced.h"
#include "CMountPointReader.h"
#include "CNPKReader.h"
#include "CNullDriver.h"
#include "COpenCLHandler.h"
#include "COpenGL1DTexture.h"
#include "COpenGL1DTextureArray.h"
#include "COpenGL2DTexture.h"
#include "COpenGL2DTextureArray.h"
#include "COpenGL3DTexture.h"
#include "COpenGLBuffer.h"
#include "COpenGLCubemapArrayTexture.h"
#include "COpenGLCubemapTexture.h"
#include "COpenGLDriver.h"
#include "COpenGLDriverFence.h"
#include "COpenGLExtensionHandler.h"
#include "COpenGLFrameBuffer.h"
#include "COpenGLMaterialRenderer.h"
#include "COpenGLMultisampleTexture.h"
#include "COpenGLMultisampleTextureArray.h"
#include "COpenGLQuery.h"
#include "COpenGLSLMaterialRenderer.h"
#include "COpenGLTexture.h"
#include "COpenGLTextureBufferObject.h"
#include "COpenGLTimestampQuery.h"
#include "COpenGLTransformFeedback.h"
#include "COpenGLVAOSpec.h"
#include "COSOperator.h"
#include "CPakReader.h"
#include "CReadFile.h"
#include "CSceneManager.h"
#include "CSceneNodeAnimatorCameraFPS.h"
#include "CSceneNodeAnimatorCameraMaya.h"
#include "CSceneNodeAnimatorDelete.h"
#include "CSceneNodeAnimatorFlyCircle.h"
#include "CSceneNodeAnimatorFlyStraight.h"
#include "CSceneNodeAnimatorFollowSpline.h"
#include "CSceneNodeAnimatorRotation.h"
#include "CSceneNodeAnimatorTexture.h"
#include "CSkinnedMeshSceneNode.h"
#include "CSkinningStateManager.h"
#include "CSkyBoxSceneNode.h"
#include "CSkyDomeSceneNode.h"
#include "CSoftware2MaterialRenderer.h"
#include "CSoftwareDriver2.h"
#include "CSoftwareTexture2.h"
#include "CSphereSceneNode.h"
#include "CTarReader.h"
#include "CVideoModeList.h"
#include "CWADReader.h"
#include "CWriteFile.h"
#include "CZipReader.h"
#include "FW_Mutex.h"
#include "IBurningShader.h"
#include "IDepthBuffer.h"
#include "IImagePresenter.h"
#include "ISceneNodeAnimatorFinishing.h"
#include "IZBuffer.h"
#include "os.h"
#include "S2DVertex.h"
#include "S4DVertex.h"
#include "SoftwareDriver2_compile_config.h"
#include "SoftwareDriver2_helper.h"
#include "irr/asset/CBAWMeshFileLoader.h"
#include "irr/asset/CBAWMeshWriter.h"
#include "irr/asset/CGeometryCreator.h"
#include "irr/asset/CImageLoaderBMP.h"
#include "irr/asset/CImageLoaderDDS.h"
#include "irr/asset/CImageLoaderJPG.h"
#include "irr/asset/CImageLoaderPNG.h"
#include "irr/asset/CImageLoaderTGA.h"
#include "irr/asset/CImageWriterBMP.h"
#include "irr/asset/CImageWriterJPG.h"
#include "irr/asset/CImageWriterPNG.h"
#include "irr/asset/CImageWriterTGA.h"
#include "irr/asset/CMeshManipulator.h"
#include "irr/asset/COBJMeshFileLoader.h"
#include "irr/asset/COverdrawMeshOptimizer.h"
#include "irr/asset/CPLYMeshFileLoader.h"
#include "irr/asset/CPLYMeshWriter.h"
#include "irr/asset/CSTLMeshFileLoader.h"
#include "irr/asset/CSTLMeshWriter.h"
#include "irr/asset/CXMeshFileLoader.h"
#include "irr/asset/IParsedShaderSource.h"
#include "irr/asset/shadercUtils.h"
#include "irr/asset/bawformat/legacy/CBAWLegacy.h"
#include "irr/video/CGPUSkinnedMesh.h"
#include "irr/video/COpenGLSpecializedShader.h"
#include "irr/asset/CGLSLScanBuiltinIncludeLoader.h"
#include "irr/asset/CGLSLSkinningBuiltinIncludeLoader.h"
#include "irr/asset/CBuiltinIncluder.h"
#include "irr/asset/IBuiltinIncludeLoader.h"
#include "irr/asset/IIncluder.h"
#include "irr/asset/IIncludeHandler.h"

#endif //__PCH_H_INCLUDED__
