#include <cstdio>
#include <cstdlib>
#include <malloc.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <Windows.h>
#include <tchar.h>

void* VulkanAlignedAlloc(void* userdata, size_t bytes, size_t alignment, VkSystemAllocationScope alloc_scope)
{
  return _aligned_malloc(bytes, alignment);
}

void* VulkanRealloc(void* userdata, void* ptr, size_t bytes, size_t alignment, VkSystemAllocationScope alloc_scope)
{
  return _aligned_realloc(ptr, bytes, alignment);
}

void VulkanFree(void* userdata, void* ptr)
{
  return _aligned_free(ptr);
}

void VulkanInternalAllocNotify(void* userdata, size_t bytes, VkInternalAllocationType alloc_type, VkSystemAllocationScope alloc_scope)
{
}

void VulkanInternalFreeNotify(void* userdata, size_t bytes, VkInternalAllocationType alloc_type, VkSystemAllocationScope alloc_scope)
{
}

#define VK_CHECK(vk_result)  do { VkResult result = (vk_result); if (result != VK_SUCCESS) { printf("%s:%d got %d!\n", __FILE__, __LINE__, result); getchar(); return 1; } } while(0)
#define ARRAY_COUNT(a)  (sizeof(a) / sizeof(a[0]))

const char* const g_EnabledInstanceExtensions[] =
{
  VK_KHR_SURFACE_EXTENSION_NAME,
  VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
};

