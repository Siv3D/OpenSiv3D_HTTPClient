# pragma once
# include <Siv3D.hpp>

# if SIV3D_BUILD_TYPE(DEBUG)
#	pragma comment (lib, "libcurl-d")
# else 
#	pragma comment (lib, "libcurl")
# endif
# pragma comment (lib, "crypt32")

namespace s3d
{
	using URL			= String;
	using URLView		= StringView;
	using HTTPHeader	= HashTable<String, String>;

	/// <summary>
	/// HTTP通信を行うクラス
	/// </summary>
	class HTTPClient
	{
	private:

		String m_responseHeader;

		Optional<int32> m_statusCode;

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
		/// 直前のレスポンスのステータスコードを取得します
		/// </summary>
		/// <returns>
		/// 直前に通信したものがないもしくは通信中の場合 none
		/// それ以外はステータスコードが返る
		/// </returns>
		[[nodiscard]] Optional<int32> statusCode() const;

		/// <summary>
		/// 直前のレスポンスのレスポンスヘッダーを取得します
		/// </summary>
		/// <returns>
		/// 直前に通信したものがないもしくは通信中の場合 none
		/// それ以外はレスポンスヘッダーが返る
		/// </returns>
		[[nodiscard]] Optional<String> responseHeader() const;

		/// <summary>
		/// ファイルをダウンロードします。
		/// </summary>
		/// <param name="url">
		/// URL
		/// </param>
		/// <param name="saveFilePath">
		/// 取得したファイルの保存先のファイルパス
		/// </param>
		bool downloadFile(URLView url, FilePathView saveFilePath);

		// [Siv3D ToDo]
		// (参考: https://curl.haxx.se/libcurl/c/progressfunc.html)
		//bool downloadFileAsync(URLView url, FilePathView saveFilePath);

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
		bool get(const URLView url, const HTTPHeader& header, const FilePathView saveFilePath);

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
		bool post(URLView url, const HTTPHeader& header, const void* src, size_t size, FilePathView saveFilePath);
	};
}
