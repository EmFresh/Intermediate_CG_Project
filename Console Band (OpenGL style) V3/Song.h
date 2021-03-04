#pragma once
/*I see you found my easter egg ;)
* this is an opengl version of my initial project console band 
* you wont see any notes for a little while
* hope you enjoy
*/

#include <functional>
#include <thread>
#include <filesystem>
#include <cwchar>
#include <cmath>
#include <midifile/MidiFile.h>
#include <GameEmGine.h>
#include <EmGineAudioPlayer.h>

#pragma region using / Namespace Stuff
using std::vector;
using std::string;
using std::wstring;
using std::pair;
using std::function;
using std::thread;
using std::stoi;
namespace fs = std::filesystem;
#pragma endregion

typedef GameEmGine Game;

wstring cDir(wstring dir)
{
	for(auto& a : dir)
		if(a == L'\\')
			a = L'/';

	const wchar_t* tmp;
	if(dir.length() > 1)
		while(bool(tmp = wcsstr(dir.c_str(), L"//")))
		{
			memmove_s((wchar_t*)tmp, sizeof(wchar_t) * wcslen(tmp), tmp + 1, sizeof(wchar_t) * wcslen(tmp + 1));
			dir[wcslen(dir.c_str()) - 1] = 0;
		}

	return dir;
}

#define HEALTH_CAP 25
class Song:public Scene
{
#pragma region Structs
	struct Fret
	{
		Fret() = default;
		Fret(float a, int b, bool c) { pos = a, duration = b, hammerOn = c; };

		float pos;
		int duration;
		bool hammerOn;
		Model* note = nullptr;
	};
#pragma endregion

	void reset()
	{

		AudioPlayer::stopAll();
		lastHealth = currentHealth = (HEALTH_CAP / 2) + 1;
		for(auto& a : colliCountGuitar)a = 0;
		for(auto& a : countAmountGuitar)a = 0;
		for(auto& a : colliCountBass)a = 0;
		for(auto& a : countAmountBass)a = 0;
		for(auto& a : colliCountDrum)a = 0;
		for(auto& a : countAmountDrum)a = 0;
		for(auto& a : *disiNotes)
			a.clear();

		firstLyric = 0;

		for(auto& a : fs::directory_iterator(songDir))
		{
			wstring ogg = cDir(a.path());
			if(ogg.substr(ogg.find_last_of('.')) == L".ogg")
				if(ogg.substr(ogg.find_last_of('/') + 1, ogg.find_last_of('.') - ogg.find_last_of('/') - 1) != L"preview")
					AudioPlayer::createAudioStream(string(ogg.begin(), ogg.end()).c_str());
		}

		for(int a = 0; a < (int)guitarTrackTmp->size(); a++)
			guitarTrackTmp[0][a].clear();

		guitarTrackTmp[0] = guitarTrack[0];
		//bassTrackTmp->clear(),
		//	*bassTrackTmp = *bassTrack;
		//drumTrackTmp->clear(),
		//	*drumTrackTmp = *drumTrack;

		barCount = notesHit = 0;

		AudioPlayer::playAll();

		AudioPlayer::setMasterVolume(.05f);
		if(AudioPlayer::size())
			incriment = (float)AudioPlayer::getPosition(0);
		start = true;
	}

#pragma region File I/O

