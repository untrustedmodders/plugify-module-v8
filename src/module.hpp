#pragma once

#include "module_loader.hpp"
#include "task_scheduler.hpp"

#include <plugify/call.hpp>
#include <plugify/callback.hpp>
#include <plugify/language_module.hpp>
#include <plugify/logger.hpp>
#include <plugify/method.hpp>
#include <plugify/extension.hpp>
#include <plugify/provider.hpp>
#include <plugify/enum_object.hpp>
#include <plugify/enum_value.hpp>
#include <plugify/binding.hpp>
#include <plugify/alias.hpp>

#include <plg/any.hpp>
#include <plg/string.hpp>
#include <plg/vector.hpp>

#include <v8.h>

using namespace plugify;

namespace v8lm {
	using JsFunction = std::shared_ptr<v8::Global<v8::Function>>;
	using JsExternalMap = std::unordered_map<void*, JsFunction>;
	using JsExceptionList = std::vector<std::pair<v8::Global<v8::Promise>, v8::Global<v8::Message>>>;

	struct JsMethodData {
		JitCallback jitCallback;
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

	class V8LanguageModule final : public ILanguageModule {
	public:
		V8LanguageModule() = default;

		// ILanguageModule
		Result<InitData> Initialize(const Provider& provider, const Extension& module) override;
		void Shutdown() override;
		void OnUpdate(std::chrono::milliseconds dt) override;

		Result<LoadData> OnPluginLoad(const Extension& plugin) override;
		void OnPluginStart(const Extension& plugin) override;
		void OnPluginUpdate(const Extension& plugin, std::chrono::milliseconds dt) override;
		void OnPluginEnd(const Extension& plugin) override;
		void OnMethodExport(const Extension& plugin) override;
		bool IsDebugBuild() override;

		static V8LanguageModule* Get(v8::Isolate* isolate) { return static_cast<V8LanguageModule*>(isolate->GetData(v8::Isolate::GetNumberOfDataSlots() - 1)); }
		const std::unique_ptr<Provider>& GetProvider() const { return _provider; }

	private:
		Result<JsMethodData> GenerateMethodExport(const Method& method, v8::Local<v8::Context> context, v8::Local<v8::Object> exports);
		v8::MaybeLocal<v8::Module> CreateInternalModule(const Extension& plugin);
		v8::MaybeLocal<v8::Module> CreateExternalModule(const Extension& plugin);
		void CreateEnumObject(const Property& paramType);
		void CreateEnumObject(const Method& method);
		v8::Local<v8::Value> ConvertAlias(const Alias& alias);
		v8::Local<v8::Array> ConvertBinding(const Binding& binding);
		v8::Local<v8::Value> GetInvalidValueForType(ValueType type, std::string_view invalidValue);
		bool CreateClassObject(const Class& classData);
		v8::Local<v8::Function> FindJavascriptMethod(MemAddr addr) const;
		void AddToFunctionsMap(void* funcAddr, const JsFunction& funcObj);
		void AddToObjectsVec(v8::Global<v8::Object>&& anyObj);
		JsFunction FindExternal(void* funcAddr) const;
		void* FindInternal(v8::Local<v8::Function> object) const;

		JsType GetType(v8::Local<v8::Value> value);
		template<typename T>
		std::optional<T> ValueFromObject(v8::Local<v8::Value> value);
		template<class T> requires(std::is_signed_v<T> || std::is_unsigned_v<T>)
		std::optional<T> ValueFromIntegerObject(v8::Local<v8::Value> value);
		template<class T> requires(std::is_floating_point_v<T>)
		std::optional<T> ValueFromNumberObject(v8::Local<v8::Value> value);
		template<typename T>
		std::optional<plg::vector<T>> ArrayFromObject(v8::Local<v8::Value> arrayValue);
		v8::Local<v8::Value> CreateJsObject(v8::Local<v8::Function> ctor, std::span<v8::Local<v8::Value>> args);
		v8::Local<v8::Value> CreateJsObject();
		template<typename T>
		v8::Local<v8::Value> CreateJsObject(const T& value);
		template<typename T>
		v8::Local<v8::Value> CreateJsObjectList(std::span<const T> arrayArg);
		template<typename T>
		v8::Local<v8::Value> CreateJsObjectList(const plg::vector<T>& arrayArg);
		std::optional<void*> GetOrCreateFunctionValue(const Method& method, v8::Local<v8::Value> value);
		v8::Local<v8::Value> GetOrCreateFunctionObject(const Method& method, void* funcAddr);
		template<typename T>
		std::optional<T> GetObjectAttrAsValue(v8::Local<v8::Object> object, std::string_view attrName);
		bool IsSubclassOf(v8::Local<v8::Context> context, v8::Local<v8::Function> childClass, v8::Local<v8::Function> parentClass);

