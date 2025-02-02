#include "js.hpp"
#include "js_scope.hpp"

#include <libplatform/libplatform.h>

using namespace v8lm;

namespace {

	std::unique_ptr<v8::Platform> platform = nullptr;

	void AppendModulePath(std::string& output,
						  const fs::path& base_path,
						  const std::vector<fs::path>& paths) {
		if (paths.empty()) {
			return;
		}
		int i = paths.size() - 1;
		std::format_to(std::back_inserter(output),
					   "    loading {}\n", fs::relative(paths[i], base_path).string());
		for (i--; i >= 0; i--) {
			std::format_to(std::back_inserter(output),
						   "       from {}\n", fs::relative(paths[i], base_path).string());
		}
	}

	bool IsValidImport(std::string_view name) {
		if (name.empty())
			return false;
		bool is_path = name.starts_with("./") || name.starts_with("../") || name.starts_with("/");
		bool is_file = name.ends_with(".mjs");
		bool is_link = name.ends_with("https://") || name.ends_with("http://");
		return (is_path && is_file) || (!is_path && !is_file && !is_link);
	}

	std::string ToImportPath(const fs::path& dir, const std::string& spec) {
		return spec.ends_with(".mjs") ? (dir / spec).lexically_normal().string() : spec;
	}

	std::string EscapeString(std::string_view value) {
		std::string result;
		result.reserve(value.size() + 2);
		result += '"';
		for (char c : value) {
			switch (c) {
				case '"':
					result += "\\\"";
					break;
				case '\\':
					result += "\\\\";
					break;
				case '/':
					result += "\\/";
					break;
				case '\b':
					result += "\\b";
					break;
				case '\f':
					result += "\\f";
					break;
				case '\n':
					result += "\\n";
					break;
				case '\r':
					result += "\\r";
					break;
				case '\t':
					result += "\\t";
					break;
				default:
					result += c;
					break;
			}
		}
		result += '"';
		return result;
	}

	std::string MakeInvalidImportError(std::string_view name) {
		return std::format("Invalid module name: '{}'. Valid imports must begin with ./ or ../", name);
	}

	v8::Local<v8::Message> MakeErrorMessage(v8::Isolate* isolate,
											v8::Local<v8::Value> exception) {
		v8::Local<v8::Message> message =
				v8::Exception::CreateMessage(isolate, exception);
		if (!exception->IsNativeError() &&
			(message.IsEmpty() || message->GetStackTrace().IsEmpty())) {
			// Create a stack trace manually if it's missing.
			exception = v8::Exception::Error(
					v8::String::NewFromUtf8Literal(isolate, "Uncaught Exception."));
			message = v8::Exception::CreateMessage(isolate, exception);
		}
		return message;
	}

	bool ReadFile(const fs::path& path, std::string* content,
				  std::string* error) {
		std::ifstream file(path, std::ios::binary);
		if (!file) {
			*error = std::format("Failed to read '{}': {}", path.string(), strerror(errno));
			return false;
		}
		std::string s((std::istreambuf_iterator<char>(file)),
					   std::istreambuf_iterator<char>());
		*content = std::move(s);
		return true;
	}

} // namespace

// static
bool Js::Init(const char* directory_path, const char* icu_data_file) {
#ifndef NDEBUG
	// Enables calling RequestGarbageCollectionForTesting to catch memory leaks
	// at shutdown.
	v8::V8::SetFlagsFromString("--expose_gc");
#endif
	platform = v8::platform::NewDefaultPlatform();
	if (!v8::V8::InitializeICUDefaultLocation(nullptr, icu_data_file))
		return false;
	v8::V8::InitializeExternalStartupData(directory_path);
	v8::V8::InitializePlatform(platform.get());
	ASSERT(v8::V8::Initialize());
	return true;
}

// static
void Js::Shutdown() {
	v8::V8::Dispose();
	v8::V8::DisposePlatform();
	platform.reset();
}