	void openSong(string songFolder = "songs/TTFAF/")
	{

		smf::MidiFile file;
		file.read((songDir = songFolder) + "notes.mid");

		if(!file.status()) { OutputDebugStringA("Song failed to load"); return; }

		file.linkNotePairs();
		file.doTimeAnalysis();
		string trackStr;

		for(auto& a : *guitarTrack)
			a.clear();
		for(auto& a : *bassTrack)
			a.clear();
		for(auto& a : *drumTrack)
			a.clear();


		lyrics->clear();
		lyricTiming->clear();
		vocalTrack->clear();

		//Sprite lyricBox("Game Files/Lyrics Box.txt");

		vocalTrack->assign(120, vector<pair<long, long>>());

		for(int a = 0; a < file.size(); a++)
			for(int b = 0; b < file[a].size(); b++)
			{

				int tempo = 0;

				if(file[a][b].isTrackName())
				{
					trackStr = string(file[a][b].begin() + 3, file[a][b].end());
					//if(trackStr == "PART VOCALS")
					//	continue;
					//if(trackStr == "PART GUITAR")
					//	continue;
					//if(trackStr == "PART BASS")
					//	continue;
					//if(trackStr == "PART DRUMS")
					//	continue;
					//
					//break;
				}

				if(file[a][b].isTimeSignature())
				{
					//	tempo = file[a][b].getTempoSeconds();
					tempo;
				}

				if(file[a][b].isMetaMessage())
				{
					tempo = file[a][b].getMetaType();
					tempo;
				}

				//if(trackStr == "PART VOCALS")
				//{
				//
				//	if(file[a][b].isText())
				//	{
				//		if(file[a][b][3] == '[')
				//			continue;
				//
				//		if(lyricTiming->size())
				//		{
				//
				//			unsigned int timeInSec = (unsigned int)(file.getTimeInSeconds(a, b) * 1000);
				//			if(file[a][b].size() <= 3)
				//				continue;
				//			if((file[a][b][3] != '+' && [&]()->unsigned int
				//				{
				//					int b = 0; for(auto& a : lyrics[0].back())b += a.size(); return b;
				//				}() + string(file[a][b].begin() + 3, file[a][b].end()).size() + 2 < /*(unsigned int)lyricBox.getWidth() - 1*/5) &&
				//				(timeInSec - lyricTiming[0].back().back().second < (unsigned int)750))
				//
				//				lyrics[0].back().push_back(" " + string(file[a][b].begin() + 3, file[a][b].end())),
				//					lyricTiming[0].back().push_back({file.getTimeInSeconds(a, b) * 1000, file.getTimeInSeconds(a, b) * 1000});
				//			else
				//			{
				//				lyrics->push_back(vector<string>());
				//				lyricTiming->push_back(vector<pair<unsigned int, unsigned int>>());
				//
				//				if(lyrics[0][lyrics->size() - 2].size())
				//					if(lyrics[0][lyrics->size() - 2].back().size())
				//						if(lyrics[0][lyrics->size() - 2].back().back() == '-')
				//						{
				//							function<void()> sylableGroup = [&]()
				//							{
				//								lyricTiming[0].back().insert(lyricTiming[0].back().begin(), lyricTiming[0][lyricTiming[0].size() - 2].back()),
				//									lyrics[0].back().insert(lyrics[0].back().begin(), lyrics[0][lyrics[0].size() - 2].back());
				//
				//								lyricTiming[0][lyricTiming[0].size() - 2].pop_back(),
				//									lyrics[0][lyrics[0].size() - 2].pop_back();
				//
				//								if(lyricTiming[0].size() - 2 > -1)
				//								{
				//									lyricTiming[0][lyricTiming[0].size() - 2].back().second = (unsigned int)(file.getTimeInSeconds(a, b) * 1000);
				//
				//									if(lyrics[0][lyrics->size() - 2].back().back() == L'-')
				//										sylableGroup();
				//								}
				//							};
				//
				//							sylableGroup();
				//
				//							lyrics->back().push_back(string(file[a][b].begin() + 3, file[a][b].end()));
				//							lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,
				//														  file.getTimeInSeconds(a, b) * 1000});
				//
				//							//removes the '+'s from the song 
				//							if(lyrics->back().back().back() == L'+')
				//								lyrics->back().pop_back();
				//
				//							continue;
				//						}
				//
				//				lyricTiming[0][lyricTiming[0].size() - 2].back().second = (unsigned int)(file.getTimeInSeconds(a, b) * 1000);
				//
				//				lyrics->back().push_back(string(file[a][b].begin() + 3, file[a][b].end()));
				//
				//				lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,file.getTimeInSeconds(a, b) * 1000});
				//			}
				//
				//		}
				//		else
				//		{
				//			lyrics->push_back(vector<string>());
				//			lyricTiming->push_back(vector<pair<unsigned int, unsigned int>>());
				//
				//			lyrics->back().push_back(string(file[a][b].begin() + 3, file[a][b].end()));
				//			lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,
				//										  file.getTimeInSeconds(a, b) * 1000});
				//		}
				//
				//		//removes the '+'s from the song 
				//		if(lyrics->back().size())
				//			if(lyrics->back().back().back() == L'+')
				//				lyrics->back().pop_back();
				//
				//		//the '^' is used to define spoken words
				//		if(lyrics->back().size())
				//			if(lyrics[0].back().back().back() == '^')
				//				lyrics[0].back().back().pop_back();
				//
				//		//the '=' I think is just some sorta typo
				//		if(lyrics->back().size())
				//			if(lyrics[0].back().back().back() == '=')
				//				lyrics[0].back().back().back() = '-';
				//
				//		//the '#' I don't know what it's used for
				//		if(lyrics->back().size())
				//			if(lyrics[0].back().back().back() == '#')
				//				lyrics[0].back().back().pop_back();
				//
				//		continue;
				//	}
				//
				//	if(file[a][b].isLyricText())
				//	{
				//		if(lyricTiming->size())
				//		{
				//
				//			unsigned int timeInSec = (unsigned int)(file.getTimeInSeconds(a, b) * 1000);
				//			if(file[a][b].size() <= 3)
				//				continue;
				//			if((file[a][b][3] != '+' && [&]()->unsigned int
				//				{
				//					int b = 0; for(auto& a : lyrics[0].back())b += a.size(); return b;
				//				}() + string(file[a][b].begin() + 3, file[a][b].end()).size() + 2 < /*(unsigned int)lyricBox.getWidth() - 1*/5) &&
				//				(timeInSec - lyricTiming[0].back().back().second < (unsigned int)750))
				//
				//				lyrics[0].back().push_back(" " + string(file[a][b].begin() + 3, file[a][b].end())),
				//					lyricTiming[0].back().push_back({file.getTimeInSeconds(a, b) * 1000, file.getTimeInSeconds(a, b) * 1000});
				//			else
				//			{
				//				lyrics->push_back(vector<string>());
				//				lyricTiming->push_back(vector<pair<unsigned int, unsigned int>>());
				//
				//				if(lyrics[0][lyrics->size() - 2].size())
				//					if(lyrics[0][lyrics->size() - 2].back().size())
				//						if(lyrics[0][lyrics->size() - 2].back().back() == '-')
				//						{
				//							function<void()> sylableGroup = [&]()
				//							{
				//								lyricTiming[0].back().insert(lyricTiming[0].back().begin(), lyricTiming[0][lyricTiming[0].size() - 2].back()),
				//									lyrics[0].back().insert(lyrics[0].back().begin(), lyrics[0][lyrics[0].size() - 2].back());
				//
				//								lyricTiming[0][lyricTiming[0].size() - 2].pop_back(),
				//									lyrics[0][lyrics[0].size() - 2].pop_back();
				//
				//								if(lyricTiming[0].size() - 2 > -1)
				//								{
				//									lyricTiming[0][lyricTiming[0].size() - 2].back().second = (unsigned int)(file.getTimeInSeconds(a, b) * 1000);
				//
				//									if(lyrics[0][lyrics->size() - 2].back().back() == L'-')
				//										sylableGroup();
				//								}
				//							};
				//
				//							sylableGroup();
				//
				//							lyrics->back().push_back(string(file[a][b].begin() + 3, file[a][b].end()));
				//							lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,
				//														  file.getTimeInSeconds(a, b) * 1000});
				//
				//							//removes the '+'s from the song 
				//							if(lyrics->back().back().back() == L'+')
				//								lyrics->back().pop_back();
				//
				//							continue;
				//						}
				//
				//				lyricTiming[0][lyricTiming[0].size() - 2].back().second = (unsigned int)(file.getTimeInSeconds(a, b) * 1000);
				//
				//				lyrics->back().push_back(string(file[a][b].begin() + 3, file[a][b].end()));
				//
				//				lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,file.getTimeInSeconds(a, b) * 1000});
				//			}
				//
				//		}
				//		else
				//		{
				//			lyrics->push_back(vector<string>());
				//			lyricTiming->push_back(vector<pair<unsigned int, unsigned int>>());
				//
				//			lyrics->back().push_back(string(file[a][b].begin() + 3, file[a][b].end()));
				//			lyricTiming->back().push_back({file.getTimeInSeconds(a, b) * 1000,
				//										  file.getTimeInSeconds(a, b) * 1000});
				//		}
				//
				//		//removes the '+'s from the song 
				//		if(lyrics->back().size())
				//			if(lyrics->back().back().back() == L'+')
				//				lyrics->back().pop_back();
				//
				//		//the '^' is used to define spoken words
				//		if(lyrics->back().size())
				//			if(lyrics[0].back().back().back() == '^')
				//				lyrics[0].back().back().pop_back();
				//
				//		//the '=' I think is just some sorta typo
				//		if(lyrics->back().size())
				//			if(lyrics[0].back().back().back() == '=')
				//				lyrics[0].back().back().back() = '-';
				//
				//		//the '#' I don't know what it's used for
				//		if(lyrics->back().size())
				//			if(lyrics[0].back().back().back() == '#')
				//				lyrics[0].back().back().pop_back();
				//
				//		continue;
				//	}
				//
				//	if(file[a][b].isNoteOn())
				//	{
				//		int key = file[a][b].getKeyNumber();
				//		(*vocalTrack)[key].push_back({file.getTimeInSeconds(a, b) * 1000,0});
				//		continue;
				//	}
				//
				//	if(file[a][b].isNoteOff())
				//	{
				//		int key = file[a][b].getKeyNumber();
				//		(*vocalTrack)[key][vocalTrack->at(key).size() - 1].second = (unsigned int)(file.getTimeInSeconds(a, b) * 1000);
				//		continue;
				//	}
				//	continue;
				//}

				if(instrument == "guitar")
					if(trackStr == "PART GUITAR")
					{
						if(file[a][b].isNoteOn())
						{
							int key = file[a][b].getKeyNumber();

							//gets solo section
							if(key == 12 * 8 + 7)
								NULL;
							if(key / 12 == difficulty)
								if(key % 12 < 5)
								{
									int duration = 0;
									(*guitarTrack)[key % 12].push_back({(float)(file.getTimeInSeconds(a, b) * 1000),duration = file[a][b].getTickDuration(),bool(file[a][b][2] % 2)});

									if((*guitarTrack)[key % 12].back().hammerOn)
										OutputDebugStringA("I guess that Worked?\n");
								}
							continue;
						}
					}

				if(instrument == "rhythm")
					if(trackStr == "PART BASS")
					{
						if(file[a][b].isNoteOn())
						{
							int key = file[a][b].getKeyNumber();
							int duration = 0;

							if(key / 12 == difficulty)
								if(key % 12 < 5)
									(*bassTrack)[key % 12].push_back({(float)(file.getTimeInSeconds(a, b) * 1000),duration = file[a][b].getTickDuration(),bool(file[a][b][2] % 2)});

							continue;
						}
					}

				if(instrument == "drum")
					if(trackStr == "PART DRUMS")
					{
						if(file[a][b].isNoteOn())
						{
							int key = file[a][b].getKeyNumber();
							if(key / 12 == difficulty)
								if(key % 12 < 5)
									(*drumTrack)[(key % 12)].push_back((long)(file.getTimeInSeconds(a, b) * -1000));

							continue;
						}
					}
			}
		file.clear();


		for(unsigned int a = 0; a < lyrics->size(); a++)
		{
			if(!lyrics[0][a].size())//get rid of empty vectors
				lyrics->erase(lyrics->begin() + a),
				lyricTiming->erase(lyricTiming->begin() + a),
				a--;

			if(a)//create blank spaces in long pauses
			{
				if(a < lyrics->size())
					if(!lyrics[0][a].size())
						continue;

				if(lyricTiming[0][a].front().first - lyricTiming[0][a - 1].back().first > 2000)
					lyricTiming[0][a - 1].back().second = lyricTiming[0][a - 1].back().first + 1200;
			}
		}

		if(lyricTiming->size())
			lyricTiming->back().back().second += 750;

	#pragma region Vocal Information
		for(unsigned int a = 0; a < vocalTrack->size();)
			if((120 - vocalTrack->size()) / 12 > 1)
				if((*vocalTrack)[a].size() == 0)
					vocalTrack->erase(vocalTrack->begin());
				else
					break;
			else
				vocalTrack->erase(vocalTrack->begin());

		int diff = 120 - vocalTrack->size();

		for(int a = vocalTrack->size() - 1; a >= 0; a = vocalTrack->size() - 1)
			if((120 - (diff - vocalTrack->size())) / 12 < 8)
				if((*vocalTrack)[a].size() == 0)
					vocalTrack->erase(vocalTrack->end() - 1);
				else
					break;
			else
				vocalTrack->erase(vocalTrack->end() - 1);
	#pragma endregion

		lyrics;
		lyricTiming;
		guitarTrack;
		//AudioPlayer::stopAll();
		//for(auto &a : fs::directory_iterator(songDir))
		//{
		//	string ogg = cDir(a.path());
		//
		//	if(ogg.substr(ogg.find_last_of('.') + 1) == L"ogg")
		//		if(ogg.substr(ogg.find_last_of('/')+1, ogg.find_last_of('.') - ogg.find_last_of('/')-1) != L"preview")
		//			AudioPlayer::createAudioStream(string(ogg.begin(), ogg.end()).c_str());
		//}

		reset();

	}
#pragma endregion	

