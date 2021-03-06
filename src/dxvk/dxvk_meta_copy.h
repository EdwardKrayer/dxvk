#pragma once

#include <mutex>
#include <unordered_map>

#include "../spirv/spirv_code_buffer.h"

#include "dxvk_barrier.h"
#include "dxvk_cmdlist.h"
#include "dxvk_hash.h"
#include "dxvk_resource.h"

namespace dxvk {

  /**
   * \brief Copy pipeline
   * 
   * Stores the objects for a single pipeline
   * that is used for fragment shader copies.
   */
  struct DxvkMetaCopyPipeline {
    VkRenderPass          renderPass;
    VkDescriptorSetLayout dsetLayout;
    VkPipelineLayout      pipeLayout;
    VkPipeline            pipeHandle;
  };

  /**
   * \brief Copy pipeline key
   * 
   * Used to look up copy pipelines based
   * on the copy operation they support.
   */
  struct DxvkMetaCopyPipelineKey {
    VkImageViewType       viewType;
    VkFormat              format;
    VkSampleCountFlagBits samples;

    bool eq(const DxvkMetaCopyPipelineKey& other) const {
      return this->viewType == other.viewType
          && this->format   == other.format
          && this->samples  == other.samples;
    }

    size_t hash() const {
      return (uint32_t(format)  << 8)
           ^ (uint32_t(samples) << 4)
           ^ (uint32_t(viewType));
    }
  };

  /**
   * \brief Copy framebuffer and render pass
   * 
   * Creates a framebuffer and render
   * pass object for an image view.
   */
  class DxvkMetaCopyRenderPass : public DxvkResource {

  public:

    DxvkMetaCopyRenderPass(
      const Rc<vk::DeviceFn>&   vkd,
      const Rc<DxvkImageView>&  dstImageView,
      const Rc<DxvkImageView>&  srcImageView,
            bool                discardDst);
    
    ~DxvkMetaCopyRenderPass();

    VkRenderPass renderPass() const {
      return m_renderPass;
    }

    VkFramebuffer framebuffer() const {
      return m_framebuffer;
    }

  private:

    const Rc<vk::DeviceFn>  m_vkd;

    const Rc<DxvkImageView> m_dstImageView;
    const Rc<DxvkImageView> m_srcImageView;
    
    VkRenderPass  m_renderPass  = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

    VkRenderPass createRenderPass(bool discard) const;

    VkFramebuffer createFramebuffer() const;

  };

  /**
   * \brief Meta copy objects
   * 
   * Meta copy operations are necessary in order
   * to copy data between color and depth images.
   */
  class DxvkMetaCopyObjects : public RcObject {

  public:

    DxvkMetaCopyObjects(const Rc<vk::DeviceFn>& vkd);
    ~DxvkMetaCopyObjects();

    /**
     * \brief Queries color format for d->c copies
     * 
     * Returns the color format that we need to use
     * as the destination image view format in case
     * of depth to color image copies.
     * \param [in] format Depth format
     * \returns Corresponding color format
     */
    VkFormat getCopyDestinationFormat(
            VkImageAspectFlags    dstAspect,
            VkImageAspectFlags    srcAspect,
            VkFormat              srcFormat) const;

    /**
     * \brief Creates pipeline for meta copy operation
     * 
     * \param [in] viewType Image view type
     * \param [in] dstFormat Destination image format
     * \param [in] dstSamples Destination sample count
     * \returns Compatible pipeline for the operation
     */
    DxvkMetaCopyPipeline getPipeline(
            VkImageViewType       viewType,
            VkFormat              dstFormat,
            VkSampleCountFlagBits dstSamples);

  private:

    struct FragShaders {
      VkShaderModule frag1D;
      VkShaderModule frag2D;
      VkShaderModule fragMs;
    };

    Rc<vk::DeviceFn> m_vkd;

    VkSampler m_sampler;

    VkShaderModule m_shaderVert;
    VkShaderModule m_shaderGeom;

    FragShaders m_color;
    FragShaders m_depth;

    std::mutex m_mutex;

    std::unordered_map<
      DxvkMetaCopyPipelineKey,
      DxvkMetaCopyPipeline,
      DxvkHash, DxvkEq> m_pipelines;
    
    VkSampler createSampler() const;
    
    VkShaderModule createShaderModule(
      const SpirvCodeBuffer&          code) const;
    
    DxvkMetaCopyPipeline createPipeline(
      const DxvkMetaCopyPipelineKey&  key);

    VkRenderPass createRenderPass(
      const DxvkMetaCopyPipelineKey&  key) const;
    
    VkDescriptorSetLayout createDescriptorSetLayout() const;
    
    VkPipelineLayout createPipelineLayout(
            VkDescriptorSetLayout     descriptorSetLayout) const;
    
    VkPipeline createPipelineObject(
      const DxvkMetaCopyPipelineKey&  key,
            VkPipelineLayout          pipelineLayout,
            VkRenderPass              renderPass);
    
  };
  
}