// static
double Js::MonotonicallyIncreasingTime() {
	return platform->MonotonicallyIncreasingTime();
}

Js::Js(Settings setting)
	: _delegate(setting.delegate),
	  _task_queue(setting.task_queue),
	  _base_path(std::move(setting.base_path)),
	  _lib_path(std::move(setting.lib_path)),
	  _suppress_next_script_result(false) {

	_allocator = std::unique_ptr<v8::ArrayBuffer::Allocator>(v8::ArrayBuffer::Allocator::NewDefaultAllocator());

	v8::Isolate::CreateParams params;
	params.array_buffer_allocator = _allocator.get();

	_isolate = v8::Isolate::New(params);
	// The number of data slots is a fixed constant in v8. Make sure
	// we have enough for our usage:
	// 0 is Js*.
	// 1 is JsApi*.
	ASSERT(_isolate->GetNumberOfDataSlots() == 4);
	_isolate->SetData(0, this);
	_isolate->SetCaptureStackTraceForUncaughtExceptions(true);
	_isolate->SetHostImportModuleDynamicallyCallback(ImportDynamic);
	_isolate->SetPromiseRejectCallback(HandlePromiseRejectCallback);

	v8::Locker locker(_isolate);
	v8::Isolate::Scope isolate_scope(_isolate);
	v8::HandleScope handle_scope(_isolate);
	_context.Reset(_isolate, v8::Context::New(_isolate));

	//_strings = std::make_unique<JsStrings>(_isolate);
}

Js::~Js() {
	_isolate->SetData(0, nullptr);
	//_strings.reset();
	_dynamic_imports.clear();
	_modules.clear();
	_context.Reset();
#ifndef NDEBUG
	_isolate->RequestGarbageCollectionForTesting(
			v8::Isolate::kFullGarbageCollection);
#endif
	_isolate->Dispose();
}

v8::Local<v8::String> Js::MakeString(std::string_view s) {
	if (s.empty()) [[unlikely]] {
		return v8::String::Empty(_isolate);
	} else {
		return v8::String::NewFromUtf8(_isolate, s.data(), v8::NewStringType::kNormal,
									   static_cast<int>(s.size()))
				.ToLocalChecked();
	}
}

v8::Local<v8::String> Js::MakeString(std::wstring_view s) {
	if (s.empty()) [[unlikely]] {
		return v8::String::Empty(_isolate);
	} else {
		return v8::String::NewFromTwoByte(_isolate, reinterpret_cast<const uint16_t*>(s.data()), v8::NewStringType::kNormal,
										  static_cast<int>(s.size()))
				.ToLocalChecked();
	}
}

std::string Js::ToString(v8::Local<v8::Value> value) {
	ASSERT(!value.IsEmpty());
	if (value->IsString()) {
		v8::Local<v8::String> v8s = value.As<v8::String>();
		std::string s;
		s.resize(v8s->Utf8Length(_isolate));
		v8s->WriteUtf8(_isolate, s.data());
		return s;
	} else if (value->IsModuleNamespaceObject()) {
		return "[Module]";
	} else if (value->IsSymbolObject()) {
		return "[SymbolObject]";
	} else if (value->IsSymbol()) {
		return ToString(value.As<v8::Symbol>()->Description(_isolate));
	} else {
		v8::String::Utf8Value v(_isolate, value);
		return *v != nullptr ? std::string(*v, static_cast<size_t>(v.length())) : ToString(value->TypeOf(_isolate));
	}
}

std::string Js::ToStringOr(v8::Local<v8::Value> value,
						   std::string_view or_string) {
	if (value.IsEmpty()) {
		return std::string(or_string);
	} else {
		return ToString(value);
	}
}

bool Js::GetBooleanOr(v8::Local<v8::Object> object, std::string_view key,
					  bool fallback) {
	v8::MaybeLocal<v8::Value> value = object->Get(context(), MakeString(key));
	if (!value.IsEmpty()) {
		v8::Local<v8::Value> v = value.ToLocalChecked();
		if (v->IsBoolean()) {
			return v.As<v8::Boolean>()->Value();
		}
	}
	return fallback;
}