	bool moveLeft, moveRight, moveForward, moveBack, moveUp, moveDown,
		rotLeft, rotRight, rotUp, rotDown,
		tab = false;


	void init()
	{
		setSkyBox("Skyboxes/skybox/");

		//note = new Model("Models/Note/note.obj", "Note");
		//Game::getMainCamera()->rotate(-45, {1,0,0});
		Game::getMainCamera()->translate({note->getWidth() * 2,0,-4});

		Game::getMainCamera()->enableFPSMode(true);
		Game::setBackgroundColour(0, 0, 0);
		//FrustumPeramiters frustum{55,(float)Game::getWindowWidth() / Game::getWindowHeight(),0.001f,500};
		//Game::setCameraType(Camera::FRUSTUM, &frustum);

		for(int a = 0; a < 5; a++)
			Game::addModel(&(fretBoard[a] = Model(*note))),
			(fretBoard)[a].setColour((*fretColour)[a] / 2),
			(fretBoard)[a].enableFPSMode(),
			(fretBoard)[a].rotate({45,0,0}),

			//change model location
			(fretBoard)[a].translate(note->getWidth() * a, 0, 0),
			LightManager::addLight(fretLit + a),
			fretLit[a].setParent(fretBoard + a);


		openSong();

		keyPressed =
			[&](int key, int mod)->void
		{
			if(key == GLFW_KEY_R)
			{
				Game::getMainCamera()->reset();
				Game::setCameraPosition({0,0,-5});
			}

			if(key == GLFW_KEY_F5)
				Shader::refresh();

			static bool sky = false;
			if(key == GLFW_KEY_SPACE)
				enableSkyBox(sky = !sky);


			if(key == 'A')
				moveLeft = true;

			if(key == 'D')
				moveRight = true;

			if(key == 'W')
				moveForward = true;

			if(key == 'S')
				moveBack = true;

			if(key == 'Q')
				moveDown = true;

			if(key == 'E')
				moveUp = true;


			if(key == GLFW_KEY_LEFT)
				rotLeft = true;

			if(key == GLFW_KEY_RIGHT)
				rotRight = true;

			if(key == GLFW_KEY_UP)
				rotUp = true;

			if(key == GLFW_KEY_DOWN)
				rotDown = true;
		};

		keyReleased =
			[&](int key, int mod)->void
		{
			if(key == 'A')
				moveLeft = false;

			if(key == 'D')
				moveRight = false;

			if(key == 'W')
				moveForward = false;

			if(key == 'S')
				moveBack = false;

			if(key == 'Q')
				moveDown = false;

			if(key == 'E')
				moveUp = false;


			if(key == GLFW_KEY_LEFT)
				rotLeft = false;

			if(key == GLFW_KEY_RIGHT)
				rotRight = false;

			if(key == GLFW_KEY_UP)
				rotUp = false;

			if(key == GLFW_KEY_DOWN)
				rotDown = false;
		};

	}

