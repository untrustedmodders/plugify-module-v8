namespace builtin {
    namespace dls {
        // Store assemblies with unique handles
        static inline std::unordered_map<void*, AssemblyPtr> assemblies;

        void Open(const v8::FunctionCallbackInfo<v8::Value>& args) {
            v8::Isolate* isolate = args.GetIsolate();

            if (args.Length() < 1 || !args[0]->IsString()) {
                g_v8lm.ThrowException("Expected 1 argument: path");
                return;
            }

            try {
                std::filesystem::path path = g_v8lm.ToPath(args[0]);

                auto assemblyResult = g_v8lm.GetProvider()
                    ->Resolve<IAssemblyLoader>()
                    ->Load(path, LoadFlag::Default);

                if (!assemblyResult) {
                    g_v8lm.ThrowException(assemblyResult.error());
                    return;
                }

                auto assembly = *assemblyResult;
                if (!assembly || !assembly->IsValid()) {
                    g_v8lm.ThrowException("Invalid assembly");
                    return;
                }

                void* handle = assembly->GetHandle();
                assemblies.try_emplace(handle, std::move(assembly));

                args.GetReturnValue().Set(v8::External::New(isolate, handle));
            } catch (const std::exception& e) {
                g_v8lm.ThrowException(e.what());
            }
        }

        void Close(const v8::FunctionCallbackInfo<v8::Value>& args) {
            if (args.Length() < 1 || !args[0]->IsExternal()) {
                g_v8lm.ThrowException("Expected 1 argument: handle");
                return;
            }

            void* handle = args[0].As<v8::External>()->Value();

            auto it = assemblies.find(handle);
            if (it == assemblies.end()) {
                g_v8lm.ThrowException("Invalid handle");
                return;
            }

            if (auto loader = g_v8lm.GetProvider()->Resolve<IAssemblyLoader>()) {
                auto result = loader->Unload(it->second);
                if (!result) {
                    g_v8lm.ThrowException(result.error());
                    return;
                }
            }

            assemblies.erase(it);
        }

        void Symbol(const v8::FunctionCallbackInfo<v8::Value>& args) {
            v8::Isolate* isolate = args.GetIsolate();

            if (args.Length() < 2 || !args[0]->IsExternal() || !args[1]->IsString()) {
                g_v8lm.ThrowException("Expected 2 arguments: handle, name");
                return;
            }

            void* handle = args[0].As<v8::External>()->Value();

            auto it = assemblies.find(handle);
            if (it == assemblies.end()) {
                g_v8lm.ThrowException("Invalid handle");
                return;
            }

            std::string name = g_v8lm.ToString(args[1]);
            auto symbolResult = it->second->GetSymbol(name);
            if (!symbolResult) {
                g_v8lm.ThrowException(symbolResult.error());
                return;
            }

            uintptr_t addr = *symbolResult;
            args.GetReturnValue().Set(
                v8::BigInt::NewFromUnsigned(isolate, static_cast<uint64_t>(addr))
            );
        }

        void Terminate() {
            assemblies.clear();
        }

        // Init function
        v8::Local<v8::Object> Init(v8::Isolate* isolate) {
            v8::Local<v8::Context> context = isolate->GetCurrentContext();
            v8::Local<v8::Object> exports = v8::Object::New(isolate);

            // Create a `Dls` object
            v8::Local<v8::Object> dls = v8::Object::New(isolate);

            // Bind functions to the `Dls` object
            dls->Set(context,
                g_v8lm.MakeString("open"),
                v8::FunctionTemplate::New(isolate, Open)
                    ->GetFunction(context).ToLocalChecked()
            ).Check();

            dls->Set(context,
                g_v8lm.MakeString("close"),
                v8::FunctionTemplate::New(isolate, Close)
                    ->GetFunction(context).ToLocalChecked()
            ).Check();

            dls->Set(context,
                g_v8lm.MakeString("symbol"),
                v8::FunctionTemplate::New(isolate, Symbol)
                    ->GetFunction(context).ToLocalChecked()
            ).Check();

            // Set the `Dls` object as the exports
            exports->Set(context, g_v8lm.MakeString("dls"), dls).Check();

            return exports;
        }
    }
}