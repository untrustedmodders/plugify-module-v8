namespace builtin {
	// Helper: Convert v8::Value to std::string
	std::string ToStdString(v8::Isolate* isolate, v8::Local<v8::Value> value) {
		v8::String::Utf8Value utf8(isolate, value);
		if (*utf8) {
			return { *utf8, static_cast<size_t>(utf8.length()) };
		}
		return "undefined";
	}

	namespace assert {
		// assert function
		void AssertFn(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 2 || !args[0]->IsBoolean() || !args[1]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Boolean and string argument is required"));
				return;
			}

			bool expr = args[0]->BooleanValue(isolate);
			std::string message = ToStdString(isolate, args[1]);

			ASSERT(expr && message.c_str());
		}

		// assert_eq function
		void AssertEqFn(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Two string argument is required"));
				return;
			}

			std::string expect = ToStdString(isolate, args[0]);
			std::string actual = ToStdString(isolate, args[1]);

			ASSERT(expect == actual && "assert_eq failed: values are not equal");
		}

		// assert_neq function
		void AssertNeqFn(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Two string argument is required"));
				return;
			}

			std::string expect = ToStdString(isolate, args[0]);
			std::string actual = ToStdString(isolate, args[1]);

			ASSERT(expect != actual && "assert_neq failed: values are equal");
		}

		// Init function
		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			v8::Local<v8::Object> exports = v8::Object::New(isolate);

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "ok"),
						 v8::FunctionTemplate::New(isolate, AssertFn)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "equal"),
						 v8::FunctionTemplate::New(isolate, AssertEqFn)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "notEqual"),
						 v8::FunctionTemplate::New(isolate, AssertNeqFn)->GetFunction(context).ToLocalChecked()
								 ).Check();

			return exports;
		}
	}
}