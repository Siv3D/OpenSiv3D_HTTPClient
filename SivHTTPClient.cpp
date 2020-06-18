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

		static int XferInfo(HTTPProgress* progress, curl_off_t dlTotal, curl_off_t dlNow, curl_off_t ulTotal, curl_off_t ulNow)
		{
			progress->downloadNowSize = static_cast<int64>(dlNow);
			progress->uploadNowSize = static_cast<int64>(ulNow);

			if (dlTotal != 0L)
			{
				progress->downloadTotalSize = static_cast<int64>(dlTotal);
			}
			if (ulTotal != 0L)
			{
				progress->downloadTotalSize = static_cast<int64>(ulTotal);
			}

			if (progress->cancelCommunication)
			{
				return 1;
			}

			return 0;
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

	HTTPProgress::HTTPProgress(URLView url)
		: url(url)
	{
	}

	Optional<double> s3d::HTTPProgress::getDownloadProgress() const
	{
		if (!downloadTotalSize)
		{
			return none;
		}

		return static_cast<double>(downloadNowSize) / downloadTotalSize.value();
	}

	Optional<double> s3d::HTTPProgress::getUploadProgress() const
	{
		if (!uploadTotalSize)
		{
			return none;
		}

		return static_cast<double>(uploadNowSize) / uploadTotalSize.value();
	}

	HTTPResponse AsyncHTTPTask::innerDownloadTask()
	{
		m_progressValue.status = HTTPAsyncStatus::Working;
		{
			if (!m_writer)
			{
				m_progressValue.status = HTTPAsyncStatus::Failed;
				return HTTPResponse{};
			}
		}

		::CURL* curl = ::curl_easy_init();
		{
			if (!curl)
			{
				m_progressValue.status = HTTPAsyncStatus::Failed;
				return HTTPResponse{};
			}
		}

		const std::string urlUTF8 = Unicode::ToUTF8(m_progressValue.url);
		::curl_easy_setopt(curl, ::CURLOPT_URL, urlUTF8.c_str());

		::curl_easy_setopt(curl, ::CURLOPT_WRITEFUNCTION, detail::CallbackWrite);
		::curl_easy_setopt(curl, ::CURLOPT_WRITEDATA, &m_writer);

		::curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, detail::XferInfo);
		::curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &m_progressValue);

		::curl_easy_setopt(curl, ::CURLOPT_NOPROGRESS, 0L);

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
			m_writer.clear();
			if (m_progressValue.cancelCommunication)
			{
				m_progressValue.status = HTTPAsyncStatus::Canceled;
			}
			else
			{
				m_progressValue.status = HTTPAsyncStatus::Failed;
			}
			return HTTPResponse{};
		}

		m_writer.close();
		m_progressValue.status = HTTPAsyncStatus::Succeeded;
		return HTTPResponse(headerString);
	}

	AsyncHTTPTask::AsyncHTTPTask(URLView url, FilePathView path)
		: m_progressValue(url)
		, m_response()
		, m_writer(path)
		, m_task()
	{
		m_task = CreateConcurrentTask(&AsyncHTTPTask::innerDownloadTask, this);
	}

	AsyncHTTPTask::~AsyncHTTPTask()
	{
		if (currentStatus() == HTTPAsyncStatus::Working)
		{
			cancelTask();
			//libcurl側でfailするのでログ出力はそれに任せてもよいかも知れない
			LOG_FAIL(U"Cancel of Download.");
		}
	}

	const HTTPProgress& AsyncHTTPTask::getProgress() const
	{
		return m_progressValue;
	}

	const HTTPResponse& AsyncHTTPTask::getResponse() const
	{
		return m_response;
	}

	const HTTPAsyncStatus& AsyncHTTPTask::currentStatus() const
	{
		return m_progressValue.status;
	}

	void AsyncHTTPTask::cancelTask()
	{
		m_progressValue.cancelCommunication = true;
	}

	bool AsyncHTTPTask::isDone()
	{
		if (!m_task.is_done())
		{
			return false;
		}

		if (m_task.valid())
		{
			m_response = m_task.get();
		}

		return true;
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

	AsyncHTTPTask HTTPClient::downloadFileAsync(URLView url, FilePathView saveFilePath)
	{
		return AsyncHTTPTask(url, saveFilePath);
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
