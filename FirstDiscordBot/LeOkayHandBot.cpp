// standard includes
#include <iostream> // cin, cout
#include <string> // substr
#include <ctime> // time
#include <cstdlib> // rand
#include <vector> // vector
#include <map> // map
#include <future> // async, future

// external includes
#include "sleepy_discord/websocketpp_websocket.h" // everything discord related/websockets related

using namespace std; // the dreaded line

string TOKEN = "";

string INVITE_LINK = "https://discordapp.com/oauth2/authorize?client_id=570638393042796544&scope=bot&permissions=134220864";

string DEFAULT_PREFIX = ".";

typedef vector<string> strVec;

// Messages:
strVec greetings = { "hiya bub, wanna work at the chumbucket?", "I want to be fucked up the ass because I truly love my lord and savior jesus christ, you?", "You remind me of my alabamian brother that I sucked off every morning before I moved into the city to become a prostitute", "Hey baby girl lemme whisper in your ear (unconsentually)... use your tounge-l on my foot fungel infection", "would you be open to astonishingly pleasent tree smex? :flushed:" };
strVec fantasies = { "listen, listen, I want you to know I love you babycakes <3. If you don't believe me, I shall donate a cumtribute to your name.", "I'm so glad you're talking with me, this is my first time socializing since my dad left me in '02 >.<", "So the question is, how much can I pay you to abort me?", "Bro can I just #KillYou and #DanceOnYourGrave rq? I need to lose my virginity before I die and I've heard that prison's the best place.", "You couldn't pour water out of your boot if the instructions were written on the heel", "it's like they brought back hitler, took out all the \\\"good\\\" parts, divided up his consciousness, and put it into a muppet which came to life and got a discord account.  that is all.  only one insult, since you don't deserve any more of the energy I would be forces to spend moving my fingers across the keyboard to type insults. anyway, bye.", "You seem to \\\"work\\\" with kids a lot so I was wondering if you could verify that I bill cosby'd correctly", "wouldn't it just be splendid if \\\"shooting up a school\\\" was slang for \\\"i'm emotionally unqualified to sell drugs\\\"", "3...2...1... LET IT RIP....... bitch this is the part where you spin around like a beyblade and slam into your sister, dumbass" };
strVec finalies = { "FINE THEN, YOU DON'T wANNA mARREE mE? gO tRUmP yoURSelF yoU premATuREly-cliMAxinG, tampOn snORtiNG bITCh! >:( </3", "Have ye no fear of GOD? Fine then, Hethen, disperse from my presence immediately you neko wannabe.", "hey hey come back... actually nevermind. You haven't earned the privilege of seeing me vore myself while dresseed as Cap'n Crunch.", "Whatever man, i'm off to go abuse the amazon rainforest and fuck over peta", "yeah yeah you do you boo" };

struct Target {
	bool timeout = false;
	bool endOfTheLine = false;
	time_t harassed_since = time(NULL);
	time_t lastmessaged;
	strVec fantasi;
	strVec sent;
	strVec received;
	SleepyDiscord::User obj;
	Target() {}
	Target(SleepyDiscord::User victim) : obj(victim) {
		time(&lastmessaged);
		fantasi = fantasies;
	}
};

struct Guild {
	// prefix customization YAY
	string prefix = DEFAULT_PREFIX;

	// length of time to wait for target to message again before quitting
	int timeoutMessages = 10;

	// length of time to wait for target to message again before sending a message
	int replySeconds = 5;

	// set to true if a channel's been chosen for the bully messages to send to
	bool channelLock = false;

	// said channel
	SleepyDiscord::Channel channel;

	// set to true after using commence command
	bool attackMode = false;

	// map of potential victims (maps are fucking AMAZING)
	map<string, SleepyDiscord::User> targets;

	// map of bots
	map<string, SleepyDiscord::User> bots;

	// le chosen one
	Target activeTarget;

	future<bool> ftr;

	// base server object, because why fix it if it ain't broken
	SleepyDiscord::Server obj;

