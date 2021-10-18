/*
 * Copyright (c) 2021-2022 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authors:
 * - Christophe Riccio <christophe@lunarg.com>
 */

#include <vulkan/vulkan_core.h>
#if defined(__APPLE__)
#include <vulkan/vulkan_beta.h>
#endif

#define VP_MAX_PROFILE_NAME_SIZE 256U

#define VP_KHR_minimum_requirements 1
#define VP_KHR_MINIMUM_REQUIREMENTS_SPEC_VERSION 1
#define VP_KHR_MINIMUM_REQUIREMENTS_NAME "VP_KHR_minimum_requirements"

#define VP_KHR_1_2_roadmap_2022 1
#define VP_KHR_1_2_ROADMAP_2022_SPEC_VERSION 1
#define VP_KHR_1_2_ROADMAP_2022_NAME "VP_KHR_1_2_roadmap_2022"
#define VP_KHR_1_2_ROADMAP_2022_MIN_VERSION VK_MAKE_VERSION(1, 2, 142)

#define VP_LUNARG_1_1_desktop_portability_2022 1
#define VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_SPEC_VERSION 1
#define VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_NAME "VP_LUNARG_1_1_desktop_portability_2022"
#define VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_MIN_API_VERSION VK_MAKE_VERSION(1, 1, 142)

#define countof(arr) sizeof(arr) / sizeof(arr[0])

typedef struct VpProfileProperties {
    char profileName[VP_MAX_PROFILE_NAME_SIZE];
    uint32_t specVersion;
} VpProfileProperties;

VkResult vpCreateDevice(VkPhysicalDevice physicalDevice, const VpProfileProperties *pProfile, const VkDeviceCreateInfo *pCreateInfo,
                        const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);

VkResult vpEnumerateDeviceProfiles(VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *pPropertyCount,
                                   VpProfileProperties *pProperties);

void vpGetProfileFeatures(const VpProfileProperties *pProfile, void *pNext);

void vpEnumerateProfileExtensionProperties(const VpProfileProperties *pProfile, uint32_t *pPropertyCount,
                                           VkExtensionProperties *pProperties);

typedef enum VpStructureArea { VP_STRUCTURE_FEATURES = 0, VP_STRUCTURE_PROPERTIES } VpStructureArea;

void vpEnumerateProfileStructureTypes(const VpProfileProperties *pProfile, VpStructureArea structureArea,
                                      uint32_t *pStructureTypesCount, VkStructureType *pStructureTypes);

// Implementation details:
#include <cstring>
#include <vector>

static const VkExtensionProperties VP_KHR_1_2_ROADMAP_2022_EXTENSIONS[] = {
    VkExtensionProperties{VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME, 2},
    VkExtensionProperties{VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_PRIVATE_DATA_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME, 1}};

static const VkExtensionProperties VP_KHR_1_1_DESKTOP_PORTABILITY_2022_EXTENSIONS[] = {
    // Vulkan 1.2 extensions
    VkExtensionProperties{VK_KHR_8BIT_STORAGE_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME,
                          1},  // Not supported by Intel 6000 https://vulkan.gpuinfo.org/displayreport.php?id=11332#extensions
    VkExtensionProperties{VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME,
                          1},  // Not supported by Intel 6000 https://vulkan.gpuinfo.org/displayreport.php?id=11332#extensions
    VkExtensionProperties{VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME, 1},

// Additional Vulkan extensions
// VkExtensionProperties{VK_KHR_SWAPCHAIN_EXTENSION_NAME, 70}, VkExtensionProperties{VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME,
// 1},
#if defined(__APPLE__)
    VkExtensionProperties{VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME, 1},  // MacOS only
#endif
    VkExtensionProperties{VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
                          1},  // Not supported by Intel 520 https://vulkan.gpuinfo.org/displayreport.php?id=12491#extensions
    VkExtensionProperties{VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME, 1},
    VkExtensionProperties{VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME, 1}};

static const VkStructureType VP_KHR_1_2_ROADMAP_2022_FEATURE_STRUCTURE_TYPES[] = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES_KHR,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES_EXT,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES_EXT,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT};

static const VkStructureType VP_KHR_1_2_ROADMAP_2022_PROPERTY_STRUCTURE_TYPES[] = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};