bool Js::IsSubclassOf(v8::Local<v8::Function> child_class, v8::Local<v8::Function> parent_class) {
	v8::Local<v8::Value> childPrototype;
	if (!child_class->Get(context(), MakeString("prototype")).ToLocal(&childPrototype) ||
		!childPrototype->IsObject()) {
		return false;
	}

	v8::Local<v8::Value> currentProto = child_class->Get(context(), MakeString("__proto__")).ToLocalChecked();

	while (currentProto->IsObject()) {
		v8::Local<v8::Object> currentObject = currentProto.As<v8::Object>();
		if (currentObject == parent_class) {
			return true;
		}

		if (!currentObject->Get(context(), MakeString("__proto__")).ToLocal(&currentProto)) {
			break;
		}
	}

	return false;
}

void Js::ThrowError(std::string_view error) {
	_isolate->ThrowError(MakeString(error));
}

void Js::ThrowRangeError(std::string_view error) {
	_isolate->ThrowException(v8::Exception::RangeError(MakeString(error)));
}

void Js::ThrowTypeError(std::string_view error, v8::Local<v8::Value> value) {
	v8::Local<v8::String> name = value->TypeOf(_isolate);
	ThrowTypeError(std::format("{}, but {} provided", error, ToStringOr(name, "<invalid>")));
}

void Js::ThrowTypeError(std::string_view error) {
	_isolate->ThrowException(v8::Exception::TypeError(MakeString(error)));
}

void Js::ThrowIllegalConstructor() {
	ThrowTypeError("Illegal constructor");
}

void Js::ThrowIllegalInvocation() {
	ThrowTypeError("Illegal invocation");
}

void Js::ThrowInvalidArgument() {
	ThrowError("Invalid argument");
}

void Js::ReportException(v8::Local<v8::Message> message) {
	std::string str = ToString(message->Get());

	std::vector<std::string> trace;
	v8::Local<v8::StackTrace> stack_trace = message->GetStackTrace();
	if (!stack_trace.IsEmpty()) {
		int count = stack_trace->GetFrameCount();
		trace.reserve(static_cast<size_t>(count));
		for (int i = 0; i < count; ++i) {
			v8::Local<v8::StackFrame> frame = stack_trace->GetFrame(_isolate, i);
			std::string file = ToStringOr(frame->GetScriptName(), "<script>");
			std::string function = ToStringOr(frame->GetFunctionName(), "<top>");
			int line = frame->GetLineNumber();
			trace.emplace_back(std::format("{} ({}:{})", function, file, line));
		}
	}

	_delegate->OnJavascriptException(std::move(str), std::move(trace));
}

void Js::SuppressNextScriptResult() {
	_suppress_next_script_result = true;
}

std::string Js::ExecuteScript(std::string_view source) {
	JsScope scope(*this);
	v8::TryCatch try_catch(scope.isolate);

	// This is the ResourceName used in ImportDynamic below.
	auto resource_name = MakeString("<console>");
	constexpr int line_offset = 0;
	constexpr int column_offset = 0;
	constexpr bool is_shared_cross_origin = false;
	constexpr int script_id = -1;
	auto source_map_url = v8::Local<v8::Value>();
	constexpr bool is_opaque = false;
	constexpr bool is_warm = false;
	constexpr bool is_module = false;
	auto host_defined_options = v8::Local<v8::PrimitiveArray>();
	v8::ScriptOrigin origin(resource_name, line_offset, column_offset,
							is_shared_cross_origin, script_id, source_map_url,
							is_opaque, is_warm, is_module, host_defined_options);

	v8::Local<v8::String> s = MakeString(source);
	v8::MaybeLocal<v8::Script> script =
			v8::Script::Compile(scope.context, s, &origin);

	if (try_catch.HasCaught()) {
		ReportException(try_catch.Message());
		return {};
	}

	v8::MaybeLocal<v8::Value> result =
			script.ToLocalChecked()->Run(scope.context);

	if (try_catch.HasCaught()) {
		_suppress_next_script_result = false;
		ReportException(try_catch.Message());
		return {};
	}

	if (_suppress_next_script_result) {
		_suppress_next_script_result = false;
		return {};
	}

	return ToString(result.ToLocalChecked());
}

