
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

	bool HTTPClient::InitCURL()
	{
		return (::CURLE_OK == ::curl_global_init(CURL_GLOBAL_ALL));
	}

	void HTTPClient::CleanupCURL()
	{
		::curl_global_cleanup();
	}

	Optional<int32> HTTPClient::statusCode() const
	{
		return m_statusCode;
	}

	Optional<String> HTTPClient::responseHeader() const
	{
		if (!statusCode()) 
		{
			return none;
		}
		if (m_responseHeader.isEmpty())
		{
			return none;
		}
		return m_responseHeader;
	}

	bool HTTPClient::downloadFile(const URLView url, FilePathView saveFilePath)
	{
		BinaryWriter writer(saveFilePath);
		{
			if (!writer)
			{
				return false;
			}
		}

		::CURL* curl = ::curl_easy_init();
		{
			if (!curl)
			{
				return false;
			}
		}

		const std::string urlUTF8 = Unicode::ToUTF8(url);
		::curl_easy_setopt(curl, ::CURLOPT_URL, urlUTF8.c_str());

		::curl_easy_setopt(curl, ::CURLOPT_WRITEFUNCTION, detail::CallbackWrite);
		::curl_easy_setopt(curl, ::CURLOPT_WRITEDATA, &writer);

		// レスポンスヘッダーの設定
		{
			if (!m_responseHeader.isEmpty())
			{
				m_responseHeader.clear();
			}
			::curl_easy_setopt(curl, ::CURLOPT_HEADERFUNCTION, detail::HeaderCallback);
			::curl_easy_setopt(curl, ::CURLOPT_HEADERDATA, &m_responseHeader);
		}

		m_statusCode.reset();

		const ::CURLcode result = ::curl_easy_perform(curl);
		::curl_easy_cleanup(curl);

		if (result != ::CURLE_OK)
		{
			LOG_FAIL(U"curl failed (CURLcode: {})"_fmt(result));
			writer.clear();
			return false;
		}

		// ステータスコードの設定
		{
			if (!m_responseHeader.isEmpty()) {
				m_statusCode = ParseOpt<int32>(m_responseHeader.split('\n')[0].split(' ')[1]);
			}
		}

		return true;
	}

	bool HTTPClient::get(const URLView url, const HTTPHeader& header, const FilePathView saveFilePath)
	{
		BinaryWriter writer(saveFilePath);
		{
			if (!writer)
			{
				return false;
			}
		}

		::CURL* curl = ::curl_easy_init();
		{
			if (!curl)
			{
				return false;
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
		{
			if (!m_responseHeader.isEmpty())
			{
				m_responseHeader.clear();
			}
			::curl_easy_setopt(curl, ::CURLOPT_HEADERFUNCTION, detail::HeaderCallback);
			::curl_easy_setopt(curl, ::CURLOPT_HEADERDATA, &m_responseHeader);
		}

		m_statusCode.reset();

		const ::CURLcode result = ::curl_easy_perform(curl);
		::curl_easy_cleanup(curl);
		::curl_slist_free_all(header_slist);

		if (result != ::CURLE_OK)
		{
			LOG_FAIL(U"curl failed (CURLcode: {})"_fmt(result));
			writer.clear();
			return false;
		}

		// ステータスコードの設定
		{
			if (!m_responseHeader.isEmpty()) {
				m_statusCode = ParseOpt<int32>(m_responseHeader.split('\n')[0].split(' ')[1]);
			}
		}

		return true;
	}

	bool HTTPClient::post(const URLView url, const HTTPHeader& header, const void* src, size_t size, const FilePathView saveFilePath)
	{
		BinaryWriter writer(saveFilePath);
		{
			if (!writer)
			{
				return false;
			}
		}

		::CURL* curl = ::curl_easy_init();
		{
			if (!curl)
			{
				return false;
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
		{
			if (!m_responseHeader.isEmpty())
			{
				m_responseHeader.clear();
			}
			::curl_easy_setopt(curl, ::CURLOPT_HEADERFUNCTION, detail::HeaderCallback);
			::curl_easy_setopt(curl, ::CURLOPT_HEADERDATA, &m_responseHeader);
		}

		m_statusCode.reset();

		const ::CURLcode result = ::curl_easy_perform(curl);
		::curl_easy_cleanup(curl);
		::curl_slist_free_all(header_slist);

		if (result != ::CURLE_OK)
		{
			LOG_FAIL(U"curl failed (CURLcode: {})"_fmt(result));
			writer.clear();
			return false;
		}

		// ステータスコードの設定
		{
			if (!m_responseHeader.isEmpty()) {
				m_statusCode = ParseOpt<int32>(m_responseHeader.split('\n')[0].split(' ')[1]);
			}
		}

		return true;
	}
}
