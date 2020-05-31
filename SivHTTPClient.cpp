
# include "HTTPClient.hpp"
# define CURL_STATICLIB
# include <curl/curl.h>

namespace s3d
{
	namespace detail
	{
		static size_t CallbackWrite(char* ptr, size_t size, size_t nmemb, IWriter* pWriter)
		{
			const size_t size_bytes = (size * nmemb);

			pWriter->write(static_cast<const void*>(ptr), size_bytes);

			return size_bytes;
		}

		static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, String* gotData)
		{
			const size_t size_bytes = (size * nitems);
			std::string str;
			str.append(buffer, size_bytes);
			gotData->append(Unicode::Widen(str));

			return size_bytes;
		}
	}

	HTTPResponse::HTTPResponse(const String& header)
		: m_header(header)
	{
		if (m_header.isEmpty())
		{
			return;
		}

		const Array<String> splitStr = m_header.split('\n');
		if (splitStr.isEmpty())
		{
			return;
		}

		const Array<String> splitStr2 = splitStr.front().split(' ');
		if (splitStr2.size() < 2)
		{
			return;
		}

		m_statusCode = ParseOr<int32>(splitStr2[1], InvalidStatusCode);
	}

	bool HTTPResponse::isValid() const
	{
		return m_statusCode != InvalidStatusCode;
	}

	HTTPResponse::operator bool() const
	{
		return isValid();
	}

	const String& HTTPResponse::getHeader() const
	{
		return m_header;
	}

	int32 HTTPResponse::getStatusCode() const
	{
		return m_statusCode;
	}

	bool HTTPClient::InitCURL()
	{
		return (::CURLE_OK == ::curl_global_init(CURL_GLOBAL_ALL));
	}

	void HTTPClient::CleanupCURL()
	{
		::curl_global_cleanup();
	}

	HTTPResponse HTTPClient::downloadFile(const URLView url, FilePathView saveFilePath)
	{

		BinaryWriter writer(saveFilePath);
		{
			if (!writer)
			{
				return HTTPResponse{};
			}
		}

		::CURL* curl = ::curl_easy_init();
		{
			if (!curl)
			{
				return HTTPResponse{};
			}
		}

		const std::string urlUTF8 = Unicode::ToUTF8(url);
		::curl_easy_setopt(curl, ::CURLOPT_URL, urlUTF8.c_str());

		::curl_easy_setopt(curl, ::CURLOPT_WRITEFUNCTION, detail::CallbackWrite);
		::curl_easy_setopt(curl, ::CURLOPT_WRITEDATA, &writer);

		// レスポンスヘッダーの設定
		String headerString;
		{
			::curl_easy_setopt(curl, ::CURLOPT_HEADERFUNCTION, detail::HeaderCallback);
			::curl_easy_setopt(curl, ::CURLOPT_HEADERDATA, &headerString);
		}


		const ::CURLcode result = ::curl_easy_perform(curl);
		::curl_easy_cleanup(curl);

		if (result != ::CURLE_OK)
		{
			LOG_FAIL(U"curl failed (CURLcode: {})"_fmt(result));
			writer.clear();
			return HTTPResponse{};
		}

		return HTTPResponse(headerString);
	}


	HTTPResponse HTTPClient::get(const URLView url, const HTTPHeader& header, const FilePathView saveFilePath)
	{
		BinaryWriter writer(saveFilePath);
		{
			if (!writer)
			{
				return HTTPResponse{};
			}
		}

		::CURL* curl = ::curl_easy_init();
		{
			if (!curl)
			{
				return HTTPResponse{};
			}
		}

		// ヘッダの追加
		::curl_slist* header_slist = nullptr;

		for (auto [f, s] : header)
		{
			const std::string text = U"{}: {}"_fmt(f, s).toUTF8();
			header_slist = ::curl_slist_append(header_slist, text.c_str());
		}

		::curl_easy_setopt(curl, ::CURLOPT_HTTPHEADER, header_slist);


		const std::string urlUTF8 = Unicode::ToUTF8(url);
		::curl_easy_setopt(curl, ::CURLOPT_URL, urlUTF8.c_str());

		::curl_easy_setopt(curl, ::CURLOPT_WRITEFUNCTION, detail::CallbackWrite);
		::curl_easy_setopt(curl, ::CURLOPT_WRITEDATA, &writer);


		// レスポンスヘッダーの設定
		String headerString;
		{
			::curl_easy_setopt(curl, ::CURLOPT_HEADERFUNCTION, detail::HeaderCallback);
			::curl_easy_setopt(curl, ::CURLOPT_HEADERDATA, &headerString);
		}

		const ::CURLcode result = ::curl_easy_perform(curl);
		::curl_easy_cleanup(curl);
		::curl_slist_free_all(header_slist);

		if (result != ::CURLE_OK)
		{
			LOG_FAIL(U"curl failed (CURLcode: {})"_fmt(result));
			writer.clear();
			return HTTPResponse{};
		}

		return HTTPResponse(headerString);
	}

	HTTPResponse HTTPClient::post(const URLView url, const HTTPHeader& header, const void* src, size_t size, const FilePathView saveFilePath)
	{
		BinaryWriter writer(saveFilePath);
		{
			if (!writer)
			{
				return HTTPResponse{};
			}
		}

		::CURL* curl = ::curl_easy_init();
		{
			if (!curl)
			{
				return HTTPResponse{};
			}
		}

		// ヘッダの追加
		::curl_slist* header_slist = nullptr;

		for (auto [f, s] : header)
		{
			const std::string text = U"{}: {}"_fmt(f, s).toUTF8();
			header_slist = ::curl_slist_append(header_slist, text.c_str());
		}

		::curl_easy_setopt(curl, ::CURLOPT_HTTPHEADER, header_slist);

		const std::string urlUTF8 = Unicode::ToUTF8(url);
		::curl_easy_setopt(curl, ::CURLOPT_URL, urlUTF8.c_str());

		// POST
		{
			::curl_easy_setopt(curl, ::CURLOPT_POST, 1L);
			::curl_easy_setopt(curl, ::CURLOPT_POSTFIELDS, const_cast<char*>(static_cast<const char*>(src)));
			::curl_easy_setopt(curl, ::CURLOPT_POSTFIELDSIZE, static_cast<long>(size));
		}

		::curl_easy_setopt(curl, ::CURLOPT_WRITEFUNCTION, detail::CallbackWrite);
		::curl_easy_setopt(curl, ::CURLOPT_WRITEDATA, &writer);

		// レスポンスヘッダーの設定
		String headerString;

		{
			::curl_easy_setopt(curl, ::CURLOPT_HEADERFUNCTION, detail::HeaderCallback);
			::curl_easy_setopt(curl, ::CURLOPT_HEADERDATA, &headerString);
		}

		const ::CURLcode result = ::curl_easy_perform(curl);
		::curl_easy_cleanup(curl);
		::curl_slist_free_all(header_slist);

		if (result != ::CURLE_OK)
		{
			LOG_FAIL(U"curl failed (CURLcode: {})"_fmt(result));
			writer.clear();
			return HTTPResponse{};
		}

		return HTTPResponse(headerString);
	}

}