	void cameraMovement()
	{
		// Movement
		if(moveLeft)
			Game::translateCameraBy({-.05f,0,0});
		if(moveRight)
			Game::translateCameraBy({.05f,0,0});
		if(moveForward)
			Game::translateCameraBy({0,0,.05f});
		if(moveBack)
			Game::translateCameraBy({0,0,-.05f});
		if(moveUp)
			Game::translateCameraBy({0,.05f,0});
		if(moveDown)
			Game::translateCameraBy({0,-.05f,0});

		// Rotation
		if(rotLeft)
			Game::rotateCameraBy({0,-.5f,0});
		if(rotRight)
			Game::rotateCameraBy({0,.5f,0});
		if(rotUp)
			Game::rotateCameraBy({.5f,0,0});
		if(rotDown)
			Game::rotateCameraBy({-.5f,0,0});
	}

	void update(double dt)
	{
		cameraMovement();
		if(AudioPlayer::size())
			incriment = (float)AudioPlayer::getPosition(0);

		playSongMovement();
		playButtonPress();
	}

	void playSongMovement()
	{
		//Note Movement
		if(instrument == "guitar")
			for(short a = 0; a < (short)guitarTrackTmp->size(); a++)
			{
				for(int b = countAmountGuitar[a]; b < (int)(*guitarTrackTmp)[a].size(); b++)
				{

					// moves the song along
					(*guitarTrackTmp)[a][b].pos = ((*guitarTrack)[a][b].pos / noteSpeed - incriment / noteSpeed + fretboardPosition);

					//stop checking if nothing can be seen
					if((*guitarTrackTmp)[a][b].pos > 20)
						break;

					//initialize note model
					if(!(*guitarTrackTmp)[a][b].note)
						Game::addModel((*guitarTrackTmp)[a][b].note = new Model(*note)),
						(*guitarTrackTmp)[a][b].note->setColour((*fretColour)[a]),
						(*guitarTrackTmp)[a][b].note->enableFPSMode(),
						(*guitarTrackTmp)[a][b].note->rotate({45,0,0});

					//change model location
					(*guitarTrackTmp)[a][b].note->translate(note->getWidth() * a, 0, (float)(*guitarTrackTmp)[a][b].pos);

					if(b > countAmountGuitar[a])
					{
						//note drop off (removes notes from draw list)
						if((*guitarTrackTmp)[a][b - 1].pos + noteOffset < 0)
							countAmountGuitar[a]++,
							Game::removeModel((*guitarTrackTmp)[a][b - 1].note),
							delete (*guitarTrackTmp)[a][b - 1].note,
							(*guitarTrackTmp)[a][b - 1].note = nullptr;

						if(((*guitarTrackTmp)[a][b - 1].pos >= (abs((*guitarTrackTmp)[a][b - 1].pos) - abs((*guitarTrackTmp)[a][b].pos))))//if the note below is off screen? (actually idk)
							if((*disiNotes)[a].size() > 0)
								if((unsigned int)b == (*disiNotes)[a][0])
									(*disiNotes)[a].erase((*disiNotes)[a].begin());
					}

					//if(!invisable(a, b))
					//	if((*guitarTrackTmp)[a][b].pos + noteOffset < 20 && (*guitarTrackTmp)[a][b].pos + noteOffset >= 0)
					//		if((*guitarTrackTmp)[a][b].pos + noteOffset < fretboardPosition + note->getDepth())//draw notes above the fret board
					//		{
					//			///Normal
					//			con->toConsoleBuffer((*notes)[(*guitarTrackTmp)[a][b].hammerOn], (centerTrack)+(a * 13 + 2), (*guitarTrackTmp)[a][b].pos + noteOffset, (*fretNoteColour)[a]);
					//		}
					//		else//draw notes below the fret board
					//			con->toConsoleBuffer((*notes)[(*guitarTrackTmp)[a][b].hammerOn], (centerTrack)+(a * 13 + 2), (*guitarTrackTmp)[a][b].pos + noteOffset, FG_RED);
				}
			}
		//else if(instrument == "rhythm")
		//	for(short a = 0; a < (short)bassTrackTmp->size(); a++)
		//	{
		//		for(int b = countAmountBass[a]; b < (int)(*bassTrackTmp)[a].size(); b++)
		//		{
		//
		//			// moves the song along
		//			(*bassTrackTmp)[a][b] = (*bassTrack)[a][b] / noteSpeed + incriment / noteSpeed + fretboardPosition;
		//
		//			//stop checking if nothing can be seen
		//			if((*bassTrackTmp)[a][b] <= -3)
		//				break;
		//
		//			if(b > countAmountBass[a])
		//			{
		//				//note drop off
		//				if((*bassTrackTmp)[a][b - 1] + noteOffset < con->getHeight())
		//					countAmountBass[a]++;
		//
		//				if(((*bassTrackTmp)[a][b - 1] >= (abs((*bassTrackTmp)[a][b]) - abs((*bassTrackTmp)[a][b - 1]))))//if the note below is off screen? (actually idk)
		//					if((*disiNotes)[a].size() > 0)
		//						if((unsigned int)b == (*disiNotes)[a][0])
		//							(*disiNotes)[a].erase((*disiNotes)[a].begin());
		//
		//			}
		//			if(!invisable(a, b))
		//				if((*bassTrackTmp)[a][b] + noteOffset > -3 && (*bassTrackTmp)[a][b] + noteOffset < con->getHeight())
		//					if((*bassTrackTmp)[a][b] + noteOffset < fretboardPosition + notes[0][0].getHeight())//draw notes above the fret board
		//					{
		//						///Normal
		//						con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*bassTrackTmp)[a][b] + noteOffset, (*fretNoteColour)[a]);
		//
		//						///Collision Test
		//						//con->toConsoleBuffer((*notes)[0], (centerTrack) +(a * 13 + 2), (*songTemp)[a][b] + noteOffset,
		//						//					 b != colliCount[a] ? (*noteColour)[a] : FG_INTENSIFY | FG_WHITE);
		//
		//					}
		//					else//draw notes below the fret board
		//						con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*bassTrackTmp)[a][b] + noteOffset, FG_RED);
		//		}
		//	}
		//else if(instrument == "drum")
		//	for(short a = 0; a < (short)drumTrackTmp->size(); a++)
		//	{
		//		for(int b = countAmountDrum[a]; b < (int)(*drumTrackTmp)[a].size(); b++)
		//		{
		//			// moves the song along
		//			(*drumTrackTmp)[a][b] = (*drumTrack)[a][b] / noteSpeed + incriment / noteSpeed + fretboardPosition;
		//
		//			//stop checking if nothing can be seen
		//			if((*drumTrackTmp)[a][b] <= -3)
		//				break;
		//
		//			if(b > countAmountDrum[a])
		//			{
		//				//note drop off
		//				if((*drumTrackTmp)[a][b - 1] + noteOffset > con->getHeight())
		//					countAmountDrum[a]++;
		//
		//				if(((*drumTrackTmp)[a][b - 1] >= (abs((*drumTrackTmp)[a][b]) - abs((*drumTrackTmp)[a][b - 1]))))//if the note below is off screen? (actually idk)
		//					if((*disiNotes)[a].size() > 0)
		//						if((unsigned int)b == (*disiNotes)[a][0])
		//							(*disiNotes)[a].erase((*disiNotes)[a].begin());
		//
		//			}
		//
		//
		//			if(!invisable(a, b))
		//				if((*drumTrackTmp)[a][b] + noteOffset > -3 && (*drumTrackTmp)[a][b] + noteOffset < con->getHeight())
		//					if((*drumTrackTmp)[a][b] + noteOffset < fretboardPosition + notes[0][0].getHeight())//draw notes above the fret board
		//					{
		//						///Normal
		//						if(!a)
		//							con->toConsoleBuffer((*notes)[2], (centerTrack)+1, (*drumTrackTmp)[a][b] + noteOffset + 1, (*padNoteColour)[0]);
		//						else
		//							a--,
		//							con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*drumTrackTmp)[a + 1][b] + noteOffset, (*padNoteColour)[a + 1]),
		//							a++;
		//
		//					}
		//					else//draw notes below the fret board
		//					{
		//						if(!a)
		//							con->toConsoleBuffer((*notes)[2], (centerTrack)+1, (*drumTrackTmp)[a][b] + noteOffset + 1, FG_RED);
		//						else
		//							a--,
		//							con->toConsoleBuffer((*notes)[0], (centerTrack)+(a * 13 + 2), (*drumTrackTmp)[a + 1][b] + noteOffset, FG_RED),
		//							a++;
		//					}
		//		}
		//	}
		//
		////Lyrics Movement
		//static Sprite lyricBox("Game Files/Lyrics Box.txt");
		//con->toConsoleBuffer(lyricBox, con->getWidth() / 2 - lyricBox.getWidth() / 2, 0);
		//for(unsigned int a = firstLyric; a < lyrics->size(); a++)
		//{
		//	if(incriment > (long)lyricTiming[0][a].back().second)
		//	{
		//		firstLyric++;
		//		continue;
		//	}
		//
		//	if(incriment > (long)lyricTiming[0][a].front().first)
		//	{
		//		string phrase;
		//		for(auto& b : lyrics[0][a])
		//			phrase += b;
		//
		//		1 > lyrics[0][a].size() - 1 ?
		//			con->toConsoleBuffer(lyrics[0][a][0], (con->getWidth() / 2 - phrase.size() / 2), 2,
		//				lyricTiming[0][a][0].first <= (unsigned int)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
		//			:
		//			con->toConsoleBuffer(lyrics[0][a][0], (con->getWidth() / 2 - phrase.size() / 2), 2,
		//				lyricTiming[0][a][0].first <= (unsigned int)incriment && lyricTiming[0][a][1].first > (unsigned int)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
		//			;
		//
		//		unsigned int length = 0;
		//		for(unsigned int b = 1; b < lyrics[0][a].size(); b++)
		//			b + 1 > lyrics[0][a].size() - 1 ?
		//			con->toConsoleBuffer(lyrics[0][a][b], con->getWidth() / 2 - phrase.size() / 2 + (length += lyrics[0][a][b - 1].size()), 2,
		//				lyricTiming[0][a][b].first <= (unsigned int)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
		//			:
		//			con->toConsoleBuffer(lyrics[0][a][b], con->getWidth() / 2 - phrase.size() / 2 + (length += lyrics[0][a][b - 1].size()), 2,
		//				lyricTiming[0][a][b].first <= (unsigned int)incriment && lyricTiming[0][a][b + 1].first > (unsigned int)incriment ? FG_GREEN | FG_INTENSIFY : FG_WHITE | FG_INTENSIFY)
		//			;
		//	}
		//
		//	break;
		//	//	lastLyricPosition = lyricPosition;
		//}

	}

