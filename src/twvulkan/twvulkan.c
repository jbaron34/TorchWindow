#include <twvulkan_private.h>

int min(int a, int b){
    if(a < b){ return a; }
    else{ return b; }
}

int max(int a, int b){
    if(a > b){ return a; }
    else{ return b; }
}

char** catStringLists(const char **a, unsigned int nA, const char **b, unsigned int nB){
    if(nA + nB == 0){
        return NULL;
    }else if(nA == 0){
        return b;
    }else if(nB == 0){
        return a;
    }

    char **result = malloc(sizeof(*result) * (nA + nB));

    memcpy(result, a, sizeof(*result) * nA);
    memcpy(result + nA, b, sizeof(*result) * nB);

    return result;
}

/* Size of each input chunk to be
   read and allocate for. */
#ifndef  READALL_CHUNK
#define  READALL_CHUNK  262144
#endif

#define  READALL_OK          0  /* Success */
#define  READALL_INVALID    -1  /* Invalid parameters */
#define  READALL_ERROR      -2  /* Stream error */
#define  READALL_TOOMUCH    -3  /* Too much input */
#define  READALL_NOMEM      -4  /* Out of memory */

/* This function returns one of the READALL_ constants above.
   If the return value is zero == READALL_OK, then:
     (*dataptr) points to a dynamically allocated buffer, with
     (*sizeptr) chars read from the file.
     The buffer is allocated for one extra char, which is NUL,
     and automatically appended after the data.
   Initial values of (*dataptr) and (*sizeptr) are ignored.
*/
int readall(FILE *in, const char **dataptr, unsigned int *sizeptr)
{
    char  *data = NULL, *temp;
    unsigned int size = 0;
    unsigned int used = 0;
    unsigned int n;

    /* None of the parameters can be NULL. */
    if (in == NULL || dataptr == NULL || sizeptr == NULL)
        return READALL_INVALID;

    /* A read error already occurred? */
    if (ferror(in))
        return READALL_ERROR;

    while (1) {

        if (used + READALL_CHUNK + 1 > size) {
            size = used + READALL_CHUNK + 1;

            /* Overflow check. Some ANSI C compilers
               may optimize this away, though. */
            if (size <= used) {
                free(data);
                return READALL_TOOMUCH;
            }

            temp = realloc(data, size);
            if (temp == NULL) {
                free(data);
                return READALL_NOMEM;
            }
            data = temp;
        }

        n = fread(data + used, 1, READALL_CHUNK, in);
        if (n == 0)
            break;

        used += n;
    }

    if (ferror(in)) {
        free(data);
        return READALL_ERROR;
    }

    temp = realloc(data, used + 1);
    if (temp == NULL) {
        free(data);
        return READALL_NOMEM;
    }
    data = temp;
    data[used] = '\0';

    *dataptr = data;
    *sizeptr = used;

    return READALL_OK;
}

int twCheckVkValidationLayerSupport(
    unsigned int reqLayerCount,
    const char * const *reqLayerNames
)
{
    uint32_t availableLayerCount;
    VkLayerProperties *availableLayers;
    int i;

    VkResult vkResult = vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);
    availableLayers = malloc(sizeof(*availableLayers) * availableLayerCount);
    vkResult = vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

    for (i = 0; i<reqLayerCount; i++){
        int j;
        int layerFound = TW_FALSE;
        for (j = 0; j<availableLayerCount; j++) {
            if (strcmp(reqLayerNames[i], availableLayers[j].layerName) == 0) {
                layerFound = TW_TRUE;
                break;
            }
        }
        if (!layerFound) { return TW_FALSE; }
    }

    free(availableLayers);

    return TW_TRUE;
}

int twCheckVkInstanceExtensionSupport(
    const char * const *reqExtNames,
    unsigned int reqExtCount
)
{
    VkResult vkResult;
    VkExtensionProperties *availExtProperties;
    unsigned int availExtCount;
    
    if (reqExtCount == 0){
        return TW_TRUE;
    }

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &availExtCount, NULL);
    if(vkResult!=VK_SUCCESS){ return TW_FALSE; }
    availExtProperties = malloc(sizeof(*availExtProperties) * availExtCount);
    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &availExtCount, availExtProperties);
    if(vkResult!=VK_SUCCESS){ return TW_FALSE; }

    int i;
    int j;
    for (i=0; i<reqExtCount; i++){
        int available = TW_FALSE;
        for (j=0; j<availExtCount; j++){
            if (strcmp(reqExtNames[i], availExtProperties[j].extensionName) == 0){
                available = TW_TRUE;
                break;
            }
        }
        if (!available){ return TW_FALSE; }
    }

    free(availExtProperties);

    return TW_TRUE;
}

