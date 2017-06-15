#include <cstdio>
#include <cstdlib>
#include <malloc.h>
#include <vulkan/vulkan.h>

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

static uint32_t VulkanMajorVersion(uint32_t api_version)
{
  return api_version >> 22;
}

static uint32_t VulkanMinorVersion(uint32_t api_version)
{
  return (api_version << 10) >> 22;
}

static uint32_t VulkanPatchVersion(uint32_t api_version)
{
  return (api_version << 22) >> 22;
}

int main()
{
  VkInstanceCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

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
    printf("  Vulkan API %u.%u.%u\n", VulkanMajorVersion(properties.apiVersion), VulkanMinorVersion(properties.apiVersion), VulkanPatchVersion(properties.apiVersion));
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

  VkDevice device = {};
  VkDeviceQueueCreateInfo queue_create_info = {};
  VkDeviceCreateInfo device_create_info = {};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.pQueueCreateInfos = &queue_create_info;
  VK_CHECK(vkCreateDevice(physical_devices[0], &device_create_info, &callbacks, &device));

  vkDestroyDevice(device, &callbacks);
  vkDestroyInstance(instance, &callbacks);
  printf("Vulkan succeeded!\n");
  getchar();

  return 0;
}