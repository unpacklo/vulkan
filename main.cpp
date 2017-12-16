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
  case WM_KEYDOWN:
  {
    switch (wParam)
    {
    case VK_ESCAPE:
    {
      PostQuitMessage(0);
      return 0;
    }
    default:
    {
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    }
    break;
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

  uint32_t surface_formats_count;
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_formats_count, nullptr)); // WTF, Intel needs this function call to get the count first... is this a bug?
  if (surface_formats_count > 32)
  {
    surface_formats_count = 32;
  }
  VkSurfaceFormatKHR surface_formats[32] = {};
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_formats_count, surface_formats));

  for (uint32_t i = 0; i < surface_formats_count; ++i)
  {
    printf("surface_formats[%u]:\n", i);
    printf("  format = %d\n", surface_formats[i].format);
    printf("  colorspace = %d\n", surface_formats[i].colorSpace);
  }

  VkSurfaceFormatKHR surface_format = surface_formats[0];

  uint32_t present_modes_count;
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, nullptr)); // WTF, Intel needs this function call to get the count first... is this a bug?
  if (present_modes_count > 32)
  {
    present_modes_count = 32;
  }
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
  swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
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
  cmd_buffer_alloc_info.commandBufferCount = 2;
  VkCommandBuffer clear_color_cmd[2] = {};
  VK_CHECK(vkAllocateCommandBuffers(device, &cmd_buffer_alloc_info, clear_color_cmd));

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

  VkDescriptorSetLayoutBinding layout_binding = {};
  layout_binding.binding = 0;
  layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layout_binding.descriptorCount = 1;
  layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  layout_binding.pImmutableSamplers = NULL;

  VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
  descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptor_layout.pNext = nullptr;
  descriptor_layout.bindingCount = 1;
  descriptor_layout.pBindings = &layout_binding;

  VkDescriptorSetLayout desc_layout = {};
  VK_CHECK(vkCreateDescriptorSetLayout(device, &descriptor_layout, &callbacks, &desc_layout));

  VkPipelineLayout pipeline_layout = {};
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
  pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_create_info.pNext = nullptr;
  pipeline_layout_create_info.pushConstantRangeCount = 0;
  pipeline_layout_create_info.pPushConstantRanges = nullptr;
  pipeline_layout_create_info.setLayoutCount = 1;
  pipeline_layout_create_info.pSetLayouts = &desc_layout;
  VK_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_create_info, &callbacks, &pipeline_layout));

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

  VkDynamicState dynamic_state_enables[VK_DYNAMIC_STATE_RANGE_SIZE] = {};
  VkPipelineDynamicStateCreateInfo dynamic_create_info = {};
  dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_create_info.pNext = nullptr;
  dynamic_create_info.pDynamicStates = dynamic_state_enables;
  dynamic_create_info.dynamicStateCount = 0;

  VkVertexInputBindingDescription vi_binding = {};
  VkVertexInputAttributeDescription vi_attribs[2] = {};
  VkPipelineVertexInputStateCreateInfo vi = {};
  vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vi.pNext = nullptr;
  vi.flags = 0;
  vi.vertexBindingDescriptionCount = 1;
  vi.pVertexBindingDescriptions = &vi_binding;
  vi.vertexAttributeDescriptionCount = 2;
  vi.pVertexAttributeDescriptions = vi_attribs;

  VkPipelineInputAssemblyStateCreateInfo ia = {};
  ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  ia.pNext = nullptr;
  ia.flags = 0;
  ia.primitiveRestartEnable = VK_FALSE;
  ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineRasterizationStateCreateInfo rs = {};
  rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rs.pNext = nullptr;
  rs.flags = 0;
  rs.polygonMode = VK_POLYGON_MODE_FILL;
  rs.cullMode = VK_CULL_MODE_BACK_BIT;
  rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rs.depthClampEnable = VK_FALSE;
  rs.rasterizerDiscardEnable = VK_FALSE;
  rs.depthBiasEnable = VK_FALSE;
  rs.depthBiasConstantFactor = 0;
  rs.depthBiasClamp = 0;
  rs.depthBiasSlopeFactor = 0;
  rs.lineWidth = 1.0f;

  VkPipelineColorBlendStateCreateInfo cb = {};
  cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  cb.pNext = nullptr;
  cb.flags = 0;

  VkPipelineColorBlendAttachmentState att_state[1] = {};
  att_state[0].colorWriteMask = 0xf;
  att_state[0].blendEnable = VK_FALSE;
  att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
  att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
  att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  cb.attachmentCount = 1;
  cb.pAttachments = att_state;
  cb.logicOpEnable = VK_FALSE;
  cb.logicOp = VK_LOGIC_OP_NO_OP;
  cb.blendConstants[0] = 1.0f;
  cb.blendConstants[1] = 1.0f;
  cb.blendConstants[2] = 1.0f;
  cb.blendConstants[3] = 1.0f;

  VkPipelineViewportStateCreateInfo vp = {};
  vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  vp.pNext = nullptr;
  vp.flags = 0;
  vp.viewportCount = 1;
  dynamic_state_enables[dynamic_create_info.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
  vp.scissorCount = 1;
  dynamic_state_enables[dynamic_create_info.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
  vp.pScissors = nullptr;
  vp.pViewports = nullptr;

  VkPipelineDepthStencilStateCreateInfo ds = {};
  ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  ds.pNext = nullptr;
  ds.flags = 0;
  ds.depthTestEnable = VK_TRUE;
  ds.depthWriteEnable = VK_TRUE;
  ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  ds.depthBoundsTestEnable = VK_FALSE;
  ds.minDepthBounds = 0;
  ds.maxDepthBounds = 0;
  ds.stencilTestEnable = VK_FALSE;
  ds.back.failOp = VK_STENCIL_OP_KEEP;
  ds.back.passOp = VK_STENCIL_OP_KEEP;
  ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
  ds.back.compareMask = 0;
  ds.back.reference = 0;
  ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
  ds.back.writeMask = 0;
  ds.front = ds.back;

  VkPipelineMultisampleStateCreateInfo ms = {};
  ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  ms.pNext = nullptr;
  ms.flags = 0;
  ms.pSampleMask = nullptr;
  ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  ms.sampleShadingEnable = VK_FALSE;
  ms.alphaToCoverageEnable = VK_FALSE;
  ms.alphaToOneEnable = VK_FALSE;
  ms.minSampleShading = 0.0;

  VkPipeline pipeline = {};
  VkGraphicsPipelineCreateInfo pipeline_create_info = {};
  pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_create_info.pNext = nullptr;
  pipeline_create_info.layout = pipeline_layout;
  pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_create_info.basePipelineIndex = 0;
  pipeline_create_info.flags = 0;
  pipeline_create_info.pVertexInputState = &vi;
  pipeline_create_info.pInputAssemblyState = &ia;
  pipeline_create_info.pRasterizationState = &rs;
  pipeline_create_info.pColorBlendState = &cb;
  pipeline_create_info.pTessellationState = nullptr;
  pipeline_create_info.pMultisampleState = &ms;
  pipeline_create_info.pDynamicState = &dynamic_create_info;
  pipeline_create_info.pViewportState = &vp;
  pipeline_create_info.pDepthStencilState = &ds;
  pipeline_create_info.pStages = nullptr;
  pipeline_create_info.stageCount = 0;
  pipeline_create_info.renderPass = render_pass;
  pipeline_create_info.subpass = 0;
  //VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, &callbacks, &pipeline));

  VkClearValue clear_values_white[2] = {};
  clear_values_white[0].color.float32[0] = 1.0f;
  clear_values_white[0].color.float32[1] = 1.0f;
  clear_values_white[0].color.float32[2] = 1.0f;
  clear_values_white[0].color.float32[3] = 1.0f;
  clear_values_white[1].depthStencil.depth = 1.0f;
  clear_values_white[1].depthStencil.stencil = 0;

  VkClearValue clear_values_black[2] = {};
  clear_values_black[0].color.float32[0] = 0.0f;
  clear_values_black[0].color.float32[1] = 0.0f;
  clear_values_black[0].color.float32[2] = 0.0f;
  clear_values_black[0].color.float32[3] = 0.0f;
  clear_values_black[1].depthStencil.depth = 1.0f;
  clear_values_black[1].depthStencil.stencil = 0;
  
  VkSemaphore img_acq_sem = {};
  VkSemaphoreCreateInfo sem_create_info = {};
  sem_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VK_CHECK(vkCreateSemaphore(device, &sem_create_info, &callbacks, &img_acq_sem));

// typedef struct VkFenceCreateInfo {
//     VkStructureType       sType;
//     const void*           pNext;
//     VkFenceCreateFlags    flags;
// } VkFenceCreateInfo;

  VkFence submit_fence = {};
  VkFenceCreateInfo fence_create_info = {};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  VK_CHECK(vkCreateFence(device, &fence_create_info, &callbacks, &submit_fence));

  uint32_t current_buffer = {};
  //VK_CHECK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, img_acq_sem, VK_NULL_HANDLE, &current_buffer)); // There's a bug here in the validation layer where if you did not get the swapchain images, you will crash!

  VkRenderPassBeginInfo rp_begin = {};
  rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rp_begin.pNext = nullptr;
  rp_begin.renderPass = render_pass;
  rp_begin.framebuffer = framebuffers[current_buffer];
  rp_begin.renderArea.offset.x = 0;
  rp_begin.renderArea.offset.y = 0;
  rp_begin.renderArea.extent.width = 1264;
  rp_begin.renderArea.extent.height = 681;
  rp_begin.clearValueCount = 2;
  rp_begin.pClearValues = clear_values_white;

  VkCommandBufferBeginInfo cmd_buf_info = {};
  cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_buf_info.pNext = nullptr;
  cmd_buf_info.flags = 0;
  cmd_buf_info.pInheritanceInfo = nullptr;

  VkDescriptorPoolSize type_count = {};
  type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  type_count.descriptorCount = 1;

  VkDescriptorPoolCreateInfo descriptor_pool = {};
  descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptor_pool.pNext = nullptr;
  descriptor_pool.maxSets = 1;
  descriptor_pool.poolSizeCount = 1;
  descriptor_pool.pPoolSizes = &type_count;
  VkDescriptorPool desc_pool = {};
  VK_CHECK(vkCreateDescriptorPool(device, &descriptor_pool, &callbacks, &desc_pool));

  VkDescriptorSetAllocateInfo desc_alloc_info = {};
  desc_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  desc_alloc_info.pNext = nullptr;
  desc_alloc_info.descriptorPool = desc_pool;
  desc_alloc_info.descriptorSetCount = 1;
  desc_alloc_info.pSetLayouts = &desc_layout;
  VkDescriptorSet desc_set = {};
  VK_CHECK(vkAllocateDescriptorSets(device, &desc_alloc_info, &desc_set));

  VkWriteDescriptorSet writes = {};
  writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes.pNext = nullptr;
  writes.dstSet = desc_set;
  writes.descriptorCount = 1;
  writes.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writes.pBufferInfo = &buffer_info;
  writes.dstArrayElement = 0;
  writes.dstBinding = 0;
  vkUpdateDescriptorSets(device, 1, &writes, 0, nullptr);

  VkClearColorValue clear_color_value_black = {};
  VkClearColorValue clear_color_value_white = {};
  clear_color_value_white.float32[0] = 1.0f;
  clear_color_value_white.float32[1] = 1.0f;
  clear_color_value_white.float32[2] = 1.0f;
  clear_color_value_white.float32[3] = 1.0f;

  VkImageSubresourceRange swapchain_image_subresource_range = {};
  swapchain_image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  swapchain_image_subresource_range.baseMipLevel = 0;
  swapchain_image_subresource_range.levelCount = 1;
  swapchain_image_subresource_range.baseArrayLayer = 0;
  swapchain_image_subresource_range.layerCount = 1;

// typedef struct VkImageMemoryBarrier {
//     VkStructureType            sType;
//     const void*                pNext;
//     VkAccessFlags              srcAccessMask;
//     VkAccessFlags              dstAccessMask;
//     VkImageLayout              oldLayout;
//     VkImageLayout              newLayout;
//     uint32_t                   srcQueueFamilyIndex;
//     uint32_t                   dstQueueFamilyIndex;
//     VkImage                    image;
//     VkImageSubresourceRange    subresourceRange;
// } VkImageMemoryBarrier;

  // Set up the image layout transition.
  VkImageMemoryBarrier img_mem_barrier = {};
  img_mem_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  img_mem_barrier.pNext = nullptr;
  img_mem_barrier.srcQueueFamilyIndex = queue_family_index;
  img_mem_barrier.dstQueueFamilyIndex = queue_family_index;
  img_mem_barrier.subresourceRange = swapchain_image_subresource_range;

  // Set up the command buffer for clearing the framebuffers.
  VK_CHECK(vkBeginCommandBuffer(clear_color_cmd[0], &cmd_buf_info));
  img_mem_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  img_mem_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  img_mem_barrier.image = swapchain_images[0];
  vkCmdPipelineBarrier(clear_color_cmd[0], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_mem_barrier);
  vkCmdClearColorImage(clear_color_cmd[0], swapchain_images[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color_value_white, 1, &swapchain_image_subresource_range);
  img_mem_barrier.srcAccessMask = 0;
  img_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  img_mem_barrier.image = swapchain_images[0];
  vkCmdPipelineBarrier(clear_color_cmd[0], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_mem_barrier);
  VK_CHECK(vkEndCommandBuffer(clear_color_cmd[0]));

  VK_CHECK(vkBeginCommandBuffer(clear_color_cmd[1], &cmd_buf_info));
  img_mem_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  img_mem_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  img_mem_barrier.image = swapchain_images[1];
  vkCmdPipelineBarrier(clear_color_cmd[1], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_mem_barrier);
  vkCmdClearColorImage(clear_color_cmd[1], swapchain_images[1], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color_value_black, 1, &swapchain_image_subresource_range);
  img_mem_barrier.srcAccessMask = 0;
  img_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  img_mem_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  img_mem_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  img_mem_barrier.image = swapchain_images[1];
  vkCmdPipelineBarrier(clear_color_cmd[1], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_mem_barrier);
  VK_CHECK(vkEndCommandBuffer(clear_color_cmd[1]));

// typedef struct VkSubmitInfo {
//     VkStructureType                sType;
//     const void*                    pNext;
//     uint32_t                       waitSemaphoreCount;
//     const VkSemaphore*             pWaitSemaphores;
//     const VkPipelineStageFlags*    pWaitDstStageMask;
//     uint32_t                       commandBufferCount;
//     const VkCommandBuffer*         pCommandBuffers;
//     uint32_t                       signalSemaphoreCount;
//     const VkSemaphore*             pSignalSemaphores;
// } VkSubmitInfo;

  VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &img_acq_sem;
  submit_info.pWaitDstStageMask = &wait_dst_stage_mask;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = clear_color_cmd;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &img_acq_sem;

// typedef struct VkPresentInfoKHR {
//     VkStructureType          sType;
//     const void*              pNext;
//     uint32_t                 waitSemaphoreCount;
//     const VkSemaphore*       pWaitSemaphores;
//     uint32_t                 swapchainCount;
//     const VkSwapchainKHR*    pSwapchains;
//     const uint32_t*          pImageIndices;
//     VkResult*                pResults;
// } VkPresentInfoKHR;

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.pNext = nullptr;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &img_acq_sem;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain;
  present_info.pImageIndices = &current_buffer;
  present_info.pResults = nullptr;

  MSG msg;
  bool running = true;
  int frame = 0;

  while (running)
  {
    // if (!(frame % (1000)))
    // {
    //   printf("Frame %d\n", frame);
    // }

    ++frame;
    if (VK_SUCCESS == vkAcquireNextImageKHR(device, swapchain, 0, img_acq_sem, VK_NULL_HANDLE, &current_buffer))
    {
      submit_info.pCommandBuffers = clear_color_cmd + current_buffer;
      VK_CHECK(vkWaitForFences(device, 1, &submit_fence, VK_TRUE, UINT64_MAX));
      VK_CHECK(vkResetFences(device, 1, &submit_fence));
      VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, submit_fence));
      VK_CHECK(vkQueuePresentKHR(queue, &present_info));
    }

    while (BOOL message_result = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
    {
      if (message_result == -1)
      {
        // handle the error and possibly exit
        running = false;
        break;
      }
      else if (msg.message == WM_QUIT)
      {
        running = false;
        break;
      }
      else
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }

  // Wait for the last submission to flush before destroying everything.
  VK_CHECK(vkWaitForFences(device, 1, &submit_fence, VK_TRUE, UINT64_MAX));
  VK_CHECK(vkResetFences(device, 1, &submit_fence));

  vkDestroyDescriptorPool(device, desc_pool, &callbacks);
  for (uint32_t i = 0; i < swapchain_image_count; ++i)
  {
    vkDestroyFramebuffer(device, framebuffers[i], &callbacks);
  }
  vkDestroyRenderPass(device, render_pass, &callbacks);
  vkDestroyFence(device, submit_fence, &callbacks);
  vkDestroySemaphore(device, img_acq_sem, &callbacks);
  vkFreeMemory(device, uniform_device_memory, &callbacks);
  vkDestroyPipelineLayout(device, pipeline_layout, &callbacks);
  vkDestroyDescriptorSetLayout(device, desc_layout, &callbacks);
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

  DestroyWindow(hwnd);
  getchar();

  return 0;
}
