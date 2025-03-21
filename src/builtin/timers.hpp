namespace builtin {
	namespace timers {
		void SetTimeout(const v8::FunctionCallbackInfo<v8::Value>& args) {
			auto* self = V8LanguageModule::Get(args.GetIsolate());
			self->SetTimeout(args);
		}
		void ClearTimeout(const v8::FunctionCallbackInfo<v8::Value>& args) {
			auto* self = V8LanguageModule::Get(args.GetIsolate());
			self->ClearTimeout(args);
		}

		// Init function
		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			v8::Local<v8::Object> exports = v8::Object::New(isolate);

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "setTimeout"),
						 v8::FunctionTemplate::New(isolate, SetTimeout)->GetFunction(context).ToLocalChecked()
								 ).Check();

			exports->Set(context,
						 v8::String::NewFromUtf8Literal(isolate, "clearTimeout"),
						 v8::FunctionTemplate::New(isolate, ClearTimeout)->GetFunction(context).ToLocalChecked()
								 ).Check();

			return exports;
		}
	}
}