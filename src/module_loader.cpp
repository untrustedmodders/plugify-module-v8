#include "module_loader.hpp"
#include <fstream>

using namespace v8lm;

namespace {
	fs::path NormalizePath(const fs::path& filePath) {
		std::filesystem::path canonicalPath = filePath.lexically_normal();
		canonicalPath.make_preferred();
		return canonicalPath;
	}

	bool CheckExists(const fs::path& filePath, fs::path& path) {
		std::error_code ec;
		fs::path normalizedPath = NormalizePath(filePath);
		if (fs::exists(filePath, ec)) {
			path = std::move(normalizedPath);
			return true;
		}
		return false;
	}

	bool SearchModuleWithExtInDir(const fs::path& dir, const std::string& requiredModule, fs::path& path) {
		return CheckExists(dir / requiredModule, path) || (dir.filename() != "node_modules" && CheckExists(dir / "node_modules" / requiredModule, path));
	}

	bool SearchModuleInDir(const fs::path& dir, const std::string& requiredModule, fs::path& path) {
		bool isJs = requiredModule.ends_with(".js") || requiredModule.ends_with(".mjs") || requiredModule.ends_with(".cjs") || requiredModule.ends_with(".json");
		if (isJs && SearchModuleWithExtInDir(dir, requiredModule, path))
			return true;
		return SearchModuleWithExtInDir(dir, requiredModule + ".js", path) ||
			   SearchModuleWithExtInDir(dir, requiredModule + ".mjs", path) ||
			   SearchModuleWithExtInDir(dir, requiredModule + ".cjs", path) ||
			   SearchModuleWithExtInDir(dir, requiredModule + "/package.json", path) ||
			   SearchModuleWithExtInDir(dir, requiredModule + "/index.js", path);
	}
}

bool ModuleLoader::Search(const fs::path& requiredDir, const std::string& requiredModule, fs::path& path) {
	if (SearchModuleInDir(requiredDir, requiredModule, path)) {
		return true;
	}
	if (!requiredDir.empty() && requiredModule.find('/') != std::string::npos && !requiredModule.ends_with(".js") && !requiredModule.ends_with(".mjs")) {
		fs::path currentPath;
		bool first = true;
		for (const auto& part : requiredDir) {
			currentPath /= part;
			if (first) {
				first = false;
				continue;
			}
			if (part != "node_modules") {
				if (SearchModuleInDir(currentPath, requiredModule, path)) {
					return true;
				}
			}
		}
	}

	if (SearchModuleInDir(_libPath, requiredModule, path)) {
		return true;
	}

	std::error_code ec;
	for (auto const& entry : fs::directory_iterator(_pluginsPath, ec)) {
		if (entry.is_directory(ec)) {
			if (SearchModuleInDir(entry.path(), requiredModule, path)) {
				return true;
			}
		}
	}

	return false;
}

bool ModuleLoader::Load(const fs::path& path, std::string& content) {
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) {
		content = strerror(errno);
		return false;
	}
	content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return true;
}