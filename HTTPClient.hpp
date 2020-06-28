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
		[[nodiscard]] bool isValid() const;

		/// <summary>
		/// isVaild()
		/// </summary>
		[[nodiscard]] explicit operator bool() const;

		/// <summary>
		/// レスポンスヘッダーを返します。
		/// </summary>
		[[nodiscard]] const String& getHeader() const;

		/// <summary>
		/// ステータスコードを返します。
		/// </summary>
		[[nodiscard]] int32 getStatusCode() const;
	};

	struct HTTPProgress
	{
		HTTPProgress() = default;

		explicit HTTPProgress(URLView url);

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
		[[nodiscard]] Optional<double> getDownloadProgress() const;

		/// <summary>
		/// アップロードの進行状況の割合。不明の場合 none
		/// </summary>
		[[nodiscard]] Optional<double> getUploadProgress() const;

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

		/// <summary>
		/// 通信の進行状況クラスを返します
		/// </summary>
		[[nodiscard]] const HTTPProgress& getProgress() const;

		/// <summary>
		/// Tips: isDoneを実行しないと無効なレスポンスしか返ってきません
		/// </summary>
		[[nodiscard]] const HTTPResponse& getResponse() const;

		/// <summary>
		/// 現在のステータスを返します
		/// </summary>
		[[nodiscard]] const HTTPAsyncStatus& currentStatus() const;

		/// <summary>
		/// 実行中の通信をキャンセルします
		/// </summary>
		void cancelTask();

		/// <summary>
		/// 通信が完了したかを返します
		/// 1回の通信で1度しかtrueを返しません
		/// </summary>
		[[nodiscard]] bool isDone();
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

		/// <summary>
		/// 非同期でファイルをダウンロードします
		/// </summary>
		/// <param name="url">
		/// URL
		/// </param>
		/// <param name="saveFilePath">
		/// 取得したファイルの保存先のファイルパス
		/// </param>
		[[nodiscard]] AsyncHTTPTask downloadFileAsync(URLView url, FilePathView saveFilePath);

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
