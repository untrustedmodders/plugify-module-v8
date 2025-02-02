#pragma once

#include "task_queue.hpp"
#include "assert.hpp"

#include <v8.h>

namespace v8lm {

	// Wrapper around a v8 Isolate and Context.
	//
	// This class manages module loading.
	//
	// v8 is multi-threaded, and multiple threads may enter the Javascript context.
	// Only one thread will be executing it at a time though.
	//
	// Sometimes we enter Javascript from the main thread, but sometimes it also
	// enters from a background thread (e.g. to run background tasks while main
	// is swapping buffers and waiting for vsync, or to run async tasks that have
	// resolved).
	class Js final {
	public:
		static bool Init(const char* directory_path, const char* icu_data_file);
		static void Shutdown();
		static double MonotonicallyIncreasingTime();

		class Delegate {
		public:
			virtual ~Delegate() = default;

			virtual v8::Local<v8::Module> OnSyntheticModuleLoaded(std::string_view path) { return {}; }
			virtual void OnMainModuleLoaded() {}
			virtual void OnJavascriptException(std::string message, std::vector<std::string> stack_trace) {}
		};

		struct Settings {
			Delegate* delegate;
			TaskQueue* task_queue;
			fs::path base_path;
			fs::path lib_path;
		};

		// All of these dependencies must outlive the Js object.
		// If the object is deleted, then TaskQueue must *not* run any pending tasks
		// anymore.
		explicit Js(Settings setting);
		~Js();

		static Js* Get(v8::Isolate* isolate) {
			return static_cast<Js*>(isolate->GetData(0));
		}

		static Js* GetCurrent() { return Get(v8::Isolate::GetCurrent()); }

		v8::Isolate* isolate() { return _isolate; }
		v8::Local<v8::Context> context() { return _context.Get(_isolate); }
		v8::Local<v8::Object> global() { return context()->Global(); }
		//JsStrings* strings() { return _strings.get(); }

		v8::Local<v8::String> MakeString(std::string_view s);
		v8::Local<v8::String> MakeString(std::wstring_view s);
		std::string ToString(v8::Local<v8::Value> value);
		std::string ToStringOr(v8::Local<v8::Value> value,
							   std::string_view or_string);

		bool GetBooleanOr(v8::Local<v8::Object> object, std::string_view key,
						  bool fallback);

		bool IsSubclassOf(v8::Local<v8::Function> child_class, v8::Local<v8::Function> parent_class);

		void ThrowRangeError(std::string_view error);
		void ThrowError(std::string_view error);
		void ThrowTypeError(std::string_view error, v8::Local<v8::Value> value);
		void ThrowTypeError(std::string_view error);
		void ThrowIllegalConstructor();
		void ThrowIllegalInvocation();
		void ThrowInvalidArgument();
		void ReportException(v8::Local<v8::Message> message);
		void HandleUncaughtExceptionsInPromises();

		v8::Local<v8::Module> LoadMainModule(std::string_view name);

		std::string ExecuteScript(std::string_view source);
		void SuppressNextScriptResult();

	private:
		v8::Local<v8::Module> LoadModuleByPath(const std::string& path,
											   v8::Local<v8::Promise::Resolver> resolver);

		v8::Local<v8::Module> LoadModuleTree(
				v8::Local<v8::Context> context, const std::string& path,
				std::vector<fs::path>& paths);

		static v8::MaybeLocal<v8::Module> ResolveModule(
				v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
				v8::Local<v8::FixedArray> import_assertions,
				v8::Local<v8::Module> referrer);

		v8::Local<v8::Module> LoadModule(
				const std::string& path,
				const std::vector<fs::path>& paths);

		v8::Local<v8::String> LoadModuleSource(
				const std::string& path,
				const std::vector<fs::path>& paths);

		v8::Local<v8::Module> CompileModule(
				v8::Local<v8::String> source, const std::string& path,
				const std::vector<fs::path>& paths);

		static void OnMainModuleResolve(
				const v8::FunctionCallbackInfo<v8::Value>& info);
		static void OnMainModuleFailure(
				const v8::FunctionCallbackInfo<v8::Value>& info);

		static void OnDynamicModuleResolve(
				const v8::FunctionCallbackInfo<v8::Value>& info);
		static void OnDynamicModuleFailure(
				const v8::FunctionCallbackInfo<v8::Value>& info);

		static v8::MaybeLocal<v8::Promise> ImportDynamic(
				v8::Local<v8::Context> context,
				v8::Local<v8::Data> host_defined_options,
				v8::Local<v8::Value> resource_name,
				v8::Local<v8::String> specifier,
				v8::Local<v8::FixedArray> import_assertions);

		v8::MaybeLocal<v8::Promise> ImportDynamic(v8::Local<v8::Value> resource_name,
												  v8::Local<v8::String> specifier);

		void ImportDynamic(const std::string& path);

		static void HandlePromiseRejectCallback(v8::PromiseRejectMessage message);
		void RemovePendingFailedPromise(v8::Local<v8::Promise> promise);

		Delegate* _delegate;
		TaskQueue* _task_queue;
		fs::path _base_path;
		fs::path _lib_path;

		std::unique_ptr<v8::ArrayBuffer::Allocator> _allocator;

		v8::Isolate* _isolate;
		v8::Global<v8::Context> _context;
		std::unordered_map<std::string, v8::Global<v8::Module>> _modules;
		std::unordered_map<int, std::string> _module_path_by_id;
		std::unordered_map<std::string, v8::Global<v8::Promise::Resolver>>
				_dynamic_imports;
		std::vector<std::pair<v8::Global<v8::Promise>, v8::Global<v8::Message>>>
				_failed_promises;

		//std::unique_ptr<JsStrings> _strings;

		bool _suppress_next_script_result;
	};
}