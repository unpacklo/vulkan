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

int main(int argc, char* argv[])
{
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

  HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, hInstance, NULL);

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
  VK_CHECK(vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices));
  printf("%u physical device(s) found!\n", num_physical_devices);

  for (uint32_t i = 0; i < num_physical_devices; ++i)
  {
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
    printf("physical_devices[%u]:\n", i);
    printf("  Vulkan API %u.%u.%u\n", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
    printf("  Vulkan driver version: %u\n", properties.driverVersion);
    printf("  Vendor: %u\n", properties.vendorID);
    printf("  Device: %u\n", properties.deviceID);
    printf("  Device type: %u\n", properties.deviceType);
    printf("  Device name: %s\n", properties.deviceName);
    printf("\n");
  }

  uint32_t num_queue_properties = 16;
  VkQueueFamilyProperties queue_properties[16] = {};
  vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[0], &num_queue_properties, queue_properties);

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
  VK_CHECK(vkCreateDevice(physical_devices[0], &device_create_info, &callbacks, &device));

  VkQueue queue = {};
  vkGetDeviceQueue(device, queue_family_index, 0, &queue);

  VkWin32SurfaceCreateInfoKHR surface_create_info = {};
  surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_create_info.hinstance = hInstance;
  surface_create_info.hwnd = GetActiveWindow();
  VkSurfaceKHR surface = {};
  VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surface_create_info, &callbacks, &surface));

  VkSurfaceCapabilitiesKHR surface_capabilities = {};
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_devices[0], surface, &surface_capabilities));

  VkBool32 surface_supported = VK_FALSE;
  VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[0], queue_family_index, surface, &surface_supported));
  printf("Surface %#p supported: %s\n", surface, surface_supported == VK_TRUE ? "true" : "false");

  VkSwapchainCreateInfoKHR swapchain_create_info = {};
  swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_create_info.surface = surface;
  swapchain_create_info.minImageCount = 2;
  swapchain_create_info.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  swapchain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  swapchain_create_info.imageExtent = surface_capabilities.currentExtent;
  swapchain_create_info.imageArrayLayers = 1;
  swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_create_info.queueFamilyIndexCount = 1;
  swapchain_create_info.pQueueFamilyIndices = &queue_family_index;
  swapchain_create_info.preTransform = surface_capabilities.currentTransform;
  swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_create_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

  VkSwapchainKHR swapchain = {};  
  vkCreateSwapchainKHR(device, &swapchain_create_info, &callbacks, &swapchain);

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

  vkDestroySwapchainKHR(device, swapchain, &callbacks);
  vkDestroySurfaceKHR(instance, surface, &callbacks);
  vkDestroyDevice(device, &callbacks);
  vkDestroyInstance(instance, &callbacks);

  return 0;
}