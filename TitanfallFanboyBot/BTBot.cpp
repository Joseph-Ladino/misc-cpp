#include <curl/curl.h> // libcurl
#include <algorithm> // transform, tolower
#include <iostream> // cout, cin
#include <thread> // async shit
#include <regex> // replace
#include <map> // map
#include "my_headers/stringutils.h" // strSplit, strJoin (this one's written by me, wanted to give a go at a quick header file)
#include "sleepy_discord/websocketpp_websocket.h" // SleepyDiscord, rapidjson, libcurl

using namespace std;
using namespace rapidjson;

SleepyDiscord::DiscordClient* cli = nullptr;

// you don't gotta make your own server class if you store the server you get on start (big brain)
SleepyDiscord::Server server;

string prefix = "bt!";

// thanks libcurl team for writing documentation
CURL* hnd = curl_easy_init();
CURLcode ret;

// Polymorphism is great
struct Command {
	vector<string> names;
	string description;

	bool returns = true;

	virtual void doShit(vector<string> args, SleepyDiscord::Message msg) {}
	virtual string reply(vector<string> args, SleepyDiscord::Message msg) { return ""; }
	void sendError(SleepyDiscord::Snowflake<SleepyDiscord::Channel> id) { cli->sendMessage(id, "Joey did a fucky again"); }
	string identify() { return "**" + names[0] + "** - " + description; }

	Command() {};
};

vector<Command*> commands;

struct Help : Command {
	string reply(vector<string> args, SleepyDiscord::Message msg) {
		string out = ">>> ";
		for(auto &com : commands) out += com->identify() + "\n";
		return out;
	}

	Help() {
		names = { "help", "h", "commands" };
		description = "Lists commands and descriptions";
	}
};

struct Hello : Command {
	string reply(vector<string> args, SleepyDiscord::Message msg) {
		return "It's good to see you, Pilot";
	}

	Hello() {
		names = { "hello", "hi", "howdy", "aloha", "ping", "sup" };
		description = "Say hello to my little friend";
	}
};

struct ChangePrefix : Command {
	string reply(vector<string> args, SleepyDiscord::Message msg) {
		if(args.size() < 2) cli->sendMessage(msg.channelID, identify());

		// make input lowercase before setting prefix
		transform(args[1].begin(), args[1].end(), args[1].begin(), tolower);
		prefix = args[1];

		// update bot's status to the new help command
		cli->updateStatus(prefix + "help", 0, SleepyDiscord::Status::idle, false);
		return "Prefix set to: **" + prefix + "**";
	}

	ChangePrefix() {
		names = { "prefix", "p" };
		description = "Changes prefix because why not";
	}
};

struct Lovecalc : Command {

	// static function so I can use it in other commands
	static string calc(vector<SleepyDiscord::User> &mentions) {
		if(mentions.size() < 2) return "this command requires you to ping 2 people...do it, no balls";
		float inc = 100.0f;
		float dif = abs(stof(mentions[0].discriminator) - stof(mentions[1].discriminator));
		float total = (dif > 0.0f) ? inc - (inc * dif / 9999.0f) : 0.0f;
		return "The human concept of love requires admiration, attraction, devotion, and respect. Conclusion: I detect that they are " + to_string((int)total) + "% in love";
	}

	string reply(vector<string> args, SleepyDiscord::Message msg) { return calc(msg.mentions); }

	Lovecalc() {
		names = { "lovecalc", "cupid", "tinder" };
		description = "Used to determine how much two people love eachother (according to my observations)";
	}
};

struct Question : Command {
	string reply(vector<string> args, SleepyDiscord::Message msg) {

		// so far all it does is the love calc but i'll add more eventually
		if(find(args.begin(), args.end(), "love") != args.end() && msg.mentions.size() >= 2) return Lovecalc::calc(msg.mentions);

		return "Sorry Pilot, I was unable to find an answer";
	}

	Question() {
		names = { "does" };
		description = "Ask a question Pilot, I might have an answer";
	}
};

struct Urban : Command {

	SleepyDiscord::Message last;
	Document query;
	string buffer;
	int index = 0;