void twGetRequiredVkInstanceExtensions(
    unsigned int *pExtentionCount,
    const char * const **pExtensionNames
)
{
    SDL_Window* window;

    unsigned int numReqExtensions = TW_NUM_REQUIRED_VK_INSTANCE_EXTENSIONS; // == 0
    const char **extensionsArray = NULL; //[TW_NUM_REQUIRED_VK_INSTANCE_EXTENSIONS] = {
        //VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_SPEC_VERSION,
        //VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
    //};

    unsigned int SDLExtCount;
    char **SDLExtensions;

    // Need to create an SDL window to query SDL required vulkan instance extensions. Stupid unnecessary requirement.
    window = SDL_CreateWindow(
        "dummywindow",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1, 1,
        SDL_WINDOW_VULKAN
    );

    SDL_Vulkan_GetInstanceExtensions(window, &SDLExtCount, NULL);
    SDLExtensions = malloc(sizeof(*SDLExtensions) * SDLExtCount);
    SDL_Vulkan_GetInstanceExtensions(window, &SDLExtCount, SDLExtensions);

    // Can get rid of dummy window now.
    SDL_DestroyWindow(window);

    *pExtentionCount = numReqExtensions + SDLExtCount;

    *pExtensionNames = catStringLists(
        SDLExtensions, SDLExtCount,
        extensionsArray, numReqExtensions
    );

    if (twCheckVkInstanceExtensionSupport(*pExtensionNames, *pExtentionCount) != TW_TRUE){
        printf("Fatal Error - missing required vulkan instance extensions.\n");
    }
    
}

void twSetupVkInstance(TwWindow window){
    VkResult result;
    VkApplicationInfo appInfo = {0};
    VkInstanceCreateInfo createInfo = {0};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "RenderTorch";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 1);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 2, 148);

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    twGetRequiredVkInstanceExtensions(
        &createInfo.enabledExtensionCount,
        &createInfo.ppEnabledExtensionNames
    );

    createInfo.enabledLayerCount = 0;//1;
    const char *validationLayers[] = {
    //    "VK_LAYER_KHRONOS_validation",
    };
    createInfo.ppEnabledLayerNames = validationLayers;
    if(twCheckVkValidationLayerSupport(
        createInfo.enabledLayerCount,
        createInfo.ppEnabledLayerNames
    ) != TW_TRUE) { printf("Fatal Error - twCheckVkValidationLayerSupport returned TW_FALSE\n"); }

    result = vkCreateInstance(&createInfo, NULL, &window->instance);
    if (result != VK_SUCCESS) { printf("Fatal Error - Unable to create Vulkan Instance! Code: %i\n", result); }

    free(createInfo.ppEnabledExtensionNames);
}

int twIsVkPhysicalDeviceSuitable(VkPhysicalDevice device){
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        return TW_FALSE;
    }

    return TW_TRUE;
}

void twSetVkPhysicalDevice(TwWindow window){
    VkPhysicalDevice *availableDevices;
    unsigned int deviceCount = 0;

    vkEnumeratePhysicalDevices(window->instance, &deviceCount, NULL);
    if (deviceCount == 0){
        printf("Fatal Error - 0 physical devices found.\n");
    }

    availableDevices = malloc(sizeof(*availableDevices) * deviceCount);
    vkEnumeratePhysicalDevices(window->instance, &deviceCount, availableDevices);

    for (int i=0; i<deviceCount; i++){
        if(twIsVkPhysicalDeviceSuitable(availableDevices[i])){
            window->physicalDevice = availableDevices[i];
            free(availableDevices);
            return;
        }
    }
    printf("Fatal Error - No suitable GPU found.\n");
    free(availableDevices);
}