static const VkStructureType VP_KHR_1_1_DESKTOP_PORTABILITY_2022_FEATURE_STRUCTURE_TYPES[] = {
#if defined(__APPLE__)
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR,
#endif
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

static const VkStructureType VP_KHR_1_1_DESKTOP_PORTABILITY_2022_PROPERTY_STRUCTURE_TYPES[] = {
#if defined(__APPLE__)
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR,
#endif
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES_EXT,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES,
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};

struct VpFormatProperties {
    VkFormat format;
    VkFormatFeatureFlags linearTilingFeatures;
    VkFormatFeatureFlags optimalTilingFeatures;
    VkFormatFeatureFlags bufferFeatures;
};

inline bool vpCheckExtension(const VkExtensionProperties *supportedProperties, std::size_t supportedSize,
                             const char *requestedExtension) {
    for (size_t i = 0, n = supportedSize; i < n; ++i) {
        if (strcmp(supportedProperties[i].extensionName, requestedExtension) == 0) return true;
    }

    return false;
}

inline bool vpCheckMemoryProperty(const VkPhysicalDeviceMemoryProperties &memoryProperties,
                                  const VkMemoryPropertyFlags &memoryPropertyFlags) {
    assert(&memoryProperties != nullptr);

    for (size_t i = 0, n = memoryProperties.memoryTypeCount; i < n; ++i) {
        if ((memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) return true;
    }

    return false;
}

inline bool vpCheckFormatProperty(const VkFormatProperties2 *deviceProps, const VpFormatProperties &profileProps) {
    if ((deviceProps->formatProperties.linearTilingFeatures & profileProps.linearTilingFeatures) !=
        profileProps.linearTilingFeatures) {
        return false;
    } else if ((deviceProps->formatProperties.optimalTilingFeatures & profileProps.optimalTilingFeatures) !=
               profileProps.optimalTilingFeatures) {
        return false;
    } else if ((deviceProps->formatProperties.bufferFeatures & profileProps.bufferFeatures) != profileProps.bufferFeatures) {
        return false;
    }

    return true;
}

inline bool vpCheckQueueFamilyProperty(const VkQueueFamilyProperties *queueFamilyProperties, std::size_t queueFamilyPropertiesCount,
                                       const VkQueueFamilyProperties &profileQueueFamilyPropertie) {
    assert(queueFamilyProperties != nullptr);

    for (size_t i = 0, n = queueFamilyPropertiesCount; i < n; ++i) {
        if (queueFamilyProperties[i].queueCount < profileQueueFamilyPropertie.queueCount) {
            continue;
        } else if (queueFamilyProperties[i].timestampValidBits < profileQueueFamilyPropertie.timestampValidBits) {
            continue;
        } else if (queueFamilyProperties[i].minImageTransferGranularity.width >
                   profileQueueFamilyPropertie.minImageTransferGranularity.width) {
            continue;
        } else if (queueFamilyProperties[i].minImageTransferGranularity.height >
                   profileQueueFamilyPropertie.minImageTransferGranularity.height) {
            continue;
        } else if (queueFamilyProperties[i].minImageTransferGranularity.depth >
                   profileQueueFamilyPropertie.minImageTransferGranularity.depth) {
            continue;
        } else if ((queueFamilyProperties[i].queueFlags & profileQueueFamilyPropertie.queueFlags) !=
                   profileQueueFamilyPropertie.queueFlags) {
            continue;
        }

        return true;
    }

    return false;
}

inline void *vpGetStructure(void *pNext, VkStructureType type) {
    if (pNext == nullptr) {
        return nullptr;
    }

    struct VkStruct {
        VkStructureType sType;
        void *pNext;
    };

    VkStruct *p = static_cast<VkStruct *>(pNext);

    if (p->sType == type) {
        return pNext;
    } else {
        return vpGetStructure(p->pNext, type);
    }
}

inline void vpGetProfileFeatures(const VpProfileProperties *pProfile, void *pNext) {
    if (pProfile == nullptr || pNext == nullptr) return;

    struct VkStruct {
        VkStructureType sType;
        void *pNext;
    };

    VkStruct *p = static_cast<VkStruct *>(pNext);

    if (strcmp(pProfile->profileName, VP_KHR_MINIMUM_REQUIREMENTS_NAME) == 0)
        return;
    else if (strcmp(pProfile->profileName, VP_KHR_1_2_ROADMAP_2022_NAME) == 0) {
        while (p != nullptr) {
            switch (p->sType) {
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2: {
                    VkPhysicalDeviceFeatures2 *features = (VkPhysicalDeviceFeatures2 *)p;
                    features->features.robustBufferAccess = VK_TRUE;
                    features->features.imageCubeArray = VK_TRUE;
                    features->features.independentBlend = VK_TRUE;
                    features->features.sampleRateShading = VK_TRUE;
                    features->features.drawIndirectFirstInstance = VK_TRUE;
                    features->features.depthBiasClamp = VK_TRUE;
                    features->features.samplerAnisotropy = VK_TRUE;
                    features->features.occlusionQueryPrecise = VK_TRUE;
                    features->features.fragmentStoresAndAtomics = VK_TRUE;
                    features->features.shaderStorageImageExtendedFormats = VK_TRUE;
                    features->features.shaderStorageImageReadWithoutFormat = VK_TRUE;
                    features->features.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
                    features->features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
                    features->features.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
                    features->features.shaderStorageImageArrayDynamicIndexing = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES: {
                    VkPhysicalDeviceVulkan11Features *features = (VkPhysicalDeviceVulkan11Features *)p;
                    features->multiview = VK_TRUE;
                    features->samplerYcbcrConversion = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES: {
                    VkPhysicalDeviceVulkan12Features *features = (VkPhysicalDeviceVulkan12Features *)p;
                    features->samplerMirrorClampToEdge = VK_TRUE;
                    features->shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE;
                    features->uniformBufferStandardLayout = VK_TRUE;
                    features->shaderSubgroupExtendedTypes = VK_TRUE;
                    features->separateDepthStencilLayouts = VK_TRUE;
                    features->hostQueryReset = VK_TRUE;
                    features->bufferDeviceAddress = VK_TRUE;
                    features->vulkanMemoryModel = VK_TRUE;
                    features->vulkanMemoryModelDeviceScope = VK_TRUE;
                    features->vulkanMemoryModelAvailabilityVisibilityChains = VK_TRUE;
                    features->subgroupBroadcastDynamicId = VK_TRUE;
                    features->imagelessFramebuffer = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR: {
                    VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR *features =
                        (VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR *)p;
                    features->shaderTerminateInvocation = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR: {
                    VkPhysicalDeviceSynchronization2FeaturesKHR *features = (VkPhysicalDeviceSynchronization2FeaturesKHR *)p;
                    features->synchronization2 = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES_KHR: {
                    VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR *features =
                        (VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR *)p;
                    features->shaderZeroInitializeWorkgroupMemory = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT: {
                    VkPhysicalDeviceImageRobustnessFeaturesEXT *features = (VkPhysicalDeviceImageRobustnessFeaturesEXT *)p;
                    features->robustImageAccess = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT: {
                    VkPhysicalDeviceInlineUniformBlockFeaturesEXT *features = (VkPhysicalDeviceInlineUniformBlockFeaturesEXT *)p;
                    features->inlineUniformBlock = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES_EXT: {
                    VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT *features =
                        (VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT *)p;
                    features->pipelineCreationCacheControl = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES_EXT: {
                    VkPhysicalDevicePrivateDataFeaturesEXT *features = (VkPhysicalDevicePrivateDataFeaturesEXT *)p;
                    features->privateData = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT: {
                    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT *features =
                        (VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT *)p;
                    features->shaderDemoteToHelperInvocation = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT: {
                    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT *features = (VkPhysicalDeviceSubgroupSizeControlFeaturesEXT *)p;
                    features->subgroupSizeControl = VK_TRUE;
                    features->computeFullSubgroups = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT: {
                    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *features =
                        (VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *)p;
                    features->texelBufferAlignment = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT: {
                    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *features =
                        (VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *)p;
                    features->extendedDynamicState2 = VK_TRUE;
                } break;
                default:
                    break;
            }
            p = static_cast<VkStruct *>(p->pNext);
        }
    } else if (strcmp(pProfile->profileName, VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_NAME) == 0) {
        while (p != nullptr) {
            switch (p->sType) {
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2: {
                    VkPhysicalDeviceFeatures2 *features = (VkPhysicalDeviceFeatures2 *)p;
                    features->features.depthBiasClamp = VK_TRUE;
                    features->features.depthClamp = VK_TRUE;
                    features->features.drawIndirectFirstInstance = VK_TRUE;
                    features->features.dualSrcBlend = VK_TRUE;
                    features->features.fillModeNonSolid = VK_TRUE;
                    features->features.fragmentStoresAndAtomics = VK_TRUE;
                    features->features.fullDrawIndexUint32 = VK_TRUE;
                    features->features.imageCubeArray = VK_TRUE;
                    features->features.independentBlend = VK_TRUE;
                    features->features.inheritedQueries = VK_TRUE;
                    features->features.largePoints = VK_TRUE;
                    features->features.multiDrawIndirect = VK_TRUE;
                    features->features.multiViewport = VK_TRUE;
                    features->features.occlusionQueryPrecise = VK_TRUE;
                    features->features.robustBufferAccess = VK_TRUE;
                    features->features.sampleRateShading = VK_TRUE;
                    features->features.samplerAnisotropy = VK_TRUE;
                    features->features.shaderClipDistance = VK_TRUE;
                    features->features.shaderImageGatherExtended = VK_TRUE;
                    features->features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
                    features->features.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
                    features->features.shaderStorageImageArrayDynamicIndexing = VK_TRUE;
                    features->features.shaderStorageImageExtendedFormats = VK_TRUE;
                    features->features.shaderStorageImageWriteWithoutFormat = VK_TRUE;
                    features->features.shaderTessellationAndGeometryPointSize = VK_TRUE;
                    features->features.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
                    features->features.tessellationShader = VK_TRUE;
                    features->features.textureCompressionBC = VK_TRUE;
                    features->features.vertexPipelineStoresAndAtomics = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES: {
                    VkPhysicalDeviceImagelessFramebufferFeatures *features = (VkPhysicalDeviceImagelessFramebufferFeatures *)p;
                    features->imagelessFramebuffer = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES: {
                    VkPhysicalDevice16BitStorageFeatures *features = (VkPhysicalDevice16BitStorageFeatures *)p;
                    features->storageBuffer16BitAccess = VK_TRUE;
                    features->uniformAndStorageBuffer16BitAccess = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES: {
                    VkPhysicalDeviceMultiviewFeatures *features = (VkPhysicalDeviceMultiviewFeatures *)p;
                    features->multiview = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES: {
                    VkPhysicalDeviceDescriptorIndexingFeatures *features = (VkPhysicalDeviceDescriptorIndexingFeatures *)p;
                    features->shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE;
                    features->shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE;
                    features->shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
                    features->shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
                    features->shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE;
                    features->descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
                    features->descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
                    features->descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
                    features->descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
                    features->descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;
                    features->descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
                    features->descriptorBindingPartiallyBound = VK_TRUE;
                    features->descriptorBindingVariableDescriptorCount = VK_TRUE;
                    features->runtimeDescriptorArray = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES: {
                    VkPhysicalDeviceHostQueryResetFeatures *features = (VkPhysicalDeviceHostQueryResetFeatures *)p;
                    features->hostQueryReset = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES: {
                    VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *features =
                        (VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *)p;
                    features->shaderSubgroupExtendedTypes = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES: {
                    VkPhysicalDeviceUniformBufferStandardLayoutFeatures *features =
                        (VkPhysicalDeviceUniformBufferStandardLayoutFeatures *)p;
                    features->uniformBufferStandardLayout = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES: {
                    VkPhysicalDeviceShaderDrawParametersFeatures *features = (VkPhysicalDeviceShaderDrawParametersFeatures *)p;
                    features->shaderDrawParameters = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES: {
                    VkPhysicalDevice8BitStorageFeatures *features = (VkPhysicalDevice8BitStorageFeatures *)p;
                    features->storageBuffer8BitAccess = VK_TRUE;
                    features->storagePushConstant8 = VK_TRUE;
                    features->uniformAndStorageBuffer8BitAccess = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES: {
                    VkPhysicalDeviceShaderFloat16Int8Features *features = (VkPhysicalDeviceShaderFloat16Int8Features *)p;
                    features->shaderInt8 = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES: {
                    VkPhysicalDeviceSamplerYcbcrConversionFeatures *features = (VkPhysicalDeviceSamplerYcbcrConversionFeatures *)p;
                    features->samplerYcbcrConversion = VK_TRUE;
                } break;
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES: {
                    VkPhysicalDeviceVariablePointersFeatures *features = (VkPhysicalDeviceVariablePointersFeatures *)p;
                    features->variablePointersStorageBuffer = VK_TRUE;
                    features->variablePointers = VK_TRUE;
                } break;
#if defined(__APPLE__)
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR: {
                    VkPhysicalDevicePortabilitySubsetFeaturesKHR *features = (VkPhysicalDevicePortabilitySubsetFeaturesKHR *)p;
                    features->vertexAttributeAccessBeyondStride = VK_TRUE;
                    features->separateStencilMaskRef = VK_TRUE;
                    features->mutableComparisonSamplers = VK_TRUE;
                    features->multisampleArrayImage = VK_TRUE;
                    features->imageViewFormatSwizzle = VK_TRUE;
                    features->imageViewFormatReinterpretation = VK_TRUE;
                    features->events = VK_TRUE;
                    features->constantAlphaColorBlendFactors = VK_TRUE;
                } break;
#endif
                default:
                    break;
            }
            p = static_cast<VkStruct *>(p->pNext);
        }
    }
}

inline VkResult vpCreateDevice(VkPhysicalDevice physicalDevice, const VpProfileProperties *pProfile,
                               const VkDeviceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    if (pProfile == nullptr) {
        return vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    } else if (strcmp(pProfile->profileName, VP_KHR_MINIMUM_REQUIREMENTS_NAME) == 0) {
        return vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    } else if (strcmp(pProfile->profileName, VP_KHR_1_2_ROADMAP_2022_NAME) == 0) {
        std::vector<const char *> extensions;
        for (int i = 0, n = countof(VP_KHR_1_2_ROADMAP_2022_EXTENSIONS); i < n; ++i) {
            extensions.push_back(VP_KHR_1_2_ROADMAP_2022_EXTENSIONS[i].extensionName);
        }

        for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i) {
            if (vpCheckExtension(VP_KHR_1_2_ROADMAP_2022_EXTENSIONS, countof(VP_KHR_1_2_ROADMAP_2022_EXTENSIONS),
                                 pCreateInfo->ppEnabledExtensionNames[i])) {
                continue;
            }
            extensions.push_back(pCreateInfo->ppEnabledExtensionNames[i]);
        }

        void *pProfileNext = nullptr;

        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = nullptr;
        pProfileNext = &deviceFeatures2;

        VkPhysicalDeviceVulkan11Features deviceVulkan11Features = {};
        deviceVulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        deviceVulkan11Features.pNext = pProfileNext;
        pProfileNext = &deviceVulkan11Features;

        VkPhysicalDeviceVulkan12Features deviceVulkan12Features = {};
        deviceVulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        deviceVulkan12Features.pNext = pProfileNext;
        pProfileNext = &deviceVulkan12Features;

        VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR deviceShaderTerminateFeatures = {};
        deviceShaderTerminateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR;
        deviceShaderTerminateFeatures.pNext = pProfileNext;
        pProfileNext = &deviceShaderTerminateFeatures;

        VkPhysicalDeviceSynchronization2FeaturesKHR deviceSynchrization2Features = {};
        deviceSynchrization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
        deviceSynchrization2Features.pNext = pProfileNext;
        pProfileNext = &deviceSynchrization2Features;

        VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR deviceZeroInitFeatures = {};
        deviceZeroInitFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES_KHR;
        deviceZeroInitFeatures.pNext = pProfileNext;
        pProfileNext = &deviceZeroInitFeatures;

        VkPhysicalDeviceImageRobustnessFeaturesEXT deviceImageRobustnessFeatures = {};
        deviceImageRobustnessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT;
        deviceImageRobustnessFeatures.pNext = pProfileNext;
        pProfileNext = &deviceImageRobustnessFeatures;

        VkPhysicalDeviceInlineUniformBlockFeaturesEXT deviceInlineBlockFeatures = {};
        deviceInlineBlockFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT;
        deviceInlineBlockFeatures.pNext = pProfileNext;
        pProfileNext = &deviceInlineBlockFeatures;

        VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT devicePipelineCreationFeatures = {};
        devicePipelineCreationFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES_EXT;
        devicePipelineCreationFeatures.pNext = pProfileNext;
        pProfileNext = &devicePipelineCreationFeatures;

        VkPhysicalDevicePrivateDataFeaturesEXT devicePrivateDataFeatures = {};
        devicePrivateDataFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES_EXT;
        devicePrivateDataFeatures.pNext = pProfileNext;
        pProfileNext = &devicePrivateDataFeatures;

        VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT deviceShaderDemoteFeatures = {};
        deviceShaderDemoteFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT;
        deviceShaderDemoteFeatures.pNext = pProfileNext;
        pProfileNext = &deviceShaderDemoteFeatures;

        VkPhysicalDeviceSubgroupSizeControlFeaturesEXT deviceSubgroupSizeFeatures = {};
        deviceSubgroupSizeFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT;
        deviceSubgroupSizeFeatures.pNext = pProfileNext;
        pProfileNext = &deviceSubgroupSizeFeatures;

        VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT deviceTexelBufferFeatures = {};
        deviceTexelBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT;
        deviceTexelBufferFeatures.pNext = pProfileNext;
        pProfileNext = &deviceTexelBufferFeatures;

        VkPhysicalDeviceExtendedDynamicState2FeaturesEXT deviceExtendedDynamicState2Features = {};
        deviceExtendedDynamicState2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
        deviceExtendedDynamicState2Features.pNext = pProfileNext;
        pProfileNext = &deviceExtendedDynamicState2Features;

        vpGetProfileFeatures(pProfile, pProfileNext);

        void *pRoot = const_cast<void *>(pCreateInfo->pNext);
        void *pNext = pRoot;

        if (pCreateInfo->pEnabledFeatures != nullptr) {
            deviceFeatures2.features = *pCreateInfo->pEnabledFeatures;
        }
        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2) == nullptr &&
            pCreateInfo->pEnabledFeatures == nullptr) {
            deviceFeatures2.pNext = pNext;
            pNext = &deviceFeatures2;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES) == nullptr) {
            deviceVulkan11Features.pNext = pNext;
            pNext = &deviceVulkan11Features;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES) == nullptr) {
            deviceVulkan12Features.pNext = pNext;
            pNext = &deviceVulkan12Features;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR) == nullptr) {
            deviceShaderTerminateFeatures.pNext = pNext;
            pNext = &deviceShaderTerminateFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR) == nullptr) {
            deviceSynchrization2Features.pNext = pNext;
            pNext = &deviceSynchrization2Features;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES_KHR) == nullptr) {
            deviceZeroInitFeatures.pNext = pNext;
            pNext = &deviceZeroInitFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT) == nullptr) {
            deviceImageRobustnessFeatures.pNext = pNext;
            pNext = &deviceImageRobustnessFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT) == nullptr) {
            deviceInlineBlockFeatures.pNext = pNext;
            pNext = &deviceInlineBlockFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES_EXT) == nullptr) {
            devicePipelineCreationFeatures.pNext = pNext;
            pNext = &devicePipelineCreationFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES_EXT) == nullptr) {
            devicePrivateDataFeatures.pNext = pNext;
            pNext = &devicePrivateDataFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT) == nullptr) {
            deviceShaderDemoteFeatures.pNext = pNext;
            pNext = &deviceShaderDemoteFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT) == nullptr) {
            deviceSubgroupSizeFeatures.pNext = pNext;
            pNext = &deviceSubgroupSizeFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT) == nullptr) {
            deviceTexelBufferFeatures.pNext = pNext;
            pNext = &deviceTexelBufferFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT) == nullptr) {
            deviceExtendedDynamicState2Features.pNext = pNext;
            pNext = &deviceExtendedDynamicState2Features;
        }

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = pNext;
        deviceCreateInfo.queueCreateInfoCount = pCreateInfo->queueCreateInfoCount;
        deviceCreateInfo.pQueueCreateInfos = pCreateInfo->pQueueCreateInfos;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = static_cast<const char *const *>(extensions.data());
        deviceCreateInfo.pEnabledFeatures = pCreateInfo->pEnabledFeatures != nullptr ? &deviceFeatures2.features : nullptr;
        return vkCreateDevice(physicalDevice, &deviceCreateInfo, pAllocator, pDevice);
    } else if (strcmp(pProfile->profileName, VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_NAME) == 0) {
        std::vector<const char *> extensions;
        for (int i = 0, n = countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_EXTENSIONS); i < n; ++i) {
            extensions.push_back(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_EXTENSIONS[i].extensionName);
        }

        for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i) {
            if (vpCheckExtension(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_EXTENSIONS,
                                 countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_EXTENSIONS),
                                 pCreateInfo->ppEnabledExtensionNames[i])) {
                continue;
            }
            extensions.push_back(pCreateInfo->ppEnabledExtensionNames[i]);
        }

        void *pProfileNext = nullptr;

        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = nullptr;
        pProfileNext = &deviceFeatures2;

        VkPhysicalDeviceImagelessFramebufferFeatures deviceImagelessFeatures = {};
        deviceImagelessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
        deviceImagelessFeatures.pNext = pProfileNext;
        pProfileNext = &deviceImagelessFeatures;

        VkPhysicalDevice16BitStorageFeatures device16BitFeatures = {};
        device16BitFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
        device16BitFeatures.pNext = pProfileNext;
        pProfileNext = &device16BitFeatures;

        VkPhysicalDeviceMultiviewFeatures deviceMultiviewFeatures = {};
        deviceMultiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
        deviceMultiviewFeatures.pNext = pProfileNext;
        pProfileNext = &deviceMultiviewFeatures;

        VkPhysicalDeviceDescriptorIndexingFeatures deviceDescriptorInxedingFeatures = {};
        deviceDescriptorInxedingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        deviceDescriptorInxedingFeatures.pNext = pProfileNext;
        pProfileNext = &deviceDescriptorInxedingFeatures;

        VkPhysicalDeviceHostQueryResetFeatures deviceHostQueryResetFeatures = {};
        deviceHostQueryResetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
        deviceHostQueryResetFeatures.pNext = pProfileNext;
        pProfileNext = &deviceHostQueryResetFeatures;

        VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures deviceShaderSubgroupFeatures = {};
        deviceShaderSubgroupFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES;
        deviceShaderSubgroupFeatures.pNext = pProfileNext;
        pProfileNext = &deviceShaderSubgroupFeatures;

        VkPhysicalDeviceUniformBufferStandardLayoutFeatures deviceUniformBufferFeatures = {};
        deviceUniformBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES;
        deviceUniformBufferFeatures.pNext = pProfileNext;
        pProfileNext = &deviceUniformBufferFeatures;

        VkPhysicalDeviceShaderDrawParametersFeatures deviceShaderDrawFeatures = {};
        deviceShaderDrawFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
        deviceShaderDrawFeatures.pNext = pProfileNext;
        pProfileNext = &deviceShaderDrawFeatures;

        VkPhysicalDevice8BitStorageFeatures device8BitFeatures = {};
        device8BitFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES;
        device8BitFeatures.pNext = pProfileNext;
        pProfileNext = &device8BitFeatures;

        VkPhysicalDeviceShaderFloat16Int8Features deviceShaderFloatFeatures = {};
        deviceShaderFloatFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
        deviceShaderFloatFeatures.pNext = pProfileNext;
        pProfileNext = &deviceShaderFloatFeatures;

        VkPhysicalDeviceSamplerYcbcrConversionFeatures deviceSamplerYcbcrFeatures = {};
        deviceSamplerYcbcrFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
        deviceSamplerYcbcrFeatures.pNext = pProfileNext;
        pProfileNext = &deviceSamplerYcbcrFeatures;

        VkPhysicalDeviceVariablePointersFeatures deviceVariableFeatures = {};
        deviceVariableFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES;
        deviceVariableFeatures.pNext = pProfileNext;
        pProfileNext = &deviceVariableFeatures;

#if defined(__APPLE__)
        VkPhysicalDevicePortabilitySubsetFeaturesKHR devicePortabilitySubset = {};
        devicePortabilitySubset.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR;
        devicePortabilitySubset.pNext = pProfileNext;
        pProfileNext = &devicePortabilitySubset;
#endif

        vpGetProfileFeatures(pProfile, pProfileNext);

        void *pRoot = const_cast<void *>(pCreateInfo->pNext);
        void *pNext = pRoot;

        if (pCreateInfo->pEnabledFeatures != nullptr) {
            deviceFeatures2.features = *pCreateInfo->pEnabledFeatures;
        }
        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2) == nullptr &&
            pCreateInfo->pEnabledFeatures == nullptr) {
            deviceFeatures2.pNext = pNext;
            pNext = &deviceFeatures2;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES) == nullptr) {
            deviceImagelessFeatures.pNext = pNext;
            pNext = &deviceImagelessFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES) == nullptr) {
            device16BitFeatures.pNext = pNext;
            pNext = &device16BitFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES) == nullptr) {
            deviceMultiviewFeatures.pNext = pNext;
            pNext = &deviceMultiviewFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES) == nullptr) {
            deviceDescriptorInxedingFeatures.pNext = pNext;
            pNext = &deviceDescriptorInxedingFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES) == nullptr) {
            deviceHostQueryResetFeatures.pNext = pNext;
            pNext = &deviceHostQueryResetFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES) == nullptr) {
            deviceShaderSubgroupFeatures.pNext = pNext;
            pNext = &deviceShaderSubgroupFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES) == nullptr) {
            deviceUniformBufferFeatures.pNext = pNext;
            pNext = &deviceUniformBufferFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES) == nullptr) {
            deviceShaderDrawFeatures.pNext = pNext;
            pNext = &deviceShaderDrawFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES) == nullptr) {
            device8BitFeatures.pNext = pNext;
            pNext = &device8BitFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES) == nullptr) {
            deviceShaderFloatFeatures.pNext = pNext;
            pNext = &deviceShaderFloatFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES) == nullptr) {
            deviceSamplerYcbcrFeatures.pNext = pNext;
            pNext = &deviceSamplerYcbcrFeatures;
        }

        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES) == nullptr) {
            deviceVariableFeatures.pNext = pNext;
            pNext = &deviceVariableFeatures;
        }

