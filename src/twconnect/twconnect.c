#include "twconnect_private.h"

void removeSpacesAndQuotes(char * string){
    const char* d = string;
    do {
        while (*d == ' ' || *d == '\"') {
            ++d;
        }
    } while (*string++ = *d++);
}

void twCreateWindowProcess(
    TwParentDetails parentDetails,
    TwReceivedChildDetails childDetails,
    const char *name,
    const char *dir
){
    TwShareableParentDetails shareableParentDetails;

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    HANDLE childProcessHandle;
    HANDLE pipe;

    char pipeName[256] = "\\\\.\\pipe\\";

    if (strlen(name) <= 247){
        strcat(pipeName, name);
        removeSpacesAndQuotes(pipeName);
        printf("pipeName: %s\n", pipeName);
    }else{
        printf("unable to create window - name too long");
        return;
    }

    // create a monodirectional father->child named pipe
    pipe = CreateNamedPipeA(
        pipeName,             // name of the pipe
        PIPE_ACCESS_DUPLEX,   // send and receive
        PIPE_TYPE_BYTE,       // send data as a byte stream
        1,                    // only one instance
        0, 0, 0, NULL);       // default junk
    if (pipe == INVALID_HANDLE_VALUE) printf("Could not create pipe - CreateNamedPipeA failed\n");

#elif defined(__linux__)
    int fdConnSocket, fdDataSocket;
    struct sockaddr_un saddr = {0};

    fdConnSocket = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fdConnSocket < 0) {
        printf("Could not create socket - socket failed\n");
        return;
    }

    saddr.sun_family = AF_LOCAL;
    if (strlen(name) <= 90){
        strcpy(saddr.sun_path, "/tmp/torchwindow");
        strcat(saddr.sun_path, name);
        removeSpacesAndQuotes(saddr.sun_path);
        printf("saddr.sun_path: %s\n", saddr.sun_path);
    }
    else{ printf("unable to create window - name too long"); return; }

    if (bind(fdConnSocket, (const struct sockaddr *) &saddr, sizeof(saddr)) < 0){
        printf("Could not bind socket - bind failed\n");
        perror("bind");
        return;
    }

#endif
    
    shareableParentDetails = calloc(1, sizeof(*shareableParentDetails));
    convertParentDetailsToShareable(shareableParentDetails, parentDetails);

    // spawn child process
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;

        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);

        memset(&pi, 0, sizeof(pi));

        const char *exeName = "\\torchwindow.exe";
        char *exePath = malloc(strlen(dir) + strlen(exeName) + 1);
        strcpy(exePath, dir);
        strcat(exePath, exeName);

        if (!CreateProcessA(    // using ASCII variant to be compatible with argv
            exePath,            // executable name
            (char*) name,       // command line args
            NULL, NULL, TRUE,   // default junk
            // CREATE_NEW_CONSOLE, // launch in another console window
            CREATE_NO_WINDOW,   // launch without a console window
            NULL, (char *) dir, // more junk
            &si, &pi))          // final useless junk
            {printf("Could not create process - CreateProcessA failed\n"); return;}
        free(exePath);
        childDetails->childProcess = pi.hProcess;
    }
#elif defined(__linux__)
    {
        int fdOutput;
        pid_t pIDChild = fork();
        if(!pIDChild){
            chdir(dir);
            fdOutput = open("/dev/null", O_WRONLY | O_CREAT, 0666);
            dup2(fdOutput, 1);
            execl("torchwindow", name, NULL);
            close(fdOutput);
            exit(0);
        }
        printf("pIDChild: %i\n", pIDChild);
    }
