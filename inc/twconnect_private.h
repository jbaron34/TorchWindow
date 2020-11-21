#pragma once

#ifndef TW_CONNECT_PRIVATE_H
#define TW_CONNECT_PRIVATE_H

#include "twcommon.h"
#include "twvulkan.h"
#include "twcuda.h"

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#include <windows.h>
#include <dxgi1_2.h>
#include <aclapi.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#endif


typedef struct TwShareableParentDetails_T *TwShareableParentDetails;

typedef struct TwShareableChildDetails_T *TwShareableChildDetails;


struct TwShareableParentDetails_T{
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    HANDLE parentHandle;
#elif defined(__linux__)
    pid_t pIDParent;
#endif
    uint16_t winWidth;
    uint16_t winHeight;
    uint16_t texWidth;
    uint16_t texHeight;
};

struct TwShareableChildDetails_T{
    size_t memSize;
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    HANDLE textureMemory;
    HANDLE textureUpdated;
    HANDLE updateConsumed;
#elif defined(__linux__)
    int fdTextureMemory;
    int fdTextureUpdated;
    int fdUpdateConsumed;
#endif
};

void removeSpacesAndQuotes(char * string);

void convertParentDetailsToShareable(TwShareableParentDetails shareableParentDetails, TwParentDetails parentDetails);

void getChildDetailsFromShareable(TwReceivedChildDetails childDetails, TwShareableChildDetails shareableChildDetails);

void convertChildDetailsToShareable(
    VkDevice device,
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    HANDLE parentHandle,
#elif defined(__linux__)
    pid_t pIDParent,
#endif
    TwShareableChildDetails shareableChildDetails,
    TwChildDetails childDetails
);

void getParentDetailsFromShareable(TwReceivedParentDetails parentDetails, TwShareableParentDetails shareableParentDetails);

void twCreateTextureImage(VkDevice device, uint32_t width, uint32_t height, VkImage *image);
void twCreateTextureMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkImage textureImage, VkDeviceMemory *memory, size_t *size);
void twCreateTextureSemaphores(VkDevice device, VkSemaphore *textureUpdated, VkSemaphore *updateConsumed);

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
void getWinSecurityAttributes(SECURITY_ATTRIBUTES *sa);
#elif defined(__linux__)
void sendFds(int fdSocket, TwShareableChildDetails details);
void recvFds(int fdSocket, TwShareableChildDetails details);
#endif

#endif /* TW_CONNECT_PRIVATE_H */