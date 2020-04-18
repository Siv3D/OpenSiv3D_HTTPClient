# pragma once
# include <Siv3D.hpp>

# if SIV3D_BUILD_TYPE(DEBUG)
#	pragma comment (lib, "lib/libcurl-d")
# else 
#	pragma comment (lib, "lib/libcurl")
# endif
# pragma comment (lib, "crypt32")

namespace s3d
{
	using URL			= String;
	using URLView		= StringView;
	using HTTPHeader	= HashTable<String, String>;

	/// <summary>
	/// HTTP�ʐM���s���N���X
	/// </summary>
	class HTTPClient
	{
	public:

		/// <summary>
		/// HTTPClient ���g���A�v���P�[�V�����ŁA�ŏ��� 1 �񂾂��Ăяo���܂��B([Siv3D ToDo]: �G���W�����ɑg�ݍ���)
		/// </summary>
		static bool InitCURL();

		/// <summary>
		/// HTTPClient ���g���A�v���P�[�V�����ŁA�Ō�� 1 �񂾂��Ăяo���܂��B([Siv3D ToDo]: �G���W�����ɑg�ݍ���)
		/// </summary>
		static void CleanupCURL();

		HTTPClient() = default;

		/// <summary>
		/// �t�@�C�����_�E�����[�h���܂��B
		/// </summary>
		/// <param name="url">
		/// URL
		/// </param>
		/// <param name="saveFilePath">
		/// �擾�����t�@�C���̕ۑ���̃t�@�C���p�X
		/// </param>
		bool downloadFile(URLView url, FilePathView saveFilePath);

		// [Siv3D ToDo]
		// (�Q�l: https://curl.haxx.se/libcurl/c/progressfunc.html)
		//bool downloadFileAsync(URLView url, FilePathView saveFilePath);

		/// <summary>
		/// HTTP-POST���N�G�X�g�𑗂�܂�
		/// </summary>
		/// <param name="url">
		/// URL
		/// </param>
		/// <param name="header">
		/// �w�b�_
		/// </param>
		/// <param name="src">
		/// ���M����f�[�^�̐擪�|�C���^
		/// </param>
		/// <param name="size">
		/// ���M����f�[�^�̃T�C�Y�i�o�C�g�j
		/// </param>
		/// <param name="saveFilePath">
		/// �擾�����t�@�C���̕ۑ���̃t�@�C���p�X
		/// </param>
		bool post(URLView url, const HTTPHeader& header, const void* src, size_t size, FilePathView saveFilePath);
	};
}
