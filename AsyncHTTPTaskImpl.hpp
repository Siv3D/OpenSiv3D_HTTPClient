# pragma once
# include "HTTPClient.hpp"

namespace s3d {
	class AsyncHTTPTask::AsyncHTTPTaskImpl
	{

	private:

		HTTPProgress m_progressValue;

		HTTPResponse m_response;

		BinaryWriter m_writer;

		ConcurrentTask<HTTPResponse> m_task;

		bool m_autoFollowLocation = true;

		HTTPResponse innerDownloadTask();

	public:

		AsyncHTTPTaskImpl() = default;

		AsyncHTTPTaskImpl(URLView url, FilePathView path, bool autoFollowLocation = true);

		~AsyncHTTPTaskImpl();

		const HTTPProgress& getProgress() const;

		const HTTPResponse& getResponse() const;

		//enum class : {None,Working,Canceled,Failed,Succeeded}
		const HTTPAsyncStatus& currentStatus() const;

		void cancelTask();

		//é¿çséûÇ…task.get()
		bool isDone();
	};
}