v8::Local<v8::Module> Js::LoadMainModule(std::string_view name) {
	v8::TryCatch try_catch(_isolate);

	std::string path = name.ends_with(".mjs") ? (_base_path / name).lexically_normal().string() : std::string(name);

	v8::Local<v8::Module> module = LoadModuleByPath(path, {});

	if (try_catch.HasCaught()) {
		ReportException(try_catch.Message());
		_delegate->OnMainModuleLoaded();
		return {};
	}

	return module;
}

// LoadModuleByPath is used in two flows:
// 1. Loading the main module
// 2. Loading a dynamically imported module.
//
// The "resolver" is set for the second case.
//
// LoadModuleByPath returns empty module if the module couldn't be located,
// instantiated, etc. In those cases, an exception has been thrown.
//
// Otherwise, the module is either fully loaded and ready, or has a top-level
// await and is still pending. In those case, a resolver or reject handler
// will handle cases 1 and 2 later.
v8::Local<v8::Module> Js::LoadModuleByPath(
		const std::string& path,
		v8::Local<v8::Promise::Resolver> resolver) {
	std::vector<fs::path> paths = { path };

	v8::Local<v8::Context> context = this->context();

	v8::Local<v8::Module> module = LoadModuleTree(context, path, paths);
	if (module.IsEmpty()) {
		return {};
	}

	// At this stage, the module is compiled but not instantiated yet.
	// Instantiate it now; this will recursively instantiate its dependencies
	// that haven't been instantiated yet.

	if (!module->InstantiateModule(context, ResolveModule).FromMaybe(false)) {
		return {};
	}

	v8::Local<v8::Value> result;
	if (!module->Evaluate(context).ToLocal(&result)) {
		return {};
	}

	v8::Local<v8::Promise> promise = result.As<v8::Promise>();

	if (resolver.IsEmpty()) {
		// Loading the main module.
		if (promise->State() == v8::Promise::kPending) {
			UNUSED(promise->Then(
					context,
					v8::Function::New(context, OnMainModuleResolve).ToLocalChecked(),
					v8::Function::New(context, OnMainModuleFailure).ToLocalChecked()));
		} else {
			_delegate->OnMainModuleLoaded();
		}
	} else {
		// Dynamic import: pass the result to the resolver.
		v8::Local<v8::Value> ns = module->GetModuleNamespace();
		if (promise->State() == v8::Promise::kPending) {
			v8::Local<v8::Array> data = v8::Array::New(_isolate, 2);
			ASSERT(data->Set(context, 0, resolver).FromMaybe(false));
			ASSERT(data->Set(context, 1, ns).FromMaybe(false));
			UNUSED(promise->Then(
					context,
					v8::Function::New(context, OnDynamicModuleResolve, data)
							.ToLocalChecked(),
					v8::Function::New(context, OnDynamicModuleFailure, resolver)
							.ToLocalChecked()));
		} else if (promise->State() == v8::Promise::kFulfilled) {
			ASSERT(resolver->Resolve(context, ns).FromMaybe(false));
		} else {
			RemovePendingFailedPromise(promise);
			ASSERT(resolver->Reject(context, promise->Result()).FromMaybe(false));
		}
	}

	return module;
}

