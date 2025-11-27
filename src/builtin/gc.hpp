namespace builtin {
    namespace gc {
        void ForceGC(const v8::FunctionCallbackInfo<v8::Value>& args) {
            v8::Isolate* isolate = args.GetIsolate();
            while (!isolate->IdleNotificationDeadline(1.0)) {}
            args.GetReturnValue().Set(v8::Undefined(isolate));
        }

        // Init function
        /*v8::Local<v8::Object> Init(v8::Isolate* isolate) {
            v8::Local<v8::Object> exports = v8::Object::New(isolate);
            return exports;
        }*/
    }
}