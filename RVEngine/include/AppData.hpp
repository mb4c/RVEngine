#pragma once
#include <string>
#include <vector>
#include <filesystem>

class Application;
class Input;

struct AppData
{
	Application* app;
	Input* input;
	std::vector<std::filesystem::path> dropPaths;
	bool fileDropped = false;
};