		template<typename T>
		void* CreateValue(v8::Local<v8::Value> value);
		template<typename T>
		void* CreateArray(v8::Local<v8::Value> value);

		void SetFallbackReturn(ValueType retType, ReturnSlot& ret);
		bool SetReturn(v8::Local<v8::Value> result, const Property& retType, ReturnSlot& ret);
		bool SetRefParam(v8::Local<v8::Value> object, const Property& paramType, ParametersSpan& params, size_t index);
		v8::Local<v8::Value> ParamToObject(const Property& paramType, ParametersSpan& params, size_t index);
		v8::Local<v8::Value> ParamRefToObject(const Property& paramType, ParametersSpan& params, size_t index);

		struct ArgsScope {
			Parameters params;
			std::inplace_vector<std::pair<void*, ValueType>, Signature::kMaxFuncArgs> storage; // used to store array temp memory

			explicit ArgsScope(size_t size);
			~ArgsScope();
		};
		
		void BeginExternalCall(ValueType retType, ArgsScope& a) const;
		v8::Local<v8::Value> MakeExternalCallWithObject(const Property& retType, JitCall::CallingFunc func, const ArgsScope& a, Return& ret);
		bool PushObjectAsParam(const Property& paramType, v8::Local<v8::Value> item, ArgsScope& a);
		bool PushObjectAsRefParam(const Property& paramType, v8::Local<v8::Value> item, ArgsScope& a);
		v8::Local<v8::Value> StorageValueToObject(const Property& paramType, const ArgsScope& a, size_t index);

		struct ModuleInfo {
			std::unordered_map<std::string, v8::Global<v8::Module>> resolveCache;
			v8::Global<v8::Value> cJSValue;
			v8::Global<v8::Module> module;
		};

		Result<v8::Local<v8::Data>> ExecuteModule(v8::Local<v8::Context> context, const fs::path& requiringDir, const std::string& moduleName);
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
												  v8::Local<v8::String> specifier,
												  v8::Local<v8::FixedArray> importAssertions);

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

		void InternalCall(const Method& method, MemAddr data, uint64_t* params, size_t count, void* ret);
		void ExternalCall(const Method& method, MemAddr data, uint64_t* parameters, size_t count, void* ret);

		uint32_t AddTask(std::chrono::milliseconds delay, Action action, bool repeat = false) {
			return _taskScheduler.AddTask(delay, std::move(action), repeat);
		}

		void RemoveTask(uint32_t id) {
			_taskScheduler.RemoveTask(id);
		}

	private:
		std::unique_ptr<Provider> _provider;
		std::unique_ptr<v8::ArrayBuffer::Allocator> _allocator;
		v8::Isolate* _isolate;
		v8::Global<v8::Context> _context;
		v8::Global<v8::Function> _pluginClassObject;
		v8::Global<v8::Function> _vector2ClassObject;
		v8::Global<v8::Function> _vector3ClassObject;
		v8::Global<v8::Function> _vector4ClassObject;
		v8::Global<v8::Function> _matrix4x4ClassObject;
		v8::Global<v8::Function> _bindClassMethodsFunc;
		v8::Global<v8::Function> _require;
		v8::Global<v8::Function> _getESMMain;
		struct PluginData {
			v8::Global<v8::Module> module;
			v8::Global<v8::Object> instance;
			v8::Global<v8::Function> update;
			v8::Global<v8::Function> start;
			v8::Global<v8::Function> end;
		};
		std::map<UniqueId, PluginData> _pluginsMap;
		std::vector<JsMethodData> _jsMethods;
		std::vector<v8::Global<v8::Object>> _jsObjects;
		struct JitHolder {
			JitCallback jitCallback;
			JitCall jitCall;
		};
		std::vector<JitHolder> _moduleFunctions;
		struct ExternalHolder {
			JitCallback jitCallback;
			JitCall jitCall;
			JsFunction object;
		};
		std::vector<ExternalHolder> _externalFunctions;
		std::vector<JsMethodData> _internalFunctions;
		JsExternalMap _externalMap;

		std::unique_ptr<ModuleLoader> _moduleLoader;
		std::unordered_map<fs::path, v8::Global<v8::Module>, plg::path_hash> _pathToModule;
		std::unordered_multimap<int, ModuleInfo> _idToModuleInfo;
		std::unordered_map<fs::path, v8::Global<v8::Promise::Resolver>, plg::path_hash> _dynamicImports;
		JsExceptionList _failedPromises;

		TaskScheduler _taskScheduler;
	};

	V8LanguageModule g_v8lm;
}