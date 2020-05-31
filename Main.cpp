
# include <Siv3D.hpp> // OpenSiv3D v0.4.3
# include "HTTPClient.hpp"

std::string CreateTestJSONData()
{
	JSONWriter json;
	json.startObject();
	{
		json.key(U"body").write(U"Hello, Siv3D!");
		json.key(U"date").write(DateTime::Now().format());
	}
	json.endObject();

	return json.get().toUTF8();
}

void Main()
{
	if (!HTTPClient::InitCURL())
	{
		return;
	}

# if 0

	//
	// HTTP GET
	//

	HTTPClient client;

	const FilePath localFilePath = U"logo.png";
	
	if (const HTTPResponse response = client.downloadFile(U"https://raw.githubusercontent.com/Siv3D/siv3d.docs.images/master/logo/logo.png", localFilePath)) {
		Print << response.getHeader();
		Print << response.getStatusCode();
	}

	const Texture texture(localFilePath);

	while (System::Update())
	{
		texture.draw();
	}

# elif 0

	//
	// HTTP GET - Bearer Authentication
	//

	HTTPClient client;

	const URL url = U"https://httpbin.org/bearer";
	const HTTPHeader header = {
		{ U"Authorization", U"Bearer RequestFromSiv3D" }
	};
	const FilePath localFilePath = U"resultAuth.json";

	if (const HTTPResponse response = client.get(url, header, localFilePath))
	{
		Print << TextReader(localFilePath).readAll();
		Print << response.getStatusCode();
	}
	else
	{
		Print << U"Failed";
	}

	while (System::Update())
	{

	}

# else

	//
	// HTTP POST
	//

	HTTPClient client;

	const URL url = U"https://httpbin.org/post";
	const HTTPHeader header = {
		{ U"Content-Type", U"application/json" }
	};
	const std::string json = CreateTestJSONData();
	const FilePath localFilePath = U"result.json";

	if (const HTTPResponse response = client.post(url, header, json.data(), json.size(), localFilePath))
	{
		Print << TextReader(localFilePath).readAll();
		Print << response.getStatusCode();
	}
	else
	{
		Print << U"Failed";
	}

	while (System::Update())
	{

	}

# endif

	HTTPClient::CleanupCURL();
}