int twSetQueueFamilyIndices(TwWindow window){
    VkResult vkResult;
    unsigned int queueFamilyCount;
    VkQueueFamilyProperties *queueFamilies;
    SDL_Window *tempWindow;
    VkSurfaceKHR tempSurface;
    unsigned int i;

    int foundGraphicsQFI = TW_FALSE;
    int foundPresentQFI = TW_FALSE;

    vkGetPhysicalDeviceQueueFamilyProperties(window->physicalDevice, &queueFamilyCount, NULL);
    queueFamilies = malloc(sizeof(*queueFamilies) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(window->physicalDevice, &queueFamilyCount, queueFamilies);

    tempWindow = SDL_CreateWindow(
        "dummywindow",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1, 1,
        SDL_WINDOW_VULKAN
    );

    if(!SDL_Vulkan_CreateSurface(tempWindow, window->instance, &tempSurface)){
        printf("SDL_Vulkan_CreateSurface error: %s\n", SDL_GetError());
    }

    for (i=0; i<queueFamilyCount; i++){
        VkBool32 surfaceSupport = VK_FALSE;

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            window->graphicsQFIndex = i;
            foundGraphicsQFI = TW_TRUE;
            if (foundGraphicsQFI & foundPresentQFI){
                break;
            }
        }

        vkGetPhysicalDeviceSurfaceSupportKHR(window->physicalDevice, i, tempSurface, &surfaceSupport);
        if (surfaceSupport == VK_TRUE){
            window->presentQFIndex = i;
            foundPresentQFI = TW_TRUE;
            if (foundGraphicsQFI & foundPresentQFI){
                break;
            }
        }
    }
    
    vkDestroySurfaceKHR(window->instance, tempSurface, NULL);
    SDL_DestroyWindow(tempWindow);
    free(queueFamilies);

    if(foundGraphicsQFI & foundPresentQFI){
        return TW_TRUE;
    } else {
        return TW_FALSE;
    }
}

int twCheckVkDeviceExtensionSupport(
    VkPhysicalDevice device,
    unsigned int reqExtCount,
    const char * const *reqExtNames
)
{
    VkResult vkResult;
    VkExtensionProperties *availExtProperties;
    unsigned int availExtCount;
    vkResult = vkEnumerateDeviceExtensionProperties(device, NULL, &availExtCount, NULL);
    if(vkResult!=VK_SUCCESS){
        return TW_FALSE;
    }
    availExtProperties = malloc(sizeof(*availExtProperties) * availExtCount);
    vkResult = vkEnumerateDeviceExtensionProperties(device, NULL, &availExtCount, availExtProperties);
    if(vkResult!=VK_SUCCESS){
        return TW_FALSE;
    }

    int i;
    int j;
    for (i=0; i<reqExtCount; i++){
        int available = TW_FALSE;
        for (j=0; j<availExtCount; j++){
            if (strcmp(reqExtNames[i], availExtProperties[j].extensionName) == 0){
                available = TW_TRUE;
                break;
            }
        }
        if (!available){
            return TW_FALSE;
        }
    }

    free(availExtProperties);

    return TW_TRUE;
}

void twSetupVkLogicalDevice(TwWindow window){
    unsigned int i;
    VkResult vkResult;

    VkDeviceCreateInfo createInfo = {0};
    VkPhysicalDeviceFeatures deviceFeatures = {0};

    if(twSetQueueFamilyIndices(window) == TW_FALSE){
        printf("Unable to create VkDevice - suitable queue not found.\n");
    }

    unsigned int queueFamilyIndices[2] = {window->graphicsQFIndex, window->presentQFIndex};

    VkDeviceQueueCreateInfo queueCreateInfos[2] = {0};

    createInfo.queueCreateInfoCount = 2;
    float queuePriority = 1.0f;
    for (i=0; i<2; i++){
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = queueFamilyIndices[i];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
        if(queueFamilyIndices[0] == queueFamilyIndices[1]){
            createInfo.queueCreateInfoCount = 1;
            break;
        }
    }
    createInfo.pQueueCreateInfos = queueCreateInfos;

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    const char *extensionsArray[TW_NUM_REQUIRED_VK_DEVICE_EXTENSIONS] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        NULL,
        NULL
    };
    extensionsArray[1] = twExternalSemaphoreDeviceExtName();
    extensionsArray[2] = twExternalMemoryDeviceExtName();
    createInfo.enabledExtensionCount = TW_NUM_REQUIRED_VK_DEVICE_EXTENSIONS;
    createInfo.ppEnabledExtensionNames = extensionsArray;
    
    if(
        twCheckVkDeviceExtensionSupport(
            window->physicalDevice,
            createInfo.enabledExtensionCount,
            createInfo.ppEnabledExtensionNames
        ) == TW_FALSE
    ){
        printf("Some required device extensions unavailable.\n");
    }
    
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.ppEnabledLayerNames = NULL;

    vkResult = vkCreateDevice(window->physicalDevice, &createInfo, NULL, &window->device);
    if (vkResult != VK_SUCCESS){
        printf("Unable to create VkDevice - vkCreateDevice failed.\n");
    }

    vkGetDeviceQueue(window->device, window->graphicsQFIndex, 0, &window->graphicsQueue);

    vkGetDeviceQueue(window->device, window->presentQFIndex, 0, &window->presentQueue);

    if (!(window->graphicsQueue != NULL & window->presentQueue != NULL)){
        printf("Unable to get vkQueues - vkGetDeviceQueue failed.\n");
    }
}