const char* const g_EnabledDeviceExtensions[] =
{
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const char* const g_EnabledValidationLayers[] =
{
  "VK_LAYER_LUNARG_standard_validation",
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_DESTROY:
  {
    PostQuitMessage(0);
    return 0;
  }
  default:
  {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
  }
}

struct Mat4
{
  float m[16];
};

int main(int argc, char* argv[])
{
  printf("Vulkan header version: %u\n", VK_HEADER_VERSION);
  int width = 1280;
  int height = 720;
  static TCHAR szWindowClass[] = _T("vulkan");
  static TCHAR szTitle[] = _T("Vulkan");
  HINSTANCE hInstance = GetModuleHandle(NULL);
  WNDCLASSEX wcex = {};

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

  if (!RegisterClassEx(&wcex))
  {
    MessageBox(NULL,
      _T("Call to RegisterClassEx failed!"),
      _T("Win32 Guided Tour"),
      NULL);

    return 1;
  }

  HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

  if (hwnd == NULL)
  {
    char buffer[512] = {};
    DWORD result = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buffer, sizeof(buffer), NULL);
    printf("%s\n", buffer);
    getchar();
    return 1;
  }

  ShowWindow(hwnd, SW_SHOW);

  VkInstanceCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  info.enabledExtensionCount = ARRAY_COUNT(g_EnabledInstanceExtensions);
  info.ppEnabledExtensionNames = g_EnabledInstanceExtensions;
  info.enabledLayerCount = ARRAY_COUNT(g_EnabledValidationLayers);
  info.ppEnabledLayerNames = g_EnabledValidationLayers;

  VkAllocationCallbacks callbacks = {};
  callbacks.pfnAllocation = VulkanAlignedAlloc;
  callbacks.pfnReallocation = VulkanRealloc;
  callbacks.pfnFree = VulkanFree;
  callbacks.pfnInternalAllocation = VulkanInternalAllocNotify;
  callbacks.pfnInternalFree = VulkanInternalFreeNotify;

  VkInstance instance = {};
  VK_CHECK(vkCreateInstance(&info, &callbacks, &instance));

  uint32_t num_physical_devices = 8;
  VkPhysicalDevice physical_devices[8] = {};
  VkPhysicalDeviceMemoryProperties memory_properties[8] = {};
  VK_CHECK(vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices));
  printf("%u physical device(s) found!\n", num_physical_devices);

  for (uint32_t i = 0; i < num_physical_devices; ++i)
  {
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
    printf("physical_devices[%u]:\n", i);
    printf("  Vulkan API %u.%u.%u\n", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
    printf("  Vulkan driver version: %u.%u.%u\n", VK_VERSION_MAJOR(properties.driverVersion), VK_VERSION_MINOR(properties.driverVersion), VK_VERSION_PATCH(properties.driverVersion));
    printf("  Vendor: %u\n", properties.vendorID);
    printf("  Device: %u\n", properties.deviceID);
    printf("  Device type: %u\n", properties.deviceType);
    printf("  Device name: %s\n", properties.deviceName);
    printf("\n");
    vkGetPhysicalDeviceMemoryProperties(physical_devices[i], memory_properties + i);
  }

  VkPhysicalDevice physical_device = physical_devices[0];
  uint32_t num_queue_properties = 16;
  VkQueueFamilyProperties queue_properties[16] = {};
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_properties, queue_properties);

  for (uint32_t i = 0; i < num_queue_properties; ++i)
  {
    printf("queue_properties[%u]:\n", i);
    printf("  Flags: %u\n", queue_properties[i].queueFlags);
    printf("  Queue count: %u\n", queue_properties[i].queueCount);
    printf("  Min image transfer granularity: (%u, %u, %u)\n", queue_properties[i].minImageTransferGranularity.width, queue_properties[i].minImageTransferGranularity.height, queue_properties[i].minImageTransferGranularity.depth);
  }

  float queue_priorities[32] = {};
  uint32_t queue_family_index = 0;
  VkDeviceQueueCreateInfo queue_create_info = {};
  queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = queue_family_index;
  queue_create_info.queueCount = queue_properties[0].queueCount;
  queue_create_info.pQueuePriorities = queue_priorities;

  VkDeviceCreateInfo device_create_info = {};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.pQueueCreateInfos = &queue_create_info;
  device_create_info.enabledExtensionCount = ARRAY_COUNT(g_EnabledDeviceExtensions);
  device_create_info.ppEnabledExtensionNames = g_EnabledDeviceExtensions;

  VkDevice device = {};
  VK_CHECK(vkCreateDevice(physical_device, &device_create_info, &callbacks, &device));

  VkQueue queue = {};
  vkGetDeviceQueue(device, queue_family_index, 0, &queue);

  VkWin32SurfaceCreateInfoKHR surface_create_info = {};
  surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_create_info.hinstance = hInstance;
  surface_create_info.hwnd = GetActiveWindow();
  VkSurfaceKHR surface = {};
  VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surface_create_info, &callbacks, &surface));

  VkSurfaceCapabilitiesKHR surface_capabilities = {};
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities));

  VkBool32 surface_supported = VK_FALSE;
  VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, surface, &surface_supported));
  printf("\n");
  printf("Surface %#p supported: %s\n", surface, surface_supported == VK_TRUE ? "true" : "false");
  printf("\n");

  uint32_t surface_formats_count = 32;
  VkSurfaceFormatKHR surface_formats[32] = {};
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_formats_count, surface_formats));

  for (uint32_t i = 0; i < surface_formats_count; ++i)
  {
    printf("surface_formats[%u]:\n", i);
    printf("  format = %d\n", surface_formats[i].format);
    printf("  colorspace = %d\n", surface_formats[i].colorSpace);
  }

  VkSurfaceFormatKHR surface_format = surface_formats[0];

  uint32_t present_modes_count = 32;
  VkPresentModeKHR present_modes[32] = {};
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, present_modes));

  printf("\n");
  for (uint32_t i = 0; i < present_modes_count; ++i)
  {
    printf("present_modes[%u] = %d\n", i, present_modes[i]);
  }

  VkSwapchainCreateInfoKHR swapchain_create_info = {};
  swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_create_info.surface = surface;
  swapchain_create_info.minImageCount = 2;
  swapchain_create_info.imageFormat = surface_format.format;
  swapchain_create_info.imageColorSpace = surface_format.colorSpace;
  swapchain_create_info.imageExtent = surface_capabilities.currentExtent;
  swapchain_create_info.imageArrayLayers = 1;
  swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_create_info.queueFamilyIndexCount = 1;
  swapchain_create_info.pQueueFamilyIndices = &queue_family_index;
  swapchain_create_info.preTransform = surface_capabilities.currentTransform;
  swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_create_info.presentMode = present_modes[0];

  VkSwapchainKHR swapchain = {};
  VK_CHECK(vkCreateSwapchainKHR(device, &swapchain_create_info, &callbacks, &swapchain));

  uint32_t swapchain_image_count = {};
  VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, nullptr));

  VkImage swapchain_images[8] = {};
  VkImageView swapchain_image_views[8] = {};
  VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images));

  for (uint32_t i = 0; i < swapchain_image_count; ++i)
  {
    VkImageViewCreateInfo color_image_view = {};
    color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    color_image_view.pNext = nullptr;
    color_image_view.flags = 0;
    color_image_view.image = swapchain_images[i];
    color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    color_image_view.format = surface_format.format;
    color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
    color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
    color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
    color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
    color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_image_view.subresourceRange.baseMipLevel = 0;
    color_image_view.subresourceRange.levelCount = 1;
    color_image_view.subresourceRange.baseArrayLayer = 0;
    color_image_view.subresourceRange.layerCount = 1;
    VK_CHECK(vkCreateImageView(device, &color_image_view, &callbacks, swapchain_image_views + i));
  }

  VkCommandPoolCreateInfo cmd_pool_info = {};
  cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmd_pool_info.queueFamilyIndex = queue_family_index;
  VkCommandPool cmd_pool = {};
  VK_CHECK(vkCreateCommandPool(device, &cmd_pool_info, &callbacks, &cmd_pool));

  VkCommandBufferAllocateInfo cmd_buffer_alloc_info = {};
  cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd_buffer_alloc_info.commandPool = cmd_pool;
  cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd_buffer_alloc_info.commandBufferCount = 1;
  VkCommandBuffer cmd_buffer = {};
  VK_CHECK(vkAllocateCommandBuffers(device, &cmd_buffer_alloc_info, &cmd_buffer));

  VkImageCreateInfo image_create_info = {};
  image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_create_info.imageType = VK_IMAGE_TYPE_2D;
  image_create_info.format = VK_FORMAT_D16_UNORM;
  image_create_info.extent.width = width;
  image_create_info.extent.height = height;
  image_create_info.extent.depth = 1;
  image_create_info.mipLevels = 1;
  image_create_info.arrayLayers = 1;
  image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VkImage depth_buffer = {};
  VK_CHECK(vkCreateImage(device, &image_create_info, &callbacks, &depth_buffer));

  VkMemoryRequirements mem_reqs = {};
  vkGetImageMemoryRequirements(device, depth_buffer, &mem_reqs);
  VkMemoryAllocateInfo mem_alloc_info = {};
  mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_alloc_info.allocationSize = mem_reqs.size;

  for (uint32_t i = 0; i < memory_properties[0].memoryTypeCount; ++i)
  {
    // wtf is this check doing?  Why does this work?
    bool is_supported_memory_type = (mem_reqs.memoryTypeBits & (1 << i)) > 0;
    if (is_supported_memory_type)
    {
      if ((memory_properties[0].memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
      {
        mem_alloc_info.memoryTypeIndex = i;
      }
    }
  }

  VkDeviceMemory depth_buffer_memory = {};
  VK_CHECK(vkAllocateMemory(device, &mem_alloc_info, &callbacks, &depth_buffer_memory));
  VK_CHECK(vkBindImageMemory(device, depth_buffer, depth_buffer_memory, 0));
  VkImageViewCreateInfo depth_view_info = {};
  depth_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  depth_view_info.image = depth_buffer;
  depth_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  depth_view_info.format = VK_FORMAT_D16_UNORM;
  depth_view_info.components.r = VK_COMPONENT_SWIZZLE_R;
  depth_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
  depth_view_info.components.b = VK_COMPONENT_SWIZZLE_B;
  depth_view_info.components.a = VK_COMPONENT_SWIZZLE_A;
  depth_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  depth_view_info.subresourceRange.baseMipLevel = 0;
  depth_view_info.subresourceRange.levelCount = 1;
  depth_view_info.subresourceRange.baseArrayLayer = 0;
  depth_view_info.subresourceRange.layerCount = 1;
  VkImageView depth_image_view = {};
  VK_CHECK(vkCreateImageView(device, &depth_view_info, &callbacks, &depth_image_view));

  VkBufferCreateInfo buffer_create_info = {};
  buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  buffer_create_info.size = sizeof(Mat4);
  buffer_create_info.queueFamilyIndexCount = 0;
  buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VkBuffer uniform_buffer = {};
  VK_CHECK(vkCreateBuffer(device, &buffer_create_info, &callbacks, &uniform_buffer));

  VkMemoryRequirements memory_requirements = {};
  vkGetBufferMemoryRequirements(device, uniform_buffer, &memory_requirements);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = memory_requirements.size;

  VkFlags required_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  // Look for a memory type that matches what we're looking for.
  for (uint32_t i = 0; i < memory_properties[0].memoryTypeCount; ++i)
  {
    if ((memory_requirements.memoryTypeBits & (1 << i)) && ((memory_properties[0].memoryTypes[i].propertyFlags & required_mask) == required_mask))
    {
      alloc_info.memoryTypeIndex = i;
      break;
    }
  }
  
  VkDeviceMemory uniform_device_memory = {};
  VK_CHECK(vkAllocateMemory(device, &alloc_info, &callbacks, &uniform_device_memory));

  uint8_t* mapped_uniform_data = NULL;
  VK_CHECK(vkMapMemory(device, uniform_device_memory, 0, memory_requirements.size, 0, (void**)&mapped_uniform_data));

  Mat4 mvp = {};
  mvp.m[0] = 1.0f;
  mvp.m[5] = 1.0f;
  mvp.m[10] = 1.0f;
  mvp.m[15] = 1.0f;
  memmove(mapped_uniform_data, &mvp, sizeof(mvp));
  vkUnmapMemory(device, uniform_device_memory);

  VK_CHECK(vkBindBufferMemory(device, uniform_buffer, uniform_device_memory, 0));

  VkDescriptorBufferInfo buffer_info = {};
  buffer_info.buffer = uniform_buffer;
  buffer_info.offset = 0;
  buffer_info.range = sizeof(mvp);

  VkSemaphore img_acq_sem = {};
  VkSemaphoreCreateInfo sem_create_info = {};
  sem_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VK_CHECK(vkCreateSemaphore(device, &sem_create_info, &callbacks, &img_acq_sem));

  uint32_t current_buffer = {};
  VK_CHECK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, img_acq_sem, VK_NULL_HANDLE, &current_buffer)); // There's a bug here in the validation layer where if you did not get the swapchain images, you will crash!

  VkAttachmentDescription attachments[2] = {};
  attachments[0].format = surface_format.format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  attachments[1].format = VK_FORMAT_D16_UNORM;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference color_reference = {};
  color_reference.attachment = 0;
  color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_reference = {};
  depth_reference.attachment = 1;
  depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.flags = 0;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = nullptr;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_reference;
  subpass.pResolveAttachments = nullptr;
  subpass.pDepthStencilAttachment = &depth_reference;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = nullptr;

  VkRenderPassCreateInfo rp_info = {};
  rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rp_info.attachmentCount = 2;
  rp_info.pAttachments = attachments;
  rp_info.subpassCount = 1;
  rp_info.pSubpasses = &subpass;
  rp_info.dependencyCount = 0;
  rp_info.pDependencies = nullptr;

  VkRenderPass render_pass = {};
  VK_CHECK(vkCreateRenderPass(device, &rp_info, &callbacks, &render_pass));

  VkImageView framebuffer_attachments[2] = {};
  framebuffer_attachments[1] = depth_image_view;

  VkFramebufferCreateInfo fb_info = {};
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = nullptr;
  fb_info.renderPass = render_pass;
  fb_info.attachmentCount = 2;
  fb_info.pAttachments = framebuffer_attachments;
  fb_info.width = 1264; // This is so fail, is the windows title bar eating up space?
  fb_info.height = 681;
  fb_info.layers = 1;

  VkFramebuffer framebuffers[2] = {};

  for (uint32_t i = 0; i < swapchain_image_count; ++i)
  {
    framebuffer_attachments[0] = swapchain_image_views[i];
    VK_CHECK(vkCreateFramebuffer(device, &fb_info, &callbacks, framebuffers + i));
  }
  
  MSG msg;

  while (BOOL message_result = GetMessage(&msg, NULL, 0, 0))
  {
    if (message_result == -1)
    {
      // handle the error and possibly exit
      break;
    }
    else
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  for (uint32_t i = 0; i < swapchain_image_count; ++i)
  {
    vkDestroyFramebuffer(device, framebuffers[i], &callbacks);
  }
  vkDestroyRenderPass(device, render_pass, &callbacks);
  vkDestroySemaphore(device, img_acq_sem, &callbacks);
  vkFreeMemory(device, uniform_device_memory, &callbacks);
  vkDestroyBuffer(device, uniform_buffer, &callbacks);
  vkDestroyImageView(device, depth_image_view, &callbacks);
  vkFreeMemory(device, depth_buffer_memory, &callbacks);
  vkDestroyImage(device, depth_buffer, &callbacks);
  vkDestroyCommandPool(device, cmd_pool, &callbacks);
  for (uint32_t i = 0; i < swapchain_image_count; ++i)
  {
    vkDestroyImageView(device, swapchain_image_views[i], &callbacks);
  }
  vkDestroySwapchainKHR(device, swapchain, &callbacks);
  vkDestroySurfaceKHR(instance, surface, &callbacks);
  vkDestroyDevice(device, &callbacks);
  vkDestroyInstance(instance, &callbacks);

  getchar();

  return 0;
}