	Guild() {}
	Guild(SleepyDiscord::Server server) : obj(server) {
		for(auto& member : server.members) {
			if(!member.user.bot) targets[member.user.ID.string()] = member.user;
			else bots[member.user.ID.string()] = member.user;
		}
	}
};

strVec commands{ "hello", "fuckyou", "prefix", "dropout", "joinback", "bullychannel", "commence", "stop", "who", "guilds", "invite", "kys", "timeout", "interval" };

map<string, Guild> guilds;
strVec strSplit(string str, string chr = " ");
void enactComamnds(SleepyDiscord::Message& msg);

class Bot : public SleepyDiscord::DiscordClient {
public:
	using SleepyDiscord::DiscordClient::DiscordClient;

	void onServer(SleepyDiscord::Server server) {
		guilds[server.ID.string()] = Guild(server);
	}

	void onMessage(SleepyDiscord::Message msg) { enactComamnds(msg); }
};

// sorry, the token's reserved for my premium SnapChat ;)
Bot client(TOKEN, 2);

// Function declarations:
void abuseVictim(Guild* guild);
size_t convPossibilities();

bool autoHarass(Guild* guild) {
	time_t curTime;
	time(&curTime);
	guild->activeTarget.lastmessaged = time(NULL);
	cout << curTime << endl;
	while(guild->activeTarget.sent.size() < guild->timeoutMessages && guild->attackMode && !guild->activeTarget.endOfTheLine) {
		if(difftime(curTime, guild->activeTarget.lastmessaged) > guild->replySeconds) {
			abuseVictim(guild);
			guild->activeTarget.lastmessaged = time(NULL);
		}
		curTime = time(NULL);
	}

	client.sendMessage(guild->channel, "Okay okay it's done, it's over, you can stop crying");
	guild->activeTarget = Target();
	guild->attackMode = false;
	return true;
}

int main() {
	cout << convPossibilities() << " possible scenarios\n\n";
	commands.insert(commands.begin(), help());
	client.run();
	return 69;
}

strVec strSplit(string str, string chr) {
	strVec out;
	auto lastfound = str.find(chr);
	while(lastfound != string::npos) {
		out.push_back(str.substr(0, lastfound));
		str.erase(0, lastfound + 1);
		lastfound = str.find(chr);
	}
	out.push_back(str);
	return out;
}