void twCreateInstance(TwWindow window, const char *name){
    SDL_Init(SDL_INIT_VIDEO);

    twSetupVkInstance(window);
    twSetVkPhysicalDevice(window);
    twSetupVkLogicalDevice(window);
}

void twCreateSDLWindow(TwWindow window, const char *name){
    window->window = SDL_CreateWindow(
        name,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        window->parentDetails.winWidth, window->parentDetails.winHeight,
        SDL_WINDOW_VULKAN
    );
}

void twCreateSurface(TwWindow window){
    VkBool32 supported = VK_FALSE;
    SDL_Vulkan_CreateSurface(window->window, window->instance, &window->surface);
    vkGetPhysicalDeviceSurfaceSupportKHR(window->physicalDevice, window->presentQFIndex, window->surface, &supported);
    if(supported != VK_TRUE){ printf("Surface not supported by device."); }
}

void twQuerySwapChainSupport(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    struct SwapChainSupportDetails_T *details
){
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details->capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details->formatCount, NULL);
    details->formats = malloc(sizeof(*details->formats) * details->formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details->formatCount, details->formats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details->presentModeCount, NULL);
    details->presentModes = malloc(sizeof(*details->presentModes) * details->presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details->presentModeCount, details->presentModes);
}

void twChooseSwapSurfaceFormat(TwWindow window){
    for (int i=0; i<window->details.formatCount; i++) {
        if (window->details.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            window->details.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            window->swapImageFormat = window->details.formats[i];
            break;
        }
    }
}

void twChooseSwapPresentMode(TwWindow window){
    for (int i=0; i<window->details.presentModeCount; i++){
        if(window->details.presentModes[i] == VK_PRESENT_MODE_FIFO_KHR){
            window->swapPresentMode = window->details.presentModes[i];
            break;
        }
    }
}

void twChooseSwapExtent(TwWindow window){
    if (window->details.capabilities.currentExtent.width != UINT32_MAX){
        window->swapExtent = window->details.capabilities.currentExtent;
        return;
    }
    window->swapExtent.width = max(
        window->details.capabilities.minImageExtent.width,
        min(
            window->details.capabilities.maxImageExtent.width,
            window->parentDetails.winWidth
        )
    );
    window->swapExtent.height = max(
        window->details.capabilities.minImageExtent.height,
        min(
            window->details.capabilities.maxImageExtent.height,
            window->parentDetails.winHeight
        )
    );
}