v8::Local<v8::Module> Js::LoadModuleTree(
		v8::Local<v8::Context> context, const std::string& path,
		std::vector<fs::path>& paths) {
	// Check for recursive loading of the same module.
	// Note that recursive imports are supported: we only load module names that
	// haven't been loaded yet (recursively) below.
	auto it = _modules.find(path);
	if (it != _modules.end()) {
		return it->second.Get(_isolate);
	}

	v8::Local<v8::Module> module = LoadModule(path, paths);
	if (module.IsEmpty()) {
		return {};
	}

	// At this stage, the module is compiled but not instantiated yet.
	// Look up its dependencies, so that they can be instantiated later too.
	int id = module->IsSyntheticModule() ? module->GetIdentityHash() : module->ScriptId();
	ASSERT(_module_path_by_id.find(id) == _module_path_by_id.end());
	_modules[path] = v8::Global<v8::Module>(_isolate, module);
	_module_path_by_id[id] = path;

	fs::path dir = path;
	dir.remove_filename();

	v8::Local<v8::FixedArray> requests = module->GetModuleRequests();
	int len = requests->Length();
	for (int i = 0; i < len; ++i) {
		v8::Local<v8::ModuleRequest> request =
				requests->Get(context, i).As<v8::ModuleRequest>();
		std::string spec = ToString(request->GetSpecifier());
		if (!IsValidImport(spec)) {
			ThrowError(MakeInvalidImportError(spec));
			return {};
		}
		std::string subpath = ToImportPath(dir, spec);
		if (_modules.find(subpath) == _modules.end()) {
			paths.emplace_back(subpath);
			bool failed = LoadModuleTree(context, subpath, paths).IsEmpty();
			paths.pop_back();
			if (failed) {
				return {};
			}
		}
	}

	return module;
}

// static
v8::MaybeLocal<v8::Module> Js::ResolveModule(
		v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
		[[maybe_unused]] v8::Local<v8::FixedArray> import_assertions,
		v8::Local<v8::Module> referrer) {
	Js* js = Js::Get(context->GetIsolate());

	int id = referrer->IsSyntheticModule() ? referrer->GetIdentityHash() : referrer->ScriptId();
	auto it = js->_module_path_by_id.find(id);
	ASSERT(it != js->_module_path_by_id.end());
	fs::path referrer_dir = it->second;
	referrer_dir.remove_filename();

	std::string spec = js->ToString(specifier);
	std::string path = ToImportPath(referrer_dir, spec);

	auto it2 = js->_modules.find(path);
	ASSERT(it2 != js->_modules.end());

	return it2->second.Get(js->_isolate);
}

v8::Local<v8::Module> Js::LoadModule(
		const std::string& path,
		const std::vector<fs::path>& paths) {

	v8::Local<v8::Module> synthetic = _delegate->OnSyntheticModuleLoaded(path);
	if (!synthetic.IsEmpty()) {
		return synthetic;
	}

	v8::Local<v8::String> source = LoadModuleSource(path, paths);
	if (source.IsEmpty()) {
		return {};
	}

	v8::Local<v8::Module> module = CompileModule(source, path, paths);
	if (module.IsEmpty()) {
		return {};
	}

	return module;
}

v8::Local<v8::String> Js::LoadModuleSource(
		const std::string& path,
		const std::vector<fs::path>& paths) {
	std::string content;
	std::string error;
	if (path.ends_with(".mjs")) {
		if (!ReadFile(path, &content, &error)) {
			error += '\n';
			AppendModulePath(error, _base_path, paths);
			ThrowError(error);
			return {};
		}
	} else {
		if (!ReadFile(_lib_path / (path + ".mjs"), &content, &error)) {
			error += '\n';
			AppendModulePath(error, _lib_path, paths);
			ThrowError(error);
			return {};
		}
	}

	return MakeString(
			std::format("const __filename = {};const __dirname = {};{}",
						EscapeString(path),
						EscapeString(fs::path(path).parent_path().string()),
						content));
}

