#pragma once
#include<iostream>

//TODO: make the validator portable or the path better
class ShaderMan
{
public:
	ShaderMan();

	// Compile all shaders in Shader directory, log any errors
	// Uses windows api to create a new process and call compilation script
	// but waits for process to finish
	static void CompileShaders();
	// Same as CompileShaders but async, must call WaitForCompile to make sure everything is compiled before using shaders
	void CompileShadersAsync();
	void WaitForCompile();

private:
	void* pi_HProc;
	void* pi_HThread;
};