void twCreateSwapChain(TwWindow window){
    VkSwapchainCreateInfoKHR createInfo = {0};

    twQuerySwapChainSupport(window->physicalDevice, window->surface, &window->details);
    twChooseSwapSurfaceFormat(window);
    twChooseSwapPresentMode(window);
    twChooseSwapExtent(window);

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = window->surface;
    createInfo.minImageCount = 2;
    createInfo.imageFormat = window->swapImageFormat.format;
    createInfo.imageColorSpace = window->swapImageFormat.colorSpace;
    createInfo.imageExtent = window->swapExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    unsigned int indices[2] = {
        window->graphicsQFIndex,
        window->presentQFIndex
    };

    if (indices[0] != indices[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = window->details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = window->swapPresentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    vkCreateSwapchainKHR(window->device, &createInfo, NULL, &window->swapChain);
    vkGetSwapchainImagesKHR(window->device, window->swapChain, &window->swapImageCount, NULL);

    window->swapImages = malloc(sizeof(*window->swapImages) * window->swapImageCount);
    vkGetSwapchainImagesKHR(window->device, window->swapChain, &window->swapImageCount, window->swapImages);
}

void twCreateSwapImageViews(TwWindow window){
    window->swapImageViews = malloc(sizeof(*window->swapImageViews) * window->swapImageCount);

    for (int i=0; i<window->swapImageCount; i++) {
        VkImageViewCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = window->swapImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = window->swapImageFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(window->device, &createInfo, NULL, &window->swapImageViews[i]);
    }
}

VkShaderModule twCreateShaderModule(VkDevice device, const char *code, unsigned int codeLength){
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeLength;
    createInfo.pCode = (const unsigned int *) code;

    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &createInfo, NULL, &shaderModule);

    return shaderModule;
}

void twLoadShaders(TwWindow window){
    unsigned int vertShaderCodeLen;
    unsigned int fragShaderCodeLen;
    const char *vertShaderCode; 
    const char *fragShaderCode;

    int readallResult;

    readallResult = readall(fopen("vert.spv", "rb"), &vertShaderCode, &vertShaderCodeLen);
    if (readallResult != READALL_OK) { printf("Unable to read vert shader file."); }

    readallResult = readall(fopen("frag.spv", "rb"), &fragShaderCode, &fragShaderCodeLen);
    if (readallResult != READALL_OK) { printf("Unable to read frag shader file."); }

    window->vertShaderModule = twCreateShaderModule(window->device, vertShaderCode, vertShaderCodeLen);
    window->fragShaderModule = twCreateShaderModule(window->device, fragShaderCode, fragShaderCodeLen);

    if (window->vertShaderModule == NULL | window->fragShaderModule == NULL){
        printf("Failed to load shaders.\n");
    }

}

void twCreateRenderPass(TwWindow window){
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = window->swapImageFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vkCreateRenderPass(window->device, &renderPassInfo, NULL, &window->renderPass);
}

void twCreateDescriptorSetLayout(TwWindow window){
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {0};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;

    vkCreateDescriptorSetLayout(window->device, &layoutInfo, NULL, &window->descriptorSetLayout);
}

void twCreateGraphicsPipeline(TwWindow window){
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = window->vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = window->fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[2] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL;
    vertexInputInfo.pVertexAttributeDescriptions = NULL;


    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) window->swapExtent.width;
    viewport.height = (float) window->swapExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D) {0, 0};
    scissor.extent = window->swapExtent;

    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &window->descriptorSetLayout;

    if (vkCreatePipelineLayout(window->device, &pipelineLayoutInfo, NULL, &window->pipelineLayout) != VK_SUCCESS) {
        printf("Unable to create pipeline layout - vkCreatePipelineLayout failed");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = window->pipelineLayout;
    pipelineInfo.renderPass = window->renderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(window->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &window->graphicsPipeline) != VK_SUCCESS) {
        printf("Unable to create graphics pipeline - vkCreateGraphicsPipelines failed");
    }
}

void twCreateFrameBuffers(TwWindow window){
    window->swapFrameBuffers = malloc(sizeof(*window->swapFrameBuffers) * window->swapImageCount);

    for (int i=0; i<window->swapImageCount; i++){
        VkFramebufferCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = window->renderPass;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = &window->swapImageViews[i];
        createInfo.width = window->swapExtent.width;
        createInfo.height = window->swapExtent.height;
        createInfo.layers = 1;
        if (vkCreateFramebuffer(window->device, &createInfo, NULL, &window->swapFrameBuffers[i]) != VK_SUCCESS) {
            printf("Unable to create swapchain framebuffers - vkCreateFramebuffer failed");
        }
    }
}

void twCreateCommandPool(TwWindow window){
    VkCommandPoolCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = window->graphicsQFIndex;

    if (vkCreateCommandPool(window->device, &createInfo, NULL, &window->commandPool) != VK_SUCCESS) {
        printf("Unable to create command pool - vkCreateCommandPool failed");
    }
}

