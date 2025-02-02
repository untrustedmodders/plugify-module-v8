namespace builtin {
	namespace readline {
		// Init function
		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			// TODO
			v8::Local<v8::Object> exports = v8::Object::New(isolate);
			return exports;
		}
	}
}