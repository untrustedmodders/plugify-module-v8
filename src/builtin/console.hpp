namespace builtin {
	namespace console {
		v8::Local<v8::String> Stringify(v8::Isolate* isolate, v8::Local<v8::Value> obj) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			// Get the JSON.stringify function
			v8::Local<v8::Value> val = v8::JSON::Stringify(context, obj).ToLocalChecked();

			// Check if the result is a string
			if (val->IsString()) {
				return val.As<v8::String>();
			}

			// Fallback in case of an error
			return g_v8lm.MakeString("[Stringify Error]");
		}

		template<plugify::Severity severity>
		void Log(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 1) {
				g_v8lm.ThrowException("Expected 1 argument: message");
				return;
			}

			v8::Local<v8::Value> firstArg = args[0];
			v8::Local<v8::String> msg;

			if (firstArg->IsString() || firstArg->IsFunction()) {
				msg = firstArg->ToString(context).ToLocalChecked();
			} else {
				msg = Stringify(isolate, firstArg);
			}

			v8::String::Utf8Value utf8(isolate, msg);
			if (*utf8) {
				g_v8lm.GetProvider()->Log(std::string_view{*utf8, static_cast<size_t>(utf8.length())}, severity);
			}
		}

		void ConsoleLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
			Log<plugify::Severity::Unknown>(args);
		}

		void ConsoleInfo(const v8::FunctionCallbackInfo<v8::Value>& args) {
			Log<plugify::Severity::Info>(args);
		}

		void ConsoleWarn(const v8::FunctionCallbackInfo<v8::Value>& args) {
			Log<plugify::Severity::Warning>(args);
		}

		void ConsoleError(const v8::FunctionCallbackInfo<v8::Value>& args) {
			Log<plugify::Severity::Error>(args);
		}

		void ConsoleDebug(const v8::FunctionCallbackInfo<v8::Value>& args) {
			Log<plugify::Severity::Debug>(args);
		}

		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			v8::Local<v8::Object> console = v8::Object::New(isolate);

			console->Set(context,
						 g_v8lm.MakeString("log"),
						 v8::FunctionTemplate::New(isolate, ConsoleLog)->GetFunction(context).ToLocalChecked()
						 ).Check();

			console->Set(context,
						 g_v8lm.MakeString("info"),
						 v8::FunctionTemplate::New(isolate, ConsoleInfo)->GetFunction(context).ToLocalChecked()
						).Check();

			console->Set(context,
						 g_v8lm.MakeString("warn"),
						 v8::FunctionTemplate::New(isolate, ConsoleWarn)->GetFunction(context).ToLocalChecked()
						).Check();

			console->Set(context,
						 g_v8lm.MakeString("error"),
						 v8::FunctionTemplate::New(isolate, ConsoleError)->GetFunction(context).ToLocalChecked()
						).Check();

			console->Set(context,
						 g_v8lm.MakeString("debug"),
						 v8::FunctionTemplate::New(isolate, ConsoleDebug)->GetFunction(context).ToLocalChecked()
						).Check();

			return console;
		}
	}
}