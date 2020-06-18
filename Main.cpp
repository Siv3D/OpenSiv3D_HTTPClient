# include "HTTPClient.hpp"
# include <Siv3D.hpp> // OpenSiv3D v0.4.3

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

int32 f(int32 y) {
	Print << y;
	return y * 3;
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

# elif 1

	HTTPClient client;

	const URL url = U"http://httpbin.org/drip?duration=2&numbytes=1000&code=200&delay=0";

	const FilePath localFilePath = U"drip.txt";

	AsyncHTTPTask task = client.downloadFileAsync(url, localFilePath);

	Font font(50);

	Rect PFrame(Arg::center(Scene::Center()), 400, 50);

	double progressPercentage = 0;

	while (System::Update()) {

		progressPercentage = task.getProgress().getDownloadProgress().value_or(0);

		PFrame.drawFrame();
		RectF(PFrame.pos, progressPercentage * PFrame.w, 50).draw(task.currentStatus() == HTTPAsyncStatus::Succeeded ? Palette::Yellowgreen : Palette::Orange);
		font(U"{:.1f}"_fmt(progressPercentage * 100), U"%").drawAt(PFrame.center());
		
		switch (task.currentStatus())
		{
		case HTTPAsyncStatus::None:
			font(U"None").drawAt(Scene::Center() + Point(0, 100));
			break;
		case HTTPAsyncStatus::Working:
			font(U"Downloading").drawAt(Scene::Center() + Point(0, 100));
			break;
		case HTTPAsyncStatus::Failed:
			font(U"Failed").drawAt(Scene::Center() + Point(0, 100));
			break;
		case HTTPAsyncStatus::Canceled:
			font(U"Canceled").drawAt(Scene::Center() + Point(0, 100));
			break;
		case HTTPAsyncStatus::Succeeded:
			font(U"Done!").drawAt(Scene::Center() + Point(0, 100));
			break;
		default:
			break;
		}

		if (MouseR.down()) {
			task.cancelTask();
		}

		if (task.isDone()) {
			if (task.getResponse()) {
				BinaryReader reader(localFilePath);
				Print << U"received size : " << reader.size();
				Print << task.getResponse().getStatusCode();
			}
			else {
				//通信失敗
				Print << U"Failed";
			}
		}
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