	void playButtonPress()
	{
		static char keyfrets[]{'A','S','D','F','G'};
		static char keypads[]{VK_SPACE,'U','I','O','P'};
		static uint guitarfrets[]{GUITAR_FRET_GREEN, GUITAR_FRET_RED, GUITAR_FRET_YELLOW, GUITAR_FRET_BLUE, GUITAR_FRET_ORANGE};
		static uint drumpads[]{DRUM_KICK_PEDAL,DRUM_PAD_RED,DRUM_PAD_YELLOW,DRUM_PAD_BLUE,DRUM_PAD_GREEN};
		static short lastStrum = -1, currentStrum = -1;
		static bool stroke;

		//if(paused || guitarstart)
		//	playPauseMenu();

		bool up = 0, down = 0;
		//Strumming
		if(!paused)
		{
			//bool newSpeed = KeyInput::stroke(VK_NUMPAD4);
			//
			//if((newSpeed) || KeyInput::stroke(VK_NUMPAD6))
			//	if(((spacingScale * 2) * (speedPercent + (newSpeed ? .25f : -.25f))) > 0 && ((float)(*notes)[0].getHeight() /
			//		((float)spacingScale / ((spacingScale * 2) * (speedPercent + (newSpeed ? .25f : -.25f)) -
			//		((spacingScale * 2) * (speedPercent + (newSpeed ? .25f : -.25f)) - spacingScale * 2) * 2))) > 0)
			//
			//		noteSpeed = spacingScale * 2,
			//		noteSpeed *= int(speedPercent += (newSpeed ? .25f : -.25f));

			if(instrument == "guitar" || instrument == "rhythm")
			{
				//LightSource::setLightAmount(5);

				//Note lights
				for(short a = 0; a < 5; a++)
				{
					fretLit[a].enableLight(false);
					fretLit[a].translate({0,0,0});
					fretLit[a].setDiffuse({255,100,0,100});
					fretLit[a].setAttenuationQuadratic(0.6f);

					if(/*KeyInput::release(keyfrets[a]) && */InputManager::getController(0)->isButtonReleased(guitarfrets[a]))
					{
						pressed[a] = false;
						//	LightSource::enableLight(a, false);
						fretBoard[a].setColour(fretColour[0][a] * .5f);
					}
					else if(/*KeyInput::press(keyfrets[a]) ||*/ InputManager::getController(0)->isButtonPressed(guitarfrets[a]))
					{
						pressed[a] = true;
						fretLit[a].enableLight(true);
						fretBoard[a].setColour(fretColour[0][a]);

					}
				}

				//Strum logic (if there is any)
				if([&]()->bool
				{
					up = /*KeyInput::press(VK_UP) ||*/ XinputManager::getController(0)->isButtonPressed(GUITAR_STRUM_UP);
						down = /*KeyInput::press(VK_DOWN) ||*/ XinputManager::getController(0)->isButtonPressed(GUITAR_STRUM_DOWN);
						currentStrum = up ? (down ? 4 : 2) : (down ? 1 : -1);

						if(currentStrum == -1)
							lastStrum = -1;
						return (lastStrum != currentStrum) && currentStrum != -1;
				} ())
				{
					lastStrum = currentStrum;

					if(collision())
					{
						if(!noteDelete())
						{
							currentHealth -= currentHealth > 0 ? 1 : 0;
							//missFX(rand());
						}
						else
							currentHealth += currentHealth < HEALTH_CAP ? .5f : 0;
					}
					else
					{
						currentHealth -= currentHealth > 0 ? 1 : 0;
						//missFX(rand());
					}
					if(currentHealth < 0)currentHealth = 0;
				}
			}
			//	else if(instrument == "drum")
			//	{
			//		//Note lights
			//		for(short a = 0; a < 5; a++)
			//		{
			//			if((/*KeyInput::release(keypads[a]) && */XinputManager::getController(0)->isButtonReleased(drumpads[a])))
			//			{
			//				pressed[a] = false;
			//				initPressed[a] = true;
			//				if((*padColour)[a] > 8)
			//					(*padColour)[a] -= 8;
			//			}
			//			else if(KeyInput::press(keypads[a]) || XinputManager::getController(0)->isButtonPressed(drumpads[a]))
			//			{
			//				if(pressed[a])
			//					initPressed[a] = false;
			//				pressed[a] = true;
			//
			//				if((*padColour)[a] < 8)
			//					(*padColour)[a] += 8;
			//			}
			//		}
			//
			//		//Drum logic (if there is any)
			//		if([&]()->bool
			//			{
			//				int count = 0;
			//					for(int a = 0; a < 5; a++)
			//						count += pressed[a] && initPressed[a];
			//					return (bool)count;
			//
			//			} ())
			//		{
			//
			//			if(collision())
			//			{
			//				if(!noteDelete())
			//					currentHealth -= currentHealth > 0 ? 1 : 0,
			//					missFX(rand());
			//				else
			//					currentHealth += currentHealth < HEALTH_CAP ? .5f : 0;
			//			}
			//			else
			//			{
			//				currentHealth -= currentHealth > 0 ? 1 : 0;
			//				missFX(rand());
			//			}
			//			if(currentHealth < 0)currentHealth = 0;
			//		}
			//
			//	}
		}
	}

#pragma region Note Alterations