#if defined(__APPLE__)
        if (vpGetStructure(pRoot, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR) == nullptr) {
            pNext = &devicePortabilitySubset;
        }
#endif

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = pNext;
        deviceCreateInfo.queueCreateInfoCount = pCreateInfo->queueCreateInfoCount;
        deviceCreateInfo.pQueueCreateInfos = pCreateInfo->pQueueCreateInfos;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = static_cast<const char *const *>(extensions.data());
        deviceCreateInfo.pEnabledFeatures = pCreateInfo->pEnabledFeatures != nullptr ? &deviceFeatures2.features : nullptr;
        return vkCreateDevice(physicalDevice, &deviceCreateInfo, pAllocator, pDevice);
    } else {
        return VK_ERROR_UNKNOWN;
    }
}

inline VkResult vpEnumerateDeviceProfiles(VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *pPropertyCount,
                                          VpProfileProperties *pProperties) {
    VkResult result = VK_SUCCESS;

    uint32_t instanceExtensionCount;
    result = vkEnumerateInstanceExtensionProperties(pLayerName, &instanceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> instanceExtensions(instanceExtensionCount);
    result = vkEnumerateInstanceExtensionProperties(pLayerName, &instanceExtensionCount, instanceExtensions.data());

    uint32_t deviceExtensionCount;
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, &deviceExtensionCount, deviceExtensions.data());

    uint32_t supportedProfileCount = 0;

    const VpProfileProperties supportedProfiles[] = {
        VpProfileProperties{VP_KHR_MINIMUM_REQUIREMENTS_NAME, VP_KHR_MINIMUM_REQUIREMENTS_SPEC_VERSION},
        VpProfileProperties{VP_KHR_1_2_ROADMAP_2022_NAME, VP_KHR_1_2_ROADMAP_2022_SPEC_VERSION},
        VpProfileProperties{VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_NAME, VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_SPEC_VERSION}};

    for (int i = 0, n = countof(supportedProfiles); i < n; ++i) {
        VkBool32 supported = VK_TRUE;

        if (strcmp(supportedProfiles[i].profileName, VP_KHR_MINIMUM_REQUIREMENTS_NAME) == 0) {
        } else if (strcmp(supportedProfiles[i].profileName, VP_KHR_1_2_ROADMAP_2022_NAME) == 0) {
            for (std::size_t i = 0, n = countof(VP_KHR_1_2_ROADMAP_2022_EXTENSIONS); i < n && supported; ++i) {
                const bool supportedInstanceExt = vpCheckExtension(instanceExtensions.data(), instanceExtensions.size(),
                                                                   VP_KHR_1_2_ROADMAP_2022_EXTENSIONS[i].extensionName);
                const bool supportedDeviceExt = vpCheckExtension(deviceExtensions.data(), deviceExtensions.size(),
                                                                 VP_KHR_1_2_ROADMAP_2022_EXTENSIONS[i].extensionName);

                if (!supportedInstanceExt && !supportedDeviceExt) {
                    supported = VK_FALSE;
                }
            }

            VkPhysicalDeviceExtendedDynamicState2FeaturesEXT deviceExtendedDynamicState = {};
            deviceExtendedDynamicState.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT;
            deviceExtendedDynamicState.pNext = nullptr;

            VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT deviceTexelBufferAlign = {};
            deviceTexelBufferAlign.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT;
            deviceTexelBufferAlign.pNext = &deviceExtendedDynamicState;

            VkPhysicalDeviceSubgroupSizeControlFeaturesEXT deviceSubgroupSize = {};
            deviceSubgroupSize.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT;
            deviceSubgroupSize.pNext = &deviceTexelBufferAlign;

            VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT deviceShaderDemote = {};
            deviceShaderDemote.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT;
            deviceShaderDemote.pNext = &deviceSubgroupSize;

            VkPhysicalDevicePrivateDataFeaturesEXT devicePrivateData = {};
            devicePrivateData.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT;
            devicePrivateData.pNext = &deviceShaderDemote;

            VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT devicePipelineCreation = {};
            devicePipelineCreation.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT;
            devicePipelineCreation.pNext = &devicePrivateData;

            VkPhysicalDeviceInlineUniformBlockFeaturesEXT deviceInlineUniformBlock = {};
            deviceInlineUniformBlock.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT;
            deviceInlineUniformBlock.pNext = &devicePipelineCreation;

            VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR deviceZeroInitialize = {};
            deviceZeroInitialize.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES_KHR;
            deviceZeroInitialize.pNext = &deviceInlineUniformBlock;

            VkPhysicalDeviceSynchronization2FeaturesKHR deviceSynchronization2 = {};
            deviceSynchronization2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
            deviceSynchronization2.pNext = &deviceZeroInitialize;

            VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR deviceShaderTerminateInvocation = {};
            deviceShaderTerminateInvocation.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR;
            deviceShaderTerminateInvocation.pNext = &deviceSynchronization2;

            VkPhysicalDeviceVulkan12Features deviceFeatures12 = {};
            deviceFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            deviceFeatures12.pNext = &deviceShaderTerminateInvocation;

            VkPhysicalDeviceVulkan11Features deviceFeatures11 = {};
            deviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
            deviceFeatures11.pNext = &deviceFeatures12;

            VkPhysicalDeviceFeatures2 deviceFeatures = {};
            deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            deviceFeatures.pNext = &deviceFeatures11;

            vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

            if (deviceFeatures.features.robustBufferAccess != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.imageCubeArray != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.independentBlend != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.sampleRateShading != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.drawIndirectFirstInstance != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.depthBiasClamp != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.samplerAnisotropy != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.occlusionQueryPrecise != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.fragmentStoresAndAtomics != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderStorageImageExtendedFormats != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderStorageImageReadWithoutFormat != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderUniformBufferArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderSampledImageArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderStorageBufferArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderStorageImageArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }

            if (deviceFeatures11.multiview != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures11.samplerYcbcrConversion != VK_TRUE) {
                supported = VK_FALSE;
            }

            if (deviceFeatures12.samplerMirrorClampToEdge != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.shaderUniformTexelBufferArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.uniformBufferStandardLayout != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.shaderSubgroupExtendedTypes != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.separateDepthStencilLayouts != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.hostQueryReset != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.bufferDeviceAddress != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.vulkanMemoryModel != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.vulkanMemoryModelDeviceScope != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.vulkanMemoryModelAvailabilityVisibilityChains != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.subgroupBroadcastDynamicId != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures12.imagelessFramebuffer != VK_TRUE) {
                supported = VK_FALSE;
            }

            if (deviceShaderTerminateInvocation.shaderTerminateInvocation != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceSynchronization2.synchronization2 != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceZeroInitialize.shaderZeroInitializeWorkgroupMemory != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceInlineUniformBlock.inlineUniformBlock != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePipelineCreation.pipelineCreationCacheControl != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePrivateData.privateData != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceShaderDemote.shaderDemoteToHelperInvocation != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceSubgroupSize.subgroupSizeControl != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceSubgroupSize.computeFullSubgroups != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceTexelBufferAlign.texelBufferAlignment != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceExtendedDynamicState.extendedDynamicState2 != VK_TRUE) {
                supported = VK_FALSE;
            }

            VkPhysicalDeviceVulkan12Properties devicePropertiesVulkan12 = {};
            devicePropertiesVulkan12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
            devicePropertiesVulkan12.pNext = nullptr;

            VkPhysicalDeviceVulkan11Properties devicePropertiesVulkan11 = {};
            devicePropertiesVulkan11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
            devicePropertiesVulkan11.pNext = &devicePropertiesVulkan12;

            VkPhysicalDeviceProperties2 deviceProperties{};
            deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            deviceProperties.pNext = &devicePropertiesVulkan11;

            vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);

            if (deviceProperties.properties.limits.maxImageDimension1D < 8192) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxImageDimension2D < 8192) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxImageDimensionCube < 8192) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxImageArrayLayers < 2048) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxUniformBufferRange < 65536) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.bufferImageGranularity > 4096) {
                supported = VK_FALSE;
            }

            if (deviceProperties.properties.limits.maxPerStageDescriptorSamplers < 64) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageDescriptorUniformBuffers < 15) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageDescriptorStorageBuffers < 30) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageDescriptorSampledImages < 200) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageDescriptorStorageImages < 16) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageResources < 200) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetSamplers < 576) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetUniformBuffers < 90) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetStorageBuffers < 96) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetSampledImages < 1800) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetStorageImages < 144) {
                supported = VK_FALSE;
            }

            if (deviceProperties.properties.limits.maxFragmentCombinedOutputResources < 16) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxComputeWorkGroupInvocations < 256) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxComputeWorkGroupSize[0] < 256 ||
                deviceProperties.properties.limits.maxComputeWorkGroupSize[1] < 256 ||
                deviceProperties.properties.limits.maxComputeWorkGroupSize[2] < 64) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.subTexelPrecisionBits < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.mipmapPrecisionBits < 6) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxSamplerLodBias < 14.0) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.pointSizeGranularity > 0.125) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.lineWidthGranularity > 0.5) {
                supported = VK_FALSE;
            }

            if (devicePropertiesVulkan11.subgroupSize < 4) {
                supported = VK_FALSE;
            }

            if (devicePropertiesVulkan12.shaderSignedZeroInfNanPreserveFloat16 != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePropertiesVulkan12.shaderSignedZeroInfNanPreserveFloat32 != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePropertiesVulkan12.maxPerStageDescriptorUpdateAfterBindInputAttachments < 7) {
                supported = VK_FALSE;
            }

        } else if (strcmp(supportedProfiles[i].profileName, VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_NAME) == 0) {
            for (std::size_t i = 0, n = countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_EXTENSIONS); i < n && supported; ++i) {
                const VkExtensionProperties &extensionProperties = VP_KHR_1_1_DESKTOP_PORTABILITY_2022_EXTENSIONS[i];

                const bool supportedInstanceExt =
                    vpCheckExtension(instanceExtensions.data(), instanceExtensions.size(), extensionProperties.extensionName);
                const bool supportedDeviceExt =
                    vpCheckExtension(deviceExtensions.data(), deviceExtensions.size(), extensionProperties.extensionName);

                if (!supportedInstanceExt && !supportedDeviceExt) {
                    supported = VK_FALSE;
                }
            }