v8::Local<v8::Module> Js::CompileModule(
		v8::Local<v8::String> source, const std::string& path,
		const std::vector<fs::path>& paths) {
	// This is the ResourceName used in ImportDynamic below.
	auto resource_name = MakeString(path);
	constexpr int line_offset = 0;
	constexpr int column_offset = 0;
	constexpr bool is_shared_cross_origin = false;
	constexpr int script_id = -1;
	auto source_map_url = v8::Local<v8::Value>();
	constexpr bool is_opaque = false;
	constexpr bool is_warm = false;
	constexpr bool is_module = true;
	auto host_defined_options = v8::Local<v8::PrimitiveArray>();
	v8::ScriptOrigin origin(resource_name, line_offset, column_offset,
							is_shared_cross_origin, script_id, source_map_url,
							is_opaque, is_warm, is_module, host_defined_options);

	v8::ScriptCompiler::Source src(source, origin);

	v8::TryCatch try_catch(_isolate);
	v8::Local<v8::Module> module;

	if (v8::ScriptCompiler::CompileModule(_isolate, &src).ToLocal(&module)) {
		return module;
	}

	ASSERT(try_catch.HasCaught());
	std::string error(ToString(try_catch.Exception()));
	error += '\n';
	AppendModulePath(error, _base_path, paths);
	try_catch.Reset();
	ThrowError(error);
	try_catch.ReThrow();
	return {};
}

// static
void Js::OnMainModuleResolve(const v8::FunctionCallbackInfo<v8::Value>& info) {
	Js::Get(info.GetIsolate())->_delegate->OnMainModuleLoaded();
}

// static
void Js::OnMainModuleFailure(const v8::FunctionCallbackInfo<v8::Value>& info) {
	ASSERT(info.Length() >= 1);
	Js* js = Js::Get(info.GetIsolate());
	v8::Local<v8::Value> error = info[0];
	v8::Local<v8::Message> message = MakeErrorMessage(js->_isolate, error);
	js->ReportException(message);
	js->_delegate->OnMainModuleLoaded();
}

// static
void Js::OnDynamicModuleResolve(
		const v8::FunctionCallbackInfo<v8::Value>& info) {
	Js* js = Js::Get(info.GetIsolate());
	v8::Local<v8::Array> data = info.Data().As<v8::Array>();
	v8::Local<v8::Promise::Resolver> resolver =
			data->Get(js->context(), 0).ToLocalChecked().As<v8::Promise::Resolver>();
	v8::Local<v8::Value> ns =
			data->Get(js->context(), 1).ToLocalChecked().As<v8::Value>();
	ASSERT(resolver->Resolve(js->context(), ns).FromMaybe(false));
}

// static
void Js::OnDynamicModuleFailure(
		const v8::FunctionCallbackInfo<v8::Value>& info) {
	Js* js = Js::Get(info.GetIsolate());
	v8::Local<v8::Promise::Resolver> resolver =
			info.Data().As<v8::Promise::Resolver>();
	ASSERT(resolver->Reject(js->context(), info[0]).FromMaybe(false));
}

// static
v8::MaybeLocal<v8::Promise> Js::ImportDynamic(
		[[maybe_unused]] v8::Local<v8::Context> context,
		[[maybe_unused]] v8::Local<v8::Data> host_defined_options,
		v8::Local<v8::Value> resource_name, v8::Local<v8::String> specifier,
		[[maybe_unused]] v8::Local<v8::FixedArray> import_assertions) {
	Js* js = Js::Get(context->GetIsolate());
	return js->ImportDynamic(resource_name, specifier);
}

v8::MaybeLocal<v8::Promise> Js::ImportDynamic(
		v8::Local<v8::Value> resource_name, v8::Local<v8::String> specifier) {
	v8::MaybeLocal<v8::Promise::Resolver> maybe_resolver =
			v8::Promise::Resolver::New(context());
	v8::Local<v8::Promise::Resolver> resolver;
	if (!maybe_resolver.ToLocal(&resolver)) {
		return {};
	}

	ASSERT(resource_name->IsString());
	std::string ref = ToString(resource_name);
	std::string spec = ToString(specifier);

	if (!IsValidImport(spec)) {
		ASSERT(resolver->Reject(context(), MakeString(MakeInvalidImportError(spec)))
					   .FromMaybe(false));
		return resolver->GetPromise();
	}

	fs::path dir;

	if (ref == "<console>") {
		dir = _base_path;
	} else {
		ASSERT(_modules.find(ref) != _modules.end());
		dir = ref;
		dir.remove_filename();
	}

	std::string path = ToImportPath(dir, spec);

	auto it = _dynamic_imports.find(path);
	if (it != _dynamic_imports.end()) {
		// Already importing; return the existing promise.
		return it->second.Get(_isolate)->GetPromise();
	}

	_dynamic_imports[path].Reset(_isolate, resolver);

	_task_queue->Post([this, path = std::move(path)]() {
		ImportDynamic(path);
	});

	return resolver->GetPromise();
}

