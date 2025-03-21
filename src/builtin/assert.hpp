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
				return ToStdString(isolate, result.As<v8::String>());
			}

			// Fallback in case of an error
			return {};
		}

		// assert function
		void Assert(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 2 || !args[0]->IsBoolean() || !args[1]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Expected 2 arguments: expression and message"));
				return;
			}

			bool expr = args[0]->BooleanValue(isolate);
			std::string message = ToStdString(isolate, args[1]);

			ASSERT_MSG(expr, message);
		}

		// equal function
		void Equal(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Expected 3 arguments: value1, value2 and message"));
				return;
			}

			std::string expect = ToStdString(isolate, args[0]);
			std::string actual = ToStdString(isolate, args[1]);
			std::string message = ToStdString(isolate, args[2]);

			ASSERT_MSG(expect == actual, message);
		}

		// notEqual function
		void NotEqual(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Expected 3 arguments: value1, value2 and message"));
				return;
			}

			std::string expect = ToStdString(isolate, args[0]);
			std::string actual = ToStdString(isolate, args[1]);
			std::string message = ToStdString(isolate, args[2]);

			ASSERT_MSG(expect != actual, message);
		}

		// deepEqual function
		void DeepEqual(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 3 || !args[0]->IsObject() || !args[1]->IsObject() || !args[2]->IsString()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Expected 3 arguments: value1, value2 and message"));
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
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Expected 3 arguments: value1, value2 and message"));
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
						 v8::String::NewFromUtf8Literal(isolate, "assert"),
						 v8::FunctionTemplate::New(isolate, Assert)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "ok"),
						 v8::FunctionTemplate::New(isolate, Assert)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "equal"),
						 v8::FunctionTemplate::New(isolate, Equal)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "notEqual"),
						 v8::FunctionTemplate::New(isolate, NotEqual)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "deepEqual"),
						 v8::FunctionTemplate::New(isolate, DeepEqual)->GetFunction(context).ToLocalChecked()
						).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "notDeepEqual"),
						 v8::FunctionTemplate::New(isolate, NotDeepEqual)->GetFunction(context).ToLocalChecked()
						).Check();

			return exports;
		}
	}
}