static const VpFormatProperties VP_KHR_1_1_DESKTOP_PORTABILITY_2022_FORMATS[] = {
                // VK_FORMAT_R8_UNORM
                {VK_FORMAT_R8_UNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8_SNORM
                {VK_FORMAT_R8_SNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8_UINT
                {VK_FORMAT_R8_UINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8_SINT
                {VK_FORMAT_R8_SINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8G8_UNORM
                {VK_FORMAT_R8G8_UNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8G8_SNORM
                {VK_FORMAT_R8G8_SNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8G8_UINT
                {VK_FORMAT_R8G8_UINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8G8_SINT
                {VK_FORMAT_R8G8_SINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8G8B8A8_UNORM
                {VK_FORMAT_R8G8B8A8_UNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8G8B8A8_SNORM
                {VK_FORMAT_R8G8B8A8_SNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8G8B8A8_UINT
                {VK_FORMAT_R8G8B8A8_UINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8G8B8A8_SINT
                {VK_FORMAT_R8G8B8A8_SINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R8G8B8A8_SRGB
                {VK_FORMAT_R8G8B8A8_SRGB,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},

                // VK_FORMAT_B8G8R8A8_SRGB
                {VK_FORMAT_B8G8R8A8_SRGB,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},

                // VK_FORMAT_A8B8G8R8_UNORM_PACK32
                {VK_FORMAT_A8B8G8R8_UNORM_PACK32,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_A8B8G8R8_SNORM_PACK32
                {VK_FORMAT_A8B8G8R8_UNORM_PACK32,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_A8B8G8R8_UINT_PACK32
                {VK_FORMAT_A8B8G8R8_UINT_PACK32,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},

                // VK_FORMAT_A8B8G8R8_SINT_PACK32
                {VK_FORMAT_A8B8G8R8_SINT_PACK32,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},

                // VK_FORMAT_A8B8G8R8_SRGB_PACK32
                {VK_FORMAT_A8B8G8R8_SRGB_PACK32,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},

                // VK_FORMAT_A2B10G10R10_UNORM_PACK32
                {VK_FORMAT_A2B10G10R10_UNORM_PACK32,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_A2B10G10R10_UINT_PACK32
                {VK_FORMAT_A2B10G10R10_UINT_PACK32,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT},

                // VK_FORMAT_R16_UNORM
                {VK_FORMAT_R16_UNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16_SNORM
                {VK_FORMAT_R16_SNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16_UINT
                {VK_FORMAT_R16_UINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16_SINT
                {VK_FORMAT_R16_SINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16_SFLOAT
                {VK_FORMAT_R16_SFLOAT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16G16_UNORM
                {VK_FORMAT_R16G16_UNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16G16_SNORM
                {VK_FORMAT_R16G16_SNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16G16_UINT
                {VK_FORMAT_R16G16_UINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16G16_SINT
                {VK_FORMAT_R16G16_SINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16G16_SFLOAT
                {VK_FORMAT_R16G16_SFLOAT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16G16B16A16_UNORM
                {VK_FORMAT_R16G16B16A16_UNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16G16B16A16_SNORM
                {VK_FORMAT_R16G16B16A16_SNORM,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16G16B16A16_UINT
                {VK_FORMAT_R16G16B16A16_UINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16G16B16A16_SINT
                {VK_FORMAT_R16G16B16A16_SINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R16G16B16A16_SFLOAT
                {VK_FORMAT_R16G16B16A16_SFLOAT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R32_UINT
                {VK_FORMAT_R32_UINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R32_SINT
                {VK_FORMAT_R32_SINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R32_SFLOAT
                {VK_FORMAT_R32_SFLOAT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R32G32_UINT
                {VK_FORMAT_R32G32_UINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R32G32_SINT
                {VK_FORMAT_R32G32_SINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R32G32_SFLOAT
                {VK_FORMAT_R32G32_SFLOAT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R32G32B32A32_UINT
                {VK_FORMAT_R32G32B32A32_UINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R32G32B32A32_SINT
                {VK_FORMAT_R32G32B32A32_SINT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_R32G32B32A32_SFLOAT
                {VK_FORMAT_R32G32B32A32_SFLOAT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT},

                // VK_FORMAT_B10G11R11_UFLOAT_PACK32
                {VK_FORMAT_B10G11R11_UFLOAT_PACK32,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT},

                // VK_FORMAT_E5B9G9R9_UFLOAT_PACK32
                {VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},

                // VK_FORMAT_D16_UNORM
                {VK_FORMAT_D16_UNORM, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},

                // VK_FORMAT_D32_SFLOAT
                {VK_FORMAT_D32_SFLOAT, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},

                // VK_FORMAT_D32_SFLOAT_S8_UINT
                {VK_FORMAT_D32_SFLOAT_S8_UINT, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},

                // VK_FORMAT_BC1_RGB_UNORM_BLOCK
                {VK_FORMAT_BC1_RGB_UNORM_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC1_RGB_SRGB_BLOCK
                {VK_FORMAT_BC1_RGB_SRGB_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC1_RGBA_UNORM_BLOCK
                {VK_FORMAT_BC1_RGBA_UNORM_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC1_RGBA_SRGB_BLOCK
                {VK_FORMAT_BC1_RGBA_SRGB_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC2_UNORM_BLOCK
                {VK_FORMAT_BC2_UNORM_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC2_SRGB_BLOCK
                {VK_FORMAT_BC2_SRGB_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC3_UNORM_BLOCK
                {VK_FORMAT_BC3_UNORM_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC3_SRGB_BLOCK
                {VK_FORMAT_BC3_SRGB_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC4_UNORM_BLOCK
                {VK_FORMAT_BC4_UNORM_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC4_SNORM_BLOCK
                {VK_FORMAT_BC4_SNORM_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC5_UNORM_BLOCK
                {VK_FORMAT_BC5_UNORM_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC5_SNORM_BLOCK
                {VK_FORMAT_BC5_SNORM_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC6H_UFLOAT_BLOCK
                {VK_FORMAT_BC6H_UFLOAT_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC6H_SFLOAT_BLOCK
                {VK_FORMAT_BC6H_SFLOAT_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC7_UNORM_BLOCK
                {VK_FORMAT_BC7_UNORM_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0},
                // VK_FORMAT_BC7_SRGB_BLOCK
                {VK_FORMAT_BC7_SRGB_BLOCK, 0,
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
                     VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                 0}};

            for (std::size_t i = 0, n = countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_FORMATS); i < n && supported; ++i) {
                const VpFormatProperties &requiredProps = VP_KHR_1_1_DESKTOP_PORTABILITY_2022_FORMATS[i];

                VkFormatProperties2 deviceProps = {};
                deviceProps.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
                vkGetPhysicalDeviceFormatProperties2(physicalDevice, requiredProps.format, &deviceProps);

                if (!vpCheckFormatProperty(&deviceProps, requiredProps)) {
                    supported = VK_FALSE;
                    break;
                }
            }

            static const VkMemoryPropertyFlags VP_KHR_1_1_DESKTOP_PORTABILITY_2022_MEMORY_TYPES[] = {
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
            for (uint32_t i = 0, n = countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_MEMORY_TYPES); i < n && supported; ++i) {
                const VkMemoryPropertyFlags memoryPropertyFlags = VP_KHR_1_1_DESKTOP_PORTABILITY_2022_MEMORY_TYPES[i];

                if (!vpCheckMemoryProperty(memoryProperties, VP_KHR_1_1_DESKTOP_PORTABILITY_2022_MEMORY_TYPES[i])) {
                    supported = VK_FALSE;
                    break;
                }
            }

            static const VkQueueFamilyProperties VP_KHR_1_1_DESKTOP_PORTABILITY_2022_QUEUE_FAMILY_PROPERTIES[] = {
                {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT,
                 1,
                 36,
                 {1, 1, 1}}};

            std::uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamily(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamily.data());
            for (uint32_t i = 0, n = countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_QUEUE_FAMILY_PROPERTIES); i < n && supported;
                 ++i) {
                if (!vpCheckQueueFamilyProperty(&queueFamily[0], queueFamilyCount,
                                                VP_KHR_1_1_DESKTOP_PORTABILITY_2022_QUEUE_FAMILY_PROPERTIES[i])) {
                    supported = VK_FALSE;
                    break;
                }
            }

            void *pNext = nullptr;

#if defined(__APPLE__)
            VkPhysicalDevicePortabilitySubsetFeaturesKHR devicePortabilitySubset = {};
            devicePortabilitySubset.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR;
            devicePortabilitySubset.pNext = pNext;
            pNext = &devicePortabilitySubset;
#endif

            VkPhysicalDeviceSamplerYcbcrConversionFeatures deviceSamplerYcbcrConversionFeatures = {};
            deviceSamplerYcbcrConversionFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
            deviceSamplerYcbcrConversionFeatures.pNext = pNext;

            VkPhysicalDeviceShaderFloat16Int8Features deviceShaderFloat16Int8Features = {};
            deviceShaderFloat16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
            deviceShaderFloat16Int8Features.pNext = &deviceSamplerYcbcrConversionFeatures;

            VkPhysicalDevice8BitStorageFeatures device8BitStorageFeatures = {};
            device8BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES;
            device8BitStorageFeatures.pNext = &deviceShaderFloat16Int8Features;

            VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParametersFeatures = {};
            shaderDrawParametersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
            shaderDrawParametersFeatures.pNext = &device8BitStorageFeatures;

            VkPhysicalDeviceImagelessFramebufferFeatures imagelessFramebufferFeatures = {};
            imagelessFramebufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
            imagelessFramebufferFeatures.pNext = &shaderDrawParametersFeatures;

            VkPhysicalDevice16BitStorageFeatures storage16bit = {};
            storage16bit.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
            storage16bit.pNext = &imagelessFramebufferFeatures;

            VkPhysicalDeviceMultiviewFeatures multiviewFeatures = {};
            multiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
            multiviewFeatures.pNext = &storage16bit;

            VkPhysicalDevice8BitStorageFeatures storage8bit = {};
            storage8bit.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES;
            storage8bit.pNext = &multiviewFeatures;

            VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {};
            descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
            descriptorIndexingFeatures.pNext = &storage8bit;

            VkPhysicalDeviceHostQueryResetFeatures queryResetFeatures = {};
            queryResetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
            queryResetFeatures.pNext = &descriptorIndexingFeatures;

            VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures subgroupExtendedTypesFeatures = {};
            subgroupExtendedTypesFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES;
            subgroupExtendedTypesFeatures.pNext = &queryResetFeatures;

            VkPhysicalDeviceUniformBufferStandardLayoutFeatures uniformBufferStandardLayoutFeatures = {};
            uniformBufferStandardLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES;
            uniformBufferStandardLayoutFeatures.pNext = &subgroupExtendedTypesFeatures;

            VkPhysicalDeviceFeatures2 deviceFeatures = {};
            deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            deviceFeatures.pNext = &uniformBufferStandardLayoutFeatures;

            vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

            if (deviceFeatures.features.robustBufferAccess != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.fullDrawIndexUint32 != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.imageCubeArray != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.independentBlend != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.inheritedQueries != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.tessellationShader != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.sampleRateShading != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.multiDrawIndirect != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.drawIndirectFirstInstance != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.depthClamp != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.depthBiasClamp != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.dualSrcBlend != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.fillModeNonSolid != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.largePoints != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.multiViewport != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.samplerAnisotropy != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.sampleRateShading != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.textureCompressionBC != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.occlusionQueryPrecise != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.vertexPipelineStoresAndAtomics != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.fragmentStoresAndAtomics != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderTessellationAndGeometryPointSize != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderImageGatherExtended != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderSampledImageArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderStorageBufferArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderStorageImageArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderStorageImageExtendedFormats != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderStorageImageWriteWithoutFormat != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderUniformBufferArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.shaderClipDistance != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceFeatures.features.tessellationShader != VK_TRUE) {
                supported = VK_FALSE;
            }

            if (imagelessFramebufferFeatures.imagelessFramebuffer != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (storage16bit.storageBuffer16BitAccess != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (storage16bit.uniformAndStorageBuffer16BitAccess != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (multiviewFeatures.multiview != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (storage8bit.storageBuffer8BitAccess != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (storage8bit.uniformAndStorageBuffer8BitAccess != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.shaderUniformTexelBufferArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.shaderStorageTexelBufferArrayDynamicIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.shaderUniformTexelBufferArrayNonUniformIndexing != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.descriptorBindingPartiallyBound != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingFeatures.runtimeDescriptorArray != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (queryResetFeatures.hostQueryReset != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (subgroupExtendedTypesFeatures.shaderSubgroupExtendedTypes != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (uniformBufferStandardLayoutFeatures.uniformBufferStandardLayout != VK_TRUE) {
                supported = VK_FALSE;
            }

            if (shaderDrawParametersFeatures.shaderDrawParameters != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (device8BitStorageFeatures.storageBuffer8BitAccess != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (device8BitStorageFeatures.storagePushConstant8 != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (device8BitStorageFeatures.uniformAndStorageBuffer8BitAccess != VK_TRUE) {
                supported = VK_FALSE;
            }

#if defined(__APPLE__)
            if (devicePortabilitySubset.vertexAttributeAccessBeyondStride != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePortabilitySubset.separateStencilMaskRef != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePortabilitySubset.mutableComparisonSamplers != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePortabilitySubset.multisampleArrayImage != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePortabilitySubset.imageViewFormatSwizzle != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePortabilitySubset.imageViewFormatReinterpretation != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePortabilitySubset.events != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (devicePortabilitySubset.constantAlphaColorBlendFactors != VK_TRUE) {
                supported = VK_FALSE;
            }
#endif

            pNext = nullptr;

#if defined(__APPLE__)
            VkPhysicalDevicePortabilitySubsetPropertiesKHR devicePortabilitySubsetProperties = {};
            devicePortabilitySubsetProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR;
            devicePortabilitySubsetProperties.pNext = pNext;
            pNext = &devicePortabilitySubsetProperties;
#endif

            VkPhysicalDeviceMaintenance3Properties deviceMaintenance3Properties = {};
            deviceMaintenance3Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
            deviceMaintenance3Properties.pNext = pNext;

            VkPhysicalDeviceDepthStencilResolveProperties deviceDepthStencilResolveProperties = {};
            deviceDepthStencilResolveProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;
            deviceDepthStencilResolveProperties.pNext = &deviceMaintenance3Properties;

            VkPhysicalDeviceInlineUniformBlockPropertiesEXT inlineUniformBlockProperties = {};
            inlineUniformBlockProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES_EXT;
            inlineUniformBlockProperties.pNext = &deviceDepthStencilResolveProperties;

            VkPhysicalDeviceMultiviewProperties multiviewProperties = {};
            multiviewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
            multiviewProperties.pNext = &inlineUniformBlockProperties;

            VkPhysicalDeviceDescriptorIndexingProperties descriptorIndexingProperties = {};
            descriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
            descriptorIndexingProperties.pNext = &multiviewProperties;

            VkPhysicalDeviceProperties2 deviceProperties{};
            deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            deviceProperties.pNext = &descriptorIndexingProperties;

            vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);

            if (deviceProperties.properties.limits.maxImageDimension1D < 16384) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxImageDimension2D < 16384) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxImageDimension3D < 2048) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxImageDimensionCube < 16384) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxImageArrayLayers < 2048) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTexelBufferElements < 67108900) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxUniformBufferRange < 65536) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxStorageBufferRange < 134217728) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPushConstantsSize < 128) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxMemoryAllocationCount < 4096) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxSamplerAllocationCount < 1024) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.bufferImageGranularity > 1024) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.sparseAddressSpaceSize < 1073741824) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxBoundDescriptorSets < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageDescriptorSamplers < 16) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageDescriptorUniformBuffers < 15) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageDescriptorStorageBuffers < 16) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageDescriptorSampledImages < 128) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageDescriptorStorageImages < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageDescriptorInputAttachments < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxPerStageResources < 128) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetSamplers < 80) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetUniformBuffers < 90) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetUniformBuffersDynamic < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetStorageBuffers < 155) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetStorageBuffersDynamic < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetSampledImages < 256) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetStorageImages < 40) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDescriptorSetInputAttachments < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxVertexInputAttributes < 28) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxVertexInputBindings < 28) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxVertexInputAttributeOffset < 2047) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxVertexInputBindingStride < 2048) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxVertexOutputComponents < 124) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTessellationGenerationLevel < 64) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTessellationPatchSize < 32) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTessellationControlPerVertexInputComponents < 124) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTessellationControlPerVertexOutputComponents < 124) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTessellationControlPerPatchOutputComponents < 120) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTessellationControlTotalOutputComponents < 2048) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTessellationEvaluationInputComponents < 124) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTessellationEvaluationOutputComponents < 124) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxFragmentInputComponents < 116) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxFragmentOutputAttachments < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxFragmentDualSrcAttachments < 1) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxFragmentCombinedOutputResources < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxComputeSharedMemorySize < 32768) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxComputeWorkGroupCount[0] < 65535 ||
                deviceProperties.properties.limits.maxComputeWorkGroupCount[1] < 65535 ||
                deviceProperties.properties.limits.maxComputeWorkGroupCount[2] < 65535) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxComputeWorkGroupInvocations < 1024) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxComputeWorkGroupSize[0] < 1024 ||
                deviceProperties.properties.limits.maxComputeWorkGroupSize[1] < 1024 ||
                deviceProperties.properties.limits.maxComputeWorkGroupSize[2] < 64) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.subPixelPrecisionBits < 4) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.subTexelPrecisionBits < 4) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.mipmapPrecisionBits < 4) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDrawIndexedIndexValue < 4294967295u) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxDrawIndirectCount < 1073740000) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxSamplerLodBias < 14) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxSamplerAnisotropy < 16) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxViewports < 16) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxViewportDimensions[0] < 16384 ||
                deviceProperties.properties.limits.maxViewportDimensions[1] < 16384) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.viewportBoundsRange[0] > -32768 ||
                deviceProperties.properties.limits.viewportBoundsRange[1] < 32767) {
                supported = VK_FALSE;
            }

            if (deviceProperties.properties.limits.minMemoryMapAlignment < 64) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.minTexelBufferOffsetAlignment > 64) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.minUniformBufferOffsetAlignment > 256) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.minStorageBufferOffsetAlignment > 64) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.minTexelOffset > -8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTexelOffset < 7) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.minTexelGatherOffset > -8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxTexelGatherOffset < 7) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.minInterpolationOffset > -0.5) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxInterpolationOffset < 0.4375) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.subPixelInterpolationOffsetBits < 4) {
                supported = VK_FALSE;
            }

            if (deviceProperties.properties.limits.maxFramebufferWidth < 16384) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxFramebufferHeight < 16384) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxFramebufferLayers < 1024) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.framebufferColorSampleCounts < 9) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.framebufferDepthSampleCounts < 9) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.framebufferStencilSampleCounts < 9) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.framebufferNoAttachmentsSampleCounts < 9) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxColorAttachments < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.sampledImageColorSampleCounts < 9) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.sampledImageIntegerSampleCounts < 9) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.sampledImageDepthSampleCounts < 9) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.sampledImageStencilSampleCounts < 9) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.storageImageSampleCounts < 1) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxSampleMaskWords > 1) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxClipDistances < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxCullDistances < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.maxCombinedClipAndCullDistances < 8) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.discreteQueuePriorities < 2) {
                supported = VK_FALSE;
            }

            if (deviceProperties.properties.limits.pointSizeRange[0] > 1.0 ||
                deviceProperties.properties.limits.pointSizeRange[1] < 64.0) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.lineWidthRange[0] > 1.0 ||
                deviceProperties.properties.limits.lineWidthRange[1] < 1.0) {
                supported = VK_FALSE;
            }

            if (deviceProperties.properties.limits.pointSizeGranularity > 0.125) {
                supported = VK_FALSE;
            }
            if (deviceProperties.properties.limits.lineWidthGranularity > 0.5) {
                supported = VK_FALSE;
            }

            if (descriptorIndexingProperties.maxUpdateAfterBindDescriptorsInAllPools < 1048576) {
                supported = VK_FALSE;
            }

            if (descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindSamplers < 16) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindUniformBuffers < 15) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindStorageBuffers < 31) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindSampledImages < 128) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindStorageImages < 8) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindInputAttachments < 128) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxPerStageUpdateAfterBindResources < 159) {
                supported = VK_FALSE;
            }

            if (descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSamplers < 80) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindUniformBuffers < 90) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic < 8) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindStorageBuffers < 155) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic < 8) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSampledImages < 640) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindStorageImages < 40) {
                supported = VK_FALSE;
            }
            if (descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindInputAttachments < 640) {
                supported = VK_FALSE;
            }

            if (multiviewProperties.maxMultiviewInstanceIndex < 134217727) {
                supported = VK_FALSE;
            }
            if (multiviewProperties.maxMultiviewViewCount < 6) {
                supported = VK_FALSE;
            }

            if (inlineUniformBlockProperties.maxInlineUniformBlockSize < 256) {
                supported = VK_FALSE;
            }

            if (deviceDepthStencilResolveProperties.independentResolve != VK_TRUE) {
                supported = VK_FALSE;
            }
            if (deviceDepthStencilResolveProperties.independentResolveNone != VK_TRUE) {
                supported = VK_FALSE;
            }
            if ((deviceDepthStencilResolveProperties.supportedDepthResolveModes & VK_RESOLVE_MODE_SAMPLE_ZERO_BIT) !=
                VK_RESOLVE_MODE_SAMPLE_ZERO_BIT) {
                supported = VK_FALSE;
            }
            if ((deviceDepthStencilResolveProperties.supportedStencilResolveModes & VK_RESOLVE_MODE_SAMPLE_ZERO_BIT) !=
                VK_RESOLVE_MODE_SAMPLE_ZERO_BIT) {
                supported = VK_FALSE;
            }

            if (deviceMaintenance3Properties.maxPerSetDescriptors < 700) {
                supported = VK_FALSE;
            }
            if (deviceMaintenance3Properties.maxMemoryAllocationSize < 2147483648) {
                supported = VK_FALSE;
            }