	static size_t populateBuffer(void* contents, size_t size, size_t nmemb, void* userp) {
		((string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}

	static string hotlink(string str) {
		string out, baseURL = "https://www.urbandictionary.com/define.php?term=";
		auto firstpiece = str.find("["), lastpiece = str.find("]");
		while (lastpiece != string::npos) {
			out += str.substr(0, lastpiece + 1);
			out += "(" + baseURL + regex_replace(str.substr(firstpiece + 1, lastpiece - firstpiece - 1), regex("\\s"), "+") + ")";
			str.erase(0, lastpiece + 1);
			firstpiece = str.find("["); lastpiece = str.find("]");
		}
		out += str;
		return out;
	}

	void doShit(vector<string> args, SleepyDiscord::Message msg) {
		args.erase(args.begin());
		if(args.size() == 0) cli->sendMessage(msg.channelID, identify());
		buffer = "";
		args[0].insert(0, "http://api.urbandictionary.com/v0/define?term=");

		curl_easy_setopt(hnd, CURLOPT_URL, strJoin(args, "+").c_str());

		ret = curl_easy_perform(hnd);

		if(ret == CURLE_OK) {
			query.Parse(buffer.c_str());
			if(query["list"].GetArray().Size() == 0) { sendError(msg.channelID); return; }

			SleepyDiscord::Embed em;
			SleepyDiscord::EmbedField fe;

			fe.name = ":thumbsdown: " + to_string(query["list"][index]["thumbs_down"].GetInt());
			fe.value = ":thumbsup: " + to_string(query["list"][index]["thumbs_up"].GetInt());
			em.fields.push_back(fe);
			em.thumbnail.url = "https://cdn.discordapp.com/avatars/656661249639186432/19946d7323b3d82f5e132f620a7739b7.png?size=4096";
			em.description = hotlink(query["list"][index]["definition"].GetString());
			em.timestamp = query["list"][index]["written_on"].GetString();
			em.title = "**" + (string)query["list"][index]["word"].GetString() + "**";
			em.color = 2012816;
			em.url = query["list"][index]["permalink"].GetString();

			last = cli->sendMessage(msg.channelID, "", em);
			return;
		}
		sendError(msg.channelID);
	}

	Urban() {
		names = { "urban", "slang" };
		description = "Looks up a term on Urban Dictionary";
		returns = false;

		// urban dictionary api shit
		curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
		curl_easy_setopt(hnd, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, populateBuffer);
		curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &buffer);
	}
};

class Bot : public SleepyDiscord::DiscordClient {
	public:
		using SleepyDiscord::DiscordClient::DiscordClient;

		void onServer(SleepyDiscord::Server guild) override {
			server = guild;
			cli->updateStatus(prefix + "help", 0, SleepyDiscord::Status::idle, false);
		}

		void onQuit() override {
			for(auto& item : commands) {
				item->names.clear();
				delete item;
			}
			commands.clear();
			curl_easy_cleanup(hnd);
			curl_global_cleanup();
			delete cli;
		}

		void onError(SleepyDiscord::ErrorCode errorCode, const string errorMessage) override {
			cout << endl << errorMessage;
			onQuit();
		}

		void onMessage(SleepyDiscord::Message msg) override {
			transform(msg.content.begin(), msg.content.end(), msg.content.begin(), tolower);
			if(msg.startsWith(prefix)) {
				vector<string> parts = strSplit(msg.content.substr(prefix.length()), " ");
				for(auto& com : commands) if(find(com->names.begin(), com->names.end(), parts[0]) != com->names.end()) {
					if(com->returns) cli->sendMessage(msg.channelID, com->reply(parts, msg));
					else com->doShit(parts, msg);
				}
			}
		}
};

void puppet() {
	string line, channel;
	SleepyDiscord::Channel* dest = nullptr;
	while(commands.size() > 0) {
		line = ""; channel = ""; dest = nullptr;
		cout << endl << "a message good sir? "; getline(cin, line);
		cout << endl << "which channel lad?" << endl;
		for(auto& chan : server.channels) cout << chan.name << endl; cout << endl;
		getline(cin, channel);
		for(auto& chan : server.channels) if(channel.compare(chan.name) == 0) dest = &chan;
		if(dest == nullptr || dest->type == SleepyDiscord::Channel::ChannelType::SERVER_CATEGORY) { cout << endl << "tsk tsk that isn't a channel lad" << endl; continue; }
		cli->sendMessage(dest->ID, line);
		cout << endl << "Message sent!";
	}
}

int main() {

	// Commands
	commands.push_back(new Help());
	commands.push_back(new Hello());
	commands.push_back(new ChangePrefix());
	commands.push_back(new Question());
	commands.push_back(new Lovecalc());
	commands.push_back(new Urban());

	string token;
	cout << "Enter token: "; getline(cin, token);

	cli = new Bot(token);

	thread conc(puppet);

	cli->run();

	conc.join();

	return 0;
}