	bool collision()
	{
		bool colli(false);
		uint maxi = std::max(std::max(guitarTrackTmp->size(), bassTrackTmp->size()), drumTrackTmp->size());

		for(uint a = 0; a < maxi; a++)
		{
			if(a < guitarTrackTmp->size())
				if(instrument == "guitar")
				{
					while(colliCountGuitar[a] < (*guitarTrackTmp)[a].size() &&
						  (*guitarTrackTmp)[a][colliCountGuitar[a]].pos + noteOffset < fretboardPosition - note->getDepth())
						colliCountGuitar[a]++;

					if(pressed[a])
						if(colliCountGuitar[a] < (*guitarTrackTmp)[a].size())
							if((*guitarTrackTmp)[a][colliCountGuitar[a]].note)
								if((*guitarTrackTmp)[a][colliCountGuitar[a]].note->collision3D(&fretBoard[a]))
									colli = true;
				}

			//if(a < bassTrackTmp->size())
			//	if(instrument == "rhythm")
			//	{
			//		while(colliCountBass[a] < (*bassTrackTmp)[a].size() &&
			//			(*bassTrackTmp)[a][colliCountBass[a]] + noteOffset > fretboardPosition + 3)
			//			colliCountBass[a]++;
			//
			//		if(pressed[a])
			//			if(colliCountBass[a] < (*bassTrackTmp)[a].size())
			//				if((*bassTrackTmp)[a][colliCountBass[a]] + noteOffset > fretboardPosition - 3)
			//					colli = true;
			//	}
			//
			//if(a < drumTrackTmp->size())
			//	if(instrument == "drum")
			//	{
			//		while(colliCountDrum[a] < (*drumTrackTmp)[a].size() &&
			//			(*drumTrackTmp)[a][colliCountDrum[a]] + noteOffset > fretboardPosition + 3)
			//			colliCountDrum[a]++;
			//
			//		if(colliCountDrum[a] < (*drumTrackTmp)[a].size())
			//			if((*drumTrackTmp)[a][colliCountDrum[a]] + noteOffset >= fretboardPosition - 3)
			//				colli = true;
			//	}
		}

		return colli;
	}