#if defined(__APPLE__)
            if (devicePortabilitySubsetProperties.minVertexInputBindingStrideAlignment < 4) {
                supported = VK_FALSE;
            }
#endif
        }

        if (supported) {
            if (pProperties != nullptr && supportedProfileCount < *pPropertyCount) {
                pProperties[supportedProfileCount] = supportedProfiles[i];
            }
            supportedProfileCount++;
        }
    }

    if (pProperties == nullptr) {
        *pPropertyCount = supportedProfileCount;
    }

    return result;
}

inline void vpEnumerateProfileExtensionProperties(const VpProfileProperties *pProfile, uint32_t *pPropertyCount,
                                                  VkExtensionProperties *pProperties) {
    if (pProperties == nullptr) {
        if (strcmp(pProfile->profileName, VP_KHR_MINIMUM_REQUIREMENTS_NAME) == 0) {
            *pPropertyCount = 0;
        } else if (strcmp(pProfile->profileName, VP_KHR_1_2_ROADMAP_2022_NAME) == 0) {
            *pPropertyCount = countof(VP_KHR_1_2_ROADMAP_2022_EXTENSIONS);
        } else if (strcmp(pProfile->profileName, VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_NAME) == 0) {
            *pPropertyCount = countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_EXTENSIONS);
        }
        return;
    }

    if (strcmp(pProfile->profileName, VP_KHR_MINIMUM_REQUIREMENTS_NAME) == 0) {
    } else if (strcmp(pProfile->profileName, VP_KHR_1_2_ROADMAP_2022_NAME) == 0) {
        for (std::size_t i = 0, n = std::min<std::size_t>(countof(VP_KHR_1_2_ROADMAP_2022_EXTENSIONS), *pPropertyCount); i < n;
             ++i) {
            pProperties[i] = VP_KHR_1_2_ROADMAP_2022_EXTENSIONS[i];
        }
    } else if (strcmp(pProfile->profileName, VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_NAME) == 0) {
        for (std::size_t i = 0, n = std::min<std::size_t>(countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_EXTENSIONS), *pPropertyCount);
             i < n; ++i) {
            pProperties[i] = VP_KHR_1_1_DESKTOP_PORTABILITY_2022_EXTENSIONS[i];
        }
    }
}

