#pragma once

#include "device.hpp"

// std
#include <memory>
#include <string>

namespace mnlt
{
    class Texture
    {
        public:
            Texture(Device &device, const std::vector<std::string> &textureFilepaths);
            Texture(Device &device, VkFormat format, VkExtent3D extent, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount);
            ~Texture();

            Texture(const Texture &) = delete;
            Texture &operator=(const Texture &) = delete;
            Texture(Texture &&) = delete;
            Texture &operator=(Texture &&) = delete;

            VkImageView imageView() const { return mTextureImageView; }
            VkSampler sampler() const { return mTextureSampler; }
            VkImage getImage() const { return mTextureImage; }
            VkImageView getImageView() const { return mTextureImageView; }
            VkDescriptorImageInfo getImageInfo() const { return mDescriptor; }
            VkImageLayout getImageLayout() const { return mTextureLayout; }
            VkExtent3D getExtent() const { return mExtent; }
            VkFormat getFormat() const { return mFormat; }

            void updateDescriptor();
            void transitionLayout(
                VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

            static std::unique_ptr<Texture> createTextureFromFile(
                Device &device, const std::vector<std::string> &filepaths);

        private:
            void createTextureImage(const std::vector<std::string> &filepaths);
            void createTextureImageView(VkImageViewType viewType);
            void createTextureSampler();

            VkDescriptorImageInfo mDescriptor{};

            Device &mDevice;
            VkImage mTextureImage = nullptr;
            VkDeviceMemory mTextureImageMemory = nullptr;
            VkImageView mTextureImageView = nullptr;
            VkSampler mTextureSampler = nullptr;
            VkFormat mFormat;
            VkImageLayout mTextureLayout;
            uint32_t mMipLevels{1};
            uint32_t mLayerCount{1};
            VkExtent3D mExtent{};
    };
}
