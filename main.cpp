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

#define VK_CHECK_RET1_ON_FAIL(vk_result)  do { VkResult result = (vk_result); if (result != VK_SUCCESS) { printf("%s:%d got %d!\n", __FILE__, __LINE__, result); getchar(); return 1; } } while(0)

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
  VK_CHECK_RET1_ON_FAIL(vkCreateInstance(&info, &callbacks, &instance));

  //const int MAX_PHYSICAL_DEVICES = 8;
  //VkPhysicalDevice physical_devices[MAX_PHYSICAL_DEVICES] = {};
  //result = VkEnumeratePhysicalDevices(instance, MAX_PHYSICAL_DEVICES, physical_devices);

  printf("Vulkan succeeded!\n");
  getchar();

  return 0;
}