void Js::ImportDynamic(const std::string& path) {
	v8::Locker locker(_isolate);
	JsScope scope(*this);
	v8::TryCatch try_catch(_isolate);

	auto it = _dynamic_imports.find(path);
	ASSERT(it != _dynamic_imports.end());

	v8::Local<v8::Promise::Resolver> resolver = it->second.Get(_isolate);
	_dynamic_imports.erase(it);

	v8::Local<v8::Module> module = LoadModuleByPath(path, resolver);
	if (!module.IsEmpty()) {
		// Everything has been handled inside LoadModuleByPath.
		try_catch.Reset();
	} else {
		// LoadModuleByPath failed to load the module.
		v8::Local<v8::Value> exception;
		if (try_catch.HasCaught()) {
			exception = try_catch.Exception();
			// Reset() is important to clear the pending exception state;
			// otherwise, v8 might crash. Repro: await on a dynamic import()
			// that has a syntax error in the imported module.
			try_catch.Reset();
		} else {
			exception = v8::Exception::Error(
					v8::String::NewFromUtf8Literal(_isolate, "Failed to import."));
		}
		ASSERT(!exception.IsEmpty());
		ASSERT(resolver->Reject(scope.context, exception).FromMaybe(false));
	}
}

void Js::RemovePendingFailedPromise(v8::Local<v8::Promise> promise) {
	auto it = _failed_promises.begin();
	while (it != _failed_promises.end()) {
		v8::Local<v8::Promise> failed_promise = it->first.Get(_isolate);
		if (failed_promise == promise) {
			it = _failed_promises.erase(it);
		} else {
			++it;
		}
	}
}

// static
void Js::HandlePromiseRejectCallback(v8::PromiseRejectMessage data) {
	if (data.GetEvent() == v8::kPromiseRejectAfterResolved ||
		data.GetEvent() == v8::kPromiseResolveAfterResolved) {
		return;
	}

	v8::Local<v8::Promise> promise = data.GetPromise();
	v8::Isolate* isolate = promise->GetIsolate();
	Js* js = Js::Get(isolate);

	if (data.GetEvent() == v8::kPromiseRejectWithNoHandler) {
		// An promise has been rejected and there is no failure handler yet.
		// A handler might be added asynchronously later, so keep this failure
		// around until Javascript has returned to native;
		// see HandleUncaughtExceptionsInPromises below.
		v8::Local<v8::Value> exception = data.GetValue();
		v8::Local<v8::Message> message = MakeErrorMessage(isolate, exception);
		js->_failed_promises.emplace_back(
				v8::Global<v8::Promise>(isolate, promise),
				v8::Global<v8::Message>(isolate, message));
	} else if (data.GetEvent() == v8::kPromiseHandlerAddedAfterReject) {
		// A handler has been added after a promise has failed;
		// ignore its exception and don't report it.
		js->RemovePendingFailedPromise(promise);
	}
}

void Js::HandleUncaughtExceptionsInPromises() {
	if (_failed_promises.empty()) {
		return;
	}

	std::vector<std::pair<v8::Global<v8::Promise>, v8::Global<v8::Message>>> list;
	_failed_promises.swap(list);

	for (const auto& [_, message] : list) {
		ReportException(message.Get(_isolate));
	}
}