void twCreateTextureSampler(TwWindow window){
    VkSamplerCreateInfo samplerInfo = {0};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(window->device, &samplerInfo, NULL, &window->textureSampler) != VK_SUCCESS) {
        printf("Unable to create texture sampler - vkCreateSampler failed");
    }
}

void twCreateDescriptorPool(TwWindow window){
    VkDescriptorPoolSize poolSize = {0};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = window->swapImageCount;

    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = window->swapImageCount;

    vkCreateDescriptorPool(window->device, &poolInfo, NULL, &window->descriptorPool);
}

void twCreateDescriptorSets(TwWindow window){
    int i;
    VkDescriptorSetLayout layouts[2] = {window->descriptorSetLayout, window->descriptorSetLayout};

    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = window->descriptorPool;
    allocInfo.descriptorSetCount = window->swapImageCount;
    allocInfo.pSetLayouts = layouts;

    window->descriptorSets = malloc(sizeof(*window->descriptorSets) * window->swapImageCount);
    vkAllocateDescriptorSets(window->device, &allocInfo, window->descriptorSets);

    for (i=0; i<window->swapImageCount; i++){
        VkDescriptorImageInfo imageInfo = {0};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = window->textureImageView;
        imageInfo.sampler = window->textureSampler;

        VkWriteDescriptorSet descriptorWrite = {0};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = window->descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(window->device, 1, &descriptorWrite, 0, NULL);
    }
}

VkCommandBuffer beginSingleTimeCommands(TwWindow window) {
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = window->commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(window->device, &allocInfo, &commandBuffer) != VK_SUCCESS){
        return NULL;
    }

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(TwWindow window, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(window->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(window->graphicsQueue);

    vkFreeCommandBuffers(window->device, window->commandPool, 1, &commandBuffer);
}

void twInitTextureImageLayout(TwWindow window) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(window);

    VkImageMemoryBarrier barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = window->textureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );


    endSingleTimeCommands(window, commandBuffer);
}

void TEMPrtInitTextureImageLayout(TwWindow window, int color) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(window);

    VkImageMemoryBarrier barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = window->textureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    
    VkClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    clearColor.float32[color % 3] = 1.0f;

    VkImageSubresourceRange subResRange = {0};
    subResRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subResRange.baseMipLevel = 0;
    subResRange.levelCount = 1;
    subResRange.baseArrayLayer = 0;
    subResRange.layerCount = 1;

    vkCmdClearColorImage(
        commandBuffer,
        window->textureImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        &clearColor,
        1,
        &subResRange
    );

    VkImageMemoryBarrier barrier2 = {0};
    barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.image = window->textureImage;
    barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier2.subresourceRange.baseMipLevel = 0;
    barrier2.subresourceRange.levelCount = 1;
    barrier2.subresourceRange.baseArrayLayer = 0;
    barrier2.subresourceRange.layerCount = 1;
    
    barrier2.srcAccessMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, NULL,
        0, NULL,
        1, &barrier2
    );

    endSingleTimeCommands(window, commandBuffer);
}

void twCreateTextureImageView(TwWindow window){
    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = window->textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_ONE;

    vkCreateImageView(window->device, &viewInfo, NULL, &window->textureImageView);
}