	bool noteDelete()
	{
		static vector<int> num;
		num.clear();

		if(instrument == "guitar")
		{
			for(uint a = 0; a < guitarTrackTmp->size(); a++)
				if(colliCountGuitar[a] < (*guitarTrackTmp)[a].size())
					if((*guitarTrackTmp)[a][colliCountGuitar[a]].pos + noteOffset < fretboardPosition + note->getDepth() && (*guitarTrackTmp)[a][colliCountGuitar[a]].pos + noteOffset >= fretboardPosition - note->getDepth())
						num.push_back(a);

			short lastOne = 0;
			bool init = true;
			for(uint a = 0; a < num.size(); a++)
			{
				if(init)
				{
					init = false;
					lastOne = a;
					continue;
				}

				if((*guitarTrackTmp)[num[lastOne]][colliCountGuitar[num[lastOne]]].pos != (*guitarTrackTmp)[num[a]][colliCountGuitar[num[a]]].pos)
				{
					float check = std::max((*guitarTrackTmp)[num[lastOne]][colliCountGuitar[num[lastOne]]].pos, (*guitarTrackTmp)[num[a]][colliCountGuitar[num[a]]].pos);

					if(check == (*guitarTrackTmp)[num[a]][colliCountGuitar[num[a]]].pos)
						num.erase(num.begin() + lastOne);
					else
						num.erase(num.begin() + a);
					a--;
					init = true;
				}
			}


			if(num.size() == 1)
			{
				for(int a = 4; a > num[0]; a--)
					if(pressed[a])
						return false;

				if(pressed[num[0]])
					notesHit++,
					(*disiNotes)[num[0]].push_back(colliCountGuitar[num[0]]++);
			}
			else
			{
				int counter = 0, sum = 0;
				for(int a = 0; a < 5; a++)
					if(sum += pressed[a], pressed[a])
						if((uint)counter < num.size())
						{
							if(num[counter++] != a)
								return false;

						}
						else
							return false;


				if((uint)sum != num.size())
					return false;


				for(auto& a : num)
					notesHit++,
					(*disiNotes)[a].push_back(colliCountGuitar[a]++);
			}
		}
		//else if(instrument == "rhythm")
		//{
		//	for(uint a = 0; a < bassTrackTmp->size(); a++)
		//		if(colliCountBass[a] < (*bassTrackTmp)[a].size())
		//			if((*bassTrackTmp)[a][colliCountBass[a]] + noteOffset > fretboardPosition - 3 && (*bassTrackTmp)[a][colliCountBass[a]] + noteOffset <= fretboardPosition + 3)
		//				num.push_back(a);
		//
		//	short lastOne = 0;
		//	bool init = true;
		//	for(uint a = 0; a < num.size(); a++)
		//	{
		//		if(init)
		//		{
		//			init = false;
		//			lastOne = a;
		//			continue;
		//		}
		//
		//		if((*bassTrackTmp)[num[lastOne]][colliCountBass[num[lastOne]]] != (*bassTrackTmp)[num[a]][colliCountBass[num[a]]])
		//		{
		//			long check = max((*bassTrackTmp)[num[lastOne]][colliCountBass[num[lastOne]]], (*bassTrackTmp)[num[a]][colliCountBass[num[a]]]);
		//
		//			if(check == (*bassTrackTmp)[num[a]][colliCountBass[num[a]]])
		//				num.erase(num.begin() + lastOne);
		//			else
		//				num.erase(num.begin() + a);
		//			a--;
		//			init = true;
		//		}
		//	}
		//
		//
		//	if(num.size() == 1)
		//	{
		//		for(int a = 4; a > num[0]; a--)
		//			if(pressed[a])
		//				return false;
		//
		//		if(pressed[num[0]])
		//			notesHit++,
		//			(*disiNotes)[num[0]].push_back(colliCountBass[num[0]]++);
		//	}
		//	else
		//	{
		//		int counter = 0, sum = 0;
		//		for(int a = 0; a < 5; a++)
		//			if(sum += pressed[a], pressed[a])
		//				if((uint)counter < num.size())
		//				{
		//					if(num[counter++] != a)
		//						return false;
		//
		//				}
		//				else
		//					return false;
		//
		//
		//		if((uint)sum != num.size())
		//			return false;
		//
		//
		//		for(auto& a : num)
		//			notesHit++,
		//			(*disiNotes)[a].push_back(colliCountBass[a]++);
		//	}
		//}
		//else if(instrument == "drum")
		//{
		//	bool hitIt = false;
		//	for(uint a = 0; a < drumTrackTmp->size(); a++)
		//		if(colliCountDrum[a] < (*drumTrackTmp)[a].size())
		//			if((*drumTrackTmp)[a][colliCountDrum[a]] + noteOffset >= fretboardPosition - 3 && (*drumTrackTmp)[a][colliCountDrum[a]] + noteOffset <= fretboardPosition + 3)
		//				if(initPressed[a] && pressed[a])
		//					hitIt = true,
		//					notesHit++,
		//					(*disiNotes)[a].push_back(colliCountDrum[a]++);
		//	if(!hitIt)
		//		return false;
		//}
		//
		//for(uint a = 0; a < AudioPlayer::size(); a++)
		//	AudioPlayer::setVolume(1, a);

		return true;
	}

