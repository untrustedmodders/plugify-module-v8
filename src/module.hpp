#include "module_loader.hpp"
#include "task_scheduler.hpp"

#include <plugify/any.hpp>
#include <plugify/jit/call.hpp>
#include <plugify/jit/callback.hpp>
#include <plugify/language_module.hpp>
#include <plugify/log.hpp>
#include <plugify/method.hpp>
#include <plugify/module.hpp>
#include <plugify/plugify_provider.hpp>
#include <plugify/plugin.hpp>
#include <plugify/plugin_descriptor.hpp>
#include <plugify/plugin_reference_descriptor.hpp>
#include <plugify/string.hpp>
#include <plugify/vector.hpp>

#include <asmjit/asmjit.h>
#include <v8.h>

namespace v8lm {
	// std::hash<std::filesystem::path> not in the C++20 standard by default
	struct path_hash {
		auto operator()(const fs::path& path) const noexcept {
			return hash_value(path);
		}
	};

	// heterogeneous lookup
	struct string_hash {
		using is_transparent = void;
		[[nodiscard]] size_t operator()(const char* txt) const {
			return std::hash<std::string_view>{}(txt);
		}
		[[nodiscard]] size_t operator()(std::string_view txt) const {
			return std::hash<std::string_view>{}(txt);
		}
		[[nodiscard]] size_t operator()(const std::string& txt) const {
			return std::hash<std::string>{}(txt);
		}
	};

	using ExecuteModuleError = std::string;
	using ExecuteModuleData = v8::Local<v8::Module>;
	using ExecuteModuleScript = v8::Local<v8::Script>;
	using ExecuteModuleResult = std::variant<ExecuteModuleError, ExecuteModuleData,ExecuteModuleScript>;

	using JsFunction = std::shared_ptr<v8::Global<v8::Function>>;
	using JsExternalMap = std::unordered_map<void*, JsFunction>;
	using JsExceptionList = std::vector<std::pair<v8::Global<v8::Promise>, v8::Global<v8::Message>>>;

	struct JsMethodData {
		plugify::JitCallback jitCallback;
		JsFunction jsFunction;
	};

	enum class JsType : uint8_t {
		Invalid,
		String,
		BigInt,
		Int32,
		Uint32,
		Number,
		Boolean,
		External,
		Vector2,
		Vector3,
		Vector4,
		Matrix4x4
	};

	class V8LanguageModule final : public plugify::ILanguageModule {
	public:
		V8LanguageModule() = default;

		// ILanguageModule
		plugify::InitResult Initialize(std::weak_ptr<plugify::IPlugifyProvider> provider, plugify::ModuleHandle module) override;
		void Shutdown() override;
		void OnUpdate(plugify::DateTime dt) override;
		plugify::LoadResult OnPluginLoad(plugify::PluginHandle plugin) override;
		void OnPluginStart(plugify::PluginHandle plugin) override;
		void OnPluginUpdate(plugify::PluginHandle plugin, plugify::DateTime dt) override;
		void OnPluginEnd(plugify::PluginHandle plugin) override;
		void OnMethodExport(plugify::PluginHandle plugin) override;
		bool IsDebugBuild() override;

		static V8LanguageModule* Get(v8::Isolate* isolate) { return static_cast<V8LanguageModule*>(isolate->GetData(0)); }
		const std::shared_ptr<plugify::IPlugifyProvider>& GetProvider() const { return _provider; }

	private:
		v8::MaybeLocal<v8::Module> CreateInternalModule(plugify::PluginHandle plugin);
		v8::MaybeLocal<v8::Module> CreateExternalModule(plugify::PluginHandle plugin);
		void GenerateEnum(plugify::PropertyHandle paramType);
		void GenerateEnum(plugify::MethodHandle method);
		v8::Local<v8::Function> FindJavascriptMethod(plugify::MemAddr addr) const;
		void AddToFunctionsMap(void* funcAddr, const JsFunction& funcObj);
		JsFunction FindExternal(void* funcAddr) const;
		void* FindInternal(v8::Local<v8::Function> object) const;

