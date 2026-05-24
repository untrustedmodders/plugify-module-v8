namespace builtin {
	namespace timers {
		void SetTimeout(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			
			if (args.Length() < 1 || !args[0]->IsFunction()) {
				g_v8lm.ThrowException("setTimeout requires a function argument.");
				return;
			}

			std::chrono::milliseconds timeout = 0ms;
			if (args.Length() >= 2 && args[1]->IsNumber()) {
				timeout = std::chrono::milliseconds(args[1].As<v8::Integer>()->Value());
			}

			auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[0].As<v8::Function>());

			uint32_t handle = g_v8lm.AddTask(timeout, [isolate, callback = std::move(callback)]() {
				v8::Isolate::Scope isolateScope(isolate);
				v8::HandleScope handleScope(isolate);
				v8::Local<v8::Context> context = isolate->GetCurrentContext();
				v8::Context::Scope contextScope(context);
				v8::TryCatch tryCatch(isolate);

				UNUSED(callback->Get(isolate)->Call(context, context->Global(), 0, {}));

				if (tryCatch.HasCaught()) {
					g_v8lm.LogError(tryCatch.Message());
					tryCatch.Reset();
				}
			});

			args.GetReturnValue().Set(handle);
		}

		void ClearTimeout(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1 || !args[0]->IsUint32()) {
				g_v8lm.ThrowException("clearTimeout requires an int argument.");
				return;
			}

			uint32_t id = args[0].As<v8::Uint32>()->Value();
			g_v8lm.RemoveTask(id);
		}

		void SetInterval(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			
			if (args.Length() < 1 || !args[0]->IsFunction()) {
				g_v8lm.ThrowException("setInterval requires a function argument.");
				return;
			}

			std::chrono::milliseconds timeout = 0ms;
			if (args.Length() >= 2 && args[1]->IsNumber()) {
				timeout = std::chrono::milliseconds(args[1].As<v8::Integer>()->Value());
			}

			auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[0].As<v8::Function>());

			uint32_t handle = g_v8lm.AddTask(timeout, [isolate, callback = std::move(callback)]() {
				v8::Isolate::Scope isolateScope(isolate);
				v8::HandleScope handleScope(isolate);
				v8::Local<v8::Context> context = isolate->GetCurrentContext();
				v8::Context::Scope contextScope(context);
				v8::TryCatch tryCatch(isolate);

				UNUSED(callback->Get(isolate)->Call(context, context->Global(), 0, {}));

				if (tryCatch.HasCaught()) {
					g_v8lm.LogError(tryCatch.Message());
					tryCatch.Reset();
				}
			}, true);

			args.GetReturnValue().Set(handle);
		}

		void ClearInterval(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1 || !args[0]->IsUint32()) {
				g_v8lm.ThrowException("clearInterval requires an int argument.");
				return;
			}

			uint32_t id = args[0].As<v8::Uint32>()->Value();
			g_v8lm.RemoveTask(id);
		}

		// Init function
		/*v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			v8::Local<v8::Object> exports = v8::Object::New(isolate);

			exports->Set(context,
						 g_v8lm.MakeString("setTimeout"),
						 v8::FunctionTemplate::New(isolate, SetTimeout)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("clearTimeout"),
						 v8::FunctionTemplate::New(isolate, ClearTimeout)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("setInterval"),
						 v8::FunctionTemplate::New(isolate, SetInterval)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("clearInterval"),
						 v8::FunctionTemplate::New(isolate, ClearInterval)->GetFunction(context).ToLocalChecked()
						 ).Check();

			return exports;
		}*/
	}
}