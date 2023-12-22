#pragma once

#include "VulkanDevice.h"


// std lib headers
#include <vector>
#include <memory>


namespace Lotus
{
    class VulkanSwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        VulkanSwapChain(VulkanDevice& deviceRef, VkExtent2D windowExtent);
        VulkanSwapChain(VulkanDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<VulkanSwapChain> previous);
        ~VulkanSwapChain();

        VulkanSwapChain(const VulkanSwapChain&) = delete;
        VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

        VkFramebuffer GetFrameBuffer(int index) const { return m_SwapChainFramebuffers[index]; }
        VkRenderPass GetRenderPass() const { return m_RenderPass; }
        VkImageView GetImageView(int index) const { return m_SwapChainImageViews[index]; }
        size_t ImageCount() const { return m_SwapChainImages.size(); }
        VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
        VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
        uint32_t Width() const { return m_SwapChainExtent.width; }
        uint32_t Height() const { return m_SwapChainExtent.height; }

        float ExtentAspectRatio() const
        {
            return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height);
        }

        VkFormat FindDepthFormat();

        VkResult AcquireNextImage(uint32_t* imageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

        bool CompareSwapFormats(const VulkanSwapChain& swapChain) const
        {
            return swapChain.m_SwapChainDepthFormat == m_SwapChainDepthFormat &&
                swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat;
        }

    private:
        void Init();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkFormat m_SwapChainImageFormat;
        VkFormat m_SwapChainDepthFormat;
        VkExtent2D m_SwapChainExtent;

        std::vector<VkFramebuffer> m_SwapChainFramebuffers;
        VkRenderPass m_RenderPass;

        std::vector<VkImage> m_DepthImages;
        std::vector<VkDeviceMemory> m_DepthImageMemorys;
        std::vector<VkImageView> m_DepthImageViews;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;

        VulkanDevice& m_Device;
        VkExtent2D m_WindowExtent;

        VkSwapchainKHR m_SwapChain;
        std::shared_ptr<VulkanSwapChain> m_OldSwapChain;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;
        size_t m_CurrentFrame = 0;
    };
}