#endif

    // connect to child process
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    if (!ConnectNamedPipe(pipe, NULL)){
        printf("Could not connect pipe - ConnectNamedPipe failed\n");
        return;
    }

    // Exchange initialisation data with child
    {
        TwShareableChildDetails shareableChildDetails;

        DWORD written = 0;
        DWORD read = 0;

        WriteFile(pipe, shareableParentDetails, sizeof(*shareableParentDetails), &written, NULL);
        if(written != sizeof(*shareableParentDetails)) printf("could not write to pipe\n");

        free(shareableParentDetails);

        shareableChildDetails = calloc(1, sizeof(*shareableChildDetails));

        // TODO add code to handle the case where read from pipe does not return
        ReadFile(pipe, shareableChildDetails, sizeof(*shareableChildDetails), &read, NULL);
        if(read != sizeof(*shareableChildDetails)) printf("read from pipe went wrong somehow\n");

        getChildDetailsFromShareable(childDetails, shareableChildDetails);

        free(shareableChildDetails);
    }

    DisconnectNamedPipe(pipe);
#elif defined(__linux__)
    if(listen(fdConnSocket, 1) < 0){
        printf("Unable to listen to socket for connections - listen failed\n");
        return;
    }

    // Exchange initialisation data with child
    {
        TwShareableChildDetails shareableChildDetails;

        int writtenCount = 0;
        int readCount = 0;

        fdDataSocket = accept(fdConnSocket, NULL, NULL);
        if(fdDataSocket < 0){
            printf("accept failed\n");
            return;
        }

        writtenCount = write(fdDataSocket, shareableParentDetails, sizeof(*shareableParentDetails));
        if(writtenCount != sizeof(*shareableParentDetails)) printf("could not write to socket\n");

        free(shareableParentDetails);

        shareableChildDetails = calloc(1, sizeof(*shareableChildDetails));

        // TODO add code to handle the case where read from pipe does not return
        readCount = read(fdDataSocket, shareableChildDetails, sizeof(*shareableChildDetails));
        if(readCount != sizeof(*shareableChildDetails)) printf("read from pipe went wrong somehow\n");

        recvFds(fdDataSocket, shareableChildDetails);

        getChildDetailsFromShareable(childDetails, shareableChildDetails);

        close(fdDataSocket);

        free(shareableChildDetails);
    }

    close(fdConnSocket);
    unlink(saddr.sun_path);
#endif
}

void twConnectWindowProcess(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkImage *textureImage,
    TwChildDetails childDetails,
    TwReceivedParentDetails parentDetails,
    const char *name
){
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    HANDLE pipe;

    char pipeName[256] = "\\\\.\\pipe\\";

    if (strlen(name) <= 247){
        strcat(pipeName, name);
        removeSpacesAndQuotes(pipeName);
    }else{
        printf("unable to create window - name too long");
        return;
    }

    pipe = CreateFileA(pipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (pipe == INVALID_HANDLE_VALUE) printf("could not connect to the pipe\n");
#elif defined(__linux__)
    int fdDataSocket;
    struct sockaddr_un saddr = {0};

    fdDataSocket = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fdDataSocket < 0) {
        printf("Could not create socket - socket failed\n");
        return;
    }

    saddr.sun_family = AF_LOCAL;
    if (strlen(name) <= 90){
        strcpy(saddr.sun_path, "/tmp/torchwindow");
        strcat(saddr.sun_path, name);
        removeSpacesAndQuotes(saddr.sun_path);
        printf("saddr.sun_path: %s\n", saddr.sun_path);
    }
    else{ printf("unable to create window - name too long\n"); return; }

    if(connect(fdDataSocket, (const struct sockaddr *) &saddr, sizeof(saddr)) < 0){
        printf("unable to connect to socket - connect failed\n"); return;
    }

#endif

    {
        TwShareableParentDetails shareableParentDetails;
        TwShareableChildDetails shareableChildDetails;
        
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
        HANDLE parentHandle;
        DWORD written = 0;
        DWORD read = 0;
#elif defined(__linux__)
        pid_t pIDParent;
        int writtenCount = 0;
        int readCount = 0;
#endif

        shareableParentDetails = calloc(1, sizeof(*shareableParentDetails));
        shareableChildDetails = calloc(1, sizeof(*shareableChildDetails));

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
        ReadFile(pipe, shareableParentDetails, sizeof(*shareableParentDetails), &read, NULL);
        if(read != sizeof(*shareableParentDetails)) printf("read from pipe went wrong somehow\n");
#elif defined(__linux__)

        readCount = read(fdDataSocket, shareableParentDetails, sizeof(*shareableParentDetails));
        if(readCount != sizeof(*shareableParentDetails)) printf("read from pipe went wrong somehow\n");
#endif

        getParentDetailsFromShareable(parentDetails, shareableParentDetails);
        pIDParent = shareableParentDetails->pIDParent;
        free(shareableParentDetails);
        
        twCreateTextureImage(device, parentDetails->texWidth, parentDetails->texHeight, textureImage);
        twCreateTextureMemory(device, physicalDevice, *textureImage, &childDetails->textureMemory, &childDetails->memSize);
        twCreateTextureSemaphores(device, &childDetails->textureUpdated, &childDetails->updateConsumed);

        convertChildDetailsToShareable(device, pIDParent, shareableChildDetails, childDetails);

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
        WriteFile(pipe, shareableChildDetails, sizeof(*shareableChildDetails), &written, NULL);
        if(written != sizeof(*shareableChildDetails)) printf("could not write to pipe\n");
#elif defined(__linux__)

        writtenCount = write(fdDataSocket, shareableChildDetails, sizeof(*shareableChildDetails));;
        if(writtenCount != sizeof(*shareableChildDetails)) printf("could not write to pipe\n");

        sendFds(fdDataSocket, shareableChildDetails);
        
#endif
        free(shareableChildDetails);
    }

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    CloseHandle(pipe);
#elif defined(__linux__)
    close(fdDataSocket);
#endif
}

