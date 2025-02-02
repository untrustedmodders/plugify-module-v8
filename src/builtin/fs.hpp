namespace builtin {
	// Helper: Convert v8::Value to std::filesystem::path
	std::filesystem::path ToStdPath(v8::Isolate* isolate, v8::Local<v8::Value> value) {
		v8::String::Utf8Value utf8(isolate, value);
		if (*utf8) {
			return std::string_view{ *utf8, static_cast<size_t>(utf8.length()) };
		}
		throw std::invalid_argument("Invalid path");
	}

	// Helper function to convert `std::chrono::time_point` to Unix timestamp in milliseconds
	double ToUnixTimestampMs(const std::filesystem::file_time_type& time) {
		using namespace std::chrono;
		auto duration = time.time_since_epoch();
		return static_cast<double>(duration_cast<milliseconds>(duration).count());
	}

	// Helper function to convert `std::chrono::time_point` to ISO 8601 string
	std::string ToIso8601(const std::filesystem::file_time_type& time) {
		using namespace std::chrono;
		auto tp = time_point_cast<system_clock::duration>(time - std::filesystem::file_time_type::clock::now() + system_clock::now());
		std::time_t time_t = system_clock::to_time_t(tp);
		std::ostringstream ss;
		ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S.%fZ");
		return ss.str();
	}

	namespace fs {
		// exists
		void Exists(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			try {
				std::filesystem::path path = ToStdPath(isolate, args[0]);
				args.GetReturnValue().Set(std::filesystem::exists(path));
			} catch (const std::exception& e) {
				isolate->ThrowException(v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked());
			}
		}

		// rename
		void Rename(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			try {
				std::filesystem::path oldpath = ToStdPath(isolate, args[0]);
				std::filesystem::path newpath = ToStdPath(isolate, args[1]);
				std::filesystem::rename(oldpath, newpath);
				args.GetReturnValue().Set(true);
			} catch (const std::exception& e) {
				isolate->ThrowException(v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked());
			}
		}

		// chown (Stub - no direct cross-platform implementation in fs)
		void Chown(const v8::FunctionCallbackInfo<v8::Value>& args) {
			args.GetReturnValue().Set(true); // Stub implementation
		}

		// rmdir
		void Rmdir(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			try {
				std::filesystem::path path = ToStdPath(isolate, args[0]);
				std::filesystem::remove(path);
				args.GetReturnValue().Set(args[0]);
			} catch (const std::exception& e) {
				isolate->ThrowException(v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked());
			}
		}

		// mkdir
		void Mkdir(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			try {
				std::filesystem::path path = ToStdPath(isolate, args[0]);
				std::filesystem::create_directory(path);
				args.GetReturnValue().Set(args[0]);
			} catch (const std::exception& e) {
				isolate->ThrowException(v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked());
			}
		}

		// stat
		void Stat(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			try {
				std::filesystem::path path = ToStdPath(isolate, args[0]);
				auto status = std::filesystem::status(path);
				auto meta = std::filesystem::status(path);

				v8::Local<v8::Object> obj = v8::Object::New(isolate);

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "dev"),
						 v8::Number::New(isolate, 0)) // fs does not support dev
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "mode"),
						 v8::Number::New(isolate, static_cast<uint32_t>(status.permissions())))
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "nlink"),
						 v8::Number::New(isolate, 1)) // Link count is usually 1 for files
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "uid"),
						 v8::Number::New(isolate, 0)) // UID is not available with fs
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "gid"),
						 v8::Number::New(isolate, 0)) // GID is not available with fs
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "rdev"),
						 v8::Number::New(isolate, 0)) // Not available
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "blksize"),
						 v8::Number::New(isolate, 4096)) // Common block size
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "ino"),
						 v8::Number::New(isolate, 0)) // Inode number is not directly available in fs
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "size"),
						 v8::Number::New(isolate, std::filesystem::file_size(path)))
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "blocks"),
						 v8::Number::New(isolate, 0)) // Not available
						.Check();

				// Timestamps in milliseconds
				auto mtime = std::filesystem::last_write_time(path);
				auto atime = std::filesystem::last_write_time(path); // No native atime support
				auto ctime = std::filesystem::last_write_time(path); // No native ctime support
				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "atimeMs"),
						 v8::Number::New(isolate, ToUnixTimestampMs(atime)))
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "mtimeMs"),
						 v8::Number::New(isolate, ToUnixTimestampMs(mtime)))
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "ctimeMs"),
						 v8::Number::New(isolate, ToUnixTimestampMs(ctime)))
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "birthtimeMs"),
						 v8::Number::New(isolate, ToUnixTimestampMs(ctime))) // Birth time not supported
						.Check();

				// Timestamps in ISO format
				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "atime"),
						 v8::String::NewFromUtf8(isolate, ToIso8601(atime).c_str()).ToLocalChecked())
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "mtime"),
						 v8::String::NewFromUtf8(isolate, ToIso8601(mtime).c_str()).ToLocalChecked())
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "ctime"),
						 v8::String::NewFromUtf8(isolate, ToIso8601(ctime).c_str()).ToLocalChecked())
						.Check();

				obj->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "birthtime"),
						 v8::String::NewFromUtf8(isolate, ToIso8601(ctime).c_str()).ToLocalChecked())
						.Check();

				args.GetReturnValue().Set(obj);
			} catch (const std::exception& e) {
				isolate->ThrowException(v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked());
			}
		}

		// readdir
		void Readdir(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			try {
				std::filesystem::path path = ToStdPath(isolate, args[0]);
				v8::Local<v8::Array> array = v8::Array::New(isolate);

				size_t i = static_cast<size_t>(-1);
				for (const auto& entry : std::filesystem::directory_iterator(path)) {
					std::string res = entry.path().string();
					array->Set(context,
							   ++i,
							   v8::String::NewFromUtf8(isolate, res.c_str(), v8::NewStringType::kNormal, static_cast<int>(res.size())).ToLocalChecked()).Check();
				}
				args.GetReturnValue().Set(array);
			} catch (const std::exception& e) {
				isolate->ThrowException(v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked());
			}
		}

		// readFile
		void ReadFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			try {
				std::filesystem::path path = ToStdPath(isolate, args[0]);
				std::ifstream file(path, std::ios::in | std::ios::binary);
				if (!file.is_open()) {
					throw std::runtime_error(std::format("Failed to open file: '{}'", strerror(errno)));
				}

				std::string s((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

				args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, s.c_str(), v8::NewStringType::kNormal, static_cast<int>(s.size())).ToLocalChecked());
			} catch (const std::exception& e) {
				isolate->ThrowException(v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked());
			}
		}

		// writeFile
		void WriteFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			try {
				std::filesystem::path path = ToStdPath(isolate, args[0]);
				std::string content = ToStdString(isolate, args[1]);

				std::ofstream file(path, std::ios::out | std::ios::trunc);
				if (!file.is_open()) {
					throw std::runtime_error(std::format("Failed to open file: '{}'", strerror(errno)));
				}
				file << content;

				args.GetReturnValue().Set(true);
			} catch (const std::exception& e) {
				isolate->ThrowException(v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked());
			}
		}

		// appendFile
		void AppendFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			try {
				std::filesystem::path path = ToStdPath(isolate, args[0]);
				std::string content = ToStdString(isolate, args[1]);

				std::ofstream file(path, std::ios::out | std::ios::ate);
				if (!file.is_open()) {
					throw std::runtime_error(std::format("Failed to open file: '{}'", strerror(errno)));
				}
				file << content;

				args.GetReturnValue().Set(true);
			} catch (const std::exception& e) {
				isolate->ThrowException(v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked());
			}
		}

		// Init function
		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> exports = v8::Object::New(isolate);

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "exists"),
						 v8::FunctionTemplate::New(isolate, Exists)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "rename"),
						 v8::FunctionTemplate::New(isolate, Rename)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "chown"),
						 v8::FunctionTemplate::New(isolate, Chown)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "rmdir"),
						 v8::FunctionTemplate::New(isolate, Rmdir)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "mkdir"),
						 v8::FunctionTemplate::New(isolate, Mkdir)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "stat"),
						 v8::FunctionTemplate::New(isolate, Stat)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "readdir"),
						 v8::FunctionTemplate::New(isolate, Readdir)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "readFile"),
						 v8::FunctionTemplate::New(isolate, ReadFile)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "writeFile"),
						 v8::FunctionTemplate::New(isolate, WriteFile)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "appendFile"),
						 v8::FunctionTemplate::New(isolate, AppendFile)->GetFunction(context).ToLocalChecked()
								 ).Check();

			return exports;
		}

	}
}