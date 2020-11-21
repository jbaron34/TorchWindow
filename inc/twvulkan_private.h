#pragma once

#ifndef TW_VULKAN_PRIVATE_H
#define TW_VULKAN_PRIVATE_H


//#include <sys\timeb.h
#include <time.h>

#include <twcommon.h>
#include <twvulkan.h>
#include <SDL.h>
#include <SDL_vulkan.h>

#define TW_NUM_REQUIRED_VK_INSTANCE_EXTENSIONS 0
#define TW_NUM_REQUIRED_VK_DEVICE_EXTENSIONS 3
#define MAX_FRAMES_IN_FLIGHT 2

struct SwapChainSupportDetails_T {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    unsigned int formatCount;
    VkPresentModeKHR *presentModes;
    unsigned int presentModeCount;
};

struct TwWindow_T
{
    const char *name;
    struct TwChildDetails_T childDetails;
    struct TwReceivedParentDetails_T parentDetails;

    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    unsigned int graphicsQFIndex;
    unsigned int presentQFIndex;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkImage textureImage;
    VkImageView textureImageView;
    SDL_Window *window;
    VkSurfaceKHR surface;
    struct SwapChainSupportDetails_T details;
    VkSurfaceFormatKHR swapImageFormat;
    VkPresentModeKHR swapPresentMode;
    VkExtent2D swapExtent;
    VkSwapchainKHR swapChain;
    unsigned int swapImageCount;
    VkImage *swapImages;
    VkImageView *swapImageViews;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer *swapFrameBuffers;
    VkCommandPool commandPool;
    VkSampler textureSampler;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;
    VkCommandBuffer *commandBuffers;
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
    VkFence *imagesInFlight;

    unsigned int currentFrame;
};

char** catStringLists(const char **a, unsigned int nA, const char **b, unsigned int nB);

int readall(FILE *in, const char **dataptr, unsigned int *sizeptr);

int twCheckVkValidationLayerSupport(unsigned int reqLayerCount, const char * const *reqLayerNames);

int twCheckVkInstanceExtensionSupport(const char * const *reqExtNames, unsigned int reqExtCount);

void twGetRequiredVkInstanceExtensions(unsigned int *pExtentionCount, const char * const **pExtensionNames);

void twSetupVkInstance(TwWindow window);

int twIsVkPhysicalDeviceSuitable(VkPhysicalDevice device);

void twSetVkPhysicalDevice(TwWindow window);

int twSetQueueFamilyIndices(TwWindow window);

int twCheckVkDeviceExtensionSupport(VkPhysicalDevice device, unsigned int reqExtCount, const char * const *reqExtNames);

void twSetupVkLogicalDevice(TwWindow window);

void twCreateTextureImage(TwWindow window);

void twCreateSDLWindow(TwWindow window, const char *name);

void twCreateSurface(TwWindow window);

void twQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface, struct SwapChainSupportDetails_T *details);

void twChooseSwapSurfaceFormat(TwWindow window);

void twChooseSwapPresentMode(TwWindow window);

void twChooseSwapExtent(TwWindow window);

void twCreateSwapChain(TwWindow window);

void twCreateSwapImageViews(TwWindow window);

VkShaderModule twCreateShaderModule(VkDevice device, const char *code, unsigned int codeLength);

void twLoadShaders(TwWindow window);

void twCreateRenderPass(TwWindow window);

void twCreateDescriptorSetLayout(TwWindow window);

void twCreateGraphicsPipeline(TwWindow window);

void twCreateFrameBuffers(TwWindow window);

void twCreateCommandPool(TwWindow window);

void twCreateTextureSampler(TwWindow window);

int twPollWindowEvents(TwWindow);


#endif /* TW_VULKAN_PRIVATE_H */