void convertParentDetailsToShareable(
    TwShareableParentDetails shareableParentDetails,
    TwParentDetails parentDetails
){
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    HANDLE pseudoCurrent;
#endif

    shareableParentDetails->winWidth = parentDetails->winWidth;
    shareableParentDetails->winHeight = parentDetails->winHeight;
    shareableParentDetails->texWidth = parentDetails->texWidth;
    shareableParentDetails->texHeight = parentDetails->texHeight;

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    pseudoCurrent = GetCurrentProcess();

    if(!DuplicateHandle(
        pseudoCurrent,
        pseudoCurrent,
        pseudoCurrent,
        &shareableParentDetails->parentHandle,
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS
    )) printf("DuplicateHandle failed - Error Code: %i\n", GetLastError());
#elif defined(__linux__)
    shareableParentDetails->pIDParent = getpid();
#endif
}

void getChildDetailsFromShareable(
    TwReceivedChildDetails childDetails,
    TwShareableChildDetails shareableChildDetails
){
    cudaError_t cdResult;
    struct cudaExternalSemaphoreHandleDesc semHandleDesc = {0};
    struct cudaExternalMemoryHandleDesc memHandleDesc = {0};
    struct cudaExternalMemoryBufferDesc memBufferDesc = {0};
    cudaExternalMemory_t cudaExtMem;

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    semHandleDesc.type = cudaExternalSemaphoreHandleTypeOpaqueWin32;

    semHandleDesc.handle.win32.handle = shareableChildDetails->updateConsumed;
#elif defined(__linux__)
    semHandleDesc.type = cudaExternalSemaphoreHandleTypeOpaqueFd;

    semHandleDesc.handle.fd = shareableChildDetails->fdUpdateConsumed;
#endif
    cudaImportExternalSemaphore(&childDetails->updateConsumed, &semHandleDesc);

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    semHandleDesc.handle.win32.handle = shareableChildDetails->textureUpdated;
#elif defined(__linux__)
    semHandleDesc.handle.fd = shareableChildDetails->fdTextureUpdated;
#endif
    cudaImportExternalSemaphore(&childDetails->textureUpdated, &semHandleDesc);

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    memHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
    memHandleDesc.handle.win32.handle = shareableChildDetails->textureMemory;
#elif defined(__linux__)
    memHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueFd;
    memHandleDesc.handle.fd = shareableChildDetails->fdTextureMemory;
#endif

    memHandleDesc.size = shareableChildDetails->memSize;
    cudaImportExternalMemory(&cudaExtMem, &memHandleDesc);

    memBufferDesc.offset = 0;
    memBufferDesc.size = shareableChildDetails->memSize;
    cudaExternalMemoryGetMappedBuffer(&childDetails->textureMemoryBuffer, cudaExtMem, &memBufferDesc);
}