void twCreateCommandBuffers(TwWindow window) {
    window->commandBuffers = malloc(sizeof(*window->commandBuffers) * window->swapImageCount);

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = window->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = window->swapImageCount;

    vkAllocateCommandBuffers(window->device, &allocInfo, window->commandBuffers);

    // Record each of the command buffers.
    for (int i=0; i < window->swapImageCount; i++){
        VkCommandBufferBeginInfo beginInfo = {0};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(window->commandBuffers[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfo = {0};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = window->renderPass;
        renderPassInfo.framebuffer = window->swapFrameBuffers[i];
        renderPassInfo.renderArea.offset = (VkOffset2D) {0, 0};
        renderPassInfo.renderArea.extent = window->swapExtent;

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(window->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(window->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, window->graphicsPipeline);
        vkCmdBindDescriptorSets(window->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, window->pipelineLayout, 0, 1, &window->descriptorSets[i], 0, NULL);
        vkCmdDraw(window->commandBuffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(window->commandBuffers[i]);

        vkEndCommandBuffer(window->commandBuffers[i]);
    }
}

void twCreateSyncObjects(TwWindow window){
    window->imageAvailableSemaphores = malloc(sizeof(*window->imageAvailableSemaphores) * MAX_FRAMES_IN_FLIGHT);
    window->renderFinishedSemaphores = malloc(sizeof(*window->renderFinishedSemaphores) * MAX_FRAMES_IN_FLIGHT);
    window->inFlightFences = malloc(sizeof(*window->inFlightFences) * MAX_FRAMES_IN_FLIGHT);
    window->imagesInFlight = calloc(window->swapImageCount, sizeof(*window->imagesInFlight));

    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(int i=0; i<MAX_FRAMES_IN_FLIGHT; i++){
        vkCreateSemaphore(window->device, &semaphoreInfo, NULL, &window->imageAvailableSemaphores[i]);
        vkCreateSemaphore(window->device, &semaphoreInfo, NULL, &window->renderFinishedSemaphores[i]);
        vkCreateFence(window->device, &fenceInfo, NULL, &window->inFlightFences[i]);
    }
}

TwWindow twCreateWindow(const char *name){
    TwWindow window;

    window = calloc(1, sizeof(*window));
    window->name = name;


    twCreateInstance(window, name);

    twConnectWindowProcess(
        window->device,
        window->physicalDevice,
        &window->textureImage,
        &window->childDetails,
        &window->parentDetails,
        name
    );
    
    twCreateSDLWindow(window, name);
    twCreateSurface(window);
    twCreateSwapChain(window);
    twCreateSwapImageViews(window);
    twLoadShaders(window);
    twCreateRenderPass(window);
    twCreateDescriptorSetLayout(window);
    twCreateGraphicsPipeline(window);
    twCreateFrameBuffers(window);
    twCreateCommandPool(window);
    twCreateTextureSampler(window);
    twInitTextureImageLayout(window);
    twCreateTextureImageView(window);
    twCreateDescriptorPool(window);
    twCreateDescriptorSets(window);
    twCreateCommandBuffers(window);
    twCreateSyncObjects(window);

    window->currentFrame = 0;

    return window;
}

void cleanupSyncObjects(TwWindow window){
    for(int i=0; i<MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(window->device, window->renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(window->device, window->imageAvailableSemaphores[i], NULL);
        vkDestroyFence(window->device, window->inFlightFences[i], NULL);
    }

    free(window->imageAvailableSemaphores);
    free(window->renderFinishedSemaphores);
    free(window->inFlightFences);
    free(window->imagesInFlight);
}

void twCleanupSwapchain(TwWindow window){
    vkDeviceWaitIdle(window->device);
    for(int i=0; i<window->swapImageCount; i++){
        vkDestroyFramebuffer(window->device, window->swapFrameBuffers[i], NULL);
    }
    vkFreeCommandBuffers(window->device, window->commandPool, window->swapImageCount, window->commandBuffers);
    vkDestroyPipeline(window->device, window->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(window->device, window->pipelineLayout, NULL);
    vkDestroyRenderPass(window->device, window->renderPass, NULL);
    for (int i=0; i<window->swapImageCount; i++){
        vkDestroyImageView(window->device, window->swapImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(window->device, window->swapChain, NULL);

}

void waitForUnminization(TwWindow window){
    SDL_Event event;
    for(;;){
        SDL_WaitEvent(&event);
        if(event.type == SDL_WINDOWEVENT){
            if(event.window.event == SDL_WINDOWEVENT_RESTORED){
                break;
            }
        }
    }
}

void recreateSwapchain(TwWindow window){
    twCleanupSwapchain(window);

    waitForUnminization(window);

    twCreateSwapChain(window);
    twCreateSwapImageViews(window);
    twCreateRenderPass(window);
    twCreateGraphicsPipeline(window);
    twCreateFrameBuffers(window);
    twCreateCommandBuffers(window);
}

void twDestroyWindow(TwWindow window){
    twCleanupSwapchain(window);
    cleanupSyncObjects(window);
    vkDestroyDescriptorPool(window->device, window->descriptorPool, NULL);
    vkDestroyImageView(window->device, window->textureImageView, NULL);
    vkDestroySampler(window->device, window->textureSampler, NULL);
    vkDestroyCommandPool(window->device, window->commandPool, NULL);
    vkDestroyDescriptorSetLayout(window->device, window->descriptorSetLayout, NULL);
    vkDestroyShaderModule(window->device, window->vertShaderModule, NULL);
    vkDestroyShaderModule(window->device, window->fragShaderModule, NULL);
    vkDestroySurfaceKHR(window->instance, window->surface, NULL);
    SDL_DestroyWindow(window->window);
    vkDestroyImage(window->device, window->textureImage, NULL);
    vkFreeMemory(window->device, window->childDetails.textureMemory, NULL);
    vkDestroySemaphore(window->device, window->childDetails.textureUpdated, NULL);
    vkDestroySemaphore(window->device, window->childDetails.updateConsumed, NULL);
    vkDestroyDevice(window->device, NULL);
    vkDestroyInstance(window->instance, NULL);

    free(window);
}

void twDrawFrame(TwWindow window){
    VkResult vkResult;
    unsigned int imageIndex;

    vkResult = vkWaitForFences(window->device, 1, &window->inFlightFences[window->currentFrame], VK_TRUE, UINT64_MAX);
    if (vkResult != VK_SUCCESS) printf("vkWaitForFences failed - Error code: %i\n", vkResult);

    vkResult = vkAcquireNextImageKHR(
        window->device,
        window->swapChain,
        UINT64_MAX,
        window->imageAvailableSemaphores[window->currentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );
    if (vkResult == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(window);
        return;
    }

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (window->imagesInFlight[imageIndex] != NULL) {
        vkResult = vkWaitForFences(window->device, 1, &window->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        if (vkResult != VK_SUCCESS) printf("vkWaitForFences failed - Error code: %i\n", vkResult);
    }
    // Mark the image as now being in use by this frame
    window->imagesInFlight[imageIndex] = window->inFlightFences[window->currentFrame];

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
        window->imageAvailableSemaphores[window->currentFrame],
        window->childDetails.textureUpdated
    };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &window->commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {
        window->renderFinishedSemaphores[window->currentFrame],
        window->childDetails.updateConsumed
    };
    submitInfo.signalSemaphoreCount = 2;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResult = vkResetFences(window->device, 1, &window->inFlightFences[window->currentFrame]);
    if (vkResult != VK_SUCCESS) printf("vkResetFences failed - Error code: %i\n", vkResult);

    vkResult = vkQueueSubmit(window->graphicsQueue, 1, &submitInfo, window->inFlightFences[window->currentFrame]);
    if (vkResult != VK_SUCCESS) printf("vkQueueSubmit failed - Error code: %i\n", vkResult);

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {window->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkResult = vkQueuePresentKHR(window->presentQueue, &presentInfo);
    if (vkResult == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(window);
        return;
    }else if (vkResult != VK_SUCCESS) printf("vkQueuePresentKHR failed - Error code: %i\n", vkResult);

    window->currentFrame = (window->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

int twPollWindowEvents(TwWindow window){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_QUIT){
            return 0;
        }
    }
    return 1;
}

void twRunWindow(TwWindow window){
    //struct timeb start, end;
    //int diff;
    unsigned int frameCount = 0;
    
    //ftime(&start);

    for(int i=0;;i++){
        if(!twPollWindowEvents(window)){
            break;
        }
        //TEMPrtInitTextureImageLayout(window, i);
        twDrawFrame(window);
        //printf("frame: %i\n", frameCount);

        char *title;
        const char *form = "%s - %i FPS";
        int length = snprintf( NULL, 0, form, window->name, frameCount) + 1;
        title = malloc(length);
        snprintf(title, length, form, window->name, frameCount);
        SDL_SetWindowTitle(window->window, title);

        free(title);
        
        frameCount++;/*

        

        ftime(&end);
        diff = (int) (1000.0 * (end.time - start.time) + (end.millitm - start.millitm));
        if(diff >= 1000){
            char *title;
            const char *form = "%s - %i FPS";
            int length = snprintf( NULL, 0, form, window->name, frameCount) + 1;
            title = malloc(length);
            snprintf(title, length, form, window->name, frameCount);
            SDL_SetWindowTitle(window->window, title);

            free(title);
            frameCount = 0;
            start = end;
        }*/
    }
}