namespace builtin {
	std::vector<std::filesystem::path> ConvertArgsToPaths(const v8::FunctionCallbackInfo<v8::Value>& args) {
		std::vector<std::filesystem::path> paths;
		paths.reserve(static_cast<size_t>(args.Length()));

		for (int i = 0; i < args.Length(); i++) {
			if (args[i]->IsString()) {
				paths.emplace_back(g_v8lm.ToPath(args[i]));
			}
		}
		return paths;
	}

	namespace path {
		// Normalize: Converts a path to its normalized form
		void Normalize(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				auto normalizedPath = std::filesystem::weakly_canonical(path);
				args.GetReturnValue().Set(g_v8lm.MakeString(normalizedPath.native()));
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// Join: Concatenate multiple paths into one
		void Join(const v8::FunctionCallbackInfo<v8::Value>& args) {
			//v8::Isolate* isolate = args.GetIsolate();

			auto paths = ConvertArgsToPaths(args);

			std::filesystem::path result;
			for (const auto& part : paths) {
				result /= part;
			}

			try {
				args.GetReturnValue().Set(g_v8lm.MakeString(result.native()));
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// Resolve: Resolve paths into an absolute path
		void Resolve(const v8::FunctionCallbackInfo<v8::Value>& args) {
			auto paths = ConvertArgsToPaths(args);

			std::filesystem::path result;
			for (const auto& part : paths) {
				if (part.is_absolute()) {
					result = part;
				} else {
					result /= part;
				}
			}

			try {
				auto path = std::filesystem::absolute(result);
				args.GetReturnValue().Set(g_v8lm.MakeString(path.native()));
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// IsAbsolute: Check if a path is absolute
		void IsAbsolute(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				bool isAbsolute = path.is_absolute();
				args.GetReturnValue().Set(v8::Boolean::New(isolate, isAbsolute));
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// Relative: Get the relative path between two paths
		void Relative(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
				g_v8lm.ThrowException("Expected 2 arguments: frompath and topath");
				return;
			}

			try {
				std::filesystem::path fromPath = g_v8lm.ToPath(args[0]);
				std::filesystem::path toPath = g_v8lm.ToPath(args[1]);

				auto relativePath = std::filesystem::relative(toPath, fromPath);
				args.GetReturnValue().Set(g_v8lm.MakeString(relativePath.native()));
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// Dirname: Get the directory name of a path
		void Dirname(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				auto dirPath = path.parent_path();
				args.GetReturnValue().Set(g_v8lm.MakeString(dirPath.native()));
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// Basename: Get the base name of a path
		void Basename(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				auto baseName = path.filename();
				args.GetReturnValue().Set(g_v8lm.MakeString(baseName.native()));
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// Extname: Get the file extension of a path
		void Extname(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				auto extName = path.extension();
				args.GetReturnValue().Set(g_v8lm.MakeString(extName.native()));
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// Initialize the module and export functions
		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> exports = v8::Object::New(isolate);
			
			exports->Set(context,
						g_v8lm.MakeString("normalize"),
						v8::FunctionTemplate::New(isolate, Normalize)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 g_v8lm.MakeString("join"),
   						 v8::FunctionTemplate::New(isolate, Join)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						g_v8lm.MakeString("resolve"),
						v8::FunctionTemplate::New(isolate, Resolve)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						g_v8lm.MakeString("isAbsolute"),
						v8::FunctionTemplate::New(isolate, IsAbsolute)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						g_v8lm.MakeString("relative"),
						v8::FunctionTemplate::New(isolate, Relative)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						g_v8lm.MakeString("dirname"),
						v8::FunctionTemplate::New(isolate, Dirname)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						g_v8lm.MakeString("basename"),
						v8::FunctionTemplate::New(isolate, Basename)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						g_v8lm.MakeString("extname"),
						v8::FunctionTemplate::New(isolate, Extname)->GetFunction(context).ToLocalChecked()
						).Check();
			
			return exports;
		}
	}
}