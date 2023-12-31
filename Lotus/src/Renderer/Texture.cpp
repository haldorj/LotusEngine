#include "lotuspch.h"
#include "Texture.h"

#include <stb_image.h>

namespace Lotus {

    Texture::Texture(std::string filePath, Device& device)
		: m_Device(device)
    {
        CreateTextureImage(filePath, m_Device);
        CreateTextureImageView(m_Device);
        CreateTextureSampler(m_Device);
    }

    Texture::~Texture()
    {
        vkDestroySampler(m_Device.GetDevice(), m_TextureSampler, nullptr);
        vkDestroyImageView(m_Device.GetDevice(), m_TextureImageView, nullptr);
        vkDestroyImage(m_Device.GetDevice(), m_TextureImage, nullptr);
        vkFreeMemory(m_Device.GetDevice(), m_TextureImageMemory, nullptr);
    }

    void Texture::CreateTextureImage(std::string filePath, Device& device)
	{
        // Loading an image /////////////////////////////
        int texWidth, texHeight, texChannels;
        // Forces the image to be loaded with an alpha channel, even if it doesn't have one
        stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        // 4 bytes per pixel
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        // Creating a staging buffer /////////////////////////////

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        device.CreateBuffer(
            imageSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffer, 
            stagingBufferMemory
        );

        void* data;
        vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device.GetDevice(), stagingBufferMemory);

        stbi_image_free(pixels);

        // Creating the image /////////////////////////////

        CreateImage(
            device,
            texWidth,
            texHeight,
            VK_FORMAT_R8G8B8A8_SRGB, 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_TextureImage,
            m_TextureImageMemory
        );

        // Copying buffer to image /////////////////////////////

        device.TransitionImageLayout(
            m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, 
            VK_IMAGE_LAYOUT_UNDEFINED, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_LayerCount
        );

        device.CopyBufferToImage(
            stagingBuffer, 
            m_TextureImage, 
            static_cast<uint32_t>(texWidth), 
            static_cast<uint32_t>(texHeight), 
            m_LayerCount
        );

        device.TransitionImageLayout(
            m_TextureImage,
            VK_FORMAT_R8G8B8A8_SRGB, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            m_LayerCount
        );

        vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);
	}

    void Texture::CreateImage(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D; // 2D image
		imageInfo.extent.width = width; // width of image extent
		imageInfo.extent.height = height; // height of image extent
		imageInfo.extent.depth = 1; // 1 because 2D image
		imageInfo.mipLevels = 1; // no mip mapping
		imageInfo.arrayLayers = 1; // number of layers in image array
		imageInfo.format = format; // format of image data
		imageInfo.tiling = tiling; // optimal for GPU access
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // don't care about initial layout
		imageInfo.usage = usage; // transfer to it and sample from it
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only used by one queue family
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling
		imageInfo.flags = 0; // no flags

        if (vkCreateImage(device.GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.GetDevice(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size; // size of memory allocation
		allocInfo.memoryTypeIndex = device.FindMemoryType(memRequirements.memoryTypeBits, properties); // index of memory type on physical device that has required bit flags

        if (vkAllocateMemory(device.GetDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device.GetDevice(), image, imageMemory, 0);
    }

    void Texture::CreateTextureImageView(Device& device)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_TextureImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.GetDevice(), &viewInfo, nullptr, &m_TextureImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void Texture::CreateTextureSampler(Device& device)
    {
        VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR; // how to render when image is magnified on screen
		samplerInfo.minFilter = VK_FILTER_LINEAR; // how to render when image is minified on screen
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // how to handle texture coordinates outside of [0, 1] range in U direction
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // how to handle texture coordinates outside of [0, 1] range in V direction
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; // how to handle texture coordinates outside of [0, 1] range in W direction
		samplerInfo.anisotropyEnable = VK_TRUE; // enable anisotropy

        VkPhysicalDeviceProperties properties{}; // physical device properties
        vkGetPhysicalDeviceProperties(m_Device.GetPhysicalDevice(), &properties); // get physical device properties

		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // anisotropy value
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // border beyond texture
		samplerInfo.unnormalizedCoordinates = VK_FALSE; // whether coords should be normalized between 0 and 1
		samplerInfo.compareEnable = VK_FALSE; // whether to compare texels to a value
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // type of comparison

		// mipmapping
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // how to handle mipmapping
		samplerInfo.minLod = 0.0f; // lower limit of mip level to use
		samplerInfo.maxLod = 0.0f; // upper limit of mip level to use
		samplerInfo.mipLodBias = 0.0f; // bias for mip level

        if (vkCreateSampler(device.GetDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
    }

}