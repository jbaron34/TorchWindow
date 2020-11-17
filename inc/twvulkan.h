#pragma once

#ifndef TW_VULKAN_H
#define TW_VULKAN_H


#include <vulkan/vulkan.h>

typedef struct TwChildDetails_T *TwChildDetails;

typedef struct TwReceivedParentDetails_T *TwReceivedParentDetails;

typedef struct TwWindow_T *TwWindow;

struct TwChildDetails_T{
    VkDeviceMemory textureMemory;
    size_t memSize;
    VkSemaphore textureUpdated;
    VkSemaphore updateConsumed;
};

struct TwReceivedParentDetails_T{
    uint16_t winWidth;
    uint16_t winHeight;
    uint16_t texWidth;
    uint16_t texHeight;
};


TwChildDetails twCreateChildDetails(int width, int height, size_t size);

TwWindow twCreateWindow(const char *name);

void twDestroyWindow(TwWindow);

void twRunWindow(TwWindow);

// Below functions are implemented by rtconnect

void twConnectWindowProcess(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkImage *textureImage,
    TwChildDetails childDetails,
    TwReceivedParentDetails parentDetails,
    const char *name
);

const char *twExternalSemaphoreDeviceExtName();
const char *twExternalMemoryDeviceExtName();

#endif /* TW_VULKAN_H */