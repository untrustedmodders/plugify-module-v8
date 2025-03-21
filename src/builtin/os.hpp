namespace builtin {
	namespace os {
		// Get the temporary directory path
		void Tmpdir(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			try {
				std::string tmpDir = std::filesystem::temp_directory_path().string();
				args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, tmpDir.c_str(), v8::NewStringType::kNormal, static_cast<int>(tmpDir.size())).ToLocalChecked());
			} catch (...) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Unable to retrieve temp directory"));
			}
		}

		// Get the user's home directory path
		void Homedir(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			const char* homeDir = std::getenv("HOME");
			if (!homeDir) {
#if V8LM_PLATFORM_WINDOWS
				homeDir = std::getenv("USERPROFILE");
#endif
			}

			if (homeDir) {
				args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, homeDir).ToLocalChecked());
			} else {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Unable to retrieve home directory"));
			}
		}

		// Get the endianness of the system
		void Endianness(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			constexpr bool isBigEndian = V8LM_IS_BIG_ENDIAN;
			if constexpr (isBigEndian) {
				args.GetReturnValue().Set(v8::String::NewFromUtf8Literal(isolate, "BE"));
			} else {
				args.GetReturnValue().Set(v8::String::NewFromUtf8Literal(isolate, "LE"));
			}
		}

		// Get the OS type
		void OSType(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			args.GetReturnValue().Set(v8::String::NewFromUtf8Literal(isolate, V8LM_OS));
		}

		// Get the platform (operating system)
		void Platform(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			args.GetReturnValue().Set(v8::String::NewFromUtf8Literal(isolate, V8LM_PLATFORM));
		}

		// Get the architecture
		void Arch(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

#if V8LM_ARCH_BITS == 64
	#if V8LM_ARCH_ARM
			char arch[] = "arm64";
	#else
			char arch[] = "x64";
	#endif
#elif V8LM_ARCH_BITS == 32
	#if V8LM_ARCH_ARM
			char arch[] = "arm";
	#else
			char arch[] = "x86";
	#endif
#endif

			args.GetReturnValue().Set(v8::String::NewFromUtf8Literal(isolate, arch));
		}

		// Initialize the module and export functions
		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> exports = v8::Object::New(isolate);
			
			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "tmpdir"),
					v8::FunctionTemplate::New(isolate, Tmpdir)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "homedir"),
					v8::FunctionTemplate::New(isolate, Homedir)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "endianness"),
					v8::FunctionTemplate::New(isolate, Endianness)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "type"),
					v8::FunctionTemplate::New(isolate, OSType)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "platform"),
					v8::FunctionTemplate::New(isolate, Platform)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
					v8::String::NewFromUtf8Literal(isolate, "arch"),
					v8::FunctionTemplate::New(isolate, Arch)->GetFunction(context).ToLocalChecked()
						 ).Check();
			
			return exports;
		}

	}
}