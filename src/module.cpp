#include "module.hpp"

#include <libplatform/libplatform.h>
#include <module_export.h>
#include <source_location>

#define LOG_PREFIX "[V8LM] "

#define ASSERT(cond)                                                                                                                                                            \
	if (!(cond)) [[unlikely]] {                                                                                                                                                 \
		constexpr std::source_location location = std::source_location::current();                                                                                              \
		std::puts(std::format("Assertion failed on {} ({}:{}) `{}`: " #cond "\n", location.file_name(), location.line(), location.column(), location.function_name()).c_str()); \
		std::terminate();                                                                                                                                                       \
	}

#define ASSERT_MSG(cond, msg)                                                                                                                                                   \
	if (!(cond)) [[unlikely]] {                                                                                                                                                 \
		constexpr std::source_location location = std::source_location::current();                                                                                              \
		std::puts(std::format("Assertion failed on {} ({}:{}) `{}`: {}\n", location.file_name(), location.line(), location.column(), location.function_name(), msg).c_str());   \
		std::terminate();                                                                                                                                                       \
	}

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b
#define UNIQUE(base) CONCAT(base, __COUNTER__)
#define UNUSED(func) [[maybe_unused]] auto UNIQUE(_) = func

using namespace plugify;

#if V8LM_PLATFORM_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#include <winhttp.h>
#else
#include <curl/curl.h>
#endif

namespace v8lm {
#include "builtin/assert.hpp"
#include "builtin/console.hpp"
#include "builtin/crypto.hpp"
#include "builtin/datagram.hpp"
#include "builtin/dns.hpp"
#include "builtin/fetch.hpp"
#include "builtin/fs.hpp"
#include "builtin/http.hpp"
#include "builtin/https.hpp"
#include "builtin/net.hpp"
#include "builtin/os.hpp"
#include "builtin/path.hpp"
#include "builtin/process.hpp"
#include "builtin/querystring.hpp"
#include "builtin/repl.hpp"
#include "builtin/readline.hpp"
#include "builtin/timers.hpp"
#include "builtin/tls.hpp"
#include "builtin/url.hpp"

	/*
		child_process	To run a child process
		cluster	To split a single Node process into multiple processes
		dgram	Provides implementation of UDP datagram sockets
		stream	To handle streaming data
		string_decoder	To decode buffer objects into strings
		timers	To execute a function after a given number of milliseconds
		tty	Provides classes used by a text terminal
		v8	To access information about V8 (the JavaScript engine)
		vm	To compile JavaScript code in a virtual machine
		zlib	To compress or decompress files
 	*/

	namespace {
		[[maybe_unused]] std::unique_ptr<v8::Platform> platform = nullptr;

		void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
			size_t start_pos{};
			while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
				str.replace(start_pos, from.length(), to);
				start_pos += to.length();
			}
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

		// Return codes:
		// [1, 3]	Number bytes used
		// 0		Sequence starts with \0
		// -1		Encoding error
		// -2		Invalid multibyte sequence
		// -3		Surrogate pair
		std::pair<short, char16_t> ConvertUtf8ToUtf16(std::span<const char> sequence) {
			const auto c8toc16 = [](char ch) -> char16_t { return static_cast<char16_t>(static_cast<uint8_t>(ch)); };

			if (sequence.empty()) {
				return { -2, {} };
			}
			const char seqCh0 = sequence[0];
			if (seqCh0 == '\0') {
				return { 0, 0 };
			}
			if ((seqCh0 & 0b11111000) == 0b11110000) {
				return { -3, {} };
			}
			if ((seqCh0 & 0b11110000) == 0b11100000) {
				if (sequence.size() < 3) {
					return { -2, {} };
				}
				const char seqCh1 = sequence[1];
				const char seqCh2 = sequence[2];
				if ((seqCh1 & 0b11000000) != 0b10000000 || (seqCh2 & 0b11000000) != 0b10000000) {
					return { -2, {} };
				}
				const char16_t ch = (c8toc16(seqCh0 & 0b00001111) << 12) | (c8toc16(seqCh1 & 0b00111111) << 6) | c8toc16(seqCh2 & 0b00111111);
				if (0xD800 <= static_cast<uint16_t>(ch) && static_cast<uint16_t>(ch) < 0xE000) {
					return { -1, {} };
				}
				return { 3, ch };
			}
			if ((seqCh0 & 0b11100000) == 0b11000000) {
				if (sequence.size() < 2) {
					return { -2, {} };
				}
				const char seqCh1 = sequence[1];
				if ((seqCh1 & 0b11000000) != 0b10000000) {
					return { -2, {} };
				}
				return { 2, (c8toc16(seqCh0 & 0b00011111) << 6) | c8toc16(seqCh1 & 0b00111111) };
			}
			if ((seqCh0 & 0b10000000) == 0b00000000) {
				return { 1, c8toc16(seqCh0) };
			}
			return { -1, {} };
		}

		// Return codes:
		// [1, 3]	Number bytes returned
		// 0		For 0x0000 symbol
		// -1		Surrogate pair
		std::pair<short, std::array<char, 4>> ConvertUtf16ToUtf8(char16_t ch16) {
			const auto c16toc8 = [](char16_t ch) -> char { return static_cast<char>(static_cast<uint8_t>(ch)); };

			if (ch16 == 0) {
				return { 0, {} };
			}
			if (static_cast<uint16_t>(ch16) < 0x80) {
				return { 1, {c16toc8(ch16), '\0'} };
			}
			if (static_cast<uint16_t>(ch16) < 0x800) {
				return { 2, {c16toc8(((ch16 & 0b11111000000) >> 6) | 0b11000000), c16toc8((ch16 & 0b111111) | 0b10000000), '\0'} };
			}
			if (0xD800 <= static_cast<uint16_t>(ch16) && static_cast<uint16_t>(ch16) < 0xE000) {
				return { -1, {}};
			}
			return { 3, {c16toc8(((ch16 & 0b1111000000000000) >> 12) | 0b11100000), c16toc8(((ch16 & 0b111111000000) >> 6) | 0b10000000), c16toc8((ch16 & 0b111111) | 0b10000000), '\0'} };
		}

		//using MethodExportError = std::string;
		//using MethodExportData = JsMethodData;
		//using MethodExportResult = std::variant<MethodExportError, MethodExportData>;

		// Generic function to check if value is in range of type N
		template<typename T, typename U = T>
		bool IsInRange(T value) {
			if constexpr (std::is_same_v<U, T>) {
				return true;
			} else if constexpr (std::is_floating_point_v<T> && std::is_floating_point_v<U>) {
				// Handle floating-point range checks
				return value >= static_cast<T>(-std::numeric_limits<U>::infinity()) &&
					   value <= static_cast<T>(std::numeric_limits<U>::infinity());
			} else if constexpr (std::is_signed_v<T> == std::is_signed_v<U>) {
				// Both T and N are signed or unsigned
				return value >= static_cast<T>(std::numeric_limits<U>::min()) &&
					   value <= static_cast<T>(std::numeric_limits<U>::max());
			} else if constexpr (std::is_unsigned_v<T> && std::is_signed_v<U>) {
				// T is unsigned, N is signed
				if (value > static_cast<T>(std::numeric_limits<U>::max())) {
					return false;
				}
			} else if constexpr (std::is_signed_v<T> && std::is_unsigned_v<U>) {
				// T is signed, N is unsigned
				if (value < 0 || static_cast<std::make_unsigned_t<T>>(value) > std::numeric_limits<U>::max()) {
					return false;
				}
			}
			return true;
		}
		
		template<typename E>
		constexpr std::underlying_type_t<E> CastEnumToIntegral(E e) noexcept {
			return static_cast<std::underlying_type_t<E>>(e);
		}
		
		constexpr JsType operator|(JsType lhs, JsType rhs) noexcept {
			return static_cast<JsType>(CastEnumToIntegral(lhs) | CastEnumToIntegral(rhs));
		}

		constexpr JsType& operator|=(JsType& lhs, JsType rhs) noexcept {
			lhs = lhs | rhs;
			return lhs;
		}

		using V8MethodCallback = void (V8LanguageModule::*)(const v8::FunctionCallbackInfo<v8::Value>& info);
		template <V8MethodCallback callback>
		struct MethodBindingHelper {
			static void Bind(v8::Isolate* isolate, v8::Local<v8::Context> context, v8::Local<v8::Object> obj, v8::Local<v8::Value> key, v8::Local<v8::External> value) {
				obj->Set(context, key,
						 v8::FunctionTemplate::New(
								 isolate,
								 [](const v8::FunctionCallbackInfo<v8::Value>& info) {
									 auto* self = static_cast<V8LanguageModule*>((v8::Local<v8::External>::Cast(info.Data()))->Value());
									 (self->*callback)(info);
								 },
								 value)->GetFunction(context).ToLocalChecked())
						.Check();
			}
		};

		v8::Local<v8::Message> MakeErrorMessage(v8::Isolate* isolate,
												v8::Local<v8::Value> exception) {
			v8::Local<v8::Message> message = v8::Exception::CreateMessage(isolate, exception);
			if (!exception->IsNativeError() &&
				(message.IsEmpty() || message->GetStackTrace().IsEmpty())) {
				// Create a stack trace manually if it's missing.
				exception = v8::Exception::Error(v8::String::NewFromUtf8Literal(isolate, "Uncaught Exception."));
				message = v8::Exception::CreateMessage(isolate, exception);
			}
			return message;
		}
	}

	template<class T>
	constexpr bool always_false_v = std::is_same_v<std::decay_t<T>, std::add_cv_t<std::decay_t<T>>>;

	template<class T>
	constexpr bool is_vector_type_v =
			std::is_same_v<T, plg::vector<bool>> ||
			std::is_same_v<T, plg::vector<char>> ||
			std::is_same_v<T, plg::vector<char16_t>> ||
			std::is_same_v<T, plg::vector<int8_t>> ||
			std::is_same_v<T, plg::vector<int16_t>> ||
			std::is_same_v<T, plg::vector<int32_t>> ||
			std::is_same_v<T, plg::vector<int64_t>> ||
			std::is_same_v<T, plg::vector<uint8_t>> ||
			std::is_same_v<T, plg::vector<uint16_t>> ||
			std::is_same_v<T, plg::vector<uint32_t>> ||
			std::is_same_v<T, plg::vector<uint64_t>> ||
			std::is_same_v<T, plg::vector<void*>> ||
			std::is_same_v<T, plg::vector<float>> ||
			std::is_same_v<T, plg::vector<double>> ||
			std::is_same_v<T, plg::vector<plg::string>> ||
			std::is_same_v<T, plg::vector<plg::variant<plg::none>>> ||
			std::is_same_v<T, plg::vector<plg::vec2>> ||
			std::is_same_v<T, plg::vector<plg::vec3>> ||
			std::is_same_v<T, plg::vector<plg::vec4>> ||
			std::is_same_v<T, plg::vector<plg::mat4x4>>;

	template<class T>
	constexpr bool is_none_type_v =
			std::is_same_v<T, plg::invalid> ||
			std::is_same_v<T, plg::none> ||
			std::is_same_v<T, plg::variant<plg::none>> ||
			std::is_same_v<T, plg::function> ||
			std::is_same_v<T, plg::any>;

	namespace detail {
		void InternalCall(MethodHandle method, MemAddr data, const JitCallback::Parameters* params, size_t count, const JitCallback::Return* ret) {
			g_v8lm.InternalCall(method, data, params, count, ret);
		}

		void ExternalCall(MethodHandle method, MemAddr data, const JitCallback::Parameters* params, size_t count, const JitCallback::Return* ret) {
			g_v8lm.ExternalCall(method, data, params, count, ret);
		}
	}

	JsType V8LanguageModule::GetType(v8::Local<v8::Value> value) {
		if (value->IsString())
			return JsType::String;
		if (value->IsBoolean())
			return JsType::Boolean;
		if (value->IsInt32())
			return JsType::Int32;
		if (value->IsUint32())
			return JsType::Uint32;
		if (value->IsNumber())
			return JsType::Number;
		if (value->IsBigInt())
			return JsType::BigInt;
		if (value->IsExternal())
			return JsType::External;
		if (value->IsObject()) {
			v8::Local<v8::Context> context = _isolate->GetCurrentContext();
			if (value->InstanceOf(context, _vector2ClassObject.Get(_isolate)).FromMaybe(false))
				return JsType::Vector2;
			if (value->InstanceOf(context, _vector3ClassObject.Get(_isolate)).FromMaybe(false))
				return JsType::Vector3;
			if (value->InstanceOf(context, _vector4ClassObject.Get(_isolate)).FromMaybe(false))
				return JsType::Vector4;
			if (value->InstanceOf(context, _matrix4x4ClassObject.Get(_isolate)).FromMaybe(false))
				return JsType::Matrix4x4;
		}
		return JsType::Invalid;
	}

#pragma region ValueFromObject

	template<typename T>
	std::optional<T> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> /*value*/) {
		static_assert(always_false_v<T>, "ValueFromObject specialization required");
	}

	template<>
	std::optional<bool> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		if (value->IsBoolean()) {
			return value.As<v8::Boolean>()->Value();
		}
		ThrowTypeError("Expected boolean", value);
		return std::nullopt;
	}

	template<>
	std::optional<char> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		if (value->IsString()) {
			const int length = value.As<v8::String>()->Length();
			if (length == 0) {
				return 0;
			}
			else if (length == 1) {
				v8::String::Utf8Value utf8(_isolate, value);
				char ch = (*utf8)[0];
				if ((ch & 0x80) == 0) {
					return ch;
				}
				// Can't pass multibyte character
				ThrowException("Multibyte character");
			}
			else {
				ThrowException("Length bigger than 1");
			}
			return std::nullopt;
		}
		ThrowTypeError("Expected string", value);
		return std::nullopt;
	}

	template<>
	std::optional<char16_t> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		if (value->IsString()) {
			const int length = value.As<v8::String>()->Length();
			if (length == 0) {
				return { 0 };
			}
			else if (length == 1) {
				v8::String::Utf8Value utf8(_isolate, value);
				auto [rc, ch] = ConvertUtf8ToUtf16({ *utf8, static_cast<size_t>(utf8.length()) });
				switch (rc) {
					case 0:
					case 1:
					case 2:
					case 3:
						return { ch };
					case -3:
						ThrowException("Surrogate pair");
						break;
					case -2:
						ThrowException("Invalid multibyte character");
						break;
					case -1:
						ThrowException("Encoding error");
						break;
				}
			}
			else {
				ThrowException("Length bigger than 1");
			}
			return std::nullopt;
		}
		ThrowTypeError("Expected string", value);
		return std::nullopt;
	}

	template<class T> requires(std::is_signed_v<T> || std::is_unsigned_v<T>)
	std::optional<T> V8LanguageModule::ValueFromIntegerObject(v8::Local<v8::Value> value) {
		if (value->IsNumber()) {
			const auto castResult = static_cast<T>(value.As<v8::Number>()->IntegerValue(_isolate->GetCurrentContext()).ToChecked());
			if (IsInRange(castResult)) {
				return castResult;
			}
			ThrowRangeError("Overflow error");
			return std::nullopt;
		} else if (value->IsBigInt()) {
			const auto castResult = static_cast<T>(value.As<v8::BigInt>()->Int64Value());
			if (IsInRange(castResult)) {
				return castResult;
			}
			ThrowRangeError("Overflow error");
			return std::nullopt;
		}
		ThrowTypeError("Expected number or bigint", value);
		return std::nullopt;
	}

	template<class T> requires(std::is_floating_point_v<T>)
	std::optional<T> V8LanguageModule::ValueFromNumberObject(v8::Local<v8::Value> value) {
		if (value->IsNumber()) {
			const auto castResult = static_cast<T>(value.As<v8::Number>()->NumberValue(_isolate->GetCurrentContext()).ToChecked());
			if (IsInRange(castResult)) {
				return castResult;
			}
			ThrowRangeError("Overflow error");
			return std::nullopt;
		}
		ThrowTypeError("Expected number", value);
		return std::nullopt;
	}

	template<>
	std::optional<int8_t> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		return ValueFromIntegerObject<int8_t>(value);
	}

	template<>
	std::optional<int16_t> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		return ValueFromIntegerObject<int16_t>(value);
	}

	template<>
	std::optional<int32_t> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		return ValueFromIntegerObject<int32_t>(value);
	}

	template<>
	std::optional<int64_t> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		return ValueFromIntegerObject<int64_t>(value);
	}

	template<>
	std::optional<uint8_t> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		return ValueFromIntegerObject<uint8_t>(value);
	}

	template<>
	std::optional<uint16_t> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		return ValueFromIntegerObject<uint16_t>(value);
	}

	template<>
	std::optional<uint32_t> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		return ValueFromIntegerObject<uint32_t>(value);
	}

	template<>
	std::optional<uint64_t> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		return ValueFromIntegerObject<uint64_t>(value);
	}

	template<>
	std::optional<void*> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		if (value->IsNumber()) {
			return reinterpret_cast<void*>(value.As<v8::Number>()->IntegerValue(_isolate->GetCurrentContext()).ToChecked());
		} else if (value->IsBigInt()) {
			return reinterpret_cast<void*>(value.As<v8::BigInt>()->Int64Value());
		} else if (value->IsExternal()) {
			return value.As<v8::External>()->Value();
		}
		ThrowTypeError("Expected bigint or external", value);
		return std::nullopt;
	}

	template<>
	std::optional<float> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		return ValueFromNumberObject<float>(value);
	}

	template<>
	std::optional<double> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		return ValueFromNumberObject<double>(value);
	}

	template<>
	std::optional<plg::string> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		if (value->IsString()) {
			v8::String::Utf8Value utf8(_isolate, value.As<v8::String>());
			return plg::string(*utf8, static_cast<size_t>(utf8.length()));
		}
		ThrowTypeError("Expected string", value);
		return std::nullopt;
	}

	template<>
	std::optional<plg::any> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		if (value->IsBoolean()) {
			return value.As<v8::Boolean>()->Value();
		}
		if (value->IsInt32()) {
			return value.As<v8::Int32>()->Value();
		}
		if (value->IsUint32()) {
			return value.As<v8::Uint32>()->Value();
		}
		if (value->IsNumber()) {
			return value.As<v8::Number>()->NumberValue(_isolate->GetCurrentContext()).ToChecked();
		}
		if (value->IsBigInt()) {
			return value.As<v8::BigInt>()->Int64Value();
		}
		if (value->IsString()) {
			v8::String::Utf8Value utf8(_isolate, value.As<v8::String>());
			return plg::string(*utf8, static_cast<size_t>(utf8.length()));
		}
		if (value->IsExternal()) {
			return value.As<v8::External>()->Value();
		}
		if (value->IsArray()) {
			v8::Local<v8::Array> arrayObject = value.As<v8::Array>();
			const uint32_t length = arrayObject->Length();
			if (length == 0) {
				return plg::vector<int64_t>();
			}
			std::vector<JsType> params;
			params.reserve(length);
			v8::Local<v8::Context> context = _isolate->GetCurrentContext();
			for (uint32_t i = 0; i < length; ++i) {
				v8::Local<v8::Value> valueObject;
				if (arrayObject->Get(context, i).ToLocal(&valueObject)) {
					params.emplace_back(GetType(valueObject));
					continue;
				}
				return std::nullopt;
			}
			const JsType& type = params[0];
			if (params.size() == 1 || std::all_of(params.begin() + 1, params.end(), [&type](const JsType& val) { return val == type; })) {
				switch (type) {
					case JsType::String: {
						if (auto array = ArrayFromObject<plg::string>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					case JsType::BigInt: {
						if (auto array = ArrayFromObject<int64_t>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					case JsType::Int32: {
						if (auto array = ArrayFromObject<int32_t>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					case JsType::Uint32: {
						if (auto array = ArrayFromObject<uint32_t>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					case JsType::Number: {
						if (auto array = ArrayFromObject<double>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					case JsType::Boolean: {
						if (auto array = ArrayFromObject<bool>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					case JsType::External: {
						if (auto array = ArrayFromObject<void*>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					case JsType::Vector2: {
						if (auto array = ArrayFromObject<plg::vec2>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					case JsType::Vector3: {
						if (auto array = ArrayFromObject<plg::vec3>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					case JsType::Vector4: {
						if (auto array = ArrayFromObject<plg::vec4>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					case JsType::Matrix4x4: {
						if (auto array = ArrayFromObject<plg::mat4x4>(value)) {
							return std::move(*array);
						}
						return std::nullopt;
					}
					default:
						break;
				}
			}
			std::string error("Array should contains same supported, but contains: [");
			bool first = true;
			for (uint32_t i = 0; i < length; ++i) {
				v8::Local<v8::Value> valueObject;
				if (arrayObject->Get(context, i).ToLocal(&valueObject)) {
					std::string valueName = ToStringOr(valueObject->TypeOf(_isolate), "<invalid>");
					if (first) {
						std::format_to(std::back_inserter(error), "'{}", valueName);
						first = false;
					} else {
						std::format_to(std::back_inserter(error), "', '{}", valueName);
					}
				}
			}
			error += "']";
			ThrowException(error);
			return std::nullopt;
		}
		if (value->IsTypedArray()) {
			v8::Local<v8::TypedArray> arr = value.As<v8::TypedArray>();
			if (value->IsInt8Array()) {
				plg::vector<int8_t> array(arr->Length());
				arr->CopyContents(array.data(), arr->ByteLength());
				return array;
			} else if (value->IsInt16Array()) {
				plg::vector<int16_t> array(arr->Length());
				arr->CopyContents(array.data(), arr->ByteLength());
				return array;
			} else if (value->IsInt32Array()) {
				plg::vector<int32_t> array(arr->Length());
				arr->CopyContents(array.data(), arr->ByteLength());
				return array;
			} else if (value->IsUint8Array() || value->IsUint8ClampedArray()) {
				plg::vector<uint8_t> array(arr->Length());
				arr->CopyContents(array.data(), arr->ByteLength());
				return array;
			} else if (value->IsUint16Array()) {
				plg::vector<uint16_t> array(arr->Length());
				arr->CopyContents(array.data(), arr->ByteLength());
				return array;
			} else if (value->IsUint32Array()) {
				plg::vector<uint32_t> array(arr->Length());
				arr->CopyContents(array.data(), arr->ByteLength());
				return array;
			} else if (value->IsFloat32Array()) {
				plg::vector<float> array(arr->Length());
				arr->CopyContents(array.data(), arr->ByteLength());
				return array;
			} else if (value->IsFloat64Array()) {
				plg::vector<double> array(arr->Length());
				arr->CopyContents(array.data(), arr->ByteLength());
				return array;
			} else if (value->IsBigInt64Array()) {
				plg::vector<int64_t> array(arr->Length());
				arr->CopyContents(array.data(), arr->ByteLength());
				return array;
			} else if (value->IsBigUint64Array()) {
				plg::vector<uint64_t> array(arr->Length());
				arr->CopyContents(array.data(), arr->ByteLength());
				return array;
			}
		}
		if (value->IsObject()) {
			v8::Local<v8::Object> object = value.As<v8::Object>();
			v8::Local<v8::Context> context = _isolate->GetCurrentContext();

			if (value->InstanceOf(context, _vector2ClassObject.Get(_isolate)).FromMaybe(false)) {
				auto x = GetObjectAttrAsValue<float>(object, "x");
				if (!x) {
					return std::nullopt;
				}
				auto y = GetObjectAttrAsValue<float>(object, "y");
				if (!y) {
					return std::nullopt;
				}
				return plg::vec2{ *x, *y };
			}
			if (value->InstanceOf(context, _vector3ClassObject.Get(_isolate)).FromMaybe(false)) {
				auto x = GetObjectAttrAsValue<float>(object, "x");
				if (!x) {
					return std::nullopt;
				}
				auto y = GetObjectAttrAsValue<float>(object, "y");
				if (!y) {
					return std::nullopt;
				}
				auto z = GetObjectAttrAsValue<float>(object, "z");
				if (!z) {
					return std::nullopt;
				}
				return plg::vec3{ *x, *y, *z };
			}
			if (value->InstanceOf(context, _vector4ClassObject.Get(_isolate)).FromMaybe(false)) {
				auto x = GetObjectAttrAsValue<float>(object, "x");
				if (!x) {
					return std::nullopt;
				}
				auto y = GetObjectAttrAsValue<float>(object, "y");
				if (!y) {
					return std::nullopt;
				}
				auto z = GetObjectAttrAsValue<float>(object, "z");
				if (!z) {
					return std::nullopt;
				}
				auto w = GetObjectAttrAsValue<float>(object, "w");
				if (!w) {
					return std::nullopt;
				}
				return plg::vec4{ *x, *y, *z, *w };
			}
		}
		ThrowTypeError("An any argument not supports python type: {} for marshalling.", value);
		return std::nullopt;
	}

	template<>
	std::optional<plg::vec2> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		if (value->IsObject() && value->InstanceOf(_isolate->GetCurrentContext(), _vector2ClassObject.Get(_isolate)).FromMaybe(false)) {
			v8::Local<v8::Object> object = value.As<v8::Object>();

			auto x = GetObjectAttrAsValue<float>(object, "x");
			if (!x) {
				return std::nullopt;
			}
			auto y = GetObjectAttrAsValue<float>(object, "y");
			if (!y) {
				return std::nullopt;
			}
			return plg::vec2{ *x, *y };
		}
		ThrowTypeError("Expected plugify.Vector2", value);
		return std::nullopt;
	}

	template<>
	std::optional<plg::vec3> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		if (value->IsObject() && value->InstanceOf(_isolate->GetCurrentContext(), _vector3ClassObject.Get(_isolate)).FromMaybe(false)) {
			v8::Local<v8::Object> object = value.As<v8::Object>();

			auto x = GetObjectAttrAsValue<float>(object, "x");
			if (!x) {
				return std::nullopt;
			}
			auto y = GetObjectAttrAsValue<float>(object, "y");
			if (!y) {
				return std::nullopt;
			}
			auto z = GetObjectAttrAsValue<float>(object, "z");
			if (!z) {
				return std::nullopt;
			}
			return plg::vec3{ *x, *y, *z };
		}
		ThrowTypeError("Expected plugify.Vector3", value);
		return std::nullopt;
	}

	template<>
	std::optional<plg::vec4> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		if (value->IsObject() && value->InstanceOf(_isolate->GetCurrentContext(), _vector4ClassObject.Get(_isolate)).FromMaybe(false)) {
			v8::Local<v8::Object> object = value.As<v8::Object>();

			auto x = GetObjectAttrAsValue<float>(object, "x");
			if (!x) {
				return std::nullopt;
			}
			auto y = GetObjectAttrAsValue<float>(object, "y");
			if (!y) {
				return std::nullopt;
			}
			auto z = GetObjectAttrAsValue<float>(object, "z");
			if (!z) {
				return std::nullopt;
			}
			auto w = GetObjectAttrAsValue<float>(object, "w");
			if (!w) {
				return std::nullopt;
			}
			return plg::vec4{ *x, *y, *z, *w };
		}
		ThrowTypeError("Expected plugify.Vector4", value);
		return std::nullopt;
	}

	template<>
	std::optional<plg::mat4x4> V8LanguageModule::ValueFromObject(v8::Local<v8::Value> value) {
		v8::Local<v8::Context> context = _isolate->GetCurrentContext();
		if (value->IsObject() && value->InstanceOf(context, _matrix4x4ClassObject.Get(_isolate)).FromMaybe(false)) {
			v8::Local<v8::Object> object = value.As<v8::Object>();
			v8::Local<v8::Value> elementsValue;
			if (!object->Get(context, v8::String::NewFromUtf8Literal(_isolate, "m")).ToLocal(&elementsValue)) {
				ThrowException("Object does not have an 'elements' property.");
				return std::nullopt;
			}
			if (!elementsValue->IsArray()) {
				ThrowException("Object's 'elements' must be a 4x4 array.");
				return std::nullopt;
			}
			v8::Local<v8::Array> elementsArray = elementsValue.As<v8::Array>();
			if (elementsArray->Length() != 4) {
				ThrowException("Object's 'elements' must have 4 rows.");
				return std::nullopt;
			}
			plg::mat4x4 matrix{};
			for (int i = 0; i < 4; ++i) {
				v8::Local<v8::Value> row;
				if (!elementsArray->Get(context, i).ToLocal(&row)) {
					ThrowException(std::format("Failed to get row '{}' from 'elements'.", i));
					return std::nullopt;
				}

				if (!row->IsArray()) {
					ThrowException(std::format("Row '{}' is not an array.", i));
					return std::nullopt;
				}

				v8::Local<v8::Array> rowArray = row.As<v8::Array>();
				if (rowArray->Length() != 4) {
					ThrowException(std::format("Row '{}' must have 4 elements.", i));
					return std::nullopt;
				}

				for (int j = 0; j < 4; ++j) {
					v8::Local<v8::Value> cellValue;
					if (!rowArray->Get(context, j).ToLocal(&cellValue)) {
						ThrowException(std::format("Failed to get element [{}][{}].", i, j));
						return std::nullopt;
					}

					auto m = ValueFromObject<float>(cellValue);
					if (!m) {
						//ThrowException(std::format("Element [{}][{}] is not a number.", i, j));
						return std::nullopt;
					}

					matrix.data[i * 4 + j] = *m;
				}
			}
			return matrix;
		}
		ThrowTypeError("Expected plugify.Matrix4x4", value);
		return std::nullopt;
	}

	template<typename T>
	std::optional<plg::vector<T>> V8LanguageModule::ArrayFromObject(v8::Local<v8::Value> arrayValue) {
		if (arrayValue->IsArray()) {
			v8::Local<v8::Array> arrayObject = arrayValue.As<v8::Array>();
			const uint32_t length = arrayObject->Length();
			plg::vector<T> array(static_cast<size_t>(length));
			v8::Local<v8::Context> context = _isolate->GetCurrentContext();
			for (uint32_t i = 0; i < length; ++i) {
				v8::Local<v8::Value> valueObject;
				if (arrayObject->Get(context, i).ToLocal(&valueObject)) {
					if (auto value = ValueFromObject<T>(valueObject)) {
						array[i] = std::move(*value);
						continue;
					}
				}
				return std::nullopt;
			}
			return array;
		}

		if (arrayValue->IsTypedArray()) {
			v8::Local<v8::TypedArray> arr = arrayValue.As<v8::TypedArray>();

			if constexpr (std::is_same_v<T, int8_t>) {
				if (arrayValue->IsInt8Array()) {
					plg::vector<int8_t> array(arr->Length());
					arr->CopyContents(array.data(), arr->ByteLength());
					return array;
				}
			}

			if constexpr (std::is_same_v<T, int16_t>) {
				if (arrayValue->IsInt16Array()) {
					plg::vector<int16_t> array(arr->Length());
					arr->CopyContents(array.data(), arr->ByteLength());
					return array;
				}
			}

			if constexpr (std::is_same_v<T, int32_t>) {
				if (arrayValue->IsInt32Array()) {
					plg::vector<int32_t> array(arr->Length());
					arr->CopyContents(array.data(), arr->ByteLength());
					return array;
				}
			}

			if constexpr (std::is_same_v<T, uint8_t>) {
				if (arrayValue->IsUint8Array() || arrayValue->IsUint8ClampedArray()) {
					plg::vector<uint8_t> array(arr->Length());
					arr->CopyContents(array.data(), arr->ByteLength());
					return array;
				}
			}

			if constexpr (std::is_same_v<T, uint16_t>) {
				if (arrayValue->IsUint16Array()) {
					plg::vector<uint16_t> array(arr->Length());
					arr->CopyContents(array.data(), arr->ByteLength());
					return array;
				}
			}

			if constexpr (std::is_same_v<T, uint32_t>) {
				if (arrayValue->IsUint32Array()) {
					plg::vector<uint32_t> array(arr->Length());
					arr->CopyContents(array.data(), arr->ByteLength());
					return array;
				}
			}

			if constexpr (std::is_same_v<T, float>) {
				if (arrayValue->IsFloat32Array()) {
					plg::vector<float> array(arr->Length());
					arr->CopyContents(array.data(), arr->ByteLength());
					return array;
				}
			}

			if constexpr (std::is_same_v<T, double>) {
				if (arrayValue->IsFloat64Array()) {
					plg::vector<double> array(arr->Length());
					arr->CopyContents(array.data(), arr->ByteLength());
					return array;
				}
			}

			if constexpr (std::is_same_v<T, int64_t>) {
				if (arrayValue->IsBigInt64Array()) {
					plg::vector<int64_t> array(arr->Length());
					arr->CopyContents(array.data(), arr->ByteLength());
					return array;
				}
			}

			if constexpr (std::is_same_v<T, uint64_t>) {
				if (arrayValue->IsBigUint64Array()) {
					plg::vector<uint64_t> array(arr->Length());
					arr->CopyContents(array.data(), arr->ByteLength());
					return array;
				}
			}
		}

		ThrowTypeError("Expected array or typearray", arrayValue);
		return std::nullopt;
	}

#pragma endregion ValueFromObject

	std::optional<void*> V8LanguageModule::GetOrCreateFunctionValue(MethodHandle method, v8::Local<v8::Value> value) {
		if (value->IsNull()) {
			return nullptr;
		}

		if (!value->IsFunction()) {
			ThrowTypeError("Expected function", value);
			return std::nullopt;
		}

		v8::Local<v8::Function> func = value.As<v8::Function>();

		if (void* const funcAddr = FindInternal(func)) {
			return funcAddr;
		}

		JsFunction funcObj = std::make_shared<v8::Global<v8::Function>>(_isolate, func);

		JitCallback callback(_jitRuntime);
		const MemAddr methodAddr = callback.GetJitFunc(method, &detail::InternalCall, funcObj.get());
		if (!methodAddr) {
			ThrowException(std::format("Lang module JIT failed to generate C++ wrapper from callback object '{}'", callback.GetError()));
			return std::nullopt;
		}

		AddToFunctionsMap(methodAddr, funcObj);
		_internalFunctions.emplace_back(std::move(callback), std::move(funcObj));

		return methodAddr;
	}

#pragma region CreateJsObject

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const bool& value) {
		return v8::Boolean::New(_isolate, value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const char& value) {
		if (value == char{ 0 }) {
			return v8::String::Empty(_isolate);
		}
		return v8::String::NewFromUtf8(_isolate, &value, v8::NewStringType::kNormal, 1).ToLocalChecked();
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const char16_t& value) {
		if (value == char16_t{ 0 }) {
			return v8::String::Empty(_isolate);
		}
		const auto [rc, out] = ConvertUtf16ToUtf8(value);
		if (rc == -1) {
			ThrowException("Surrogate pair");
			return {};
		}
		return v8::String::NewFromUtf8(_isolate, out.data(), v8::NewStringType::kNormal, rc).ToLocalChecked();
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const int8_t& value) {
		return v8::Integer::New(_isolate, value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const int16_t& value) {
		return v8::Integer::New(_isolate, value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const int32_t& value) {
		return v8::Integer::New(_isolate, value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const int64_t& value) {
		return v8::BigInt::New(_isolate, value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const uint8_t& value) {
		return v8::Integer::NewFromUnsigned(_isolate, value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const uint16_t& value) {
		return v8::Integer::NewFromUnsigned(_isolate, value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const uint32_t& value) {
		return v8::Integer::NewFromUnsigned(_isolate, value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const uint64_t& value) {
		return v8::BigInt::NewFromUnsigned(_isolate, value);
	}

	using void_t = void*;

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const void_t& value) {
		return v8::BigInt::New(_isolate, reinterpret_cast<int64_t>(const_cast<void_t&>(value)));
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const float& value) {
		return v8::Number::New(_isolate, value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const double& value) {
		return v8::Number::New(_isolate, value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const plg::string& value) {
		return MakeString(value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const std::string_view& value) {
		return MakeString(value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const std::wstring_view& value) {
		return MakeString(value);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const plg::vec2& value) {
		std::array args = { CreateJsObject(value.x), CreateJsObject(value.y) };
		return CreateJsObject(_vector2ClassObject.Get(_isolate), args);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const plg::vec3& value) {
		std::array args = { CreateJsObject(value.x), CreateJsObject(value.y), CreateJsObject(value.z) };
		return CreateJsObject(_vector3ClassObject.Get(_isolate), args);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const plg::vec4& value) {
		std::array args = { CreateJsObject(value.x), CreateJsObject(value.y), CreateJsObject(value.z), CreateJsObject(value.w) };
		return CreateJsObject(_vector4ClassObject.Get(_isolate), args);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const plg::mat4x4& value) {
		std::array args = { CreateJsObjectList(std::span(std::begin(value.data), std::end(value.data))) };
		return CreateJsObject(_matrix4x4ClassObject.Get(_isolate), args);
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const plg::invalid& value) {
		return {};
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const plg::none& value) {
		return {};
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const plg::variant<plg::none>& value) {
		return {};
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const plg::function& value) {
		return {};
	}

	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(v8::Local<v8::Function> ctor, std::span<v8::Local<v8::Value>> args) {
		if (!ctor->IsFunction()) {
			ThrowTypeError("Expected function", ctor);
			return {};
		}

		v8::Local<v8::Value> instance;
		if (!ctor->NewInstance(_isolate->GetCurrentContext(), static_cast<int>(args.size()), args.data()).ToLocal(&instance)) {
			ThrowException(std::format("Failed to create instance of {}", ToStringOr(ctor->TypeOf(_isolate), "<invalid>")));
			return {};
		}

		return instance;
	}

	template<>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObject(const plg::any& value) {
		v8::Local<v8::Value> output;
		plg::visit([this, &output](auto&& val) {
			using T = std::decay_t<decltype(val)>;
			if constexpr (is_vector_type_v<T>) {
				output = CreateJsObjectList(val);
			} else if constexpr (is_none_type_v<T>) {
				output = v8::Undefined(_isolate);
			} else {
				output = CreateJsObject(val);
			}
		}, value);
		return output;
	}

	template<typename T>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObjectList(std::span<const T> arrayArg) {
		const size_t size = arrayArg.size();
		v8::Local<v8::Array> arrayObject = v8::Array::New(_isolate, static_cast<int>(size));
		v8::Local<v8::Context> context = _isolate->GetCurrentContext();
		for (size_t i = 0; i < size; ++i) {
			v8::Local<v8::Value> valueObject = CreateJsObject(arrayArg[i]);
			if (valueObject.IsEmpty()) {
				return {};
			}
			arrayObject->Set(context, static_cast<int>(i), valueObject).FromJust();
		}
		return arrayObject;
	}

	template<typename T>
	v8::Local<v8::Value> V8LanguageModule::CreateJsObjectList(const plg::vector<T>& arrayArg) {
		const size_t size = arrayArg.size();
		v8::Local<v8::Array> arrayObject = v8::Array::New(_isolate, static_cast<int>(size));
		v8::Local<v8::Context> context = _isolate->GetCurrentContext();
		for (size_t i = 0; i < size; ++i) {
			v8::Local<v8::Value> valueObject = CreateJsObject(arrayArg[i]);
			if (valueObject.IsEmpty()) {
				return {};
			}
			arrayObject->Set(context, static_cast<int>(i), valueObject).FromJust();
		}
		return arrayObject;
	}

#pragma endregion CreateJsObject

	v8::Local<v8::Value> V8LanguageModule::GetOrCreateFunctionObject(MethodHandle method, void* funcAddr) {
		if (JsFunction func = FindExternal(funcAddr)) {
			return func->Get(_isolate);
		}
		JitCall call(_jitRuntime);

		const MemAddr callAddr = call.GetJitFunc(method, funcAddr);
		if (!callAddr) {
			ThrowException(std::format("Lang module JIT failed to generate c++ call wrapper '{}'", call.GetError()));
			return {};
		}

		JitCallback callback(_jitRuntime);

		asmjit::FuncSignature sig(asmjit::CallConvId::kCDecl);
		sig.addArg(asmjit::TypeId::kUIntPtr);
		sig.setRet(asmjit::TypeId::kVoid);

		const MemAddr methodAddr = callback.GetJitFunc(sig, method, &detail::ExternalCall, callAddr, false);
		if (!methodAddr) {
			ThrowException(std::format("Lang module JIT failed to generate c++ v8::FunctionCallback wrapper '{}'", callback.GetError()));
			return {};
		}

		v8::Local<v8::Function> object;
		if (!v8::Function::New(_isolate->GetCurrentContext(), methodAddr.RCast<v8::FunctionCallback>()).ToLocal(&object)) {
			ThrowException("Fail to create function object from function pointer");
			return {};
		}

		JsFunction funcObj = std::make_shared<v8::Global<v8::Function>>(_isolate, object);

		AddToFunctionsMap(funcAddr, funcObj);
		_externalFunctions.emplace_back(std::move(callback), std::move(call), std::move(funcObj));

		return object;
	}

	template<typename T>
	void* V8LanguageModule::CreateValue(v8::Local<v8::Value> item) {
		auto value = ValueFromObject<T>(item);
		if (value) {
			return new T(std::move(*value));
		}
		return nullptr;
	}

	template<typename T>
	void* V8LanguageModule::CreateArray(v8::Local<v8::Value> item) {
		auto array = ArrayFromObject<T>(item);
		if (array) {
			return new plg::vector<T>(std::move(*array));
		}
		return nullptr;
	}

	template<typename T>
	std::optional<T> V8LanguageModule::GetObjectAttrAsValue(v8::Local<v8::Object> object, std::string_view attrName) {
		v8::Local<v8::Value> attrObject;
		if (!object->Get(_isolate->GetCurrentContext(), MakeString(attrName)).ToLocal(&attrObject)) {
			ThrowException(std::format("'{}' property not found.", attrName));
			return std::nullopt;
		}
		return ValueFromObject<T>(attrObject);
	}

	bool V8LanguageModule::IsSubclassOf(v8::Local<v8::Context> context, v8::Local<v8::Function> childClass, v8::Local<v8::Function> parentClass) {
		v8::Local<v8::Value> childPrototype;
		if (!childClass->Get(context, v8::String::NewFromUtf8Literal(_isolate, "prototype")).ToLocal(&childPrototype) ||
			!childPrototype->IsObject()) {
			return false;
		}

		v8::Local<v8::Value> currentProto = childClass->Get(context, v8::String::NewFromUtf8Literal(_isolate, "__proto__")).ToLocalChecked();

		while (currentProto->IsObject()) {
			v8::Local<v8::Object> currentObject = currentProto.As<v8::Object>();
			if (currentObject == parentClass) {
				return true;
			}

			if (!currentObject->Get(context, v8::String::NewFromUtf8Literal(_isolate, "__proto__")).ToLocal(&currentProto)) {
				break;
			}
		}

		return false;
	}

#pragma region InternalCall

	void V8LanguageModule::SetFallbackReturn(ValueType retType, const JitCallback::Return* ret) {
		switch (retType) {
			case ValueType::Void:
				break;
			case ValueType::Bool:
			case ValueType::Char8:
			case ValueType::Char16:
			case ValueType::Int8:
			case ValueType::Int16:
			case ValueType::Int32:
			case ValueType::Int64:
			case ValueType::UInt8:
			case ValueType::UInt16:
			case ValueType::UInt32:
			case ValueType::UInt64:
			case ValueType::Pointer:
			case ValueType::Float:
			case ValueType::Double:
				// HACK: Fill all 8 byte with 0
				ret->SetReturn<uintptr_t>({});
				break;
			case ValueType::Function:
				ret->SetReturn<void*>(nullptr);
				break;
			case ValueType::String:
				ret->ConstructAt<plg::string>();
				break;
			case ValueType::Any:
				ret->ConstructAt<plg::any>();
				break;
			case ValueType::ArrayBool:
				ret->ConstructAt<plg::vector<bool>>();
				break;
			case ValueType::ArrayChar8:
				ret->ConstructAt<plg::vector<char>>();
				break;
			case ValueType::ArrayChar16:
				ret->ConstructAt<plg::vector<char16_t>>();
				break;
			case ValueType::ArrayInt8:
				ret->ConstructAt<plg::vector<int8_t>>();
				break;
			case ValueType::ArrayInt16:
				ret->ConstructAt<plg::vector<int16_t>>();
				break;
			case ValueType::ArrayInt32:
				ret->ConstructAt<plg::vector<int32_t>>();
				break;
			case ValueType::ArrayInt64:
				ret->ConstructAt<plg::vector<int64_t>>();
				break;
			case ValueType::ArrayUInt8:
				ret->ConstructAt<plg::vector<uint8_t>>();
				break;
			case ValueType::ArrayUInt16:
				ret->ConstructAt<plg::vector<uint16_t>>();
				break;
			case ValueType::ArrayUInt32:
				ret->ConstructAt<plg::vector<uint32_t>>();
				break;
			case ValueType::ArrayUInt64:
				ret->ConstructAt<plg::vector<uint64_t>>();
				break;
			case ValueType::ArrayPointer:
				ret->ConstructAt<plg::vector<void*>>();
				break;
			case ValueType::ArrayFloat:
				ret->ConstructAt<plg::vector<float>>();
				break;
			case ValueType::ArrayDouble:
				ret->ConstructAt<plg::vector<double>>();
				break;
			case ValueType::ArrayString:
				ret->ConstructAt<plg::vector<plg::string>>();
				break;
			case ValueType::ArrayAny:
				ret->ConstructAt<plg::vector<plg::any>>();
				break;
			case ValueType::ArrayVector2:
				ret->ConstructAt<plg::vector<plg::vec2>>();
				break;
			case ValueType::ArrayVector3:
				ret->ConstructAt<plg::vector<plg::vec3>>();
				break;
			case ValueType::ArrayVector4:
				ret->ConstructAt<plg::vector<plg::vec4>>();
				break;
			case ValueType::ArrayMatrix4x4:
				ret->ConstructAt<plg::vector<plg::mat4x4>>();
				break;
			case ValueType::Vector2:
				ret->SetReturn<plg::vec2>({});
				break;
			case ValueType::Vector3:
				ret->SetReturn<plg::vec3>({});
				break;
			case ValueType::Vector4:
				ret->SetReturn<plg::vec4>({});
				break;
			case ValueType::Matrix4x4:
				ret->SetReturn<plg::mat4x4>({});
				break;
			default: {
				_provider->Log(std::format(LOG_PREFIX "SetFallbackReturn unsupported type {:#x}", static_cast<uint8_t>(retType)), Severity::Fatal);
				std::terminate();
				break;
			}
		}
	}

	bool V8LanguageModule::SetReturn(v8::Local<v8::Value> result, PropertyHandle retType, const JitCallback::Return* ret) {
		switch (retType.GetType()) {
			case ValueType::Void:
				return true;
			case ValueType::Bool:
				if (auto value = ValueFromObject<bool>(result)) {
					ret->SetReturn<bool>(*value);
					return true;
				}
				break;
			case ValueType::Char8:
				if (auto value = ValueFromObject<char>(result)) {
					ret->SetReturn<char>(*value);
					return true;
				}
				break;
			case ValueType::Char16:
				if (auto value = ValueFromObject<char16_t>(result)) {
					ret->SetReturn<char16_t>(*value);
					return true;
				}
				break;
			case ValueType::Int8:
				if (auto value = ValueFromObject<int8_t>(result)) {
					ret->SetReturn<int8_t>(*value);
					return true;
				}
				break;
			case ValueType::Int16:
				if (auto value = ValueFromObject<int16_t>(result)) {
					ret->SetReturn<int16_t>(*value);
					return true;
				}
				break;
			case ValueType::Int32:
				if (auto value = ValueFromObject<int32_t>(result)) {
					ret->SetReturn<int32_t>(*value);
					return true;
				}
				break;
			case ValueType::Int64:
				if (auto value = ValueFromObject<int64_t>(result)) {
					ret->SetReturn<int64_t>(*value);
					return true;
				}
				break;
			case ValueType::UInt8:
				if (auto value = ValueFromObject<uint8_t>(result)) {
					ret->SetReturn<uint8_t>(*value);
					return true;
				}
				break;
			case ValueType::UInt16:
				if (auto value = ValueFromObject<uint16_t>(result)) {
					ret->SetReturn<uint16_t>(*value);
					return true;
				}
				break;
			case ValueType::UInt32:
				if (auto value = ValueFromObject<uint32_t>(result)) {
					ret->SetReturn<uint32_t>(*value);
					return true;
				}
				break;
			case ValueType::UInt64:
				if (auto value = ValueFromObject<uint64_t>(result)) {
					ret->SetReturn<uint64_t>(*value);
					return true;
				}
				break;
			case ValueType::Pointer:
				if (auto value = ValueFromObject<void*>(result)) {
					ret->SetReturn<void*>(*value);
					return true;
				}
				break;
			case ValueType::Float:
				if (auto value = ValueFromObject<float>(result)) {
					ret->SetReturn<float>(*value);
					return true;
				}
				break;
			case ValueType::Double:
				if (auto value = ValueFromObject<double>(result)) {
					ret->SetReturn<double>(*value);
					return true;
				}
				break;
			case ValueType::Function:
				if (auto value = GetOrCreateFunctionValue(retType.GetPrototype(), result)) {
					ret->SetReturn<void*>(*value);
					return true;
				}
				break;
			case ValueType::String:
				if (auto value = ValueFromObject<plg::string>(result)) {
					ret->ConstructAt<plg::string>(std::move(*value));
					return true;
				}
				break;
			case ValueType::Any:
				if (auto value = ValueFromObject<plg::any>(result)) {
					ret->ConstructAt<plg::any>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayBool:
				if (auto value = ArrayFromObject<bool>(result)) {
					ret->ConstructAt<plg::vector<bool>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayChar8:
				if (auto value = ArrayFromObject<char>(result)) {
					ret->ConstructAt<plg::vector<char>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayChar16:
				if (auto value = ArrayFromObject<char16_t>(result)) {
					ret->ConstructAt<plg::vector<char16_t>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayInt8:
				if (auto value = ArrayFromObject<int8_t>(result)) {
					ret->ConstructAt<plg::vector<int8_t>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayInt16:
				if (auto value = ArrayFromObject<int16_t>(result)) {
					ret->ConstructAt<plg::vector<int16_t>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayInt32:
				if (auto value = ArrayFromObject<int32_t>(result)) {
					ret->ConstructAt<plg::vector<int32_t>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayInt64:
				if (auto value = ArrayFromObject<int64_t>(result)) {
					ret->ConstructAt<plg::vector<int64_t>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayUInt8:
				if (auto value = ArrayFromObject<uint8_t>(result)) {
					ret->ConstructAt<plg::vector<uint8_t>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayUInt16:
				if (auto value = ArrayFromObject<uint16_t>(result)) {
					ret->ConstructAt<plg::vector<uint16_t>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayUInt32:
				if (auto value = ArrayFromObject<uint32_t>(result)) {
					ret->ConstructAt<plg::vector<uint32_t>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayUInt64:
				if (auto value = ArrayFromObject<uint64_t>(result)) {
					ret->ConstructAt<plg::vector<uint64_t>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayPointer:
				if (auto value = ArrayFromObject<void*>(result)) {
					ret->ConstructAt<plg::vector<void*>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayFloat:
				if (auto value = ArrayFromObject<float>(result)) {
					ret->ConstructAt<plg::vector<float>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayDouble:
				if (auto value = ArrayFromObject<double>(result)) {
					ret->ConstructAt<plg::vector<double>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayString:
				if (auto value = ArrayFromObject<plg::string>(result)) {
					ret->ConstructAt<plg::vector<plg::string>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayAny:
				if (auto value = ArrayFromObject<plg::any>(result)) {
					ret->ConstructAt<plg::vector<plg::any>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayVector2:
				if (auto value = ArrayFromObject<plg::vec2>(result)) {
					ret->ConstructAt<plg::vector<plg::vec2>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayVector3:
				if (auto value = ArrayFromObject<plg::vec3>(result)) {
					ret->ConstructAt<plg::vector<plg::vec3>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayVector4:
				if (auto value = ArrayFromObject<plg::vec4>(result)) {
					ret->ConstructAt<plg::vector<plg::vec4>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayMatrix4x4:
				if (auto value = ArrayFromObject<plg::mat4x4>(result)) {
					ret->ConstructAt<plg::vector<plg::mat4x4>>(std::move(*value));
					return true;
				}
				break;
			case ValueType::Vector2:
				if (auto value = ValueFromObject<plg::vec2>(result)) {
					ret->SetReturn<plg::vec2>(*value);
					return true;
				}
				break;
			case ValueType::Vector3:
				if (auto value = ValueFromObject<plg::vec3>(result)) {
					ret->SetReturn<plg::vec3>(*value);
					return true;
				}
				break;
			case ValueType::Vector4:
				if (auto value = ValueFromObject<plg::vec4>(result)) {
					ret->SetReturn<plg::vec4>(*value);
					return true;
				}
				break;
			case ValueType::Matrix4x4:
				if (auto value = ValueFromObject<plg::mat4x4>(result)) {
					ret->SetReturn<plg::mat4x4>(*value);
					return true;
				}
				break;
			default: {
				_provider->Log(std::format(LOG_PREFIX "SetReturn unsupported type {:#x}", static_cast<uint8_t>(retType.GetType())), Severity::Fatal);
				std::terminate();
				break;
			}
		}

		return false;
	}

	bool V8LanguageModule::SetRefParam(v8::Local<v8::Value> object, PropertyHandle paramType, const JitCallback::Parameters* params, size_t index) {
		switch (paramType.GetType()) {
			case ValueType::Bool:
				if (auto value = ValueFromObject<bool>(object)) {
					auto* const param = params->GetArgument<bool*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Char8:
				if (auto value = ValueFromObject<char>(object)) {
					auto* const param = params->GetArgument<char*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Char16:
				if (auto value = ValueFromObject<char16_t>(object)) {
					auto* const param = params->GetArgument<char16_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Int8:
				if (auto value = ValueFromObject<int8_t>(object)) {
					auto* const param = params->GetArgument<int8_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Int16:
				if (auto value = ValueFromObject<int16_t>(object)) {
					auto* const param = params->GetArgument<int16_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Int32:
				if (auto value = ValueFromObject<int32_t>(object)) {
					auto* const param = params->GetArgument<int32_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Int64:
				if (auto value = ValueFromObject<int64_t>(object)) {
					auto* const param = params->GetArgument<int64_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::UInt8:
				if (auto value = ValueFromObject<uint8_t>(object)) {
					auto* const param = params->GetArgument<uint8_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::UInt16:
				if (auto value = ValueFromObject<uint16_t>(object)) {
					auto* const param = params->GetArgument<uint16_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::UInt32:
				if (auto value = ValueFromObject<uint32_t>(object)) {
					auto* const param = params->GetArgument<uint32_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::UInt64:
				if (auto value = ValueFromObject<uint64_t>(object)) {
					auto* const param = params->GetArgument<uint64_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Pointer:
				if (auto value = ValueFromObject<void*>(object)) {
					auto* const param = params->GetArgument<void**>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Float:
				if (auto value = ValueFromObject<float>(object)) {
					auto* const param = params->GetArgument<float*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Double:
				if (auto value = ValueFromObject<double>(object)) {
					auto* const param = params->GetArgument<double*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::String:
				if (auto value = ValueFromObject<plg::string>(object)) {
					auto* const param = params->GetArgument<plg::string*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::Any:
				if (auto value = ValueFromObject<plg::any>(object)) {
					auto* const param = params->GetArgument<plg::any*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayBool:
				if (auto value = ArrayFromObject<bool>(object)) {
					auto* const param = params->GetArgument<plg::vector<bool>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayChar8:
				if (auto value = ArrayFromObject<char>(object)) {
					auto* const param = params->GetArgument<plg::vector<char>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayChar16:
				if (auto value = ArrayFromObject<char16_t>(object)) {
					auto* const param = params->GetArgument<plg::vector<char16_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayInt8:
				if (auto value = ArrayFromObject<int8_t>(object)) {
					auto* const param = params->GetArgument<plg::vector<int8_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayInt16:
				if (auto value = ArrayFromObject<int16_t>(object)) {
					auto* const param = params->GetArgument<plg::vector<int16_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayInt32:
				if (auto value = ArrayFromObject<int32_t>(object)) {
					auto* const param = params->GetArgument<plg::vector<int32_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayInt64:
				if (auto value = ArrayFromObject<int64_t>(object)) {
					auto* const param = params->GetArgument<plg::vector<int64_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayUInt8:
				if (auto value = ArrayFromObject<uint8_t>(object)) {
					auto* const param = params->GetArgument<plg::vector<uint8_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayUInt16:
				if (auto value = ArrayFromObject<uint16_t>(object)) {
					auto* const param = params->GetArgument<plg::vector<uint16_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayUInt32:
				if (auto value = ArrayFromObject<uint32_t>(object)) {
					auto* const param = params->GetArgument<plg::vector<uint32_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayUInt64:
				if (auto value = ArrayFromObject<uint64_t>(object)) {
					auto* const param = params->GetArgument<plg::vector<uint64_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayPointer:
				if (auto value = ArrayFromObject<void*>(object)) {
					auto* const param = params->GetArgument<plg::vector<void*>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayFloat:
				if (auto value = ArrayFromObject<float>(object)) {
					auto* const param = params->GetArgument<plg::vector<float>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayDouble:
				if (auto value = ArrayFromObject<double>(object)) {
					auto* const param = params->GetArgument<plg::vector<double>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayString:
				if (auto value = ArrayFromObject<plg::string>(object)) {
					auto* const param = params->GetArgument<plg::vector<plg::string>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayAny:
				if (auto value = ArrayFromObject<plg::any>(object)) {
					auto* const param = params->GetArgument<plg::vector<plg::any>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayVector2:
				if (auto value = ArrayFromObject<plg::vec2>(object)) {
					auto* const param = params->GetArgument<plg::vector<plg::vec2>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayVector3:
				if (auto value = ArrayFromObject<plg::vec3>(object)) {
					auto* const param = params->GetArgument<plg::vector<plg::vec3>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayVector4:
				if (auto value = ArrayFromObject<plg::vec4>(object)) {
					auto* const param = params->GetArgument<plg::vector<plg::vec4>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayMatrix4x4:
				if (auto value = ArrayFromObject<plg::mat4x4>(object)) {
					auto* const param = params->GetArgument<plg::vector<plg::mat4x4>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::Vector2:
				if (auto value = ValueFromObject<plg::vec2>(object)) {
					auto* const param = params->GetArgument<plg::vec2*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Vector3:
				if (auto value = ValueFromObject<plg::vec3>(object)) {
					auto* const param = params->GetArgument<plg::vec3*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Vector4:
				if (auto value = ValueFromObject<plg::vec4>(object)) {
					auto* const param = params->GetArgument<plg::vec4*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Matrix4x4:
				if (auto value = ValueFromObject<plg::mat4x4>(object)) {
					auto* const param = params->GetArgument<plg::mat4x4*>(index);
					*param = *value;
					return true;
				}
				break;
			default: {
				_provider->Log(std::format(LOG_PREFIX "SetRefParam unsupported type {:#x}", static_cast<uint8_t>(paramType.GetType())), Severity::Fatal);
				std::terminate();
			}
		}

		return false;
	}

	v8::Local<v8::Value> V8LanguageModule::ParamToObject(PropertyHandle paramType, const JitCallback::Parameters* params, size_t index) {
		switch (paramType.GetType()) {
			case ValueType::Bool:
				return CreateJsObject(params->GetArgument<bool>(index));
			case ValueType::Char8:
				return CreateJsObject(params->GetArgument<char>(index));
			case ValueType::Char16:
				return CreateJsObject(params->GetArgument<char16_t>(index));
			case ValueType::Int8:
				return CreateJsObject(params->GetArgument<int8_t>(index));
			case ValueType::Int16:
				return CreateJsObject(params->GetArgument<int16_t>(index));
			case ValueType::Int32:
				return CreateJsObject(params->GetArgument<int32_t>(index));
			case ValueType::Int64:
				return CreateJsObject(params->GetArgument<int64_t>(index));
			case ValueType::UInt8:
				return CreateJsObject(params->GetArgument<uint8_t>(index));
			case ValueType::UInt16:
				return CreateJsObject(params->GetArgument<uint16_t>(index));
			case ValueType::UInt32:
				return CreateJsObject(params->GetArgument<uint32_t>(index));
			case ValueType::UInt64:
				return CreateJsObject(params->GetArgument<uint64_t>(index));
			case ValueType::Pointer:
				return CreateJsObject(params->GetArgument<void*>(index));
			case ValueType::Float:
				return CreateJsObject(params->GetArgument<float>(index));
			case ValueType::Double:
				return CreateJsObject(params->GetArgument<double>(index));
			case ValueType::Function:
				return GetOrCreateFunctionObject(paramType.GetPrototype(), params->GetArgument<void*>(index));
			case ValueType::String:
				return CreateJsObject(*(params->GetArgument<const plg::string*>(index)));
			case ValueType::Any:
				return CreateJsObject(*(params->GetArgument<const plg::any*>(index)));
			case ValueType::ArrayBool:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<bool>*>(index)));
			case ValueType::ArrayChar8:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<char>*>(index)));
			case ValueType::ArrayChar16:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<char16_t>*>(index)));
			case ValueType::ArrayInt8:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<int8_t>*>(index)));
			case ValueType::ArrayInt16:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<int16_t>*>(index)));
			case ValueType::ArrayInt32:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<int32_t>*>(index)));
			case ValueType::ArrayInt64:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<int64_t>*>(index)));
			case ValueType::ArrayUInt8:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<uint8_t>*>(index)));
			case ValueType::ArrayUInt16:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<uint16_t>*>(index)));
			case ValueType::ArrayUInt32:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<uint32_t>*>(index)));
			case ValueType::ArrayUInt64:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<uint64_t>*>(index)));
			case ValueType::ArrayPointer:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<void*>*>(index)));
			case ValueType::ArrayFloat:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<float>*>(index)));
			case ValueType::ArrayDouble:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<double>*>(index)));
			case ValueType::ArrayString:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::string>*>(index)));
			case ValueType::ArrayAny:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::any>*>(index)));
			case ValueType::ArrayVector2:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::vec2>*>(index)));
			case ValueType::ArrayVector3:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::vec3>*>(index)));
			case ValueType::ArrayVector4:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::vec4>*>(index)));
			case ValueType::ArrayMatrix4x4:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::mat4x4>*>(index)));
			case ValueType::Vector2:
				return CreateJsObject(*(params->GetArgument<plg::vec2*>(index)));
			case ValueType::Vector3:
				return CreateJsObject(*(params->GetArgument<plg::vec3*>(index)));
			case ValueType::Vector4:
				return CreateJsObject(*(params->GetArgument<plg::vec4*>(index)));
			case ValueType::Matrix4x4:
				return CreateJsObject(*(params->GetArgument<plg::mat4x4*>(index)));
			default: {
				_provider->Log(std::format(LOG_PREFIX "ParamToObject unsupported type {:#x}", static_cast<uint8_t>(paramType.GetType())), Severity::Fatal);
				std::terminate();
				return {};
			}
		}
	}

	v8::Local<v8::Value> V8LanguageModule::ParamRefToObject(PropertyHandle paramType, const JitCallback::Parameters* params, size_t index) {
		switch (paramType.GetType()) {
			case ValueType::Bool:
				return CreateJsObject(*(params->GetArgument<bool*>(index)));
			case ValueType::Char8:
				return CreateJsObject(*(params->GetArgument<char*>(index)));
			case ValueType::Char16:
				return CreateJsObject(*(params->GetArgument<char16_t*>(index)));
			case ValueType::Int8:
				return CreateJsObject(*(params->GetArgument<int8_t*>(index)));
			case ValueType::Int16:
				return CreateJsObject(*(params->GetArgument<int16_t*>(index)));
			case ValueType::Int32:
				return CreateJsObject(*(params->GetArgument<int32_t*>(index)));
			case ValueType::Int64:
				return CreateJsObject(*(params->GetArgument<int64_t*>(index)));
			case ValueType::UInt8:
				return CreateJsObject(*(params->GetArgument<uint8_t*>(index)));
			case ValueType::UInt16:
				return CreateJsObject(*(params->GetArgument<uint16_t*>(index)));
			case ValueType::UInt32:
				return CreateJsObject(*(params->GetArgument<uint32_t*>(index)));
			case ValueType::UInt64:
				return CreateJsObject(*(params->GetArgument<uint64_t*>(index)));
			case ValueType::Pointer:
				return CreateJsObject(*(params->GetArgument<void**>(index)));
			case ValueType::Float:
				return CreateJsObject(*(params->GetArgument<float*>(index)));
			case ValueType::Double:
				return CreateJsObject(*(params->GetArgument<double*>(index)));
			case ValueType::String:
				return CreateJsObject(*(params->GetArgument<const plg::string*>(index)));
			case ValueType::Any:
				return CreateJsObject(*(params->GetArgument<const plg::any*>(index)));
			case ValueType::ArrayBool:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<bool>*>(index)));
			case ValueType::ArrayChar8:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<char>*>(index)));
			case ValueType::ArrayChar16:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<char16_t>*>(index)));
			case ValueType::ArrayInt8:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<int8_t>*>(index)));
			case ValueType::ArrayInt16:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<int16_t>*>(index)));
			case ValueType::ArrayInt32:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<int32_t>*>(index)));
			case ValueType::ArrayInt64:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<int64_t>*>(index)));
			case ValueType::ArrayUInt8:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<uint8_t>*>(index)));
			case ValueType::ArrayUInt16:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<uint16_t>*>(index)));
			case ValueType::ArrayUInt32:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<uint32_t>*>(index)));
			case ValueType::ArrayUInt64:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<uint64_t>*>(index)));
			case ValueType::ArrayPointer:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<void*>*>(index)));
			case ValueType::ArrayFloat:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<float>*>(index)));
			case ValueType::ArrayDouble:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<double>*>(index)));
			case ValueType::ArrayString:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::string>*>(index)));
			case ValueType::ArrayAny:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::any>*>(index)));
			case ValueType::ArrayVector2:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::vec2>*>(index)));
			case ValueType::ArrayVector3:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::vec3>*>(index)));
			case ValueType::ArrayVector4:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::vec4>*>(index)));
			case ValueType::ArrayMatrix4x4:
				return CreateJsObjectList(*(params->GetArgument<const plg::vector<plg::mat4x4>*>(index)));
			case ValueType::Vector2:
				return CreateJsObject(*(params->GetArgument<plg::vec2*>(index)));
			case ValueType::Vector3:
				return CreateJsObject(*(params->GetArgument<plg::vec3*>(index)));
			case ValueType::Vector4:
				return CreateJsObject(*(params->GetArgument<plg::vec4*>(index)));
			case ValueType::Matrix4x4:
				return CreateJsObject(*(params->GetArgument<plg::mat4x4*>(index)));
			default: {
				_provider->Log(std::format(LOG_PREFIX "ParamRefToObject unsupported type {:#x}", static_cast<uint8_t>(paramType.GetType())), Severity::Fatal);
				std::terminate();
				return {};
			}
		}
	}

	void V8LanguageModule::InternalCall(MethodHandle method, MemAddr data, const JitCallback::Parameters* params, size_t count, const JitCallback::Return* ret) {
		v8::Locker locker(_isolate);
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		v8::Local<v8::Context> context = _context.Get(_isolate);
		v8::Context::Scope contextScope(context);
		v8::TryCatch tryCatch(_isolate);

		v8::Global<v8::Function>& func = *data.RCast<v8::Global<v8::Function>*>();

		const auto retType = method.GetReturnType();
		const auto paramTypes = method.GetParamTypes();
		const size_t paramsCount = paramTypes.size();
		int refParamsCount = 0;

		std::vector<v8::Local<v8::Value>> args(paramsCount);

		for (size_t index = 0; index < paramsCount; ++index) {
			const PropertyHandle paramType = paramTypes[index];
			if (paramType.IsReference()) {
				++refParamsCount;
			}

			using ParamConvertionFunc = decltype(&V8LanguageModule::ParamToObject);
			ParamConvertionFunc const convertFunc = paramType.IsReference() ? &V8LanguageModule::ParamRefToObject : &V8LanguageModule::ParamToObject;
			v8::Local<v8::Value> arg = (this->*convertFunc)(paramType, params, index);
			if (arg.IsEmpty()) {
				if (tryCatch.HasCaught()) {
					ReportException(tryCatch.Message());
				}
				SetFallbackReturn(retType.GetType(), ret);
				return;
			}
			args[index] = arg;
		}

		v8::Local<v8::Value> result;
		if (!func.Get(_isolate)->Call(context, v8::Undefined(_isolate), static_cast<int>(paramsCount), args.data()).ToLocal(&result)) {
			if (tryCatch.HasCaught()) {
				ReportException(tryCatch.Message());
			}
			SetFallbackReturn(retType.GetType(), ret);
			return;
		}

		if (refParamsCount != 0) {
			if (!result->IsArray()) {
				ThrowTypeError("Expected array as return value", result);
				if (tryCatch.HasCaught()) {
					ReportException(tryCatch.Message());
				}
				SetFallbackReturn(retType.GetType(), ret);
				return;
			}

			v8::Local<v8::Array> resultArray = result.As<v8::Array>();
			if (resultArray->Length() != 1 + refParamsCount) {
				ThrowRangeError(std::format("Returned array wrong size {}, expected {}", resultArray->Length(), 1 + refParamsCount));
				if (tryCatch.HasCaught()) {
					ReportException(tryCatch.Message());
				}
				SetFallbackReturn(retType.GetType(), ret);
				return;
			}

			int k = 0;

			for (size_t index = 0; index < paramsCount; ++index) {
				const PropertyHandle paramType = paramTypes[index];
				if (!paramType.IsReference()) {
					continue;
				}

				if (!SetRefParam(resultArray->Get(context, 1 + k).ToLocalChecked(), paramType, params, index)) {
					if (tryCatch.HasCaught()) {
						ReportException(tryCatch.Message());
						tryCatch.Reset();
					}
				}
				if (++k == refParamsCount) {
					break;
				}
			}
		}

		v8::Local<v8::Value> returnValue = refParamsCount != 0 ? result.As<v8::Array>()->Get(context, 0).ToLocalChecked() : result;
		if (!SetReturn(returnValue, retType, ret)) {
			if (tryCatch.HasCaught()) {
				ReportException(tryCatch.Message());
			}
			SetFallbackReturn(retType.GetType(), ret);
			return;
		}
	}

#pragma endregion InternalCall

#pragma region ExternalCall

	V8LanguageModule::ArgsScope::ArgsScope(size_t size) : params(size) {
		storage.reserve(size);
	}

	V8LanguageModule::ArgsScope::~ArgsScope() {
		for (auto& [ptr, type] : storage) {
			switch (type) {
				case ValueType::Bool: {
					delete static_cast<bool*>(ptr);
					break;
				}
				case ValueType::Char8: {
					delete static_cast<char*>(ptr);
					break;
				}
				case ValueType::Char16: {
					delete static_cast<char16_t*>(ptr);
					break;
				}
				case ValueType::Int8: {
					delete static_cast<int8_t*>(ptr);
					break;
				}
				case ValueType::Int16: {
					delete static_cast<int16_t*>(ptr);
					break;
				}
				case ValueType::Int32: {
					delete static_cast<int32_t*>(ptr);
					break;
				}
				case ValueType::Int64: {
					delete static_cast<int64_t*>(ptr);
					break;
				}
				case ValueType::UInt8: {
					delete static_cast<uint8_t*>(ptr);
					break;
				}
				case ValueType::UInt16: {
					delete static_cast<uint16_t*>(ptr);
					break;
				}
				case ValueType::UInt32: {
					delete static_cast<uint32_t*>(ptr);
					break;
				}
				case ValueType::UInt64: {
					delete static_cast<uint64_t*>(ptr);
					break;
				}
				case ValueType::Pointer: {
					delete static_cast<void**>(ptr);
					break;
				}
				case ValueType::Float: {
					delete static_cast<float*>(ptr);
					break;
				}
				case ValueType::Double: {
					delete static_cast<double*>(ptr);
					break;
				}
				case ValueType::String: {
					delete static_cast<plg::string*>(ptr);
					break;
				}
				case ValueType::Any: {
					delete static_cast<plg::any*>(ptr);
					break;
				}
				case ValueType::ArrayBool: {
					delete static_cast<plg::vector<bool>*>(ptr);
					break;
				}
				case ValueType::ArrayChar8: {
					delete static_cast<plg::vector<char>*>(ptr);
					break;
				}
				case ValueType::ArrayChar16: {
					delete static_cast<plg::vector<char16_t>*>(ptr);
					break;
				}
				case ValueType::ArrayInt8: {
					delete static_cast<plg::vector<int8_t>*>(ptr);
					break;
				}
				case ValueType::ArrayInt16: {
					delete static_cast<plg::vector<int16_t>*>(ptr);
					break;
				}
				case ValueType::ArrayInt32: {
					delete static_cast<plg::vector<int32_t>*>(ptr);
					break;
				}
				case ValueType::ArrayInt64: {
					delete static_cast<plg::vector<int64_t>*>(ptr);
					break;
				}
				case ValueType::ArrayUInt8: {
					delete static_cast<plg::vector<uint8_t>*>(ptr);
					break;
				}
				case ValueType::ArrayUInt16: {
					delete static_cast<plg::vector<uint16_t>*>(ptr);
					break;
				}
				case ValueType::ArrayUInt32: {
					delete static_cast<plg::vector<uint32_t>*>(ptr);
					break;
				}
				case ValueType::ArrayUInt64: {
					delete static_cast<plg::vector<uint64_t>*>(ptr);
					break;
				}
				case ValueType::ArrayPointer: {
					delete static_cast<plg::vector<void*>*>(ptr);
					break;
				}
				case ValueType::ArrayFloat: {
					delete static_cast<plg::vector<float>*>(ptr);
					break;
				}
				case ValueType::ArrayDouble: {
					delete static_cast<plg::vector<double>*>(ptr);
					break;
				}
				case ValueType::ArrayString: {
					delete static_cast<plg::vector<plg::string>*>(ptr);
					break;
				}
				case ValueType::ArrayAny: {
					delete static_cast<plg::vector<plg::any>*>(ptr);
					break;
				}
				case ValueType::ArrayVector2: {
					delete static_cast<plg::vector<plg::vec2>*>(ptr);
					break;
				}
				case ValueType::ArrayVector3: {
					delete static_cast<plg::vector<plg::vec3>*>(ptr);
					break;
				}
				case ValueType::ArrayVector4: {
					delete static_cast<plg::vector<plg::vec4>*>(ptr);
					break;
				}
				case ValueType::ArrayMatrix4x4: {
					delete static_cast<plg::vector<plg::mat4x4>*>(ptr);
					break;
				}
				case ValueType::Vector2: {
					delete static_cast<plg::vec2*>(ptr);
					break;
				}
				case ValueType::Vector3: {
					delete static_cast<plg::vec3*>(ptr);
					break;
				}
				case ValueType::Vector4: {
					delete static_cast<plg::vec4*>(ptr);
					break;
				}
				case ValueType::Matrix4x4: {
					delete static_cast<plg::mat4x4*>(ptr);
					break;
				}
				default: {
					g_v8lm._provider->Log(std::format(LOG_PREFIX "[ArgsScope unhandled type {:#x}", static_cast<uint8_t>(type)), Severity::Fatal);
					std::terminate();
					break;
				}
			}
		}
	}

	void V8LanguageModule::BeginExternalCall(ValueType retType, ArgsScope& a) const {
		void* value;
		switch (retType) {
			case ValueType::String: {
				value = new plg::string();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::Any: {
				value = new plg::any();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayBool: {
				value = new plg::vector<bool>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayChar8: {
				value = new plg::vector<char>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayChar16: {
				value = new plg::vector<char16_t>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayInt8: {
				value = new plg::vector<int8_t>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayInt16: {
				value = new plg::vector<int16_t>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayInt32: {
				value = new plg::vector<int32_t>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayInt64: {
				value = new plg::vector<int64_t>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayUInt8: {
				value = new plg::vector<uint8_t>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayUInt16: {
				value = new plg::vector<uint16_t>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayUInt32: {
				value = new plg::vector<uint32_t>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayUInt64: {
				value = new plg::vector<uint64_t>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayPointer: {
				value = new plg::vector<void*>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayFloat: {
				value = new plg::vector<float>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayDouble: {
				value = new plg::vector<double>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayString: {
				value = new plg::vector<plg::string>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayAny: {
				value = new plg::vector<plg::any>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayVector2: {
				value = new plg::vector<plg::vec2>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayVector3: {
				value = new plg::vector<plg::vec3>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayVector4: {
				value = new plg::vector<plg::vec4>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::ArrayMatrix4x4: {
				value = new plg::vector<plg::mat4x4>();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::Vector2: {
				value = new plg::vec2();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::Vector3: {
				value = new plg::vec3();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::Vector4: {
				value = new plg::vec4();
				a.storage.emplace_back(value, retType);
				break;
			}
			case ValueType::Matrix4x4: {
				value = new plg::mat4x4();
				a.storage.emplace_back(value, retType);
				break;
			}
			default:
				_provider->Log(std::format(LOG_PREFIX "BeginExternalCall unsupported type {:#x}", static_cast<uint8_t>(retType)), Severity::Fatal);
				std::terminate();
				break;
		}

		a.params.AddArgument(value);
	}

	v8::Local<v8::Value> V8LanguageModule::MakeExternalCallWithObject(PropertyHandle retType, JitCall::CallingFunc func, const ArgsScope& a, JitCall::Return& ret) {
		func(a.params.GetDataPtr(), &ret);
		switch (retType.GetType()) {
			case ValueType::Void:
				return v8::Undefined(_isolate);
			case ValueType::Bool: {
				const bool val = ret.GetReturn<bool>();
				return CreateJsObject(val);
			}
			case ValueType::Char8: {
				const char val = ret.GetReturn<char>();
				return CreateJsObject(val);
			}
			case ValueType::Char16: {
				const char16_t val = ret.GetReturn<char16_t>();
				return CreateJsObject(val);
			}
			case ValueType::Int8: {
				const int8_t val = ret.GetReturn<int8_t>();
				return CreateJsObject(val);
			}
			case ValueType::Int16: {
				const int16_t val = ret.GetReturn<int16_t>();
				return CreateJsObject(val);
			}
			case ValueType::Int32: {
				const int32_t val = ret.GetReturn<int32_t>();
				return CreateJsObject(val);
			}
			case ValueType::Int64: {
				const int64_t val = ret.GetReturn<int64_t>();
				return CreateJsObject(val);
			}
			case ValueType::UInt8: {
				const uint8_t val = ret.GetReturn<uint8_t>();
				return CreateJsObject(val);
			}
			case ValueType::UInt16: {
				const uint16_t val = ret.GetReturn<uint16_t>();
				return CreateJsObject(val);
			}
			case ValueType::UInt32: {
				const uint32_t val = ret.GetReturn<uint32_t>();
				return CreateJsObject(val);
			}
			case ValueType::UInt64: {
				const uint64_t val = ret.GetReturn<uint64_t>();
				return CreateJsObject(val);
			}
			case ValueType::Pointer: {
				void* val = ret.GetReturn<void*>();
				return CreateJsObject(val);
			}
			case ValueType::Float: {
				const float val = ret.GetReturn<float>();
				return CreateJsObject(val);
			}
			case ValueType::Double: {
				const double val = ret.GetReturn<double>();
				return CreateJsObject(val);
			}
			case ValueType::Function: {
				void* const val = ret.GetReturn<void*>();
				return GetOrCreateFunctionObject(retType.GetPrototype(), val);
			}
			case ValueType::String: {
				auto* const str = ret.GetReturn<plg::string*>();
				return CreateJsObject(*str);
			}
			case ValueType::Any: {
				auto* const any = ret.GetReturn<plg::any*>();
				return CreateJsObject(*any);
			}
			case ValueType::ArrayBool: {
				auto* const arr = ret.GetReturn<plg::vector<bool>*>();
				return CreateJsObjectList<bool>(*arr);
			}
			case ValueType::ArrayChar8: {
				auto* const arr = ret.GetReturn<plg::vector<char>*>();
				return CreateJsObjectList<char>(*arr);
			}
			case ValueType::ArrayChar16: {
				auto* const arr = ret.GetReturn<plg::vector<char16_t>*>();
				return CreateJsObjectList<char16_t>(*arr);
			}
			case ValueType::ArrayInt8: {
				auto* const arr = ret.GetReturn<plg::vector<int8_t>*>();
				return CreateJsObjectList<int8_t>(*arr);
			}
			case ValueType::ArrayInt16: {
				auto* const arr = ret.GetReturn<plg::vector<int16_t>*>();
				return CreateJsObjectList<int16_t>(*arr);
			}
			case ValueType::ArrayInt32: {
				auto* const arr = ret.GetReturn<plg::vector<int32_t>*>();
				return CreateJsObjectList<int32_t>(*arr);
			}
			case ValueType::ArrayInt64: {
				auto* const arr = ret.GetReturn<plg::vector<int64_t>*>();
				return CreateJsObjectList<int64_t>(*arr);
			}
			case ValueType::ArrayUInt8: {
				auto* const arr = ret.GetReturn<plg::vector<uint8_t>*>();
				return CreateJsObjectList<uint8_t>(*arr);
			}
			case ValueType::ArrayUInt16: {
				auto* const arr = ret.GetReturn<plg::vector<uint16_t>*>();
				return CreateJsObjectList<uint16_t>(*arr);
			}
			case ValueType::ArrayUInt32: {
				auto* const arr = ret.GetReturn<plg::vector<uint32_t>*>();
				return CreateJsObjectList<uint32_t>(*arr);
			}
			case ValueType::ArrayUInt64: {
				auto* const arr = ret.GetReturn<plg::vector<uint64_t>*>();
				return CreateJsObjectList<uint64_t>(*arr);
			}
			case ValueType::ArrayPointer: {
				auto* const arr = ret.GetReturn<plg::vector<void*>*>();
				return CreateJsObjectList<void*>(*arr);
			}
			case ValueType::ArrayFloat: {
				auto* const arr = ret.GetReturn<plg::vector<float>*>();
				return CreateJsObjectList<float>(*arr);
			}
			case ValueType::ArrayDouble: {
				auto* const arr = ret.GetReturn<plg::vector<double>*>();
				return CreateJsObjectList<double>(*arr);
			}
			case ValueType::ArrayString: {
				auto* const arr = ret.GetReturn<plg::vector<plg::string>*>();
				return CreateJsObjectList<plg::string>(*arr);
			}
			case ValueType::ArrayAny: {
				auto* const arr = ret.GetReturn<plg::vector<plg::any>*>();
				return CreateJsObjectList<plg::any>(*arr);
			}
			case ValueType::ArrayVector2: {
				auto* const arr = ret.GetReturn<plg::vector<plg::vec2>*>();
				return CreateJsObjectList<plg::vec2>(*arr);
			}
			case ValueType::ArrayVector3: {
				auto* const arr = ret.GetReturn<plg::vector<plg::vec3>*>();
				return CreateJsObjectList<plg::vec3>(*arr);
			}
			case ValueType::ArrayVector4: {
				auto* const arr = ret.GetReturn<plg::vector<plg::vec4>*>();
				return CreateJsObjectList<plg::vec4>(*arr);
			}
			case ValueType::ArrayMatrix4x4: {
				auto* const arr = ret.GetReturn<plg::vector<plg::mat4x4>*>();
				return CreateJsObjectList<plg::mat4x4>(*arr);
			}
			case ValueType::Vector2: {
				const plg::vec2 val = ret.GetReturn<plg::vec2>();
				return CreateJsObject(val);
			}
			case ValueType::Vector3: {
				plg::vec3 val;
				if (ValueUtils::IsHiddenParam(retType.GetType())) {
					val = *ret.GetReturn<plg::vec3*>();
				} else {
					val = ret.GetReturn<plg::vec3>();
				}
				return CreateJsObject(val);
			}
			case ValueType::Vector4: {
				plg::vec4 val;
				if (ValueUtils::IsHiddenParam(retType.GetType())) {
					val = *ret.GetReturn<plg::vec4*>();
				} else {
					val = ret.GetReturn<plg::vec4>();
				}
				return CreateJsObject(val);
			}
			case ValueType::Matrix4x4: {
				plg::mat4x4 val = *ret.GetReturn<plg::mat4x4*>();
				return CreateJsObject(val);
			}
			default:
				ThrowTypeError(std::format("MakeExternalCallWithObject unsupported type {:#x}", static_cast<uint8_t>(retType.GetType())));
				return {};
		}
		return {};
	}

	bool V8LanguageModule::PushObjectAsParam(PropertyHandle paramType, v8::Local<v8::Value> item, ArgsScope& a) {
		const auto PushValParam = [&a](auto&& value) {
			if (!value) {
				return false;
			}
			a.params.AddArgument(*value);
			return true;
		};
		const auto PushRefParam = [&paramType, &a](void* value) {
			if (!value) {
				return false;
			}
			a.storage.emplace_back(value, paramType.GetType());
			a.params.AddArgument(value);
			return true;
		};

		switch (paramType.GetType()) {
			case ValueType::Bool:
				return PushValParam(ValueFromObject<bool>(item));
			case ValueType::Char8:
				return PushValParam(ValueFromObject<char>(item));
			case ValueType::Char16:
				return PushValParam(ValueFromObject<char16_t>(item));
			case ValueType::Int8:
				return PushValParam(ValueFromObject<int8_t>(item));
			case ValueType::Int16:
				return PushValParam(ValueFromObject<int16_t>(item));
			case ValueType::Int32:
				return PushValParam(ValueFromObject<int32_t>(item));
			case ValueType::Int64:
				return PushValParam(ValueFromObject<int64_t>(item));
			case ValueType::UInt8:
				return PushValParam(ValueFromObject<uint8_t>(item));
			case ValueType::UInt16:
				return PushValParam(ValueFromObject<uint16_t>(item));
			case ValueType::UInt32:
				return PushValParam(ValueFromObject<uint32_t>(item));
			case ValueType::UInt64:
				return PushValParam(ValueFromObject<uint64_t>(item));
			case ValueType::Pointer:
				return PushValParam(ValueFromObject<void*>(item));
			case ValueType::Float:
				return PushValParam(ValueFromObject<float>(item));
			case ValueType::Double:
				return PushValParam(ValueFromObject<double>(item));
			case ValueType::String:
				return PushRefParam(CreateValue<plg::string>(item));
			case ValueType::Any:
				return PushRefParam(CreateValue<plg::any>(item));
			case ValueType::Function:
				return PushValParam(GetOrCreateFunctionValue(paramType.GetPrototype(), item));
			case ValueType::ArrayBool:
				return PushRefParam(CreateArray<bool>(item));
			case ValueType::ArrayChar8:
				return PushRefParam(CreateArray<char>(item));
			case ValueType::ArrayChar16:
				return PushRefParam(CreateArray<char16_t>(item));
			case ValueType::ArrayInt8:
				return PushRefParam(CreateArray<int8_t>(item));
			case ValueType::ArrayInt16:
				return PushRefParam(CreateArray<int16_t>(item));
			case ValueType::ArrayInt32:
				return PushRefParam(CreateArray<int32_t>(item));
			case ValueType::ArrayInt64:
				return PushRefParam(CreateArray<int64_t>(item));
			case ValueType::ArrayUInt8:
				return PushRefParam(CreateArray<uint8_t>(item));
			case ValueType::ArrayUInt16:
				return PushRefParam(CreateArray<uint16_t>(item));
			case ValueType::ArrayUInt32:
				return PushRefParam(CreateArray<uint32_t>(item));
			case ValueType::ArrayUInt64:
				return PushRefParam(CreateArray<uint64_t>(item));
			case ValueType::ArrayPointer:
				return PushRefParam(CreateArray<void*>(item));
			case ValueType::ArrayFloat:
				return PushRefParam(CreateArray<float>(item));
			case ValueType::ArrayDouble:
				return PushRefParam(CreateArray<double>(item));
			case ValueType::ArrayString:
				return PushRefParam(CreateArray<plg::string>(item));
			case ValueType::ArrayAny:
				return PushRefParam(CreateArray<plg::any>(item));
			case ValueType::ArrayVector2:
				return PushRefParam(CreateArray<plg::vec2>(item));
			case ValueType::ArrayVector3:
				return PushRefParam(CreateArray<plg::vec3>(item));
			case ValueType::ArrayVector4:
				return PushRefParam(CreateArray<plg::vec4>(item));
			case ValueType::ArrayMatrix4x4:
				return PushRefParam(CreateArray<plg::mat4x4>(item));
			case ValueType::Vector2:
				return PushRefParam(CreateValue<plg::vec2>(item));
			case ValueType::Vector3:
				return PushRefParam(CreateValue<plg::vec3>(item));
			case ValueType::Vector4:
				return PushRefParam(CreateValue<plg::vec4>(item));
			case ValueType::Matrix4x4:
				return PushRefParam(CreateValue<plg::mat4x4>(item));
			default:
				ThrowTypeError(std::format("PushObjectAsParam unsupported type {:#x}", static_cast<uint8_t>(paramType.GetType())));
				return {};
		}

		return false;
	}

	bool V8LanguageModule::PushObjectAsRefParam(PropertyHandle paramType, v8::Local<v8::Value> item, ArgsScope& a) {
		const auto PushRefParam = [&paramType, &a](void* value) {
			if (!value) {
				return false;
			}
			a.storage.emplace_back(value, paramType.GetType());
			a.params.AddArgument(value);
			return true;
		};

		switch (paramType.GetType()) {
			case ValueType::Bool:
				return PushRefParam(CreateValue<bool>(item));
			case ValueType::Char8:
				return PushRefParam(CreateValue<char>(item));
			case ValueType::Char16:
				return PushRefParam(CreateValue<char16_t>(item));
			case ValueType::Int8:
				return PushRefParam(CreateValue<int8_t>(item));
			case ValueType::Int16:
				return PushRefParam(CreateValue<int16_t>(item));
			case ValueType::Int32:
				return PushRefParam(CreateValue<int32_t>(item));
			case ValueType::Int64:
				return PushRefParam(CreateValue<int64_t>(item));
			case ValueType::UInt8:
				return PushRefParam(CreateValue<uint8_t>(item));
			case ValueType::UInt16:
				return PushRefParam(CreateValue<uint16_t>(item));
			case ValueType::UInt32:
				return PushRefParam(CreateValue<uint32_t>(item));
			case ValueType::UInt64:
				return PushRefParam(CreateValue<uint64_t>(item));
			case ValueType::Pointer:
				return PushRefParam(CreateValue<void*>(item));
			case ValueType::Float:
				return PushRefParam(CreateValue<float>(item));
			case ValueType::Double:
				return PushRefParam(CreateValue<double>(item));
			case ValueType::String:
				return PushRefParam(CreateValue<plg::string>(item));
			case ValueType::Any:
				return PushRefParam(CreateValue<plg::any>(item));
			case ValueType::ArrayBool:
				return PushRefParam(CreateArray<bool>(item));
			case ValueType::ArrayChar8:
				return PushRefParam(CreateArray<char>(item));
			case ValueType::ArrayChar16:
				return PushRefParam(CreateArray<char16_t>(item));
			case ValueType::ArrayInt8:
				return PushRefParam(CreateArray<int8_t>(item));
			case ValueType::ArrayInt16:
				return PushRefParam(CreateArray<int16_t>(item));
			case ValueType::ArrayInt32:
				return PushRefParam(CreateArray<int32_t>(item));
			case ValueType::ArrayInt64:
				return PushRefParam(CreateArray<int64_t>(item));
			case ValueType::ArrayUInt8:
				return PushRefParam(CreateArray<uint8_t>(item));
			case ValueType::ArrayUInt16:
				return PushRefParam(CreateArray<uint16_t>(item));
			case ValueType::ArrayUInt32:
				return PushRefParam(CreateArray<uint32_t>(item));
			case ValueType::ArrayUInt64:
				return PushRefParam(CreateArray<uint64_t>(item));
			case ValueType::ArrayPointer:
				return PushRefParam(CreateArray<void*>(item));
			case ValueType::ArrayFloat:
				return PushRefParam(CreateArray<float>(item));
			case ValueType::ArrayDouble:
				return PushRefParam(CreateArray<double>(item));
			case ValueType::ArrayString:
				return PushRefParam(CreateArray<plg::string>(item));
			case ValueType::ArrayAny:
				return PushRefParam(CreateArray<plg::any>(item));
			case ValueType::ArrayVector2:
				return PushRefParam(CreateArray<plg::vec2>(item));
			case ValueType::ArrayVector3:
				return PushRefParam(CreateArray<plg::vec3>(item));
			case ValueType::ArrayVector4:
				return PushRefParam(CreateArray<plg::vec4>(item));
			case ValueType::ArrayMatrix4x4:
				return PushRefParam(CreateArray<plg::mat4x4>(item));
			case ValueType::Vector2:
				return PushRefParam(CreateValue<plg::vec2>(item));
			case ValueType::Vector3:
				return PushRefParam(CreateValue<plg::vec3>(item));
			case ValueType::Vector4:
				return PushRefParam(CreateValue<plg::vec4>(item));
			case ValueType::Matrix4x4:
				return PushRefParam(CreateValue<plg::mat4x4>(item));
			default:
				ThrowTypeError(std::format("PushObjectAsRefParam unsupported enum type {:#x}", static_cast<uint8_t>(paramType.GetType())));
				return {};
		}

		return false;
	}

	v8::Local<v8::Value> V8LanguageModule::StorageValueToObject(PropertyHandle paramType, const ArgsScope& a, size_t index) {
		switch (paramType.GetType()) {
			case ValueType::Bool:
				return CreateJsObject(*static_cast<bool*>(std::get<0>(a.storage[index])));
			case ValueType::Char8:
				return CreateJsObject(*static_cast<char*>(std::get<0>(a.storage[index])));
			case ValueType::Char16:
				return CreateJsObject(*static_cast<char16_t*>(std::get<0>(a.storage[index])));
			case ValueType::Int8:
				return CreateJsObject(*static_cast<int8_t*>(std::get<0>(a.storage[index])));
			case ValueType::Int16:
				return CreateJsObject(*static_cast<int16_t*>(std::get<0>(a.storage[index])));
			case ValueType::Int32:
				return CreateJsObject(*static_cast<int32_t*>(std::get<0>(a.storage[index])));
			case ValueType::Int64:
				return CreateJsObject(*static_cast<int64_t*>(std::get<0>(a.storage[index])));
			case ValueType::UInt8:
				return CreateJsObject(*static_cast<uint8_t*>(std::get<0>(a.storage[index])));
			case ValueType::UInt16:
				return CreateJsObject(*static_cast<uint16_t*>(std::get<0>(a.storage[index])));
			case ValueType::UInt32:
				return CreateJsObject(*static_cast<uint32_t*>(std::get<0>(a.storage[index])));
			case ValueType::UInt64:
				return CreateJsObject(*static_cast<uint64_t*>(std::get<0>(a.storage[index])));
			case ValueType::Float:
				return CreateJsObject(*static_cast<float*>(std::get<0>(a.storage[index])));
			case ValueType::Double:
				return CreateJsObject(*static_cast<double*>(std::get<0>(a.storage[index])));
			case ValueType::String:
				return CreateJsObject(*static_cast<plg::string*>(std::get<0>(a.storage[index])));
			case ValueType::Any:
				return CreateJsObject(*static_cast<plg::any*>(std::get<0>(a.storage[index])));
			case ValueType::Pointer:
				return CreateJsObject(*static_cast<void**>(std::get<0>(a.storage[index])));
			case ValueType::ArrayBool:
				return CreateJsObjectList(*static_cast<plg::vector<bool>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayChar8:
				return CreateJsObjectList(*static_cast<plg::vector<char>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayChar16:
				return CreateJsObjectList(*static_cast<plg::vector<char16_t>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayInt8:
				return CreateJsObjectList(*static_cast<plg::vector<int8_t>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayInt16:
				return CreateJsObjectList(*static_cast<plg::vector<int16_t>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayInt32:
				return CreateJsObjectList(*static_cast<plg::vector<int32_t>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayInt64:
				return CreateJsObjectList(*static_cast<plg::vector<int64_t>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayUInt8:
				return CreateJsObjectList(*static_cast<plg::vector<uint8_t>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayUInt16:
				return CreateJsObjectList(*static_cast<plg::vector<uint16_t>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayUInt32:
				return CreateJsObjectList(*static_cast<plg::vector<uint32_t>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayUInt64:
				return CreateJsObjectList(*static_cast<plg::vector<uint64_t>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayPointer:
				return CreateJsObjectList(*static_cast<plg::vector<void*>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayFloat:
				return CreateJsObjectList(*static_cast<plg::vector<float>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayDouble:
				return CreateJsObjectList(*static_cast<plg::vector<double>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayString:
				return CreateJsObjectList(*static_cast<plg::vector<plg::string>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayAny:
				return CreateJsObjectList(*static_cast<plg::vector<plg::any>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayVector2:
				return CreateJsObjectList(*static_cast<plg::vector<plg::vec2>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayVector3:
				return CreateJsObjectList(*static_cast<plg::vector<plg::vec3>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayVector4:
				return CreateJsObjectList(*static_cast<plg::vector<plg::vec4>*>(std::get<0>(a.storage[index])));
			case ValueType::ArrayMatrix4x4:
				return CreateJsObjectList(*static_cast<plg::vector<plg::mat4x4>*>(std::get<0>(a.storage[index])));
			case ValueType::Vector2:
				return CreateJsObject(*static_cast<plg::vec2*>(std::get<0>(a.storage[index])));
			case ValueType::Vector3:
				return CreateJsObject(*static_cast<plg::vec3*>(std::get<0>(a.storage[index])));
			case ValueType::Vector4:
				return CreateJsObject(*static_cast<plg::vec4*>(std::get<0>(a.storage[index])));
			case ValueType::Matrix4x4:
				return CreateJsObject(*static_cast<plg::mat4x4*>(std::get<0>(a.storage[index])));
			default:
				ThrowTypeError(std::format("StorageValueToObject unsupported type {:#x}", static_cast<uint8_t>(paramType.GetType())));
				return {};
		}
	}

	void V8LanguageModule::ExternalCall(MethodHandle method, MemAddr data, const JitCallback::Parameters* p, size_t count, const JitCallback::Return* ret) {
		// void (MethodJsCall*)(const v8::FunctionCallbackInfo<v8::Value>& args)
		const auto& args = *p->GetArgument<const v8::FunctionCallbackInfo<v8::Value>*>(0);

		const auto paramTypes = method.GetParamTypes();
		const size_t paramCount = paramTypes.size();
		const size_t size = static_cast<size_t>(args.Length());
		if (size != paramCount) {
			ThrowException(std::format("Wrong number of parameters, {} when {} required.", size, paramCount));
			return;
		}

		v8::Isolate* isolate = args.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();

		const auto retType = method.GetReturnType();
		const bool hasHiddenParam = ValueUtils::IsHiddenParam(retType.GetType());
		int refParamsCount = 0;

		ArgsScope a(hasHiddenParam + paramCount);
		JitCall::Return r;

		if (hasHiddenParam) {
			BeginExternalCall(retType.GetType(), a);
		}

		for (size_t i = 0; i < paramCount; ++i) {
			const PropertyHandle paramType = paramTypes[i];
			if (paramType.IsReference()) {
				++refParamsCount;
			}
			using PushParamFunc = decltype(&V8LanguageModule::PushObjectAsParam);
			PushParamFunc const pushParamFunc = paramType.IsReference() ? &V8LanguageModule::PushObjectAsRefParam : &V8LanguageModule::PushObjectAsParam;
			const bool pushResult = (this->*pushParamFunc)(paramType, args[static_cast<int>(i)], a);
			if (!pushResult) {
				// pushParamFunc sets error
				return;
			}
		}

		v8::Local<v8::Value> result = MakeExternalCallWithObject(retType, data.RCast<JitCall::CallingFunc>(), a, r);
		if (result.IsEmpty()) {
			// makeExternalCallFunc sets error
			return;
		}

		if (refParamsCount > 0) {
			v8::Local<v8::Array> resultArray = v8::Array::New(isolate, 1 + refParamsCount);
			
			int k = 0;
			
			resultArray->Set(context, k++, result).Check(); // retObj ref taken by array

			for (size_t i = 0, j = hasHiddenParam; i < paramCount; ++i) {
				const PropertyHandle paramType = paramTypes[i];
				if (!paramType.IsReference()) {
					continue;
				}

				v8::Local<v8::Value> ref = StorageValueToObject(paramType, a, j++);
				if (ref.IsEmpty()) {
					// storeValueFunc sets error
					return;
				}

				resultArray->Set(context, k++, ref).Check();
				if (k >= refParamsCount + 1) {
					break;
				}
			}

			result = resultArray;
		}

		args.GetReturnValue().Set(result);
	}

#pragma endregion ExternalCall

	InitResult V8LanguageModule::Initialize(std::weak_ptr<IPlugifyProvider> provider, ModuleHandle module) {
		if (!((_provider = provider.lock()))) {
			return ErrorData{ "Provider not exposed" };
		}

		_jitRuntime = std::make_shared<asmjit::JitRuntime>();

		std::error_code ec;
		const fs::path moduleBasePath = fs::absolute(module.GetBaseDir(), ec);
		if (ec) {
			return ErrorData{ "Failed to get module directory path" };
		}

		const fs::path libPath = moduleBasePath / "lib";
		if (!fs::exists(libPath, ec) || !fs::is_directory(libPath, ec)) {
			return ErrorData{ "lib directory not exists" };
		}

		const fs::path pluginsPath = fs::weakly_canonical(moduleBasePath / ".." / ".." / "plugins", ec);
		if (ec) {
			return ErrorData{ "Failed to get plugins directory path" };
		}

		if (!builtin::fetch::Initialize()) {
			return ErrorData{ "Failed to initialize fetch" };
		}

#ifndef NDEBUG
		// Enables calling RequestGarbageCollectionForTesting to catch memory leaks
		// at shutdown.
		v8::V8::SetFlagsFromString("--expose_gc");
#endif
#ifndef V8LM_EXTERNAL
		const fs::path icuDataPath = libPath / "icudtl.dat";
		platform = v8::platform::NewDefaultPlatform();
		if (!v8::V8::InitializeICUDefaultLocation(nullptr, fs::exists(icuDataPath, ec) ? icuDataPath.string().c_str() : nullptr)) {
			return ErrorData{ "Failed to initialize the ICU library bundled with V8" };
		}
		v8::V8::InitializeExternalStartupData(libPath.string().c_str());
		v8::V8::InitializePlatform(platform.get());
		if (!v8::V8::Initialize()) {
			return ErrorData{ "Failed to initialize v8" };
		}
#endif
		_allocator = std::unique_ptr<v8::ArrayBuffer::Allocator>(v8::ArrayBuffer::Allocator::NewDefaultAllocator());

		v8::Isolate::CreateParams params;
		params.array_buffer_allocator = _allocator.get();

		_isolate = v8::Isolate::New(params);
		//ASSERT(_isolate->GetNumberOfDataSlots() == 4);
		_isolate->SetData(0, this);
		_isolate->SetCaptureStackTraceForUncaughtExceptions(true);
		_isolate->SetHostImportModuleDynamicallyCallback(ImportDynamic);
		//_isolate->SetHostInitializeImportMetaObjectCallback(ImportMeta);

		_moduleLoader = std::make_unique<ModuleLoader>(libPath, pluginsPath);

		v8::Locker locker(_isolate);
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		v8::Local<v8::Context> context = v8::Context::New(_isolate);
		_context.Reset(_isolate, context);
		v8::Context::Scope contextScope(context);

		v8::Local<v8::Object> global = context->Global();

		// modules

		v8::Local<v8::Object> modules = v8::Object::New(_isolate);
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "assert"), builtin::assert::Init(_isolate)).Check();
		//modules->Set(v8::String::NewFromUtf8Literal(_isolate, "buffer"), builtin::buffer::Init(_isolate));
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "console"), builtin::console::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "crypto"), builtin::crypto::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "datagram"), builtin::datagram::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "dns"), builtin::dns::Init(_isolate)).Check();
		//modules->Set(v8::String::NewFromUtf8Literal(_isolate, "events"), builtin::events::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "fs"), builtin::fs::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "http"), builtin::http::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "https"), builtin::https::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "net"), builtin::net::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "os"), builtin::os::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "path"), builtin::path::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "process"), builtin::process::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "querystring"), builtin::path::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "readline"), builtin::readline::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "repl"), builtin::repl::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "tls"), builtin::tls::Init(_isolate)).Check();
		modules->Set(context, v8::String::NewFromUtf8Literal(_isolate, "url"), builtin::url::Init(_isolate)).Check();
		//modules->Set(v8::String::NewFromUtf8Literal(_isolate, "util"), builtin::util::Init(_isolate)).Check();
		global->Set(context, v8::String::NewFromUtf8Literal(_isolate, "modules"), modules).Check();

		// global

		global->Set(context, v8::String::NewFromUtf8Literal(_isolate, "console"),
					modules->Get(context, v8::String::NewFromUtf8Literal(_isolate, "console")).ToLocalChecked()).Check();

		global->Set(context, v8::String::NewFromUtf8Literal(_isolate, "fetch"),
					v8::FunctionTemplate::New(_isolate, builtin::fetch::Fetch)->GetFunction(context).ToLocalChecked()).Check();

		global->Set(context, v8::String::NewFromUtf8Literal(_isolate, "setTimeout"),
					v8::FunctionTemplate::New(_isolate, builtin::timers::SetTimeout)->GetFunction(context).ToLocalChecked()).Check();

		global->Set(context, v8::String::NewFromUtf8Literal(_isolate, "clearTimeout"),
					v8::FunctionTemplate::New(_isolate, builtin::timers::ClearTimeout)->GetFunction(context).ToLocalChecked()).Check();

		global->Set(context, v8::String::NewFromUtf8Literal(_isolate, "setInterval"),
					v8::FunctionTemplate::New(_isolate, builtin::timers::SetInterval)->GetFunction(context).ToLocalChecked()).Check();

		global->Set(context, v8::String::NewFromUtf8Literal(_isolate, "clearInterval"),
					v8::FunctionTemplate::New(_isolate, builtin::timers::ClearInterval)->GetFunction(context).ToLocalChecked()).Check();

		v8::Local<v8::Object> pps = v8::Object::New(_isolate);
		global->Set(context, v8::String::NewFromUtf8Literal(_isolate, "pps"), pps).Check();

		v8::Local<v8::External> self = v8::External::New(_isolate, this);
		MethodBindingHelper<&V8LanguageModule::EvalScript>::Bind(_isolate, context, global, v8::String::NewFromUtf8Literal(_isolate, "__plgjsEvalScript"), self);
		MethodBindingHelper<&V8LanguageModule::SearchModule>::Bind(_isolate, context, global, v8::String::NewFromUtf8Literal(_isolate, "__plgjsSearchModule"), self);
		MethodBindingHelper<&V8LanguageModule::LoadModule>::Bind(_isolate, context, global, v8::String::NewFromUtf8Literal(_isolate, "__plgjsLoadModule"), self);
		MethodBindingHelper<&V8LanguageModule::FindModule>::Bind(_isolate, context, global, v8::String::NewFromUtf8Literal(_isolate, "__plgjsFindModule"), self);

		ExecuteModule(context, libPath, "pps.js");

		_require.Reset(_isolate, pps->Get(context, v8::String::NewFromUtf8Literal(_isolate, "__require")).ToLocalChecked().As<v8::Function>());
		_getESMMain.Reset(_isolate, pps->Get(context, v8::String::NewFromUtf8Literal(_isolate, "getESMMain")).ToLocalChecked().As<v8::Function>());

		ExecuteModuleResult importResult = ExecuteModule(context, libPath, "plugify.mjs");
		if (auto* data = std::get_if<ExecuteModuleError>(&importResult)) {
			return ErrorData{ *data };
		}

		v8::Local<v8::Module> plugifyModule = std::get<ExecuteModuleData>(importResult);
		if (plugifyModule->GetStatus() == v8::Module::kErrored) {
			return ErrorData{ "Could not execute plugify.mjs module" };
		}
		v8::Local<v8::Object> exports = plugifyModule->GetModuleNamespace().As<v8::Object>();

		v8::Local<v8::Value> value;
		if (exports->Get(context, v8::String::NewFromUtf8Literal(_isolate, "Plugin")).ToLocal(&value) && value->IsFunction()) {
			_pluginClassObject = v8::Global<v8::Function>(_isolate, value.As<v8::Function>());
		} else {
			return ErrorData{"Failed to find plugify.Plugin type"};
		}
		if (exports->Get(context, v8::String::NewFromUtf8Literal(_isolate, "Vector2")).ToLocal(&value) && value->IsFunction()) {
			_vector2ClassObject = v8::Global<v8::Function>(_isolate, value.As<v8::Function>());
		} else {
			return ErrorData{"Failed to find plugify.Vector2 type"};
		}
		if (exports->Get(context, v8::String::NewFromUtf8Literal(_isolate, "Vector3")).ToLocal(&value) && value->IsFunction()) {
			_vector3ClassObject = v8::Global<v8::Function>(_isolate, value.As<v8::Function>());
		} else {
			return ErrorData{"Failed to find plugify.Vector3 type"};
		}
		if (exports->Get(context, v8::String::NewFromUtf8Literal(_isolate, "Vector4")).ToLocal(&value) && value->IsFunction()) {
			_vector4ClassObject = v8::Global<v8::Function>(_isolate, value.As<v8::Function>());
		} else {
			return ErrorData{"Failed to find plugify.Vector4 type"};
		}
		if (exports->Get(context, v8::String::NewFromUtf8Literal(_isolate, "Matrix4x4")).ToLocal(&value) && value->IsFunction()) {
			_matrix4x4ClassObject = v8::Global<v8::Function>(_isolate, value.As<v8::Function>());
		} else {
			return ErrorData{"Failed to find plugify.Matrix4x4 type"};
		}

		return InitResultData{{ .hasUpdate = true }};
	}

	void V8LanguageModule::Shutdown() {
		_taskScheduler.Reset();

		_pluginClassObject.Reset();
		_vector2ClassObject.Reset();
		_vector3ClassObject.Reset();
		_vector4ClassObject.Reset();
		_matrix4x4ClassObject.Reset();

		_require.Reset();
		_getESMMain.Reset();

		_moduleLoader.reset();
		_pathToModule.clear();
		_idToModuleInfo.clear();
		_dynamicImports.clear();
		_failedPromises.clear();

		_externalMap.clear();
		_internalFunctions.clear();
		_externalFunctions.clear();
		_moduleFunctions.clear();
		_jsMethods.clear();
		_pluginsMap.clear();
		_jitRuntime.reset();
		_provider.reset();

		_isolate->SetData(0, nullptr);
#ifndef NDEBUG
		_isolate->RequestGarbageCollectionForTesting(v8::Isolate::kFullGarbageCollection);
#endif
		_context.Reset();
		_isolate->Dispose();
		_allocator.reset();
#ifndef V8LM_EXTERNAL
		v8::V8::Dispose();
		v8::V8::DisposePlatform();
		platform.reset();
#endif
		builtin::fetch::Terminate();
	}

	void V8LanguageModule::OnUpdate(plugify::DateTime dt) {
		v8::Locker locker(_isolate);
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		v8::Local<v8::Context> context = _context.Get(_isolate);
		v8::Context::Scope contextScope(context);

		v8::TryCatch tryCatch(_isolate);
		_taskScheduler.Run();
		ASSERT(!tryCatch.HasCaught());

		_isolate->PerformMicrotaskCheckpoint();

		HandleUncaughtExceptionsInPromises();
	}

	LoadResult V8LanguageModule::OnPluginLoad(PluginHandle plugin) {
		const std::string_view entryPoint = plugin.GetDescriptor().GetEntryPoint();
		if (entryPoint.empty()) {
			return ErrorData{ "Incorrect entry point: empty" };
		}
		if (entryPoint.find_first_of("/\\") != std::string::npos) {
			return ErrorData{ "Incorrect entry point: contains '/' or '\\'" };
		}
		const std::string::size_type lastDotPos = entryPoint.find_last_of('.');
		if (lastDotPos == std::string::npos) {
			return ErrorData{ "Incorrect entry point: not have any dot '.' character" };
		}
		std::string_view pluginClassName(entryPoint.begin() + static_cast<ptrdiff_t>(lastDotPos + 1), entryPoint.end());
		if (pluginClassName.empty()) {
			return ErrorData{ "Incorrect entry point: empty class name part" };
		}
		std::string_view modulePathRel(entryPoint.begin(), entryPoint.begin() + static_cast<ptrdiff_t>(lastDotPos));
		if (modulePathRel.empty()) {
			return ErrorData{ "Incorrect entry point: empty module path part" };
		}

		const fs::path baseFolder(plugin.GetBaseDir());
		std::string modulePath(modulePathRel);

		ReplaceAll(modulePath, ".", { static_cast<char>(fs::path::preferred_separator) });
		fs::path filePathRelative = modulePath;
		filePathRelative.replace_extension(".mjs");
		const fs::path filePath = baseFolder / filePathRelative;
		std::error_code ec;
		if (!fs::exists(filePath, ec) || !fs::is_regular_file(filePath, ec)) {
			return ErrorData{ std::format("Module file '{}' not exist", filePath.string()) };
		}

		v8::Locker locker(_isolate);
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		v8::Local<v8::Context> context = _context.Get(_isolate);
		v8::Context::Scope contextScope(context);

		ExecuteModuleResult importResult = ExecuteModule(context, baseFolder, filePathRelative.string());
		if (auto* data = std::get_if<ExecuteModuleError>(&importResult)) {
			return ErrorData{ *data };
		}

		v8::Local<v8::Module> pluginModule = std::get<ExecuteModuleData>(importResult);
		if (pluginModule.IsEmpty()) {
			return ErrorData{ std::format("Failed to import '{}' module", filePathRelative.string()) };
		}

		v8::Local<v8::Object> exports = pluginModule->GetModuleNamespace().As<v8::Object>();

		v8::Local<v8::Value> pluginClass;
		if (!exports->Get(context, MakeString(pluginClassName)).ToLocal(&pluginClass)) {
			return ErrorData{ "Failed to find plugin class" };
		}

		if (!pluginClass->IsFunction()) {
			return ErrorData{ std::format("'{}' is not a function", pluginClassName) };
		}

		v8::Local<v8::Function> pluginCtor = pluginClass.As<v8::Function>();

		if (!IsSubclassOf(context, pluginCtor, _pluginClassObject.Get(_isolate))) {
			return ErrorData{ std::format("Class '{}' not subclass of Plugin", pluginClassName) };
		}

		PluginDescriptorHandle desc = plugin.GetDescriptor();
		std::span<const PluginReferenceDescriptorHandle> dependencies = desc.GetDependencies();

		plg::vector<std::string_view> deps;
		deps.reserve(dependencies.size());
		for (const auto& dependency : dependencies) {
			deps.emplace_back(dependency.GetName());
		}

		std::array args = {
				CreateJsObject(static_cast<int64_t>(plugin.GetId())),
				CreateJsObject(plugin.GetName()),
				CreateJsObject(plugin.GetFriendlyName()),
				CreateJsObject(desc.GetDescription()),
				CreateJsObject(desc.GetVersionName()),
				CreateJsObject(desc.GetCreatedBy()),
				CreateJsObject(desc.GetCreatedByURL()),
				CreateJsObject(plugin.GetBaseDir()),
				CreateJsObject(plugin.GetConfigsDir()),
				CreateJsObject(plugin.GetDataDir()),
				CreateJsObject(plugin.GetLogsDir()),
				CreateJsObjectList(deps)
		};
		v8::Local<v8::Object> pluginInstance;
		if (!pluginCtor->NewInstance(context, static_cast<int>(args.size()), args.data()).ToLocal(&pluginInstance)) {
			return ErrorData{ "Failed to create plugin instance" };
		}

		const auto exportedMethods = plugin.GetDescriptor().GetExportedMethods();
		std::vector<std::string> exportErrors;
		std::vector<std::pair<MethodHandle, JsMethodData>> methodsHolders;

		for (const MethodHandle method : exportedMethods) {
			JsFunction func;

			std::string_view className, methodName;
			{
				std::string_view funcName = method.GetFunctionName();
				if (const auto pos = funcName.find('.'); pos != std::string::npos) {
					className = funcName.substr(0, pos);
					methodName = funcName.substr(pos + 1);
				} else {
					methodName = funcName;
				}
			}

			const bool funcIsMethod = !className.empty();

			if (funcIsMethod) {
				v8::Local<v8::Value> classObject;
				if (exports->Get(context, MakeString(className)).ToLocal(&classObject) && classObject->IsFunction()) {
					v8::Local<v8::Value> methodObject;
					if (classObject.As<v8::Function>()->Get(context, MakeString(methodName)).ToLocal(&methodObject)) {
						if (methodObject->IsFunction()) {
							func = std::make_shared<v8::Global<v8::Function>>(_isolate, methodObject.As<v8::Function>());
						} else {
							exportErrors.emplace_back(std::format("{} ('{}' not function type)", method.GetName(), method.GetFunctionName()));
							continue;
						}
					}
				}
			}
			else {
				v8::Local<v8::Value> value;
				if (exports->Get(context, MakeString(methodName)).ToLocal(&value)) {
					if (value->IsFunction()) {
						func = std::make_shared<v8::Global<v8::Function>>(_isolate, value.As<v8::Function>());
					} else {
						exportErrors.emplace_back(std::format("{} ('{}' not function type)", method.GetName(), method.GetFunctionName()));
						continue;
					}
				}
			}

			if (!func) {
				exportErrors.emplace_back(std::format("{} (not found '{}' in module)", method.GetName(), method.GetFunctionName()));
				continue;
			}

			JitCallback callback(_jitRuntime);
			const MemAddr methodAddr = callback.GetJitFunc(method, &detail::InternalCall, func.get());
			if (!methodAddr) {
				exportErrors.emplace_back(std::format("{} (jit error: {})", method.GetName(), callback.GetError()));
				continue;
			}

			methodsHolders.emplace_back(method, JsMethodData{std::move(callback), std::move(func)});
		}

		if (!exportErrors.empty()) {
			std::string errorString = "Methods export error(s): " + exportErrors[0];
			for (auto it = std::next(exportErrors.begin()); it != exportErrors.end(); ++it) {
				std::format_to(std::back_inserter(errorString), ", {}", *it);
			}
			return ErrorData{ std::move(errorString) };
		}

		v8::Local<v8::Value> pluginStart;
		if (pluginInstance->Get(context, v8::String::NewFromUtf8Literal(_isolate, "pluginStart")).ToLocal(&pluginStart)) {
			if (!pluginStart->IsFunction()) {
				pluginStart.Clear();
			}
		}

		v8::Local<v8::Value> pluginUpdate;
		if (pluginInstance->Get(context, v8::String::NewFromUtf8Literal(_isolate, "pluginUpdate")).ToLocal(&pluginUpdate)) {
			if (!pluginUpdate->IsFunction()) {
				pluginUpdate.Clear();
			}
		}

		v8::Local<v8::Value> pluginEnd;
		if (pluginInstance->Get(context, v8::String::NewFromUtf8Literal(_isolate, "pluginEnd")).ToLocal(&pluginEnd)) {
			if (!pluginEnd->IsFunction()) {
				pluginEnd.Clear();
			}
		}

		const auto [it, result] = _pluginsMap.try_emplace(
				plugin.GetId(),
				v8::Global<v8::Module>(_isolate, pluginModule),
				v8::Global<v8::Object>(_isolate, pluginInstance),
				v8::Global<v8::Function>(_isolate, pluginUpdate.As<v8::Function>()),
				v8::Global<v8::Function>(_isolate, pluginStart.As<v8::Function>()),
				v8::Global<v8::Function>(_isolate, pluginEnd.As<v8::Function>()));
		if (!result) {
			return ErrorData{ std::format("Save plugin data to map unsuccessful") };
		}

		std::vector<MethodData> methods;
		methods.reserve(methodsHolders.size());
		_jsMethods.reserve(methodsHolders.size());

		for (auto& [method, methodData] : methodsHolders) {
			const MemAddr methodAddr = methodData.jitCallback.GetFunction();
			methods.emplace_back(method, methodAddr);
			AddToFunctionsMap(methodAddr, methodData.jsFunction);
			_jsMethods.emplace_back(std::move(methodData));
		}

		return LoadResultData{ std::move(methods), &it->second, { !pluginUpdate.IsEmpty(), !pluginStart.IsEmpty(), !pluginEnd.IsEmpty(), !exportedMethods.empty() }};
	}

	void V8LanguageModule::OnPluginStart(PluginHandle plugin) {
		const auto& [module, instance, update, start, end] = *plugin.GetData().RCast<PluginData*>();

		v8::Locker locker(_isolate);
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		v8::Local<v8::Context> context = _context.Get(_isolate);
		v8::Context::Scope contextScope(context);

		v8::Local<v8::Object> object = instance.Get(_isolate);
		v8::Local<v8::Function> function = start.Get(_isolate);

		v8::TryCatch tryCatch(_isolate);
		UNUSED(function->Call(context, object, 0, nullptr));

		if (tryCatch.HasCaught()) {
			ReportException(tryCatch.Message());
			_provider->Log(std::format(LOG_PREFIX "{}: call of 'pluginStart' failed", plugin.GetName()), Severity::Error);
		}
	}


	void V8LanguageModule::OnPluginUpdate(plugify::PluginHandle plugin, plugify::DateTime dt) {
		const auto& [module, instance, update, start, end] = *plugin.GetData().RCast<PluginData*>();

		v8::Locker locker(_isolate);
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		v8::Local<v8::Context> context = _context.Get(_isolate);
		v8::Context::Scope contextScope(context);

		v8::Local<v8::Object> object = instance.Get(_isolate);
		v8::Local<v8::Function> function = end.Get(_isolate);

		std::array args = { CreateJsObject(dt.AsSeconds()) };

		v8::TryCatch tryCatch(_isolate);
		UNUSED(function->Call(context, object, static_cast<int>(args.size()), args.data()));

		if (tryCatch.HasCaught()) {
			ReportException(tryCatch.Message());
			_provider->Log(std::format(LOG_PREFIX "{}: call of 'pluginUpdate' failed", plugin.GetName()), Severity::Error);
		}
	}

	void V8LanguageModule::OnPluginEnd(PluginHandle plugin) {
		const auto& [module, instance, update, start, end] = *plugin.GetData().RCast<PluginData*>();

		v8::Locker locker(_isolate);
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		v8::Local<v8::Context> context = _context.Get(_isolate);
		v8::Context::Scope contextScope(context);

		v8::Local<v8::Object> object = instance.Get(_isolate);
		v8::Local<v8::Function> function = end.Get(_isolate);

		v8::TryCatch tryCatch(_isolate);
		UNUSED(function->Call(context, object, 0, nullptr));

		if (tryCatch.HasCaught()) {
			ReportException(tryCatch.Message());
			_provider->Log(std::format(LOG_PREFIX "{}: call of 'pluginEnd' failed", plugin.GetName()), Severity::Error);
		}
	}

	JsFunction V8LanguageModule::FindExternal(void* funcAddr) const {
		const auto it = _externalMap.find(funcAddr);
		if (it != _externalMap.end()) {
			return std::get<JsFunction>(*it);
		}
		return nullptr;
	}

	void* V8LanguageModule::FindInternal(v8::Local<v8::Function> object) const {
		v8::Local<v8::Value> address;
		if (object->Get(_isolate->GetCurrentContext(), v8::String::NewFromUtf8Literal(_isolate, "__address")).ToLocal(&address) && address->IsExternal()) {
			return address.As<v8::External>()->Value();
		}
		return nullptr;
	}

	namespace {
		template<typename T>
		struct JsExportData {
			v8::Local<v8::String> name;
			v8::Local<T> object;
		};

		std::vector<v8::Local<v8::String>> exportNames;
		std::vector<JsExportData<v8::Function>> exportFuncs;
		std::unordered_map<std::string, JsExportData<v8::Object>, string_hash, std::equal_to<>> exportEnums;
	}

	void V8LanguageModule::GenerateEnum(plugify::PropertyHandle paramType) {
		if (const auto prototype = paramType.GetPrototype()) {
			GenerateEnum(prototype);
		}

		const auto enumerator = paramType.GetEnum();
		if (!enumerator) {
			return;
		}

		const auto name = enumerator.GetName();
		const auto values = enumerator.GetValues();
		if (exportEnums.contains(name) || values.empty()) {
			return;
		}

		v8::Local<v8::Object> enumObject = v8::Object::New(_isolate);

		v8::Local<v8::Context> context = _isolate->GetCurrentContext();
		for (const auto& value : values) {
			enumObject->Set(context, MakeString(value.GetName()),
							v8::BigInt::New(_isolate, value.GetValue())).Check();
		}

		v8::Local<v8::String> enumName = MakeString(name);
		exportNames.emplace_back(enumName);
		exportEnums.emplace(name, JsExportData<v8::Object>{enumName, enumObject});
	}

	void V8LanguageModule::GenerateEnum(plugify::MethodHandle method) {
		GenerateEnum(method.GetReturnType());
		for (const auto& paramType : method.GetParamTypes()) {
			GenerateEnum(paramType);
		}
	}

	v8::Local<v8::Function> V8LanguageModule::FindJavascriptMethod(MemAddr addr) const {
		for (const auto& [jitCallback, jsFunction] : _jsMethods) {
			if (jitCallback.GetFunction() == addr) {
				return jsFunction->Get(_isolate);
			}
		}
		return {};
	}

	v8::MaybeLocal<v8::Module> V8LanguageModule::CreateInternalModule(PluginHandle plugin) {
		if (!_pluginsMap.contains(plugin.GetId())) {
			return {};
		}

		const auto& methods = plugin.GetMethods();

		exportNames.clear();
		exportFuncs.clear();
		exportEnums.clear();

		for (const auto& [method, addr] : methods) {
			v8::Local<v8::Function> func = FindJavascriptMethod(addr);
			if (func.IsEmpty()) {
				_provider->Log(std::format(LOG_PREFIX "Not found '{}' method while CreateInternalModule for '{}' plugin", method.GetName(), plugin.GetName()), Severity::Fatal);
				std::terminate();
			}
			v8::Local<v8::String> name = MakeString(method.GetName());
			exportNames.emplace_back(name);
			exportFuncs.emplace_back(name, func);
			GenerateEnum(method);
		}

		v8::Local<v8::Module> moduleObject = v8::Module::CreateSyntheticModule(
				_isolate,
				MakeString(plugin.GetName()),
				v8::MemorySpan<const v8::Local<v8::String>>{ exportNames.begin(), exportNames.end() },
				[](v8::Local<v8::Context> context, v8::Local<v8::Module> module) -> v8::MaybeLocal<v8::Value> {
					auto _isolate = context->GetIsolate();
					for (const auto& [name, func] : exportFuncs) {
						UNUSED(module->SetSyntheticModuleExport(_isolate, name, func));
					}
					for (const auto& [name, enumerator] : exportEnums) {
						UNUSED(module->SetSyntheticModuleExport(_isolate, enumerator.name, enumerator.object));
					}
					return { True(_isolate) };
				}
		);

		return moduleObject;
	}

	v8::MaybeLocal<v8::Module> V8LanguageModule::CreateExternalModule(PluginHandle plugin) {
		const auto& methods = plugin.GetMethods();

		exportNames.clear();
		exportFuncs.clear();
		exportEnums.clear();

		for (const auto& [method, addr] : methods) {
			JitCall call(_jitRuntime);

			const MemAddr callAddr = call.GetJitFunc(method, addr);
			if (!callAddr) {
				ThrowException(std::format("Lang module JIT failed to generate c++ call wrapper '{}'", call.GetError()));
				return {};
			}

			JitCallback callback(_jitRuntime);

			asmjit::FuncSignature sig(asmjit::CallConvId::kCDecl);
			sig.addArg(asmjit::TypeId::kUIntPtr);
			sig.setRet(asmjit::TypeId::kVoid);

			// Generate function --> void (MethodJsCall*)(const v8::FunctionCallbackInfo<v8::Value>& info)
			const MemAddr methodAddr = callback.GetJitFunc(sig, method, &detail::ExternalCall, callAddr, false);
			if (!methodAddr)
				break;

			_moduleFunctions.emplace_back(std::move(callback), std::move(call));

			v8::Local<v8::Function> func;
			if (!v8::Function::New(_isolate->GetCurrentContext(), methodAddr.RCast<v8::FunctionCallback>()).ToLocal(&func)) {
				ThrowException("Fail to create function object from function pointer");
				return {};
			}

			v8::Local<v8::String> name = MakeString(method.GetName());
			exportNames.emplace_back(name);
			exportFuncs.emplace_back(name, func);
			GenerateEnum(method);
		}

		v8::Local<v8::Module> moduleObject = v8::Module::CreateSyntheticModule(
				_isolate,
				MakeString(plugin.GetName()),
				v8::MemorySpan<const v8::Local<v8::String>>{ exportNames.begin(), exportNames.end() },
				[](v8::Local<v8::Context> context, v8::Local<v8::Module> module) -> v8::MaybeLocal<v8::Value> {
					auto _isolate = context->GetIsolate();
					for (const auto& [name, func] : exportFuncs) {
						UNUSED(module->SetSyntheticModuleExport(_isolate, name, func));
					}
					for (const auto& [name, enumerator] : exportEnums) {
						UNUSED(module->SetSyntheticModuleExport(_isolate, enumerator.name, enumerator.object));
					}
					return { True(_isolate) };
				}
		);

		return moduleObject;
	}

	// we uses synthetic instead
	void V8LanguageModule::OnMethodExport(PluginHandle plugin) {
	}

	void V8LanguageModule::AddToFunctionsMap(void* funcAddr, const JsFunction& funcObj) {
		_externalMap.emplace(funcAddr, funcObj);

		v8::Local<v8::External> address = v8::External::New(_isolate, funcAddr);
		funcObj->Get(_isolate)->Set(_isolate->GetCurrentContext(), v8::String::NewFromUtf8Literal(_isolate, "__address"), address).Check();
	}

	bool V8LanguageModule::IsDebugBuild() {
		return V8LM_IS_DEBUG;
	}

	extern "C" V8LM_EXPORT ILanguageModule* GetLanguageModule() {
		return &g_v8lm;
	}

	ExecuteModuleResult V8LanguageModule::ExecuteModule(v8::Local<v8::Context> context, const fs::path& requiringDir, const std::string& moduleName) {
		fs::path path;
		std::string content;
		if (!LoadFile(requiringDir, moduleName, path, content)) {
			return content;
		}

		if (path.extension() == ".mjs") {
			v8::MaybeLocal<v8::Module> module = LoadModule(context, path, {});
			if (module.IsEmpty()) {
				return std::format("Can not instantiate '{}'", moduleName);
			}
			return module.ToLocalChecked();
		} else {
			v8::Local<v8::String> resourceName = MakeString(path.c_str());
			v8::ScriptOrigin origin(_isolate, resourceName);
			v8::Local<v8::String> source = MakeString(content);
			v8::TryCatch tryCatch(_isolate);

			v8::MaybeLocal<v8::Script> compiledScript = v8::Script::Compile(context, source, &origin);
			if (compiledScript.IsEmpty()) {
				ReportException(tryCatch.Message());
				return std::format("Can not compiled '{}'", moduleName);
			}

			v8::Local<v8::Script> script = compiledScript.ToLocalChecked();
			[[maybe_unused]] v8::MaybeLocal<v8::Value> returnVal = script->Run(context);
			if (tryCatch.HasCaught()) {
				ReportException(tryCatch.Message());
				return std::format("Can not execute '{}'", moduleName);
			}

			return script;
		}
	}

	// LoadModule is used in two flows:
	// 1. Loading the main module
	// 2. Loading a dynamically imported module.
	//
	// The "resolver" is set for the second case.
	//
	// LoadModule returns empty module if the module couldn't be located,
	// instantiated, etc. In those cases, an exception has been thrown.
	//
	// Otherwise, the module is either fully loaded and ready, or has a top-level
	// await and is still pending. In those case, a resolver or reject handler
	// will handle cases 1 and 2 later.
	v8::MaybeLocal<v8::Module> V8LanguageModule::LoadModule(v8::Local<v8::Context> context, const fs::path& path, v8::Local<v8::Promise::Resolver> resolver) {
		v8::TryCatch tryCatch(_isolate);
		v8::Local<v8::Module> module;

		if (!FetchESModuleTree(context, path).ToLocal(&module)) {
			ASSERT(tryCatch.HasCaught());
			ReportException(tryCatch.Message());
			return {};
		}

		if (module->InstantiateModule(context, ResolveModule).FromMaybe(false)) {
			v8::Local<v8::Value> result;
			if (module->Evaluate(context).ToLocal(&result)) {
				v8::Local<v8::Promise> promise = result.As<v8::Promise>();

				if (resolver.IsEmpty()) {
					// Loading the main module.
					if (promise->State() == v8::Promise::kPending) {
						UNUSED(promise->Then(
								context,
								v8::Function::New(context, OnMainModuleResolve).ToLocalChecked(),
								v8::Function::New(context, OnMainModuleFailure).ToLocalChecked()));
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
			}
		}

		if (tryCatch.HasCaught()) {
			ReportException(tryCatch.Message());
			return {};
		}

		return module;
	}

	void V8LanguageModule::EvalScript(const v8::FunctionCallbackInfo<v8::Value>& info) {
		v8::Isolate* isolate = info.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();

		bool isESM = info[1]->BooleanValue(isolate);
		if (isESM) {
			fs::path path = ToString(info[2]);
			v8::Local<v8::Module> module;

			if (!FetchESModuleTree(context, path).ToLocal(&module)) {
				return;
			}

			if (module->InstantiateModule(context, ResolveModule).FromMaybe(false)) {
				v8::MaybeLocal<v8::Value> maybeResult = module->Evaluate(context);
				v8::Local<v8::Value> result;
				if (maybeResult.ToLocal(&result)) {
					if (result->IsPromise()) {
						v8::Local<v8::Promise> resultPromise = result.As<v8::Promise>();
						while (resultPromise->State() == v8::Promise::kPending) {
							_isolate->PerformMicrotaskCheckpoint();
						}

						if (resultPromise->State() == v8::Promise::kRejected) {
							resultPromise->MarkAsHandled();
							_isolate->ThrowException(resultPromise->Result());
							return;
						}
					}
					info.GetReturnValue().Set(module->GetModuleNamespace());
				}
			}
			return;
		}

		v8::ScriptOrigin origin(_isolate, info[2]);
		v8::Local<v8::String> source = info[0]->ToString(context).ToLocalChecked();

		v8::MaybeLocal<v8::Script> script = v8::Script::Compile(context, source, &origin);
		if (script.IsEmpty()) {
			return;
		}
		v8::MaybeLocal<v8::Value> result = script.ToLocalChecked()->Run(context);
		if (result.IsEmpty()) {
			return;
		}
		info.GetReturnValue().Set(result.ToLocalChecked());
	}

	void V8LanguageModule::SearchModule(const v8::FunctionCallbackInfo<v8::Value>& info) {
		std::string moduleName = ToString(info[0]);
		fs::path requiringDir = ToString(info[1]);

		fs::path path;
		if (_moduleLoader->Search(requiringDir, moduleName, path)) {
			info.GetReturnValue().Set(MakeString(path.c_str()));
		}
	}

	void V8LanguageModule::LoadModule(const v8::FunctionCallbackInfo<v8::Value>& info) {
		fs::path path = ToString(info[0]);
		std::string content;
		if (!_moduleLoader->Load(path, content)) {
			ThrowException(std::format("Can not load '{}' - '{}'", path.string(), content));
			return;
		}

		info.GetReturnValue().Set(MakeString(content));
	}

	void V8LanguageModule::FindModule(const v8::FunctionCallbackInfo<v8::Value>& info) {
	}

	// static
	void V8LanguageModule::OnMainModuleResolve(const v8::FunctionCallbackInfo<v8::Value>& info) {
	}

	// static
	void V8LanguageModule::OnMainModuleFailure(const v8::FunctionCallbackInfo<v8::Value>& info) {
		v8::Isolate* isolate = info.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();

		ASSERT(info.Length() >= 1);
		auto* self = Get(isolate);
		v8::Local<v8::Value> error = info[0];
		v8::Local<v8::Message> message = MakeErrorMessage(isolate, error);
		self->ReportException(message);
	}

	// static
	void V8LanguageModule::OnDynamicModuleResolve(
			const v8::FunctionCallbackInfo<v8::Value>& info) {
		v8::Isolate* isolate = info.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();

		v8::Local<v8::Array> data =
				info.Data().As<v8::Array>();
		v8::Local<v8::Promise::Resolver> resolver =
				data->Get(context, 0).ToLocalChecked().As<v8::Promise::Resolver>();
		v8::Local<v8::Value> ns =
				data->Get(context, 1).ToLocalChecked().As<v8::Value>();
		ASSERT(resolver->Resolve(context, ns).FromMaybe(false));
	}

	// static
	void V8LanguageModule::OnDynamicModuleFailure(
			const v8::FunctionCallbackInfo<v8::Value>& info) {
		v8::Isolate* isolate = info.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();

		v8::Local<v8::Promise::Resolver> resolver =
				info.Data().As<v8::Promise::Resolver>();
		ASSERT(resolver->Reject(context, info[0]).FromMaybe(false));
	}

	// static
	v8::MaybeLocal<v8::Promise> V8LanguageModule::ImportDynamic(
			v8::Local<v8::Context> context,
			[[maybe_unused]] v8::Local<v8::Data> hostDefinedOptions,
			v8::Local<v8::Value> resourceName, v8::Local<v8::String> specifier,
			[[maybe_unused]] v8::Local<v8::FixedArray> importAssertions) {
		auto* self = Get(context->GetIsolate());
		return self->ImportDynamic(context, resourceName, specifier);
	}

	v8::MaybeLocal<v8::Promise> V8LanguageModule::ImportDynamic(
			v8::Local<v8::Context> context,
			v8::Local<v8::Value> resourceName,
			v8::Local<v8::String> specifier) {
		v8::MaybeLocal<v8::Promise::Resolver> maybeResolver =
				v8::Promise::Resolver::New(context);
		v8::Local<v8::Promise::Resolver> resolver;
		if (!maybeResolver.ToLocal(&resolver)) {
			return {};
		}

		ASSERT(resourceName->IsString());
		std::string moduleName = ToString(resourceName);
		std::string requiringDir = ToString(specifier);

		fs::path path;
		if (!_moduleLoader->Search(requiringDir, moduleName, path)) {
			return {};
		}

		auto it = _dynamicImports.find(path);
		if (it != _dynamicImports.end()) {
			// Already importing; return the existing promise.
			return it->second.Get(_isolate)->GetPromise();
		}

		_dynamicImports[path].Reset(_isolate, resolver);

		_taskScheduler.AddTask(0ms, [this, path = std::move(path)]() {
			ImportDynamic(path);
		});

		return resolver->GetPromise();
	}

	void V8LanguageModule::ImportDynamic(const fs::path& path) {
		v8::Locker locker(_isolate);
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		v8::Local<v8::Context> context = _context.Get(_isolate);
		v8::Context::Scope contextScope(context);
		v8::TryCatch tryCatch(_isolate);

		auto it = _dynamicImports.find(path);
		ASSERT(it != _dynamicImports.end());

		v8::Local<v8::Promise::Resolver> resolver = it->second.Get(_isolate);
		_dynamicImports.erase(it);

		v8::MaybeLocal<v8::Module> module = LoadModule(context, path, resolver);
		if (!module.IsEmpty()) {
			// Everything has been handled inside LoadModule.
			tryCatch.Reset();
		} else {
			// LoadModule failed to load the module.
			v8::Local<v8::Value> exception;
			if (tryCatch.HasCaught()) {
				exception = tryCatch.Exception();
				// Reset() is important to clear the pending exception state;
				// otherwise, v8 might crash. Repro: await on a dynamic import()
				// that has a syntax error in the imported module.
				tryCatch.Reset();
			} else {
				exception = v8::Exception::Error(v8::String::NewFromUtf8Literal(_isolate, "Failed to import."));
			}
			ASSERT(!exception.IsEmpty());
			ASSERT(resolver->Reject(context, exception).FromMaybe(false));
		}
	}

	void V8LanguageModule::RemovePendingFailedPromise(v8::Local<v8::Promise> promise) {
		auto it = _failedPromises.begin();
		while (it != _failedPromises.end()) {
			v8::Local<v8::Promise> failed_promise = it->first.Get(_isolate);
			if (failed_promise == promise) {
				it = _failedPromises.erase(it);
			} else {
				++it;
			}
		}
	}

	void V8LanguageModule::HandleUncaughtExceptionsInPromises() {
		if (_failedPromises.empty()) {
			return;
		}

		JsExceptionList list;
		_failedPromises.swap(list);

		for (const auto& [_, message] : list) {
			ReportException(message.Get(_isolate));
		}
	}

	v8::MaybeLocal<v8::Module> V8LanguageModule::FetchESModuleTree(v8::Local<v8::Context> context, const fs::path& path) {
		// Check for recursive loading of the same module.
		// Note that recursive imports are supported: we only load module names that
		// haven't been loaded yet (recursively) below.
		if (auto it = _pathToModule.find(path); it != _pathToModule.end()) {
			return it->second.Get(_isolate);
		}

		std::string content;
		if (!_moduleLoader->Load(path, content)) {
			ThrowException(std::format("Can not load '{}' - '{}'", path.string(), content));
			return {};
		}

		auto script = MakeString(std::format("const __filename = {};const __dirname = {}; {}", EscapeString(path.string()), EscapeString(path.parent_path().string()), content));

		auto resourceName = MakeString(path.c_str());
		constexpr int lineOffset = 0;
		constexpr int columnOffset = 0;
		constexpr bool isSharedCrossOrigin = false;
		constexpr int scriptId = -1;
		auto sourceMapUrl = v8::Local<v8::Value>();
		constexpr bool isOpaque = false;
		constexpr bool isWarm = false;
		constexpr bool isModule = true;
		auto hostDefinedOptions = v8::Local<v8::PrimitiveArray>();
		v8::ScriptOrigin origin(_isolate, resourceName, lineOffset, columnOffset,
								isSharedCrossOrigin, scriptId, sourceMapUrl,
								isOpaque, isWarm, isModule, hostDefinedOptions);
		v8::ScriptCompiler::Source source(script, origin);

		v8::Local<v8::Module> module;
		if (!v8::ScriptCompiler::CompileModule(_isolate, &source).ToLocal(&module)) {
			return {};
		}

		// At this stage, the module is compiled but not instantiated yet.
		// Look up its dependencies, so that they can be instantiated later too.
		_pathToModule.emplace(path, v8::Global<v8::Module>(_isolate, module));
		int id = module->IsSyntheticModule() ? module->GetIdentityHash() : module->ScriptId();
		ModuleInfo& info = _idToModuleInfo.emplace(id, ModuleInfo{})->second;
		info.module.Reset(_isolate, module);

		fs::path dir = path;
		dir.remove_filename();

		v8::Local<v8::FixedArray> requests = module->GetModuleRequests();
		int length = requests->Length();
		for (int i = 0; i < length; ++i) {
			v8::Local<v8::ModuleRequest> request = requests->Get(context, i).As<v8::ModuleRequest>();
			std::string refModuleName = ToString(request->GetSpecifier());

			if (refModuleName.starts_with(':')) {
				PluginHandle plugin = _provider->FindPlugin(refModuleName.substr(1));
				if (plugin) {
					v8::MaybeLocal<v8::Module> refModule = CreateInternalModule(plugin);
					if (refModule.IsEmpty()) {
						refModule = CreateExternalModule(plugin);
					}
					if (refModule.IsEmpty()) {
						return {};
					}
					info.resolveCache.emplace(refModuleName, v8::Global<v8::Module>(_isolate, refModule.ToLocalChecked()));
					continue;
				}
			}

			fs::path refPath;
			if (_moduleLoader->Search(dir, refModuleName, refPath)) {
				std::string refName = refPath.filename().string();
				if (refName.ends_with("package.json")) {
					std::string package;
					if (_moduleLoader->Load(refPath, package)) {
						std::array args = { MakeString(package).As<v8::Value>() };

						v8::MaybeLocal<v8::Value> maybeRet = _getESMMain.Get(_isolate)->Call(context, v8::Undefined(_isolate), static_cast<int>(args.size()), args.data());

						v8::Local<v8::Value> esmMainValue;
						if (maybeRet.ToLocal(&esmMainValue) && esmMainValue->IsString()) {
							std::string esmMain = ToString(esmMainValue);
							fs::path esmMainPath;
							if (_moduleLoader->Search(refPath, esmMain, esmMainPath)) {
								refPath = std::move(esmMainPath);
							}
						}
					}
				}
				else if (refName.ends_with(".mjs") || refName.ends_with(".js")) {
					v8::MaybeLocal<v8::Module> refModule = FetchESModuleTree(context, refPath);
					if (refModule.IsEmpty()) {
						return {};
					}
					info.resolveCache.emplace(refModuleName, v8::Global<v8::Module>(_isolate, refModule.ToLocalChecked()));
					continue;
				}
			}

			v8::MaybeLocal<v8::Module> refModule = FetchCJSModuleAsESModule(context, refPath.extension() == ".cjs" ? refPath.string() : refModuleName);
			if (refModule.IsEmpty()) {
				ThrowException(std::format("Can not resolve '{}', import by '{}'", refModuleName, path.string()));
				return {};
			}

			info.resolveCache.emplace(refModuleName, v8::Global<v8::Module>(_isolate, refModule.ToLocalChecked()));
		}

		return module;
	}

	v8::MaybeLocal<v8::Module> V8LanguageModule::FetchCJSModuleAsESModule(v8::Local<v8::Context> context, const std::string& moduleName) {
		std::array args = { MakeString(moduleName).As<v8::Value>() };

		v8::MaybeLocal<v8::Value> maybeRet = _require.Get(_isolate)->Call(context, v8::Undefined(_isolate), static_cast<int>(args.size()), args.data());
		if (maybeRet.IsEmpty()) {
			return {};
		}

		auto cJSValue = maybeRet.ToLocalChecked();
		std::vector<v8::Local<v8::String>> exports = { v8::String::NewFromUtf8Literal(_isolate, "default") };

		if (cJSValue->IsObject()) {
			auto jsObject = cJSValue->ToObject(context).ToLocalChecked();
			auto keys = jsObject->GetOwnPropertyNames(context).ToLocalChecked();
			exports.reserve(static_cast<size_t>(keys->Length()) + 1);
			for (decltype(keys->Length()) i = 0; i < keys->Length(); ++i) {
				v8::Local<v8::Value> key;
				if (keys->Get(context, i).ToLocal(&key)) {
					exports.emplace_back(key->ToString(context).ToLocalChecked());
				}
			}
		}

		v8::Local<v8::Module> syntheticModule = v8::Module::CreateSyntheticModule(
				_isolate,
				MakeString(moduleName),
				v8::MemorySpan<const v8::Local<v8::String>>{ exports.begin(), exports.end() },
				[](v8::Local<v8::Context> context, v8::Local<v8::Module> module) -> v8::MaybeLocal<v8::Value> {
					v8::Isolate* isolate = context->GetIsolate();
					auto* self = Get(isolate);

					const auto it = self->FindModuleInfo(module);
					ASSERT(it != self->_idToModuleInfo.end());
					auto cJSValue = it->second.cJSValue.Get(isolate);

					UNUSED(module->SetSyntheticModuleExport(isolate, v8::String::NewFromUtf8Literal(isolate, "default"), cJSValue));

					if (cJSValue->IsObject()) {
						auto jsObject = cJSValue->ToObject(context).ToLocalChecked();
						auto keys = jsObject->GetOwnPropertyNames(context).ToLocalChecked();
						for (decltype(keys->Length()) j = 0; j < keys->Length(); ++j) {
							v8::Local<v8::Value> key;
							v8::Local<v8::Value> value;
							if (keys->Get(context, j).ToLocal(&key) &&
								jsObject->Get(context, key).ToLocal(&value)) {
								UNUSED(module->SetSyntheticModuleExport(isolate, key->ToString(context).ToLocalChecked(), value));
							}
						}
					}

					return { v8::True(isolate) };
				});

		int id = syntheticModule->IsSyntheticModule() ? syntheticModule->GetIdentityHash() : syntheticModule->ScriptId();
		ModuleInfo& info = _idToModuleInfo.emplace(id, ModuleInfo{})->second;
		info.module.Reset(_isolate, syntheticModule);
		info.cJSValue.Reset(_isolate, maybeRet.ToLocalChecked());

		return syntheticModule;
	}

	std::unordered_multimap<int, V8LanguageModule::ModuleInfo>::iterator V8LanguageModule::FindModuleInfo(v8::Local<v8::Module> module) {
		int id = module->IsSyntheticModule() ? module->GetIdentityHash() : module->ScriptId();
		auto range = _idToModuleInfo.equal_range(id);
		for (auto it = range.first; it != range.second; ++it) {
			if (it->second.module == module) {
				return it;
			}
		}
		return _idToModuleInfo.end();
	}

	bool V8LanguageModule::LoadFile(const fs::path& requiringDir, const std::string& moduleName, fs::path& path, std::string& content) {
		if (_moduleLoader->Search(requiringDir, moduleName, path)) {
			if (!_moduleLoader->Load(path, content)) {
				content = std::format("Can not load '{}' - '{}'", path.string(), content);
				return false;
			}
		} else {
			content = std::format("Can not find '{}'", moduleName);
			return false;
		}
		return true;
	}

	// static
	v8::MaybeLocal<v8::Module> V8LanguageModule::ResolveModule(
			v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
			v8::Local<v8::FixedArray> importAttributes, v8::Local<v8::Module> referrer) {
		v8::Isolate* isolate = context->GetIsolate();
		auto* self = Get(isolate);
		const auto it1 = self->FindModuleInfo(referrer);
		ASSERT(it1 != self->_idToModuleInfo.end());
		const std::string refModuleName = self->ToString(specifier);
		auto it2 = it1->second.resolveCache.find(refModuleName);
		ASSERT(it2 != it1->second.resolveCache.end());
		return it2->second.Get(isolate);
	}

	void V8LanguageModule::ThrowException(std::string_view error) const {
		_isolate->ThrowException(MakeString(error));
	}

	void V8LanguageModule::ThrowRangeError(std::string_view error) const {
		_isolate->ThrowException(v8::Exception::RangeError(MakeString(error)));
	}

	void V8LanguageModule::ThrowTypeError(std::string_view error, v8::Local<v8::Value> value) const {
		v8::Local<v8::String> name = value->TypeOf(_isolate);
		ThrowTypeError(std::format("{}, but {} provided", error, ToStringOr(name, "<invalid>")));
	}

	void V8LanguageModule::ThrowTypeError(std::string_view error) const {
		_isolate->ThrowException(v8::Exception::TypeError(MakeString(error)));
	}

	void V8LanguageModule::ReportException(v8::Local<v8::Message> message) const {
		std::string trace = std::format(LOG_PREFIX "{}", ToString(message->Get()));
		v8::Local<v8::StackTrace> stackTrace = message->GetStackTrace();
		if (!stackTrace.IsEmpty()) {
			for (int i = 0; i < stackTrace->GetFrameCount(); ++i) {
				v8::Local<v8::StackFrame> frame = stackTrace->GetFrame(_isolate, i);
				std::string file = ToStringOr(frame->GetScriptName(), "<script>");
				std::string function = ToStringOr(frame->GetFunctionName(), "<top>");
				int line = frame->GetLineNumber();
				std::format_to(std::back_inserter(trace), "\n\t{} ({}:{})", function, file, line);
			}
		}
		g_v8lm._provider->Log(trace, Severity::Error);
	}

	v8::Local<v8::String> V8LanguageModule::MakeString(std::string_view value) const {
		if (value.empty()) [[unlikely]] {
			return v8::String::Empty(_isolate);
		} else {
			return v8::String::NewFromUtf8(
						   _isolate, value.data(), v8::NewStringType::kNormal,
						   static_cast<int>(value.size())).ToLocalChecked();
		}
	}

	v8::Local<v8::String> V8LanguageModule::MakeString(std::wstring_view value) const {
		if (value.empty()) [[unlikely]] {
			return v8::String::Empty(_isolate);
		} else {
			return v8::String::NewFromTwoByte(
						   _isolate, reinterpret_cast<const uint16_t*>(value.data()), v8::NewStringType::kNormal,
						   static_cast<int>(value.size())).ToLocalChecked();
		}
	}

	std::string V8LanguageModule::ToString(v8::Local<v8::Value> value) const {
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

	std::string V8LanguageModule::ToStringOr(v8::Local<v8::Value> value, std::string_view or_string) const {
		if (value.IsEmpty()) {
			return std::string(or_string);
		} else {
			return ToString(value);
		}
	}

	std::wstring V8LanguageModule::ToWString(v8::Local<v8::Value> value) const {
		ASSERT(!value.IsEmpty());
#if V8LM_PLATFORM_WINDOWS
		if (value->IsString()) {
			v8::Local<v8::String> v8s = value.As<v8::String>();
			std::wstring s;
			s.resize(v8s->Length());
			v8s->Write(_isolate, reinterpret_cast<uint16_t*>(s.data()));
			return s;
		} else if (value->IsModuleNamespaceObject()) {
			return L"[Module]";
		} else if (value->IsSymbolObject()) {
			return L"[SymbolObject]";
		} else if (value->IsSymbol()) {
			return ToWString(value.As<v8::Symbol>()->Description(_isolate));
		} else {
			v8::String::Value v(_isolate, value);
			return *v != nullptr ? std::wstring(reinterpret_cast<wchar_t*>(*v), static_cast<size_t>(v.length())) : ToWString(value->TypeOf(_isolate));
		}
#else
		std::string str = ToString(value);
		size_t len = str.length();
		std::wstring wstr(len, L'\0');

		size_t converted = mbstowcs(&wstr[0], str.c_str(), len);
		if (converted == static_cast<size_t>(-1)) {
			throw std::runtime_error("Conversion failed");
		}

		wstr.resize(converted);
		return wstr;
#endif
	}

	std::wstring V8LanguageModule::ToWStringOr(v8::Local<v8::Value> value, std::wstring_view or_string) const {
		if (value.IsEmpty()) {
			return std::wstring(or_string);
		} else {
			return ToWString(value);
		}
	}

	fs::path V8LanguageModule::ToPath(v8::Local<v8::Value> value) const {
		ASSERT(!value->IsString());
#if V8LM_PLATFORM_WINDOWS
		v8::String::Value utf16(_isolate, value);
		if (*utf16) {
			return std::wstring_view{ reinterpret_cast<wchar_t*>(*utf16), static_cast<size_t>(utf16.length()) };
		}
#else
		v8::String::Utf8Value utf8(_isolate, value);
		if (*utf8) {
			return std::string_view{ *utf8, static_cast<size_t>(utf8.length()) };
		}
#endif
		return {};
	}

	fs::path V8LanguageModule::ToPathOr(v8::Local<v8::Value> value, fs::path or_path) const {
		if (value.IsEmpty()) {
			return std::move(or_path);
		} else {
			return ToPath(value);
		}
	}

}// namespace v8lm
