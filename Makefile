CFLAGS = -std=c++20 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi


VulkanTest: main.cpp VkTstApp.o
	g++ $(CFLAGS) -o VulkanTest main.cpp VkTstApp.o $(LDFLAGS)

VkTstApp.o :
	g++ $(CFLAGS) -c VkTstApp.cpp -o VkTstApp.o $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest *.o
	