inline void vpEnumerateProfileStructureTypes(const VpProfileProperties *pProfile, VpStructureArea structureArea,
                                             uint32_t *pStructureTypesCount, VkStructureType *pStructureTypes) {
    if (pStructureTypes == nullptr) {
        if (strcmp(pProfile->profileName, VP_KHR_MINIMUM_REQUIREMENTS_NAME) == 0) {
            *pStructureTypesCount = 0;
        } else if (strcmp(pProfile->profileName, VP_KHR_1_2_ROADMAP_2022_NAME) == 0) {
            switch (structureArea) {
                case VP_STRUCTURE_FEATURES:
                    *pStructureTypesCount = countof(VP_KHR_1_2_ROADMAP_2022_FEATURE_STRUCTURE_TYPES);
                    break;
                case VP_STRUCTURE_PROPERTIES:
                    *pStructureTypesCount = countof(VP_KHR_1_2_ROADMAP_2022_PROPERTY_STRUCTURE_TYPES);
                    break;
            }
        } else if (strcmp(pProfile->profileName, VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_NAME) == 0) {
            switch (structureArea) {
                case VP_STRUCTURE_FEATURES:
                    *pStructureTypesCount = countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_FEATURE_STRUCTURE_TYPES);
                    break;
                case VP_STRUCTURE_PROPERTIES:
                    *pStructureTypesCount = countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_PROPERTY_STRUCTURE_TYPES);
                    break;
            }
        }
        return;
    }

    if (strcmp(pProfile->profileName, VP_KHR_MINIMUM_REQUIREMENTS_NAME) == 0) {
    } else if (strcmp(pProfile->profileName, VP_KHR_1_2_ROADMAP_2022_NAME) == 0) {
        switch (structureArea) {
            case VP_STRUCTURE_FEATURES: {
                std::size_t n =
                    std::min<std::size_t>(countof(VP_KHR_1_2_ROADMAP_2022_FEATURE_STRUCTURE_TYPES), *pStructureTypesCount);
                for (std::size_t i = 0; i < n; ++i) {
                    pStructureTypes[i] = VP_KHR_1_2_ROADMAP_2022_FEATURE_STRUCTURE_TYPES[i];
                }
            } break;
            case VP_STRUCTURE_PROPERTIES: {
                std::size_t n =
                    std::min<std::size_t>(countof(VP_KHR_1_2_ROADMAP_2022_PROPERTY_STRUCTURE_TYPES), *pStructureTypesCount);
                for (std::size_t i = 0; i < n; ++i) {
                    pStructureTypes[i] = VP_KHR_1_2_ROADMAP_2022_PROPERTY_STRUCTURE_TYPES[i];
                }
            } break;
        }
    } else if (strcmp(pProfile->profileName, VP_LUNARG_1_1_DESKTOP_PORTABILITY_2022_NAME) == 0) {
        switch (structureArea) {
            case VP_STRUCTURE_FEATURES: {
                std::size_t n = std::min<std::size_t>(countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_FEATURE_STRUCTURE_TYPES),
                                                      *pStructureTypesCount);
                for (std::size_t i = 0; i < n; ++i) {
                    pStructureTypes[i] = VP_KHR_1_2_ROADMAP_2022_FEATURE_STRUCTURE_TYPES[i];
                }
            } break;
            case VP_STRUCTURE_PROPERTIES: {
                std::size_t n = std::min<std::size_t>(countof(VP_KHR_1_1_DESKTOP_PORTABILITY_2022_PROPERTY_STRUCTURE_TYPES),
                                                      *pStructureTypesCount);
                for (std::size_t i = 0; i < n; ++i) {
                    pStructureTypes[i] = VP_KHR_1_1_DESKTOP_PORTABILITY_2022_PROPERTY_STRUCTURE_TYPES[i];
                }
            } break;
        }
    }
}