		JsType GetType(v8::Local<v8::Value> value);
		template<typename T>
		std::optional<T> ValueFromObject(v8::Local<v8::Value> value);
		template<class T>
		std::optional<T> ValueFromIntegerObject(v8::Local<v8::Value> value);
		template<class T>
		std::optional<T> ValueFromNumberObject(v8::Local<v8::Value> value);
		template<typename T>
		std::optional<plg::vector<T>> ArrayFromObject(v8::Local<v8::Value> arrayValue);
		v8::Local<v8::Value> CreateJsObject(v8::Local<v8::Function> ctor, std::span<v8::Local<v8::Value>> args);
		template<typename T>
		v8::Local<v8::Value> CreateJsObject(const T& value);
		template<typename T>
		v8::Local<v8::Value> CreateJsObjectList(std::span<const T> arrayArg);
		template<typename T>
		v8::Local<v8::Value> CreateJsObjectList(const plg::vector<T>& arrayArg);
		std::optional<void*> GetOrCreateFunctionValue(plugify::MethodHandle method, v8::Local<v8::Value> value);
		v8::Local<v8::Value> GetOrCreateFunctionObject(plugify::MethodHandle method, void* funcAddr);
		template<typename T>
		std::optional<T> GetObjectAttrAsValue(v8::Local<v8::Object> object, std::string_view attrName);
		bool IsSubclassOf(v8::Local<v8::Context> context, v8::Local<v8::Function> childClass, v8::Local<v8::Function> parentClass);

		template<typename T>
		void* CreateValue(v8::Local<v8::Value> value);
		template<typename T>
		void* CreateArray(v8::Local<v8::Value> value);

		void SetFallbackReturn(plugify::ValueType retType, const plugify::JitCallback::Return* ret);
		bool SetReturn(v8::Local<v8::Value> result, plugify::PropertyHandle retType, const plugify::JitCallback::Return* ret);
		bool SetRefParam(v8::Local<v8::Value> object, plugify::PropertyHandle paramType, const plugify::JitCallback::Parameters* params, size_t index);
		v8::Local<v8::Value> ParamToObject(plugify::PropertyHandle paramType, const plugify::JitCallback::Parameters* params, size_t index);
		v8::Local<v8::Value> ParamRefToObject(plugify::PropertyHandle paramType, const plugify::JitCallback::Parameters* params, size_t index);

		struct ArgsScope {
			plugify::JitCall::Parameters params;
			std::vector<std::pair<void*, plugify::ValueType>> storage; // used to store array temp memory

			explicit ArgsScope(size_t size);
			~ArgsScope();
		};
		
		void BeginExternalCall(plugify::ValueType retType, ArgsScope& a);
		v8::Local<v8::Value> MakeExternalCallWithObject(plugify::PropertyHandle retType, plugify::JitCall::CallingFunc func, const ArgsScope& a, plugify::JitCall::Return& ret);
		bool PushObjectAsParam(plugify::PropertyHandle paramType, v8::Local<v8::Value> item, ArgsScope& a);
		bool PushObjectAsRefParam(plugify::PropertyHandle paramType, v8::Local<v8::Value> item, ArgsScope& a);
		v8::Local<v8::Value> StorageValueToObject(plugify::PropertyHandle paramType, const ArgsScope& a, size_t index);

		struct ModuleInfo {
			std::unordered_map<std::string, v8::Global<v8::Module>> resolveCache;
			v8::Global<v8::Value> cJSValue;
			v8::Global<v8::Module> module;
		};

		ExecuteModuleResult ExecuteModule(v8::Local<v8::Context> context, const fs::path& requiringDir, const std::string& moduleName);
		v8::MaybeLocal<v8::Module> LoadModule(v8::Local<v8::Context> context, const fs::path& path, v8::Local<v8::Promise::Resolver> resolver);
		bool LoadFile(const fs::path& requiringDir, const std::string& moduleName, fs::path& path, std::string& content);
		v8::MaybeLocal<v8::Module> FetchESModuleTree(v8::Local<v8::Context> context, const fs::path& path);
		v8::MaybeLocal<v8::Module> FetchCJSModuleAsESModule(v8::Local<v8::Context> context, const std::string& moduleName);
		std::unordered_multimap<int, V8LanguageModule::ModuleInfo>::iterator FindModuleInfo(v8::Local<v8::Module> Module);
		static v8::MaybeLocal<v8::Module> ResolveModule(
				v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
				v8::Local<v8::FixedArray> importAttributes, v8::Local<v8::Module> referrer);

		void EvalScript(const v8::FunctionCallbackInfo<v8::Value>& info);
		void SearchModule(const v8::FunctionCallbackInfo<v8::Value>& info);
		void LoadModule(const v8::FunctionCallbackInfo<v8::Value>& info);
		void FindModule(const v8::FunctionCallbackInfo<v8::Value>& info);

		static void OnMainModuleResolve(const v8::FunctionCallbackInfo<v8::Value>& info);
		static void OnMainModuleFailure(const v8::FunctionCallbackInfo<v8::Value>& info);
		static void OnDynamicModuleResolve(const v8::FunctionCallbackInfo<v8::Value>& info);
		static void OnDynamicModuleFailure(const v8::FunctionCallbackInfo<v8::Value>& info);

