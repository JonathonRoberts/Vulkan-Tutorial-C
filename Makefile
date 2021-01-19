CC = clang
VULKAN_SDK_PATH = ~/development/VulkanSDK/X.X.X.X/x86_64
STB_INCLUDE_PATH = ~/libraries/stb
TINYOBJ_INCLUDE_PATH = ~/libraries/tinyobjloader-c

CFLAGS = -std=c99 -I$(STB_INCLUDE_PATH) -I $(VULKAN_SDK_PATH)/include99 -I$(TINYOBJ_INCLUDE_PATH)
LDFLAGS = -L $(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3 vulkan` -lvulkan -lpthread /usr/lib/x86_64-linux-gnu/librt.so -lm -ldl

default: main

main: main.c
	$(CC) $(CFLAGS) main.c -o a.out $(LDFLAGS)

clean:
	rm -f a.out
