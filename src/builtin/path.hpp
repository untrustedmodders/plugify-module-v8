namespace builtin {
	std::vector<std::filesystem::path> ConvertArgsToPaths(const v8::FunctionCallbackInfo<v8::Value>& args) {
		v8::Isolate* isolate = args.GetIsolate();
		std::vector<std::filesystem::path> paths;
		paths.reserve(static_cast<size_t>(args.Length()));

		for (int i = 0; i < args.Length(); i++) {
			if (args[i]->IsString()) {
				v8::String::Utf8Value utf8(isolate, args[i]);
				if (*utf8) {
					paths.emplace_back(std::string_view{ *utf8, static_cast<size_t>(utf8.length()) });
				}
			}
		}
		return paths;
	}

	namespace path {
		// Normalize: Converts a path to its normalized form
		void Normalize(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			
			if (args.Length() < 1 || !args[0]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Path argument is required"));
				return;
			}

			std::filesystem::path path = ToStdPath(isolate, args[0]);

			std::string normalizedPath = std::filesystem::weakly_canonical(path).string();
			args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, normalizedPath.c_str(), v8::NewStringType::kNormal, static_cast<int>(normalizedPath.size())).ToLocalChecked());
		}

		// Join: Concatenate multiple paths into one
		void Join(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			auto paths = ConvertArgsToPaths(args);

			std::filesystem::path result;
			for (const auto& part : paths) {
				result /= part;
			}

			std::string path = result.string();
			args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, path.c_str(), v8::NewStringType::kNormal, static_cast<int>(path.size())).ToLocalChecked());
		}

		// Resolve: Resolve paths into an absolute path
		void Resolve(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			auto paths = ConvertArgsToPaths(args);

			std::filesystem::path result;
			for (const auto& part : paths) {
				if (part.is_absolute()) {
					result = part;
				} else {
					result /= part;
				}
			}

			std::string path = std::filesystem::absolute(result).string();
			args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, path.c_str(), v8::NewStringType::kNormal, static_cast<int>(path.size())).ToLocalChecked());
		}

		// IsAbsolute: Check if a path is absolute
		void IsAbsolute(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 1 || !args[0]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Path argument is required"));
				return;
			}

			std::filesystem::path path = ToStdPath(isolate, args[0]);

			bool isAbsolute = path.is_absolute();
			args.GetReturnValue().Set(v8::Boolean::New(isolate, isAbsolute));
		}


		// Relative: Get the relative path between two paths
		void Relative(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Two path arguments are required"));
				return;
			}

			std::filesystem::path fromPath = ToStdPath(isolate, args[0]);
			std::filesystem::path toPath = ToStdPath(isolate, args[1]);

			std::string relativePath = std::filesystem::relative(toPath, fromPath).string();
			args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, relativePath.c_str(), v8::NewStringType::kNormal, static_cast<int>(relativePath.size())).ToLocalChecked());
		}

		// Dirname: Get the directory name of a path
		void Dirname(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 1 || !args[0]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Path argument is required"));
				return;
			}

			std::filesystem::path path = ToStdPath(isolate, args[0]);

			std::string dirPath = path.parent_path().string();
			args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, dirPath.c_str(), v8::NewStringType::kNormal, static_cast<int>(dirPath.size())).ToLocalChecked());
		}

		// Basename: Get the base name of a path
		void Basename(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 1 || !args[0]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Path argument is required"));
				return;
			}

			std::filesystem::path path = ToStdPath(isolate, args[0]);

			std::string baseName = path.filename().string();
			args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, baseName.c_str(), v8::NewStringType::kNormal, static_cast<int>(baseName.size())).ToLocalChecked());
		}

		// Extname: Get the file extension of a path
		void Extname(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 1 || !args[0]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Path argument is required"));
				return;
			}

			std::filesystem::path path = ToStdPath(isolate, args[0]);

			std::string extName = path.extension().string();
			args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, extName.c_str(), v8::NewStringType::kNormal, static_cast<int>(extName.size())).ToLocalChecked());
		}

		// Initialize the module and export functions
		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> exports = v8::Object::New(isolate);
			
			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "normalize"),
					v8::FunctionTemplate::New(isolate, Normalize)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "join"),
   						 v8::FunctionTemplate::New(isolate, Join)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "resolve"),
					v8::FunctionTemplate::New(isolate, Resolve)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "isAbsolute"),
					v8::FunctionTemplate::New(isolate, IsAbsolute)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "relative"),
					v8::FunctionTemplate::New(isolate, Relative)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "dirname"),
					v8::FunctionTemplate::New(isolate, Dirname)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "basename"),
					v8::FunctionTemplate::New(isolate, Basename)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "extname"),
					v8::FunctionTemplate::New(isolate, Extname)->GetFunction(context).ToLocalChecked()
								 ).Check();
			
			return exports;
		}
	}
}