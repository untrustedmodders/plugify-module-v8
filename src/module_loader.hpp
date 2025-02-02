#pragma once

namespace v8lm {
	class ModuleLoader {
	public:
		explicit ModuleLoader(fs::path libPath, fs::path pluginsPath) : _libPath(std::move(libPath)), _pluginsPath(std::move(pluginsPath)) {}
		
		bool Search(const fs::path& dir, const std::string& requiredModule, fs::path& path);
		bool Load(const fs::path& path, std::string& content);

	private:
		fs::path _libPath;
		fs::path _pluginsPath;
	};
}
