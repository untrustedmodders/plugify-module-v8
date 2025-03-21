namespace builtin {
#if V8LM_PLATFORM_WINDOWS
	// Helper: Convert v8::Value to std::wstring
	std::wstring ToStdWString(v8::Isolate* isolate, v8::Local<v8::Value> value) {
		v8::String::Value utf16(isolate, value);
		if (*utf16) {
			return { reinterpret_cast<wchar_t*>(*utf16), static_cast<size_t>(utf16.length()) };
		}
		return L"undefined";
	}

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
	
#define ToStdNString ToStdWString
#else
#define ToStdNString ToStdString
#endif

	namespace fetch {
#if V8LM_PLATFORM_WINDOWS
		static inline const wchar_t * const kDefaultUserAgent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:85.0) Gecko/20100101 Firefox/85.0";

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
		std::future<std::string> fetchAsync(std::wstring&& url, std::wstring&& method, std::string&& body) {
			return std::async(std::launch::async, [url = std::move(url), method = std::move(method), body = std::move(body)]() mutable {
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
					hRequest = WinHttpOpenRequest(hConnect, method.c_str(), path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags);
					if (!hRequest) {
						throw std::runtime_error(std::format("Failed to create HTTP request: {}", GetErrorMessage()));
					}

					BOOL result;
					if (method == L"POST") {
						const std::wstring_view additionalHeaders = L"Content-Type: application/x-www-form-urlencoded\r\n";
						result = WinHttpSendRequest(hRequest, additionalHeaders.data(), static_cast<DWORD>(additionalHeaders.size()), body.data(), static_cast<DWORD>(body.size()), static_cast<DWORD>(body.size()), 0);
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

					return response;
				} catch (const std::exception& e) {
					// Clean up in case of error
					if (hRequest) WinHttpCloseHandle(hRequest);
					if (hConnect) WinHttpCloseHandle(hConnect);

					throw e;
				}
			});
		}
#else
		static inline const char* const kDefaultUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:85.0) Gecko/20100101 Firefox/85.0";

		// Initialize libcurl globally
		bool Initialize() {
			return curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK;
		}

		// Clean up libcurl globally
		void Terminate() {
			curl_global_cleanup();
		}

		// Function to handle the HTTP request asynchronously using libcurl
		std::future<std::string> fetchAsync(std::string&& url, std::string&& method, std::string&& body) {
			return std::async(std::launch::async, [url = std::move(url), method = std::move(method), body = std::move(body)]() {
				CURL* curl = curl_easy_init();
				if (!curl) {
					throw std::runtime_error("Failed to initialize libcurl");
				}

				std::string response;
				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_USERAGENT, kDefaultUserAgent);
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
				if (method == "POST") {
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
					throw std::runtime_error(curl_easy_strerror(res));
				}

				return response;
			});
		}
#endif

		// Fetch function to be called from JavaScript
		void Fetch(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsObject()) {
				isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "Wrong number or types of arguments"));
				return;
			}

			try {
				auto url = ToStdNString(isolate, args[0]);
				v8::Local<v8::Object> options = args[1]->ToObject(context).ToLocalChecked();
				auto method = ToStdNString(isolate, options->Get(context, v8::String::NewFromUtf8Literal(isolate, "method")).ToLocalChecked());
				auto body = ToStdString(isolate, options->Get(context, v8::String::NewFromUtf8Literal(isolate, "body")).ToLocalChecked());

				v8::Global<v8::Promise::Resolver> resolver = v8::Global<v8::Promise::Resolver>(isolate, v8::Promise::Resolver::New(context).ToLocalChecked());
				args.GetReturnValue().Set(resolver.Get(isolate)->GetPromise());

				std::future<std::string> future = fetchAsync(std::move(url), std::move(method), std::move(body));

				std::thread([isolate, resolver = std::move(resolver), future = std::move(future)]() mutable {
					v8::Local<v8::Context> context = isolate->GetCurrentContext();
					try {
						std::string response = future.get();

						v8::Local<v8::Object> result = v8::Object::New(isolate);
						result->Set(context, v8::String::NewFromUtf8Literal(isolate, "statusCode"), v8::String::NewFromUtf8Literal(isolate, "200")).Check();
						result->Set(context, v8::String::NewFromUtf8Literal(isolate, "response"), v8::String::NewFromUtf8(isolate, response.c_str()).ToLocalChecked()).Check();

						ASSERT(resolver.Get(isolate)->Resolve(context, result).FromMaybe(false));
					} catch (const std::exception& e) {
						ASSERT(resolver.Get(isolate)->Reject(context, v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked()).FromMaybe(false));
					}
				}).detach();
			} catch (const std::exception& e) {
				isolate->ThrowException(v8::String::NewFromUtf8(isolate, e.what()).ToLocalChecked());
			}
		}
	}
}