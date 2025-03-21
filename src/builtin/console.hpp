namespace builtin {
	namespace console {
		v8::Local<v8::String> Stringify(v8::Isolate* isolate, v8::Local<v8::Value> obj) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> global = context->Global();

			// Get the JSON object from the global context
			v8::Local<v8::Value> json_val = global->Get(context, v8::String::NewFromUtf8Literal(isolate, "JSON")).ToLocalChecked();
			v8::Local<v8::Object> json_obj = json_val.As<v8::Object>();

			// Get the JSON.stringify function
			v8::Local<v8::Value> stringify_val = json_obj->Get(context, v8::String::NewFromUtf8Literal(isolate, "stringify")).ToLocalChecked();
			v8::Local<v8::Function> stringify_func = stringify_val.As<v8::Function>();

			// Call JSON.stringify(obj, replacer, space)
			v8::Local<v8::Value> replacer = v8::Number::New(isolate, 0);
			v8::Local<v8::Value> space = v8::Number::New(isolate, 2);
			std::array args = { obj, replacer, space };

			v8::Local<v8::Value> result;
			if (stringify_func->Call(context, json_obj, static_cast<int>(args.size()), args.data()).ToLocal(&result)) {
				return result.As<v8::String>();
			}

			// Fallback in case of an error
			return v8::String::NewFromUtf8Literal(isolate, "[Stringify Error]");
		}

		template<plugify::Severity severity>
		void Log(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 1) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Expected 1 argument"));
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
			g_v8lm.GetProvider()->Log(std::string_view{*utf8, static_cast<size_t>(utf8.length())}, severity);
		}

		void ConsoleLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
			Log<plugify::Severity::None>(args);
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
						 v8::String::NewFromUtf8Literal(isolate, "log"),
						 v8::FunctionTemplate::New(isolate, ConsoleLog)->GetFunction(context).ToLocalChecked()
								 ).Check();

			console->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "info"),
						 v8::FunctionTemplate::New(isolate, ConsoleInfo)->GetFunction(context).ToLocalChecked()
								 ).Check();

			console->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "warn"),
						 v8::FunctionTemplate::New(isolate, ConsoleWarn)->GetFunction(context).ToLocalChecked()
								 ).Check();

			console->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "error"),
						 v8::FunctionTemplate::New(isolate, ConsoleError)->GetFunction(context).ToLocalChecked()
								 ).Check();

			console->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "debug"),
						 v8::FunctionTemplate::New(isolate, ConsoleDebug)->GetFunction(context).ToLocalChecked()
								 ).Check();

			return console;
		}
	}
}