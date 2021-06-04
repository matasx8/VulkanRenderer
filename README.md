# VulkanRenderer
I'm creating a 3D rendering engine with Vulkan. The purpose of this project is to learn about graphics programming. This code is a continuation of what I learned from Ben Cook's Vulkan course.

# What has been implemented by me:
- Tried out adding a second pipeline using pipeline derivatives to draw with different shaders. Here you can see the one skull with Phong lighting and the other with Cel lighting(and how badly I need anti-aliasing)
![Capture](https://user-images.githubusercontent.com/78436416/120780451-d0603480-c530-11eb-8a2f-f67355e2c9bc.PNG)

- Shader compilation on start
  - Start a new process that runs a python script which checks the hash of Shaders directory and if it has changed compiles the shaders. So far it is not parallel, but definitely in my TODO list
- Phong lighting<br>
![ezgif com-gif-maker (1)](https://user-images.githubusercontent.com/78436416/119230657-e4199d00-bb25-11eb-8487-49971d972134.gif)

- Camera and Camera movement <br>
![preview](https://user-images.githubusercontent.com/78436416/118819153-bd592d80-b8bd-11eb-8954-88b1479cc1ff.gif)
