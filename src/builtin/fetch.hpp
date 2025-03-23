namespace builtin {
#if V8LM_PLATFORM_WINDOWS
	static std::string GetErrorMessage() {
		DWORD dwErrorCode = ::GetLastError();
		if (dwErrorCode == 0) {
			return {}; // No error message has been recorded
		}
		
		LPSTR messageBuffer = NULL;
		const DWORD size = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM  | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
										  NULL, // (not used with FORMAT_MESSAGE_FROM_SYSTEM)
										  dwErrorCode,
										  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
										  reinterpret_cast<LPSTR>(&messageBuffer),
										  0,
										  NULL);
		if (!size) {
			return std::format("Unknown error code: {}", dwErrorCode);
		}

		auto deleter = [](void* p) { ::LocalFree(p); };
		std::unique_ptr<char, decltype(deleter)> ptrBuffer(messageBuffer, deleter);
		return { ptrBuffer.get(), size };
	}
	
#define ToNString ToWString
#else
#define ToNString ToString
#endif

	namespace fetch {
		enum class Type : uint8_t {
			Get,
			Post,
			Head
		};
		
		using Callback = std::function<void(std::string)>;

#if V8LM_PLATFORM_WINDOWS
		const wchar_t* TypeToString(Type type) {
			switch (type) {
				case Type::Get: return L"GET";
				case Type::Post: return L"POST";
				case Type::Head: return L"HEAD";
				default: return L"GET";
			}
		}

		Type StringToType(std::wstring_view method) {
			if (method == L"GET") return Type::Get;
			else if (method == L"POST") return Type::Post;
			else if (method == L"HEAD") return Type::Head;
			return Type::Get;
		}

		static inline const wchar_t* const kDefaultUserAgent = L"V8-Fetcher/1.0";

		// Global WinHTTP session handle
		HINTERNET hSession = nullptr;

		// Initialize WinHTTP session once
		bool Initialize() {
			if (!hSession) {
				hSession = WinHttpOpen(kDefaultUserAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
				if (!hSession) {
					return false;
				}
			}
			return true;
		}

		// Clean up WinHTTP session
		void Terminate() {
			if (hSession) {
				WinHttpCloseHandle(hSession);
				hSession = nullptr;
			}
		}

		// Function to handle the HTTP request asynchronously using WinHTTP
		void fetchAsync(std::wstring url, Type type, std::string body, Callback onSuccess, Callback onError) {
			std::thread([url = std::move(url), type, body = std::move(body), onSuccess = std::move(onSuccess), onError = std::move(onError)]() {
				HINTERNET hConnect = nullptr;
				HINTERNET hRequest = nullptr;

				try {
					// Parse the URL
					URL_COMPONENTSW uc = {
						.dwStructSize = sizeof(uc),
						.dwSchemeLength = static_cast<DWORD>(-1),
						.dwHostNameLength = static_cast<DWORD>(-1),
						.dwUrlPathLength = static_cast<DWORD>(-1),
						.dwExtraInfoLength = static_cast<DWORD>(-1)
					};

					if (!WinHttpCrackUrl(url.c_str(), url.length(), 0, &uc)) {
						throw std::runtime_error("Failed to parse URL");
					}

					std::wstring hostName(uc.lpszHostName, uc.dwHostNameLength);
					std::wstring path(uc.lpszUrlPath, uc.dwUrlPathLength);

					// Connect to the server
					hConnect = WinHttpConnect(hSession, hostName.c_str(), uc.nPort, 0);
					if (!hConnect) {
						throw std::runtime_error(std::format("Failed to start HTTP request: {}", GetErrorMessage()));
					}

					// Create the HTTP request
					const DWORD requestFlags = uc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0;
					hRequest = WinHttpOpenRequest(hConnect, TypeToString(type), path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags);
					if (!hRequest) {
						throw std::runtime_error(std::format("Failed to create HTTP request: {}", GetErrorMessage()));
					}

					BOOL result;
					if (type == Type::Post) {
						const std::wstring_view additionalHeaders = L"Content-Type: application/x-www-form-urlencoded\r\n";
						result = WinHttpSendRequest(hRequest, additionalHeaders.data(), static_cast<DWORD>(additionalHeaders.size()), const_cast<char*>(body.data()), static_cast<DWORD>(body.size()), static_cast<DWORD>(body.size()), 0);
					} else {
						result = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
					}

					// Send the request
					if (!result) {
						throw std::runtime_error(std::format("Failed to send HTTP request: {}", GetErrorMessage()));
					}

					// Receive the response
					if (!WinHttpReceiveResponse(hRequest, nullptr)) {
						throw std::runtime_error(std::format("Failed to receive HTTP response:{}", GetErrorMessage()));
					}

					// Read the response data
					std::string response;
					DWORD bytesRead = 0;
					char buffer[4096];
					do {
						if (!WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead)) {
							throw std::runtime_error(std::format("Failed to read HTTP response data: {}", GetErrorMessage()));
						}
						response.append(buffer, bytesRead);
					} while (bytesRead > 0);

					// Clean up
					WinHttpCloseHandle(hRequest);
					WinHttpCloseHandle(hConnect);

					onSuccess(std::move(response));
				} catch (const std::exception& e) {
					// Clean up in case of error
					if (hRequest) WinHttpCloseHandle(hRequest);
					if (hConnect) WinHttpCloseHandle(hConnect);

					onError(e.what());
				}
			}).detach();
		}
#else
		const char* TypeToString(Type type) {
			switch (type) {
				case Type::Get: return "GET";
				case Type::Post: return "POST";
				case Type::Head: return "HEAD";
				default: return "GET";
			}
		}

		Type StringToType(std::string_view method) {
			if (method == "GET") return Type::Get;
			else if (method == "POST") return Type::Post;
			else if (method == "HEAD") return Type::Head;
			return Type::Get;
		}

		static inline const char* const kDefaultUserAgent = "V8-Fetcher/1.0";

		// Initialize libcurl globally
		bool Initialize() {
			return curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK;
		}

		// Clean up libcurl globally
		void Terminate() {
			curl_global_cleanup();
		}

		// Function to handle the HTTP request asynchronously using libcurl
		void fetchAsync(std::string url, Type type, std::string body, Callback onSuccess, Callback onError) {
			std::thread([url = std::move(url), type, body = std::move(body), onSuccess = std::move(onSuccess), onError = std::move(onError)]() {
				CURL* curl = curl_easy_init();
				if (!curl) {
					throw std::runtime_error("Failed to initialize libcurl");
				}

				std::string response;
				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_USERAGENT, kDefaultUserAgent);
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, TypeToString(type));
				if (type == Type::Post) {
					curl_easy_setopt(curl, CURLOPT_POST, 1L);
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
				}
				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
					std::string* response = static_cast<std::string*>(userdata);
					response->append(ptr, size * nmemb);
					return size * nmemb;
				});
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

				CURLcode res = curl_easy_perform(curl);
				curl_easy_cleanup(curl);

				if (res != CURLE_OK) {
					onError(curl_easy_strerror(res));
				} else {
					onSuccess(std::move(response));
				}
			}).detach();
		}
