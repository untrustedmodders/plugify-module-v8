namespace builtin {
	namespace process {
		// Exit the process with a specified status code
		void Exit(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() > 0 && args[0]->IsInt32()) {
				int status = args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
				exit(status);
			} else {
				exit(0); // Default exit code is 0
			}
		}

		// Get the group ID of the calling process
		void GetGid(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
#if V8LM_PLATFORM_WINDOWS
			uint32_t gid = 0;
#else
			gid_t gid = getgid(); // Get the group ID
#endif
			args.GetReturnValue().Set(v8::Uint32::New(isolate, gid));
		}

		// Get the process ID of the calling process
		void GetPid(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
#if V8LM_PLATFORM_WINDOWS
			int32_t pid = _getpid(); // Get the process ID
#else
			pid_t pid = getpid(); // Get the process ID
#endif
			args.GetReturnValue().Set(v8::Int32::New(isolate, pid));
		}

		void PopulateEnvObject(v8::Isolate* isolate, v8::Local<v8::Context> context, v8::Local<v8::Object>& envObject) {
			char** env = environ;
			for (env; *env; ++env) {
				std::string entry(*env);
				size_t delimiter = entry.find('=');
				if (delimiter != std::string::npos) {
					std::string key = entry.substr(0, delimiter);
					std::string value = entry.substr(delimiter + 1);
					envObject->Set(
									 context,
									 v8::String::NewFromUtf8(isolate, key.c_str(), v8::NewStringType::kNormal, static_cast<int>(key.size())).ToLocalChecked(),
									 v8::String::NewFromUtf8(isolate, value.c_str(), v8::NewStringType::kNormal, static_cast<int>(key.size())).ToLocalChecked()
											 ).Check();
				}
			}
		}

		// Initialize the module and export functions
		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> exports = v8::Object::New(isolate);

			// Create a `Process` object
			v8::Local<v8::Object> process = v8::Object::New(isolate);

			// Bind functions to the `Process` object
			process->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "exit"),
						 v8::FunctionTemplate::New(isolate, Exit)->GetFunction(context).ToLocalChecked()).Check();

			process->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "getgid"),
						 v8::FunctionTemplate::New(isolate, GetGid)->GetFunction(context).ToLocalChecked()).Check();

			process->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "getpid"),
						 v8::FunctionTemplate::New(isolate, GetPid)->GetFunction(context).ToLocalChecked()).Check();

			// Bind constants to the `Process` object
			process->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "title"),
						 v8::String::NewFromUtf8Literal(isolate, V8LM_NAME)).Check();

			process->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "version"),
						 v8::String::NewFromUtf8Literal(isolate, V8LM_VERSION)).Check();

			// Bind `env` to the `Process` object
			v8::Local<v8::Object> envObject = v8::Object::New(isolate);
			PopulateEnvObject(isolate, context, envObject);
			process->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "env"),
						 envObject).Check();

			// Set the `Process` object as the exports
			exports->Set(context, v8::String::NewFromUtf8Literal(isolate, "process"), process).Check();

			return exports;
		}
	}
}