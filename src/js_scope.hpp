#pragma once

#include "js.hpp"

namespace v8lm {
	struct JsScope {
		explicit JsScope(Js& js)
			: js(js),
			  isolate(js.isolate()),
			  isolate_scope(isolate),
			  handle_scope(isolate),
			  context(js.context()),
			  context_scope(context)
		{}

		const Js& js;
		v8::Isolate* isolate;
		v8::Isolate::Scope isolate_scope;
		v8::HandleScope handle_scope;
		v8::Local<v8::Context> context;
		v8::Context::Scope context_scope;

		void SetValue(v8::Local<v8::Object> object, const char* key,
					  v8::Local<v8::Value> value) const {
			v8::Local<v8::String> k =
			v8::String::NewFromUtf8(isolate, key).ToLocalChecked();
			ASSERT(object->Set(context, k, value).FromJust());
		}

		void Set(v8::Local<v8::Object> object, const char* key,
				 v8::Local<v8::Value> value) const {
			SetValue(object, key, value);
		}

		void Set(v8::Local<v8::Object> object, const char* key,
				 v8::Local<v8::Object> value) const {
			SetValue(object, key, value);
		}

		void Set(v8::Local<v8::Object> object, const char* key, bool value) const {
			SetValue(object, key, value ? v8::True(isolate) : v8::False(isolate));
		}

		void Set(v8::Local<v8::Object> object, const char* key, int value) const {
			SetValue(object, key, v8::Number::New(isolate, value));
		}

		void Set(v8::Local<v8::Object> object, const char* key, double value) const {
			SetValue(object, key, v8::Number::New(isolate, value));
		}

		void Set(v8::Local<v8::Object> object, const char* key,
				 v8::MaybeLocal<v8::Function> value) const {
			SetValue(object, key, value.ToLocalChecked());
		}

		void Set(v8::Local<v8::Object> object, const char* key,
				 v8::Local<v8::Function> value) const {
			SetValue(object, key, value);
		}

		void Set(v8::Local<v8::Object> object, const char* key,
				 v8::FunctionCallback function,
				 v8::Local<v8::Value> data = {}) const {
			SetValue(object, key, v8::Function::New(context, function, data).ToLocalChecked());
		}

		/*void Set(v8::Local<v8::Object> object, const char* key,
				 v8::AccessorNameGetterCallback get,
				 v8::Local<v8::External> data = {}) const {
			v8::Local<v8::String> k = v8::String::NewFromUtf8(isolate, key).ToLocalChecked();
			ASSERT(object->SetAccessor(context, k, get, nullptr, data).FromJust());
		}

		void Set(v8::Local<v8::Object> object, const char* key,
				 v8::AccessorNameGetterCallback get,
				 v8::AccessorNameSetterCallback set,
				 v8::Local<v8::External> data = {}) const {
			v8::Local<v8::String> k = v8::String::NewFromUtf8(isolate, key).ToLocalChecked();
			ASSERT(object->SetAccessor(context, k, get, set, data).FromJust());
		}*/
	};
}