#endif
		void ArrayBuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> self = args.This();

			v8::Local<v8::Value> body = self->Get(context, g_v8lm.MakeString("body")).ToLocalChecked();

			if (!body->IsString()) {
				g_v8lm.ThrowException("Response body is not a string");
				return;
			}

			v8::Local<v8::String> bodyString = body->ToString(context).ToLocalChecked();

			v8::String::Utf8Value utf8(isolate, bodyString);
			const char* data = *utf8;
			size_t length = utf8.length();

			v8::Local<v8::ArrayBuffer> arrayBuffer = v8::ArrayBuffer::New(isolate, length);
			std::memcpy(arrayBuffer->GetBackingStore()->Data(), data, length);
			args.GetReturnValue().Set(arrayBuffer);
		}

		void Blob(const v8::FunctionCallbackInfo<v8::Value>& args) {
			args.GetReturnValue().Set(false);
		}

		void Bytes(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> self = args.This();

			v8::Local<v8::Value> body = self->Get(context, g_v8lm.MakeString("body")).ToLocalChecked();

			if (!body->IsString()) {
				g_v8lm.ThrowException("Response body is not a string");
				return;
			}

			v8::Local<v8::String> bodyString = body->ToString(context).ToLocalChecked();

			v8::String::Utf8Value utf8(isolate, bodyString);
			const char* data = *utf8;
			size_t length = utf8.length();

			v8::Local<v8::ArrayBuffer> arrayBuffer = v8::ArrayBuffer::New(isolate, length);
			std::memcpy(arrayBuffer->GetBackingStore()->Data(), data, length);
			v8::Local<v8::Uint8Array> uint8Array = v8::Uint8Array::New(arrayBuffer, 0, length);
			args.GetReturnValue().Set(uint8Array);
		}

		void Clone(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Local<v8::Object> self = args.This();
			args.GetReturnValue().Set(self->Clone());
		}

		void FormData(const v8::FunctionCallbackInfo<v8::Value>& args) {
			args.GetReturnValue().Set(false);
		}

		void Json(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> self = args.This();

			v8::Local<v8::Value> body = self->Get(context, g_v8lm.MakeString("body")).ToLocalChecked();

			if (!body->IsString()) {
				g_v8lm.ThrowException("Response body is not a string");
				return;
			}

			v8::Local<v8::String> jsonString = body->ToString(context).ToLocalChecked();
			v8::Local<v8::Value> jsonValue;

			if (!v8::JSON::Parse(context, jsonString).ToLocal(&jsonValue)) {
				g_v8lm.ThrowException("Failed to parse JSON");
				return;
			}

			args.GetReturnValue().Set(jsonValue);
		}

		void Text(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> self = args.This();

			v8::Local<v8::Value> body = self->Get(context, g_v8lm.MakeString("body")).ToLocalChecked();

			if (!body->IsString()) {
				g_v8lm.ThrowException("Response body is not a string");
				return;
			}

			args.GetReturnValue().Set(body);
		}

		// Fetch function to be called from JavaScript
		void Fetch(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Wrong number or types of arguments");
				return;
			}

			try {
				auto url = g_v8lm.ToNString(args[0]);

				Type type{};
				std::string body;
				if (args.Length() > 1 && args[1]->IsObject()) {
					v8::Local<v8::Object> options = args[1]->ToObject(context).ToLocalChecked();
					v8::Local<v8::Value> value;
					if (options->Get(context, g_v8lm.MakeString("method")).ToLocal(&value)) {
						type = StringToType(g_v8lm.ToNString(value));
					}
					if (options->Get(context, g_v8lm.MakeString("body")).ToLocal(&value)) {
						body = g_v8lm.ToString(value);
					}
				}

				v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
				args.GetReturnValue().Set(resolver->GetPromise());

				auto resolve = std::make_shared<v8::Global<v8::Promise::Resolver>>(isolate, resolver);

				Callback onSuccess = [isolate, resolve, url](std::string response) {
					g_v8lm.AddTask(0ms, [isolate, resolve = std::move(resolve), url = std::move(url), response = std::move(response)]() {
						v8::Local<v8::Context> context = isolate->GetCurrentContext();

						v8::Local<v8::Object> result = v8::Object::New(isolate);

						result->Set(context, g_v8lm.MakeString("status"), v8::Integer::New(isolate, 200)).Check();
						result->Set(context, g_v8lm.MakeString("statusCode"), g_v8lm.MakeString("200")).Check();
						result->Set(context, g_v8lm.MakeString("statusText"), g_v8lm.MakeString("OK")).Check();
						result->Set(context, g_v8lm.MakeString("ok"), v8::Boolean::New(isolate, true)).Check();
						result->Set(context, g_v8lm.MakeString("redirected"), v8::Boolean::New(isolate, false)).Check();
						result->Set(context, g_v8lm.MakeString("url"), g_v8lm.MakeString(url)).Check();
						result->Set(context, g_v8lm.MakeString("type"), g_v8lm.MakeString("basic")).Check();
						result->Set(context, g_v8lm.MakeString("headers"), v8::Object::New(isolate)).Check();
						result->Set(context, g_v8lm.MakeString("bodyUsed"), v8::Boolean::New(isolate, !response.empty())).Check();
						result->Set(context, g_v8lm.MakeString("body"), g_v8lm.MakeString(response)).Check();

						result->Set(context, g_v8lm.MakeString("arrayBuffer"), v8::Function::New(context, ArrayBuffer).ToLocalChecked()).Check();
						result->Set(context, g_v8lm.MakeString("blob"), v8::Function::New(context, Blob).ToLocalChecked()).Check();
						result->Set(context, g_v8lm.MakeString("bytes"), v8::Function::New(context, Bytes).ToLocalChecked()).Check();
						result->Set(context, g_v8lm.MakeString("clone"), v8::Function::New(context, Clone).ToLocalChecked()).Check();
						result->Set(context, g_v8lm.MakeString("formData"), v8::Function::New(context, FormData).ToLocalChecked()).Check();
						result->Set(context, g_v8lm.MakeString("json"), v8::Function::New(context, Json).ToLocalChecked()).Check();
						result->Set(context, g_v8lm.MakeString("text"), v8::Function::New(context, Text).ToLocalChecked()).Check();

						ASSERT(resolve->Get(isolate)->Resolve(context, result).FromMaybe(false));
					});
				};

				Callback onError = [isolate, resolve](std::string error) {
					g_v8lm.AddTask(0ms, [isolate, resolve = std::move(resolve), error = std::move(error)]() {
						v8::Local<v8::Context> context = isolate->GetCurrentContext();

						ASSERT(resolve->Get(isolate)->Reject(context, g_v8lm.MakeString(error)).FromMaybe(false));
					});
				};

				fetchAsync(std::move(url), type, std::move(body), std::move(onSuccess), std::move(onError));

			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}
	}
}