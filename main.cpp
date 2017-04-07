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
  VkResult result = vkCreateInstance(&info, &callbacks, &instance);

  if (result == VK_SUCCESS)
  {
    printf("Hello vulkan!\n");
  }

  getchar();

  return 0;
}