	bool invisable(int fret, uint check)
	{
		for(uint b = 0; b < (*disiNotes)[fret].size(); b++)
			if((*disiNotes)[fret][b] == check)
				return true;

		return false;
	}
#pragma endregion

private:

	Model* note = new Model("Models/Note/note.obj", "Note");
	Model fretBoard[5];
	Light fretLit[5];

#pragma region Global Variables

	//Colours
	vector<ColourRGBA>
		* fretColour = new vector<ColourRGBA>
	{{0,255,0},{255,0,0},{255,255,0},{0,0,255},{128,0,128}};

	vector<ColourRGBA>
		* padColour = new vector<ColourRGBA>
	{{128,0,128},{255,0,0},{255,255,0},{0,0,255},{0,255,0}};

	//hit notes
	vector<vector<unsigned int>>* disiNotes = new vector<vector<unsigned int>>(5, vector<unsigned int>());

	//
	vector <int>* lines = new vector <int>;

	//Instrument note positions
	vector<vector<Fret>>
		* bassTrack = new vector<vector<Fret>>(5);
	vector<vector<Fret>>
		* bassTrackTmp = new vector<vector<Fret>>(5);
	vector<vector<Fret>>
		* guitarTrack = new vector<vector<Fret>>(5);
	vector<vector<Fret>>
		* guitarTrackTmp = new vector<vector<Fret>>(5);

	vector<vector<long>>
		* drumTrack = new vector<vector<long>>(5);
	vector<vector<long>>
		* drumTrackTmp = new vector<vector<long>>(5);

	//Lyric Information
	vector<vector<pair<long, long>>>* vocalTrack = new vector<vector<pair<long, long>>>(5);
	vector<vector<pair<long, long>>>* vocalTrackTmp = new vector<vector<pair<long, long>>>(5);

	vector<vector<string>>* lyrics = new vector<vector<string>>;

	vector<vector<pair<unsigned int, unsigned int>>>* lyricTiming = new vector<vector<pair<unsigned int, unsigned int>>>;
	vector<vector<pair<unsigned int, unsigned int>>>* lyricTimingTmp = new vector<vector<pair<unsigned int, unsigned int>>>;

	//Missilanious
	int  barCount, centerTrack, countAmountGuitar[5], countAmountBass[5], countAmountDrum[5], notesHit, noteOffset = 0, fretOffset,
		fretboardPosition, noteSpeed = 60, lastSpeed, spacingScale = lastSpeed = noteSpeed, create;
	unsigned int colliCountGuitar[5], colliCountBass[5], colliCountDrum[5], firstLyric;

	float incriment = -1;
	float  fps = 0, speedPercent = 1, currentHealth = 0, lastHealth;
	bool initPressed[5], pressed[5], start, paused = false, selected = false, exitSelect = false;
	bool healthCheck;


	int songChoice;
	string songName, songDir, instrument = "guitar";
	string percentstr;
	unsigned short difficulty = 8, notePadding = 2;
	thread* t1 = new thread;


#pragma endregion

};