void convertChildDetailsToShareable(
    VkDevice device,
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    HANDLE parentHandle,
#elif defined(__linux__)
    pid_t pIDParent,
#endif
    TwShareableChildDetails shareableChildDetails,
    TwChildDetails childDetails
){
    cudaError_t cdResult;
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    HANDLE pseudoCurrent;
    HANDLE tempMemoryHandle;
    HANDLE tempTextureUpdatedHandle;
    HANDLE tempUpdateConsumedHandle;

    VkSemaphoreGetWin32HandleInfoKHR semGetHandleInfo = {0};
    VkMemoryGetWin32HandleInfoKHR handleInfo = {0};

    PFN_vkGetSemaphoreWin32HandleKHR vkGetSemaphoreWin32HandleKHR = \
    (PFN_vkGetSemaphoreWin32HandleKHR) vkGetDeviceProcAddr(device, "vkGetSemaphoreWin32HandleKHR");

    PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR = \
    (PFN_vkGetMemoryWin32HandleKHR) vkGetDeviceProcAddr(device, "vkGetMemoryWin32HandleKHR");

    semGetHandleInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR;
    semGetHandleInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    semGetHandleInfo.semaphore = childDetails->textureUpdated;
    vkGetSemaphoreWin32HandleKHR(
        device,
        &semGetHandleInfo,
        &tempTextureUpdatedHandle
    );

    semGetHandleInfo.semaphore = childDetails->updateConsumed;
    vkGetSemaphoreWin32HandleKHR(
        device,
        &semGetHandleInfo,
        &tempUpdateConsumedHandle
    );

    handleInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
    handleInfo.memory = childDetails->textureMemory;
    handleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    vkGetMemoryWin32HandleKHR(device, &handleInfo, &tempMemoryHandle);

    shareableChildDetails->memSize = childDetails->memSize;

    pseudoCurrent = GetCurrentProcess();

    if(!DuplicateHandle(
        pseudoCurrent,
        tempTextureUpdatedHandle,
        parentHandle,
        &shareableChildDetails->textureUpdated,
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS
    )){
        printf("Memory DuplicateHandle failed: %i\n", GetLastError());
    }

    if(!DuplicateHandle(
        pseudoCurrent,
        tempUpdateConsumedHandle,
        parentHandle,
        &shareableChildDetails->updateConsumed,
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS
    )){
        printf("Memory DuplicateHandle failed: %i\n", GetLastError());
    } 

    if(!DuplicateHandle(
        pseudoCurrent,
        tempMemoryHandle,
        parentHandle,
        &shareableChildDetails->textureMemory,
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS
    )){
        printf("Memory DuplicateHandle failed: %i\n", GetLastError());
    }
#elif defined(__linux__)

    VkSemaphoreGetFdInfoKHR semGetHandleInfo = {0};
    VkMemoryGetFdInfoKHR handleInfo = {0};

    PFN_vkGetSemaphoreFdKHR vkGetSemaphoreFdKHR = \
    (PFN_vkGetSemaphoreFdKHR) vkGetDeviceProcAddr(device, "vkGetSemaphoreFdKHR");

    PFN_vkGetMemoryFdKHR vkGetMemoryFdKHR = \
    (PFN_vkGetMemoryFdKHR) vkGetDeviceProcAddr(device, "vkGetMemoryFdKHR");

    semGetHandleInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
    semGetHandleInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;

    semGetHandleInfo.semaphore = childDetails->textureUpdated;
    vkGetSemaphoreFdKHR(
        device,
        &semGetHandleInfo,
        &shareableChildDetails->fdTextureUpdated
    );

    semGetHandleInfo.semaphore = childDetails->updateConsumed;
    vkGetSemaphoreFdKHR(
        device,
        &semGetHandleInfo,
        &shareableChildDetails->fdUpdateConsumed
    );

    handleInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    handleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    handleInfo.memory = childDetails->textureMemory;
    vkGetMemoryFdKHR(device, &handleInfo, &shareableChildDetails->fdTextureMemory);

    shareableChildDetails->memSize = childDetails->memSize;
#endif

}

