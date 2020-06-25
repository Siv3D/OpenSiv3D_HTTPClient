# pragma once
# define SIV3D_CONCURRENT
# include <Siv3D.hpp>

# if SIV3D_BUILD_TYPE(DEBUG)
#	pragma comment (lib, "libcurl-d")
# else 
#	pragma comment (lib, "libcurl")
# endif
# pragma comment (lib, "crypt32")

namespace s3d
{
	using URL = String;
	using URLView = StringView;
	using HTTPHeader = HashTable<String, String>;

	/// <summary>
	/// ダウンロードの進行状況
	/// </summary>
	enum class HTTPAsyncStatus
	{
		/// <summary>
		/// ダウンロードするものが無い
		/// </summary>
		None,

		/// <summary>
		/// ダウンロード中
		/// </summary>
		Working,

		/// <summary>
		/// ダウンロード失敗
		/// </summary>
		Failed,

		/// <summary>
		/// ダウンロードがキャンセルされた
		/// </summary>
		Canceled,

		/// <summary>
		/// ダウンロード完了
		/// </summary>
		Succeeded,
	};

	class HTTPResponse
	{
	private:

		String m_header;

		static constexpr int32 InvalidStatusCode = 0;

		int32 m_statusCode = InvalidStatusCode;

	public:

		HTTPResponse() = default;

		explicit HTTPResponse(const String& header);

		/// <summary>
		/// レスポンスヘッダーのステータスコードが有効であるかを返します。
		/// </summary>
		bool isValid() const;

		/// <summary>
		/// isVaild()
		/// </summary>
		explicit operator bool() const;

		/// <summary>
		/// レスポンスヘッダーを返します。
		/// </summary>
		const String& getHeader() const;

		/// <summary>
		/// ステータスコードを返します。
		/// </summary>
		int32 getStatusCode() const;
	};

	struct HTTPProgress
	{
		HTTPProgress() = default;

		HTTPProgress(URLView url);

		/// <summary>
		/// ダウンロードするファイルの合計サイズ。不明の場合 none
		/// </summary>
		Optional<int64> downloadTotalSize;

		/// <summary>
		/// アップロードするファイルの合計サイズ。不明の場合 none
		/// </summary>
		Optional<int64> uploadTotalSize;

		/// <summary>
		/// ダウンロードしたファイルのサイズ。
		/// </summary>
		int64 downloadNowSize = 0;

		/// <summary>
		/// アップロードしたファイルのサイズ。
		/// </summary>
		int64 uploadNowSize = 0;

		/// <summary>
		/// ダウンロードの進行状況の割合。不明の場合 none
		/// </summary>
		Optional<double> getDownloadProgress() const;

		/// <summary>
		/// アップロードの進行状況の割合。不明の場合 none
		/// </summary>
		Optional<double> getUploadProgress() const;

		/// <summary>
		/// 通信先のURL
		/// </summary>
		URL url;

		HTTPAsyncStatus status = HTTPAsyncStatus::None;

		/// <summary>
		/// 通信中にtrueにすると通信をキャンセルします
		/// </summary>
		bool cancelCommunication = false;
	};

	class AsyncHTTPTask
	{
	private:

		class AsyncHTTPTaskImpl;

		std::shared_ptr<AsyncHTTPTaskImpl> pImpl;

	public:

		AsyncHTTPTask();

		explicit AsyncHTTPTask(URLView url, FilePathView path);

		~AsyncHTTPTask();

		const HTTPProgress& getProgress() const;

		const HTTPResponse& getResponse() const;

		//enum class : {None,Working,Canceled,Failed,Succeeded}
		const HTTPAsyncStatus& currentStatus() const;

		void cancelTask();

		//実行時にtask.get()
		bool isDone();
	};

	/// <summary>
	/// HTTP通信を行うクラス
	/// </summary>
	class HTTPClient
	{
	public:

		/// <summary>
		/// HTTPClient を使うアプリケーションで、最初に 1 回だけ呼び出します。([Siv3D ToDo]: エンジン内に組み込み)
		/// </summary>
		static bool InitCURL();

		/// <summary>
		/// HTTPClient を使うアプリケーションで、最後に 1 回だけ呼び出します。([Siv3D ToDo]: エンジン内に組み込み)
		/// </summary>
		static void CleanupCURL();

		HTTPClient() = default;

		/// <summary>
		/// ファイルをダウンロードします。
		/// </summary>
		/// <param name="url">
		/// URL
		/// </param>
		/// <param name="saveFilePath">
		/// 取得したファイルの保存先のファイルパス
		/// </param>
		HTTPResponse downloadFile(URLView url, FilePathView saveFilePath);

		AsyncHTTPTask downloadFileAsync(URLView url, FilePathView saveFilePath);

		/// <summary>
		/// HTTP-GETリクエストを送ります
		/// </summary>
		/// <param name="url">
		/// URL
		/// </param>
		/// <param name="header">
		/// ヘッダ
		/// </param>
		/// <param name="saveFilePath">
		/// 取得したファイルの保存先のファイルパス
		/// </param>
		HTTPResponse get(const URLView url, const HTTPHeader& header, const FilePathView saveFilePath);

		/// <summary>
		/// HTTP-POSTリクエストを送ります
		/// </summary>
		/// <param name="url">
		/// URL
		/// </param>
		/// <param name="header">
		/// ヘッダ
		/// </param>
		/// <param name="src">
		/// 送信するデータの先頭ポインタ
		/// </param>
		/// <param name="size">
		/// 送信するデータのサイズ（バイト）
		/// </param>
		/// <param name="saveFilePath">
		/// 取得したファイルの保存先のファイルパス
		/// </param>
		HTTPResponse post(URLView url, const HTTPHeader& header, const void* src, size_t size, FilePathView saveFilePath);
	};
}