		static v8::MaybeLocal<v8::Promise> ImportDynamic(
				v8::Local<v8::Context> context,
				v8::Local<v8::Data> hostDefinedOptions,
				v8::Local<v8::Value> resourceName,
				v8::Local<v8::String> specifier,
				v8::Local<v8::FixedArray> importAssertions);

		v8::MaybeLocal<v8::Promise> ImportDynamic(v8::Local<v8::Context> context,
												  v8::Local<v8::Value> resourceName,
												  v8::Local<v8::String> specifier);

		void ImportDynamic(const fs::path& path);
		void CallTimeout(uint32_t id);

		void RemovePendingFailedPromise(v8::Local<v8::Promise> promise);

	public:
		void ThrowException(std::string_view error) const;
		void ThrowRangeError(std::string_view error) const;
		void ThrowTypeError(std::string_view error, v8::Local<v8::Value> value) const;
		void ThrowTypeError(std::string_view error) const;
		void ReportException(v8::Local<v8::Message> message) const;
		void HandleUncaughtExceptionsInPromises();

		v8::Local<v8::String> MakeString(std::string_view value) const;
		v8::Local<v8::String> MakeString(std::wstring_view value) const;
		std::string ToString(v8::Local<v8::Value> value) const;
		std::string ToStringOr(v8::Local<v8::Value> value, std::string_view or_string) const;
		std::wstring ToWString(v8::Local<v8::Value> value) const;
		std::wstring ToWStringOr(v8::Local<v8::Value> value, std::wstring_view or_string) const;
		fs::path ToPath(v8::Local<v8::Value> value) const;
		fs::path ToPathOr(v8::Local<v8::Value> value, fs::path or_path) const;

		void InternalCall(plugify::MethodHandle method, plugify::MemAddr data, const plugify::JitCallback::Parameters* params, size_t count, const plugify::JitCallback::Return* ret);
		void ExternalCall(plugify::MethodHandle method, plugify::MemAddr data, const plugify::JitCallback::Parameters* params, size_t count, const plugify::JitCallback::Return* ret);

		uint32_t AddTask(std::chrono::milliseconds delay, Action action, bool repeat = false) {
			return _taskScheduler.AddTask(delay, std::move(action), repeat);
		}

		void RemoveTask(uint32_t id) {
			_taskScheduler.RemoveTask(id);
		}

	private:
		std::shared_ptr<plugify::IPlugifyProvider> _provider;
		std::shared_ptr<asmjit::JitRuntime> _jitRuntime;
		std::unique_ptr<v8::ArrayBuffer::Allocator> _allocator;
		v8::Isolate* _isolate;
		v8::Global<v8::Context> _context;
		v8::Global<v8::Function> _pluginClassObject;
		v8::Global<v8::Function> _vector2ClassObject;
		v8::Global<v8::Function> _vector3ClassObject;
		v8::Global<v8::Function> _vector4ClassObject;
		v8::Global<v8::Function> _matrix4x4ClassObject;
		v8::Global<v8::Function> _require;
		v8::Global<v8::Function> _getESMMain;
		struct PluginData {
			v8::Global<v8::Module> module;
			v8::Global<v8::Object> instance;
			v8::Global<v8::Function> update;
			v8::Global<v8::Function> start;
			v8::Global<v8::Function> end;
		};
		std::map<plugify::UniqueId, PluginData> _pluginsMap;
		std::vector<JsMethodData> _jsMethods;
		struct JitHolder {
			plugify::JitCallback jitCallback;
			plugify::JitCall jitCall;
		};
		std::vector<JitHolder> _moduleFunctions;
		struct ExternalHolder {
			plugify::JitCallback jitCallback;
			plugify::JitCall jitCall;
			JsFunction object;
		};
		std::vector<ExternalHolder> _externalFunctions;
		std::vector<JsMethodData> _internalFunctions;
		JsExternalMap _externalMap;

		std::unique_ptr<ModuleLoader> _moduleLoader;
		std::unordered_map<fs::path, v8::Global<v8::Module>, path_hash> _pathToModule;
		std::unordered_multimap<int, ModuleInfo> _idToModuleInfo;
		std::unordered_map<fs::path, v8::Global<v8::Promise::Resolver>, path_hash> _dynamicImports;
		JsExceptionList _failedPromises;

		TaskScheduler _taskScheduler;
	};

	V8LanguageModule g_v8lm;
}