void getParentDetailsFromShareable(
    TwReceivedParentDetails parentDetails,
    TwShareableParentDetails shareableParentDetails
){
    parentDetails->winWidth = shareableParentDetails->winWidth;
    parentDetails->winHeight = shareableParentDetails->winHeight;
    parentDetails->texWidth = shareableParentDetails->texWidth;
    parentDetails->texHeight = shareableParentDetails->texHeight;
}

void twCreateTextureImage(VkDevice device, uint32_t width, uint32_t height, VkImage *image){
    VkResult vkResult;
    VkDeviceSize imageSize;
    VkImageCreateInfo imageInfo = {0};
    VkExternalMemoryImageCreateInfo externInfo = {0};

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    externInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    externInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#elif defined(__linux__)
    externInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

    imageInfo.pNext = &externInfo;
    vkCreateImage(device, &imageInfo, NULL, image);
}

void twCreateTextureMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkImage textureImage, VkDeviceMemory *memory, size_t *size){
    VkMemoryAllocateInfo memAllocInfo = {0};
    VkExportMemoryAllocateInfoKHR exportInfo = {0};
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    VkExportMemoryWin32HandleInfoKHR exportInfoWin32 = {0};
    SECURITY_ATTRIBUTES sa = {0};
#endif

    VkMemoryRequirements memRequirements;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetImageMemoryRequirements(device, textureImage, &memRequirements);

    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memRequirements.size;
    *size = memRequirements.size;

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    memAllocInfo.memoryTypeIndex = -1;
    for (int i=0; i<memoryProperties.memoryTypeCount; i++){
        if(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT){
            memAllocInfo.memoryTypeIndex = i;
            break;
        }
    }
    if (memAllocInfo.memoryTypeIndex == -1) printf("Unable to find suitable memory type for texture image.\n");

    exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    exportInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    exportInfoWin32.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
    exportInfoWin32.dwAccess = DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE;
    getWinSecurityAttributes(&sa);
    exportInfoWin32.pAttributes = &sa;

    exportInfo.pNext = &exportInfoWin32;
#elif defined(__linux__)
    exportInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    memAllocInfo.pNext = &exportInfo;

    vkAllocateMemory(device, &memAllocInfo, NULL, memory);
    vkBindImageMemory(device, textureImage, *memory, 0);
}