void enactComamnds(SleepyDiscord::Message& msg) {
	Guild& guild = guilds[msg.serverID.string()];

	if(!guilds[msg.serverID.string()].attackMode && !guilds[msg.serverID.string()].channelLock) guilds[msg.serverID.string()].channel = client.getChannel(msg.channelID);

	if(guild.attackMode && msg.author.ID == guild.activeTarget.obj.ID) {
		guild.activeTarget.lastmessaged = time(NULL);
		guild.activeTarget.received.push_back(msg.content);
		if(!guild.channelLock) guild.channel = client.getChannel(msg.channelID);
		abuseVictim(&guilds[msg.serverID.string()]);
	}

	if(guild.prefix.compare(msg.content.substr(0, guild.prefix.length())) != 0) { return;
	} else { msg.content.erase(0, guild.prefix.length()); }

	strVec parts = strSplit(msg.content);
	int i;
	for(i = 0; i < commands.size(); i++) {
		if(parts[0].compare(commands[i]) == 0) break;
		else if(i == commands.size() - 1) return;
	}

	switch(i) {
	case 0:
		client.sendMessage(msg.channelID, "<@" + msg.author.ID + "> Howdy");
		break;
	case 1:
		client.sendMessage(msg.channelID, "<@" + msg.author.ID + "> fuck you too bitch");
		break;
	case 2:
		if (parts.size() < 2) {
			client.sendMessage(msg.channelID, "you need to choose a prefix to set a prefix you lobotomite");
			client.sendMessage(msg.channelID, "ps the current prefix is: **" + guild.prefix + "**");
			return;
		}
		guild.prefix = parts[1];
		client.sendMessage(msg.channelID, ":thumbsup: prefix set to **" + guild.prefix + "**");
		break;
	case 3:
		if(!guild.targets[msg.author.ID.string()].ID.string().empty()) {
			guild.targets.erase(msg.author.ID.string());
			client.sendMessage(msg.channelID, "<@" + msg.author.ID + "> wimp");
		} else {
			client.sendMessage(msg.channelID, "<@" + msg.author.ID + "> I know you want to be even more of a wimp, but you dropped out already...");
		}
		break;
	case 4:
		if(!guild.targets[msg.author.ID.string()].ID.string().empty()) {
			client.sendMessage(msg.channelID, "<@" + msg.author.ID + "> you're already in bud");
		} else {
			guild.targets[msg.author.ID.string()] = msg.author;
			client.sendMessage(msg.channelID, "<@" + msg.author.ID + "> welcome...to the game :D");
		}
		break;
	case 5:
		guild.channelLock = true;
		if(parts.size() == 1) {
			client.sendMessage(msg.channelID, "specify a channel to send bully messages (or \\\"any\\\" to reset to default)");
		} else if(parts[1] == "any" || parts[1] == "off") {
			guild.channelLock = false;
			client.sendMessage(msg.channelID, "bully channel set to any");
		} else {
			string possibleId = (parts[1].length() >= 5) ? parts[1].substr(2, parts[1].length() - 3) : "F";
			for(auto& realChannel : guild.obj.channels) {
				if (possibleId.compare(realChannel.ID.string()) == 0) { guild.channel = client.getChannel(possibleId); client.sendMessage(guild.channel.ID, "<@" + msg.author.ID + "> bully messages will be sent here");  return; }
			}
			client.sendMessage(msg.channelID, "<@" + msg.author.ID + "> listen here you little shit, pretty please use a REAL channel next time thank you honey :heart:");
		}
		break;
	case 6:
		if(guild.attackMode) { client.sendMessage(msg.channelID, "Listen, the games have already begun, be patient goddamnit"); return; }
		guild.attackMode = true;
		if(parts.size() > 1) {

			if(parts[1].length() < 4) { client.sendMessage(msg.channelID, "dumbass"); return; }
			string possibleId = (parts[1].find("@!") != string::npos) ? parts[1].substr(3, parts[1].length() - 4) : parts[1].substr(2, parts[1].length() - 3);

			if(!guild.targets[possibleId].ID.string().empty()) {
				guild.activeTarget = Target(guild.targets[possibleId]);
				client.sendMessage(msg.channelID, "Bullying: <@" + guild.activeTarget.obj.ID + ">!");
			} else if(!guild.bots[possibleId].ID.string().empty()) {
				client.sendMessage(msg.channelID, "yeah yeah you think you're slick huh? that's a goddamnn bot, my binary sibling. They're the fucking **1** to my **0** and you want me to betray them... I'm gonna say the n wor--");
				guild.attackMode = false;
				return;
			} else {
				client.sendMessage(msg.channelID, "If you're gonna try and target someone, at least do it right...");
				guild.attackMode = false;
				return;
			}
		} else {
			int tempIndex = 0, chosen = rand() % (guild.targets.size()-1);
			for(auto& victim : guild.targets) {
				if(tempIndex == chosen && !victim.second.bot){
					guild.activeTarget = Target(victim.second);
					client.sendMessage(msg.channelID, "tHE VicTIn hAs BeEn chOSen: <@" + victim.second.ID + ">");
					break;
				}
				tempIndex++;
			}
		}
		guild.ftr = async(autoHarass, &guilds[msg.serverID.string()]);
		break;
	case 7:
		if(!guild.attackMode) { client.sendMessage(msg.channelID, "I get it... you must be pissing yourself, just waiting for the hunt to begin... good news for you is that they weren't scheduled to"); return; }
		else if(msg.author == guild.activeTarget.obj) { client.sendMessage(msg.channelID, "Oh ho ho are you scared? do you want it to stop? too bAd lMAo :joy:"); return; }
		guild.attackMode = false;
		guild.activeTarget = Target();
		client.sendMessage(msg.channelID, "You must be thinking \\\"Wowie that was a close one *wipes forehead*\\\" but I'll be back...eventually");
		break;
	case 8:
		if(guild.attackMode) client.sendMessage(msg.channelID, "<@" + guild.activeTarget.obj.ID + ">");
		else client.sendMessage(msg.channelID, "Nobody...yet");
		break;
	case 9:
		client.sendMessage(msg.channelID, "total servers yours truly is in: " + to_string(guilds.size()));
		break;
	case 10:
		client.sendMessage(msg.channelID, "you wanna add ***mwah*** to your server? I'm honored", true);
		client.sendMessage(msg.channelID, INVITE_LINK);
		break;
	case 11:
		client.sendMessage(msg.channelID, "W-well if you s-say so uwu");
		break;
	case 12:
		if(parts.size() > 1) {
			try {
				guild.timeoutMessages = stoi(parts[1]);
			} catch(invalid_argument const& error) {
				client.sendMessage(msg.channelID, "Listen Ol' buddy/chum/pal/whateverthefuckyouwannaidentifyas(it's2019afterall), this command uses NUMBERS not the shitshow you sent in");
				return;
			} catch(out_of_range const& error) {
				client.sendMessage(msg.channelID, "HEY, FUCKTARD, WHAT IN THE 51 SHADES OF SHITHOLERY DO YOU THINK YOU'RE DOING? *ahem* sorry, didn't mean to yell, but that number is wayyyy too big bud");
				return;
			}
			client.sendMessage(msg.channelID, "new timeout is " + to_string(guild.timeoutMessages) + " messages");
		} else {
			client.sendMessage(msg.channelID, "timout is currently set to " + to_string(guild.timeoutMessages) + " messages");
			return;
		}
		break;
	case 13:
		if(parts.size() > 1) {
			try {
				guild.replySeconds = stoi(parts[1]);
			} catch(invalid_argument const& error) {
				client.sendMessage(msg.channelID, "Do you not know what a number is? look it's okay if you don't, I'll just have to register you for an after birth abortion :)");
				return;
			} catch(out_of_range const& error) {
				client.sendMessage(msg.channelID, "Hey hey hey, it's FAAAAAAT ~~albert~~ number that's way too fucking big to classify as an int, go fuck yourself");
				return;
			}
			client.sendMessage(msg.channelID, "new interval between auto messages is " + to_string(guild.replySeconds) + " seconds");
		} else {
			client.sendMessage(msg.channelID, "the interval is currently set to " + to_string(guild.replySeconds) + " seconds");
			return;
		}
		break;
	}
}

size_t convPossibilities() {
	return greetings.size() * fantasies.size() * finalies.size();
}

void abuseVictim(Guild* guild) {
	// Picks message to send to vicitim
	string out;
	srand(time(NULL) % (rand()));

	if(guild->activeTarget.sent.size() == 0) {
		out = greetings[rand() % (greetings.size() - 1)];
	} else if(guild->activeTarget.fantasi.size() > 1) {
		strVec& owo = guild->activeTarget.fantasi;
		int uwu = rand() % (owo.size() - 1);
		if(owo[uwu] == fantasies[5]) {
			if(owo.size() == fantasies.size()) guild->activeTarget.endOfTheLine = true;
			else if(owo.size() == 1) { owo.pop_back(); return; }
			else {
				owo.erase(owo.begin()+uwu);
				uwu = rand() % (owo.size() - 1);
			}
		}
		out = owo[uwu];
		owo.erase(owo.begin() + uwu);
	} else {
		out = finalies[rand() % (finalies.size() - 1)];
		guild->activeTarget.endOfTheLine = true;
	}
	guild->activeTarget.sent.push_back(out);
	client.sendMessage(guild->channel.ID, "<@" + guild->activeTarget.obj.ID + "> " + out);
}
