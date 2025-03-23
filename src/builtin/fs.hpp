namespace builtin {
	// Helper function to convert `std::chrono::time_point` to Unix timestamp in milliseconds
	double ToUnixTimestampMs(const std::filesystem::file_time_type& time) {
		using namespace std::chrono;
		auto duration = time.time_since_epoch();
		return static_cast<double>(duration_cast<milliseconds>(duration).count());
	}

	// Helper function to convert `std::chrono::time_point` to ISO 8601 string
	std::string ToIso8601(const std::filesystem::file_time_type& time) {
		using namespace std::chrono;
		auto tp = time_point_cast<system_clock::duration>(time - std::filesystem::file_time_type::clock::now() + system_clock::now());
		std::time_t time_t = system_clock::to_time_t(tp);
		std::ostringstream ss;
		ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S.%fZ");
		return ss.str();
	}

	namespace fs {
		// existsSync
		void ExistsSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);
				args.GetReturnValue().Set(std::filesystem::exists(path));
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// renameSync
		void RenameSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
				g_v8lm.ThrowException("Expected 2 arguments: oldpath and newpath");
				return;
			}

			try {
				std::filesystem::path oldPath = g_v8lm.ToPath(args[0]);
				std::filesystem::path newPath = g_v8lm.ToPath(args[1]);
				std::filesystem::rename(oldPath, newPath);
				args.GetReturnValue().Set(true);
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// chownSync (Stub - no direct cross-platform implementation in fs)
		void ChownSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
			args.GetReturnValue().Set(true); // Stub implementation
		}

		// rmdirSync
		void RmdirSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);
				bool removed = std::filesystem::remove(path);
				args.GetReturnValue().Set(removed);
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// mkdirSync
		void MkdirSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}
			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);
				bool created = std::filesystem::create_directory(path);
				args.GetReturnValue().Set(created);
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// statSync
		void StatSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);
				auto status = std::filesystem::status(path);
				auto meta = std::filesystem::status(path);

				v8::Local<v8::Object> obj = v8::Object::New(isolate);

				obj->Set(context,
						 g_v8lm.MakeString("dev"),
						 v8::Number::New(isolate, 0)) // fs does not support dev
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("mode"),
						 v8::Number::New(isolate, static_cast<uint32_t>(status.permissions())))
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("nlink"),
						 v8::Number::New(isolate, 1)) // Link count is usually 1 for files
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("uid"),
						 v8::Number::New(isolate, 0)) // UID is not available with fs
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("gid"),
						 v8::Number::New(isolate, 0)) // GID is not available with fs
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("rdev"),
						 v8::Number::New(isolate, 0)) // Not available
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("blksize"),
						 v8::Number::New(isolate, 4096)) // Common block size
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("ino"),
						 v8::Number::New(isolate, 0)) // Inode number is not directly available in fs
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("size"),
						 v8::Number::New(isolate, std::filesystem::file_size(path)))
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("blocks"),
						 v8::Number::New(isolate, 0)) // Not available
						.Check();

				// Timestamps in milliseconds
				auto mtime = std::filesystem::last_write_time(path);
				auto atime = std::filesystem::last_write_time(path); // No native atime support
				auto ctime = std::filesystem::last_write_time(path); // No native ctime support
				obj->Set(context,
						 g_v8lm.MakeString("atimeMs"),
						 v8::Number::New(isolate, ToUnixTimestampMs(atime)))
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("mtimeMs"),
						 v8::Number::New(isolate, ToUnixTimestampMs(mtime)))
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("ctimeMs"),
						 v8::Number::New(isolate, ToUnixTimestampMs(ctime)))
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("birthtimeMs"),
						 v8::Number::New(isolate, ToUnixTimestampMs(ctime))) // Birth time not supported
						.Check();

				// Timestamps in ISO format
				obj->Set(context,
						 g_v8lm.MakeString("atime"),
						 g_v8lm.MakeString(ToIso8601(atime)))
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("mtime"),
						 g_v8lm.MakeString(ToIso8601(mtime)))
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("ctime"),
						 g_v8lm.MakeString(ToIso8601(ctime)))
						.Check();

				obj->Set(context,
						 g_v8lm.MakeString("birthtime"),
						 g_v8lm.MakeString(ToIso8601(ctime)))
						.Check();

				args.GetReturnValue().Set(obj);
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// readdirSync
		void ReaddirSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();

			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);
				v8::Local<v8::Array> array = v8::Array::New(isolate);

				size_t i = static_cast<size_t>(-1);
				for (const auto& entry : std::filesystem::directory_iterator(path)) {
					const auto& res = entry.path().native();
					array->Set(context,
							   ++i,
							  g_v8lm.MakeString(res)).Check();
				}
				args.GetReturnValue().Set(array);
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// readFileSync
		void ReadFileSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1 || !args[0]->IsString()) {
				g_v8lm.ThrowException("Expected 1 argument: path");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				std::ifstream file(path, std::ios::in | std::ios::binary);
				if (!file.is_open()) {
					throw std::system_error{errno, std::generic_category()};
				}

				std::string s((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

				args.GetReturnValue().Set(g_v8lm.MakeString(s));
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// writeFileSync
		void WriteFileSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
				g_v8lm.ThrowException("Expected 2 arguments: path, content");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);
				std::string content = g_v8lm.ToString(args[1]);

				std::ofstream file(path, std::ios::out | std::ios::trunc);
				if (!file.is_open()) {
					throw std::system_error{errno, std::generic_category()};
				}
				file << content;

				args.GetReturnValue().Set(true);
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// appendFileSync
		void AppendFileSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
				g_v8lm.ThrowException("Expected 2 arguments: path, content");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);
				std::string content = g_v8lm.ToString(args[1]);

				std::ofstream file(path, std::ios::out | std::ios::ate);
				if (!file.is_open()) {
					throw std::system_error{errno, std::generic_category()};
				}
				file << content;

				args.GetReturnValue().Set(true);
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		/////////////////////////////

		template<typename T>
		void Execute(v8::Isolate* isolate, std::shared_ptr<v8::Global<v8::Function>> callback, std::string error, T result) {
			g_v8lm.AddTask(0ms, [isolate, callback = std::move(callback), error = std::move(error), result = std::move(result)]() {
				v8::Local<v8::Context> context = isolate->GetCurrentContext();
				v8::TryCatch tryCatch(isolate);

				v8::Local<v8::Value> errorVal = error.empty() ? v8::Null(isolate).As<v8::Value>() : g_v8lm.MakeString(error).As<v8::Value>();
				v8::Local<v8::Value> resultVal;

				if constexpr (std::same_as<T, bool>) {
					resultVal = v8::Boolean::New(isolate, result);
				} else if constexpr (std::same_as<T, std::string>) {
					resultVal = g_v8lm.MakeString(result);
				} else if constexpr (std::same_as<T, std::vector<std::filesystem::path>>) {
					resultVal = v8::Array::New(isolate, static_cast<int>(result.size()));
					for (size_t i = 0; i < result.size(); ++i) {
						resultVal.As<v8::Array>()->Set(context, static_cast<uint32_t>(i), g_v8lm.MakeString(result[i].native())).Check();
					}
				} else if constexpr (std::same_as<T, std::unordered_map<std::string, std::string>>) {
					resultVal = v8::Object::New(isolate);
					for (const auto& [key, value] : result) {
						resultVal.As<v8::Object>()->Set(context, g_v8lm.MakeString(key), g_v8lm.MakeString(value)).Check();
					}
				}

				std::array args = { errorVal, resultVal };
				UNUSED(callback->Get(isolate)->Call(context, v8::Undefined(isolate), static_cast<int>(args.size()), args.data()));

				if (tryCatch.HasCaught()) {
					g_v8lm.ReportException(tryCatch.Message());
					tryCatch.Reset();
				}
			});
		}

		// exists
		void Exists(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
				g_v8lm.ThrowException("Expected 2 arguments: path and callback");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[1].As<v8::Function>());

				std::thread([isolate, path = std::move(path), callback = std::move(callback)]() mutable {
					std::string error;
					bool result{};
					try {
						result = std::filesystem::exists(path); // Indicate success
					} catch (const std::exception& e) {
						error = e.what();
					}

					Execute(isolate, std::move(callback), std::move(error), std::move(result));
				}).detach();
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// rename
		void Rename(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
				g_v8lm.ThrowException("Expected 3 arguments: oldpath, newpath and callback");
				return;
			}

			try {
				std::filesystem::path oldPath = g_v8lm.ToPath(args[0]);
				std::filesystem::path newPath = g_v8lm.ToPath(args[1]);

				auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[2].As<v8::Function>());

				std::thread([isolate, oldPath = std::move(oldPath), newPath = std::move(newPath), callback = std::move(callback)]() mutable {
					std::string error;
					bool result{};
					try {
						std::filesystem::rename(oldPath, newPath);
						result = true; // Indicate success
					} catch (const std::exception& e) {
						error = e.what();
					}

					Execute(isolate, std::move(callback), std::move(error), std::move(result));
				}).detach();
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// chown (Stub - no direct cross-platform implementation in fs)
		void Chown(const v8::FunctionCallbackInfo<v8::Value>& args) {
			args.GetReturnValue().Set(true); // Stub implementation
		}

		// rmdir
		void Rmdir(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
				g_v8lm.ThrowException("Expected 2 arguments: path and callback");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[1].As<v8::Function>());

				std::thread([isolate, path = std::move(path), callback = std::move(callback)]() mutable {
					std::string error;
					bool result{};
					try {
						result = std::filesystem::remove(path); // Indicate success
					} catch (const std::exception& e) {
						error = e.what();
					}

					Execute(isolate, std::move(callback), std::move(error), std::move(result));
				}).detach();
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// mkdir
		void Mkdir(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
				g_v8lm.ThrowException("Expected 2 arguments: path and callback");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[1].As<v8::Function>());

				std::thread([isolate, path = std::move(path), callback = std::move(callback)]() mutable {
					std::string error;
					bool result{};
					try {
						result = std::filesystem::create_directory(path); // Indicate success
					} catch (const std::exception& e) {
						error = e.what();
					}

					Execute(isolate, std::move(callback), std::move(error), std::move(result));
				}).detach();
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// stat
		void Stat(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
				g_v8lm.ThrowException("Expected 2 arguments: path and callback");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[1].As<v8::Function>());

				std::thread([isolate, path = std::move(path), callback = std::move(callback)]() mutable {
					std::string error;
					std::unordered_map<std::string, std::string> result;
					try {
						auto status = std::filesystem::status(path);
						//auto meta = std::filesystem::status(path);
						auto mtime = std::filesystem::last_write_time(path);
						auto atime = std::filesystem::last_write_time(path); // No native atime support
						auto ctime = std::filesystem::last_write_time(path); // No native ctime support
						
						result["dev"] = std::to_string(0); // fs does not support dev
						result["mode"] = std::to_string(static_cast<uint32_t>(status.permissions()));
						result["nlink"] = std::to_string(1); // Link count is usually 1 for files
						result["uid"] = std::to_string(0); // UID is not available with fs
						result["gid"] = std::to_string(0); // GID is not available with fs
						result["rdev"] = std::to_string(0); // Not available
						result["blksize"] = std::to_string(4096); // Common block size
						result["ino"] = std::to_string(0); // Inode number is not directly available in fs
						result["size"] = std::to_string(std::filesystem::file_size(path));
						result["blocks"] = std::to_string(0); // Not available
						
						// Timestamps in milliseconds
						result["atimeMs"] = std::to_string(ToUnixTimestampMs(atime));
						result["mtimeMs"] = std::to_string(ToUnixTimestampMs(mtime));
						result["ctimeMs"] = std::to_string(ToUnixTimestampMs(ctime));
						result["birthtimeMs"] = std::to_string(ToUnixTimestampMs(ctime)); // Birth time not supported

						// Timestamps in ISO format
						result["atime"] = ToIso8601(atime);
						result["mtime"] = ToIso8601(mtime);
						result["ctime"] = ToIso8601(ctime);
						result["birthtime"] = ToIso8601(ctime);
					} catch (const std::exception& e) {
						error = e.what();
					}

					Execute(isolate, std::move(callback), std::move(error), std::move(result));
				}).detach();
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// readdir
		void Readdir(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
				g_v8lm.ThrowException("Expected 2 arguments: path and callback");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[1].As<v8::Function>());

				std::thread([isolate, path = std::move(path), callback = std::move(callback)]() mutable {
					std::string error;
					std::vector<std::filesystem::path> result;
					try {
						for (const auto& entry : std::filesystem::directory_iterator(path)) {
							result.emplace_back(entry.path());
						}
					} catch (const std::exception& e) {
						error = e.what();
					}

					Execute(isolate, std::move(callback), std::move(error), std::move(result));
				}).detach();
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// readFile
		void ReadFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
				g_v8lm.ThrowException("Expected 2 arguments: path and callback");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);

				auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[1].As<v8::Function>());

				std::thread([isolate, path = std::move(path), callback = std::move(callback)]() mutable {
					std::string error;
					std::string result;
					try {
						std::ifstream file(path, std::ios::in | std::ios::binary);
						if (!file.is_open()) {
							throw std::system_error{errno, std::generic_category()};
						}
						
						result = { (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>() };
					} catch (const std::exception& e) {
						error = e.what();
					}

					Execute(isolate, std::move(callback), std::move(error), std::move(result));
				}).detach();
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// writeFile
		void WriteFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsFunction()) {
				g_v8lm.ThrowException("Expected 3 arguments: path, content, and callback");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);
				std::string content = g_v8lm.ToString(args[1]);

				auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[2].As<v8::Function>());

				std::thread([isolate, path = std::move(path), content = std::move(content), callback = std::move(callback)]() mutable {
					std::string error;
					bool result{};
					try {
						std::ofstream file(path, std::ios::out | std::ios::trunc);
						if (!file.is_open()) {
							throw std::system_error{errno, std::generic_category()};
						}
						file << content;
						
						result = true; // Indicate success
					} catch (const std::exception& e) {
						error = e.what();
					}

					Execute(isolate, std::move(callback), std::move(error), std::move(result));
				}).detach();
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// appendFile
		void AppendFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
			v8::Isolate* isolate = args.GetIsolate();

			if (args.Length() < 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsFunction()) {
				g_v8lm.ThrowException("Expected 3 arguments: path, content, and callback");
				return;
			}

			try {
				std::filesystem::path path = g_v8lm.ToPath(args[0]);
				std::string content = g_v8lm.ToString(args[1]);

				auto callback = std::make_shared<v8::Global<v8::Function>>(isolate, args[2].As<v8::Function>());

				std::thread([isolate, path = std::move(path), content = std::move(content), callback = std::move(callback)]() mutable {
					std::string error;
					bool result{};
					try {
						std::ofstream file(path, std::ios::out | std::ios::app); // Open in append mode
						if (!file.is_open()) {
							throw std::system_error{errno, std::generic_category()};
						}
						file << content;
						
						result = true; // Indicate success
					} catch (const std::exception& e) {
						error = e.what();
					}

					Execute(isolate, std::move(callback), std::move(error), std::move(result));
				}).detach();
			} catch (const std::exception& e) {
				g_v8lm.ThrowException(e.what());
			}
		}

		// Init function
		v8::Local<v8::Object> Init(v8::Isolate* isolate) {
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::Local<v8::Object> exports = v8::Object::New(isolate);

			
			exports->Set(context,
						 g_v8lm.MakeString("exists"),
						 v8::FunctionTemplate::New(isolate, Exists)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("rename"),
						 v8::FunctionTemplate::New(isolate, Rename)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("chown"),
						 v8::FunctionTemplate::New(isolate, Chown)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("rmdir"),
						 v8::FunctionTemplate::New(isolate, Rmdir)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("mkdir"),
						 v8::FunctionTemplate::New(isolate, Mkdir)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("stat"),
						 v8::FunctionTemplate::New(isolate, Stat)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("readdir"),
						 v8::FunctionTemplate::New(isolate, Readdir)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("readFile"),
						 v8::FunctionTemplate::New(isolate, ReadFile)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("writeFile"),
						 v8::FunctionTemplate::New(isolate, WriteFile)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("appendFile"),
						 v8::FunctionTemplate::New(isolate, AppendFile)->GetFunction(context).ToLocalChecked()
						 ).Check();
								 

			exports->Set(context,
						 g_v8lm.MakeString("existsSync"),
						 v8::FunctionTemplate::New(isolate, ExistsSync)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("renameSync"),
						 v8::FunctionTemplate::New(isolate, RenameSync)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("chownSync"),
						 v8::FunctionTemplate::New(isolate, ChownSync)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("rmdirSync"),
						 v8::FunctionTemplate::New(isolate, RmdirSync)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("mkdirSync"),
						 v8::FunctionTemplate::New(isolate, MkdirSync)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("statSync"),
						 v8::FunctionTemplate::New(isolate, StatSync)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("readdirSync"),
						 v8::FunctionTemplate::New(isolate, ReaddirSync)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("readFileSync"),
						 v8::FunctionTemplate::New(isolate, ReadFileSync)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("writeFileSync"),
						 v8::FunctionTemplate::New(isolate, WriteFileSync)->GetFunction(context).ToLocalChecked()
						 ).Check();

			exports->Set(context,
						 g_v8lm.MakeString("appendFileSync"),
						 v8::FunctionTemplate::New(isolate, AppendFileSync)->GetFunction(context).ToLocalChecked()
						 ).Check();

			return exports;
		}

	}
}