void twCreateTextureSemaphores(VkDevice device, VkSemaphore *textureUpdated, VkSemaphore *updateConsumed){
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    VkExportSemaphoreCreateInfo exportInfo = {0};
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    VkExportSemaphoreWin32HandleInfoKHR exportInfoWin32 = {0};
    SECURITY_ATTRIBUTES sa = {0};
#endif

    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    exportInfo.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    exportInfoWin32.sType = VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR;
    exportInfoWin32.dwAccess = DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE;
    getWinSecurityAttributes(&sa);
    exportInfoWin32.pAttributes = &sa;
    
    exportInfo.pNext = &exportInfoWin32;
#elif defined(__linux__)
    exportInfo.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    semaphoreInfo.pNext = &exportInfo;

    vkCreateSemaphore(
        device,
        &semaphoreInfo,
        NULL,
        textureUpdated
    );
    vkCreateSemaphore(
        device,
        &semaphoreInfo,
        NULL,
        updateConsumed
    );
}

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
int twIsProcessStillRunning(HANDLE handle){
    DWORD exitCode;
    GetExitCodeProcess(handle, &exitCode);
    if(exitCode == STILL_ACTIVE) return 1;
    else return 0;
}
#elif defined(__linux__)
int twIsProcessStillRunning(pid_t pid){
    return 0;
}
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
void getWinSecurityAttributes(SECURITY_ATTRIBUTES *sa){
    PSECURITY_DESCRIPTOR pSd = calloc(1, SECURITY_DESCRIPTOR_MIN_LENGTH + 2 * sizeof(void **));
    PSID *ppSID = (PSID *)((PBYTE)pSd + SECURITY_DESCRIPTOR_MIN_LENGTH);
    PACL *ppACL = (PACL *)((PBYTE)ppSID + sizeof(PSID *));

    InitializeSecurityDescriptor(pSd, SECURITY_DESCRIPTOR_REVISION);
    SID_IDENTIFIER_AUTHORITY sidIA = SECURITY_WORLD_SID_AUTHORITY;
    AllocateAndInitializeSid(&sidIA, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, ppSID);

    EXPLICIT_ACCESSA ea = {0};
    ea.grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = INHERIT_ONLY;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPTSTR)* ppSID;

    SetEntriesInAclA(1, &ea, NULL, ppACL);
    SetSecurityDescriptorDacl(pSd, TRUE, *ppACL, FALSE);

    sa->nLength = sizeof(*sa);
    sa->lpSecurityDescriptor = pSd;
    sa->bInheritHandle = TRUE;
}
#elif defined(__linux__)
void sendFds(int fdSocket, TwShareableChildDetails details){
    char dummy = '$';
    struct msghdr msg;
    struct iovec iov;

    struct cmsghdr* cmsg;

    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    iov.iov_base = &dummy;
    iov.iov_len = sizeof(dummy);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = CMSG_LEN(sizeof(int));

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

    *(int*) CMSG_DATA(cmsg) = details->fdTextureMemory;
    if(sendmsg(fdSocket, &msg, 0) < 0){ printf("sendmsg memory failed\n"); return; }

    *(int*) CMSG_DATA(cmsg) = details->fdTextureUpdated;
    if(sendmsg(fdSocket, &msg, 0) < 0){ printf("sendmsg upsem failed\n"); return; }

    *(int*) CMSG_DATA(cmsg) = details->fdUpdateConsumed;
    if(sendmsg(fdSocket, &msg, 0) < 0){ printf("sendmsg consem failed\n"); return; }
}

void recvFds(int fdSocket, TwShareableChildDetails details){
    char buf[1];
    struct iovec iov;
    struct msghdr msg;

    struct cmsghdr *cmsg;

    char cms[CMSG_SPACE(sizeof(int))];

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = cms;
    msg.msg_controllen = sizeof(cms);

    if(recvmsg(fdSocket, &msg, 0) <= 0){ printf("recvmsg memory failed\n"); return; }
    cmsg = CMSG_FIRSTHDR(&msg);
    memcpy(&details->fdTextureMemory, CMSG_DATA(cmsg), sizeof(int));

    if(recvmsg(fdSocket, &msg, 0) <= 0){ printf("recvmsg upsem failed\n"); return; }
    cmsg = CMSG_FIRSTHDR(&msg);
    memcpy(&details->fdTextureUpdated, CMSG_DATA(cmsg), sizeof(int));

    if(recvmsg(fdSocket, &msg, 0) <= 0){ printf("recvmsg consem failed\n"); return; }
    cmsg = CMSG_FIRSTHDR(&msg);
    memcpy(&details->fdUpdateConsumed, CMSG_DATA(cmsg), sizeof(int));
}
#endif

const char *twExternalSemaphoreDeviceExtName(){
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    return VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
#elif defined(__linux__)
    return VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
#elif
    return NULL;
#endif
}

const char *twExternalMemoryDeviceExtName(){
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
    return VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
#elif defined(__linux__)
    return VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
#elif
    return NULL; // remove this once adapted
#endif
}
