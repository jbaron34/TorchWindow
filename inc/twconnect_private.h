#pragma once

#ifndef TW_CONNECT_PRIVATE_H
#define TW_CONNECT_PRIVATE_H

#include "twcommon.h"
#include "twvulkan.h"
#include "twcuda.h"

#include <windows.h>
#include <dxgi1_2.h>
#include <aclapi.h>
#include <vulkan/vulkan_win32.h>


typedef struct TwShareableParentDetails_T *TwShareableParentDetails;

typedef struct TwShareableChildDetails_T *TwShareableChildDetails;


struct TwShareableParentDetails_T{
    HANDLE parentHandle;
    uint16_t winWidth;
    uint16_t winHeight;
    uint16_t texWidth;
    uint16_t texHeight;
};

struct TwShareableChildDetails_T{
    HANDLE textureMemory;
    size_t memSize;
    HANDLE textureUpdated;
    HANDLE updateConsumed;
};

void removeSpacesAndQuotes(char * string);

void convertParentDetailsToShareable(TwShareableParentDetails shareableParentDetails, TwParentDetails parentDetails);
void getChildDetailsFromShareable(TwReceivedChildDetails childDetails, TwShareableChildDetails shareableChildDetails);
void convertChildDetailsToShareable(VkDevice device, HANDLE parentHandle, TwShareableChildDetails shareableChildDetails, TwChildDetails childDetails);
void getParentDetailsFromShareable(TwReceivedParentDetails parentDetails, TwShareableParentDetails shareableParentDetails);

void twCreateTextureImage(VkDevice device, uint32_t width, uint32_t height, VkImage *image);
void twCreateTextureMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkImage textureImage, VkDeviceMemory *memory, size_t *size);
void twCreateTextureSemaphores(VkDevice device, VkSemaphore *textureUpdated, VkSemaphore *updateConsumed);

void getWinSecurityAttributes(SECURITY_ATTRIBUTES *sa);


#endif /* TW_CONNECT_PRIVATE_H */