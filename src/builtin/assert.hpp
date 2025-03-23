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
		std::string Stringify(v8::Isolate* isolate, v8::Local<v8::Value> obj) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			// Get the JSON.stringify function
			v8::Local<v8::Value> val = v8::JSON::Stringify(context, obj).ToLocalChecked();

			// Check if the result is a string
			if (val->IsString()) {
				return g_v8lm.ToString(val.As<v8::String>());
			}

			// Fallback in case of an error
			return {};
		}

		// assert function
		void Assert(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 2 || !args[0]->IsBoolean() || !args[1]->IsString()) {
				g_v8lm.ThrowException("Expected 2 arguments: expression and message");
				return;
			}

			bool expr = args[0]->BooleanValue(isolate);
			std::string message = g_v8lm.ToString(args[1]);

			ASSERT_MSG(expr, message);
		}

		// equal function
		void Equal(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
				g_v8lm.ThrowException("Expected 3 arguments: value1, value2 and message");
				return;
			}

			std::string expect = g_v8lm.ToString(args[0]);
			std::string actual = g_v8lm.ToString(args[1]);
			std::string message = g_v8lm.ToString(args[2]);

			ASSERT_MSG(expect == actual, message);
		}

		// notEqual function
		void NotEqual(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
				g_v8lm.ThrowException("Expected 3 arguments: value1, value2 and message");
				return;
			}

			std::string expect = g_v8lm.ToString(args[0]);
			std::string actual = g_v8lm.ToString(args[1]);
			std::string message = g_v8lm.ToString(args[2]);

			ASSERT_MSG(expect != actual, message);
		}

		// deepEqual function
		void DeepEqual(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 3 || !args[0]->IsObject() || !args[1]->IsObject() || !args[2]->IsString()) {
				g_v8lm.ThrowException("Expected 3 arguments: value1, value2 and message");
				return;
			}

			std::string expect = Stringify(isolate, args[0]);
			std::string actual = Stringify(isolate, args[1]);
			std::string message = Stringify(isolate, args[2]);

			ASSERT_MSG(expect == actual, message);
		}

		// notDeepEqual function
		void NotDeepEqual(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 3 || !args[0]->IsObject() || !args[1]->IsObject() || !args[2]->IsString()) {
				g_v8lm.ThrowException("Expected 3 arguments: value1, value2 and message");
				return;
			}

			std::string expect = Stringify(isolate, args[0]);
			std::string actual = Stringify(isolate, args[1]);
			std::string message = Stringify(isolate, args[2]);

			ASSERT_MSG(expect != actual, message);
		}

		// Init function
		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			v8::Local<v8::Object> exports = v8::Object::New(isolate);

			exports->Set(context,
						 g_v8lm.MakeString("assert"),
						 v8::FunctionTemplate::New(isolate, Assert)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 g_v8lm.MakeString("ok"),
						 v8::FunctionTemplate::New(isolate, Assert)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 g_v8lm.MakeString("equal"),
						 v8::FunctionTemplate::New(isolate, Equal)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 g_v8lm.MakeString("notEqual"),
						 v8::FunctionTemplate::New(isolate, NotEqual)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 g_v8lm.MakeString("deepEqual"),
						 v8::FunctionTemplate::New(isolate, DeepEqual)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 g_v8lm.MakeString("notDeepEqual"),
						 v8::FunctionTemplate::New(isolate, NotDeepEqual)->GetFunction(context).ToLocalChecked()
						).Check();

			return exports;
		}
	}
}
