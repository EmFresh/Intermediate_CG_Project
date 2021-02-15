#include "EmGineAudioPlayer.h"
//#include <iostream>
#include <string>

#pragma region Static Variables
unsigned int EmGineAudioPlayer::stopIndex = 0;
AudioSystem* EmGineAudioPlayer::m_system;
AudioChannelGroup* EmGineAudioPlayer::m_mainChannelGroup;
std::vector<AudioControl*>* EmGineAudioPlayer::m_control;
#pragma endregion

void EmGineAudioPlayer::init(int channels)
{
	if(m_system)
		return;

	m_control = new std::vector<AudioControl*>;

	if(FMOD::System_Create(&m_system))
	{
		delete m_control;
		return;
	}

	int driverCount;
	if(m_system->getNumDrivers(&driverCount))
	{

		delete m_control;
		return;
	}

	printError(m_system->init(channels, FMOD_INIT_NORMAL, nullptr), "Line 28");
}

void EmGineAudioPlayer::disable()
{
	m_system->release();
	m_system->close();
	m_control->clear();

	delete m_system;
}

bool EmGineAudioPlayer::createAudio(const char* file, std::string tag)
{
	Audio* newSound;

	if(m_system->createSound(file, FMOD_3D | FMOD_CREATECOMPRESSEDSAMPLE | FMOD_ACCURATETIME, nullptr, &newSound))
	{
		printf("failed to create Audio\n");
		return false;
	}
	m_control->push_back(new AudioControl{newSound,nullptr,new Listener});

	printError(m_system->playSound(m_control[0][m_control->size() - 1]->sound, m_mainChannelGroup, true, &m_control->back()->channel), "Line 50");
	
	return true;
}

bool EmGineAudioPlayer::createAudioStream(const char* file, std::string tag)
{
	Audio* newSound;
	if(m_system->createStream(file, FMOD_3D | FMOD_ACCURATETIME, nullptr, &newSound))
	{
		printf("failed to create Audio Stream\n");
		return false;
	}

	m_control->push_back(new AudioControl{newSound,nullptr,new Listener});
	printError(m_system->playSound(m_control[0][m_control->size() - 1]->sound, m_mainChannelGroup, true, &m_control->back()->channel), "Line 64");
	
	return true;
}

void EmGineAudioPlayer::play(bool loop, bool newInst, unsigned int index, unsigned int from, unsigned int to, FMOD_TIMEUNIT unit)
{
	if(newInst && m_control[0][index])
		m_control->push_back(m_control[0][index]),
		m_control->push_back(nullptr),
		index = (unsigned int)m_control->size() - 1;



	if(!m_control[0][index] || (m_control[0][index] ? isStoped(index) : false))
		printError(m_system->playSound(m_control[0][index]->sound, nullptr, true, &m_control[0][index]->channel), "Line 78");

	FMOD_MODE mod;
	if(loop)
		m_control[0][index]->channel->getMode(&mod),
		!(mod & FMOD_LOOP_NORMAL) ? printError(m_control[0][index]->channel->setMode(FMOD_LOOP_NORMAL), "Line 81") : void(),
		printError(m_control[0][index]->channel->setLoopCount(-1), "Line 82"),
		from < to ? printError(m_control[0][index]->channel->setLoopPoints(from, unit, to, unit), "Line 83") :
		void();
	else
		printError(m_control[0][index]->channel->setMode(FMOD_LOOP_OFF), "Line 86");


	printError(m_control[0][index]->channel->setPaused(false), "Line 89");
	cleanup();
}

void EmGineAudioPlayer::play(std::string tag, bool loop, bool newInstance, unsigned int from, unsigned int to, FMOD_TIMEUNIT unit)
{
	int index = 0;
	for(; index < (int)m_control->size(); index++)
		if(m_control[0][index]->tag == tag)
			break;
	play(loop, newInstance, index, from, to, unit);
}

void EmGineAudioPlayer::playAll(bool loop, unsigned int from, unsigned int to, FMOD_TIMEUNIT unit)
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg), "Line 101");
	unsigned int length;
	printError(cg->setPaused(true), "Line 103");
	FMOD_MODE mod;
	for(unsigned int index = 0; index < m_control->size(); index++)
		if(loop)
		{
			m_control[0][index]->channel->getMode(&mod);
			!(mod & FMOD_LOOP_NORMAL) ? 
				printError(m_control[0][index]->channel->setMode(FMOD_LOOP_NORMAL), "Line 108") : void();

			//printError(m_control[0][index]->channel->setMode(FMOD_LOOP_NORMAL), "Line 111");
			printError(m_control[0][index]->sound->getLength(&length, unit), "Line 112");

			from < to&& from >= 0 ? to < length ?
				printError(m_control[0][index]->channel->setLoopPoints(from, unit, to, unit), "Line 115")/*true*/ :
				printError(m_control[0][index]->channel->setLoopPoints(from, unit, length - 1, unit), "Line 116")/*false*/ :
				void()/*else*/;

			printError(m_control[0][index]->channel->setLoopCount(-1), "Line 119");

		}
		else
			printError(m_control[0][index]->channel->setMode(FMOD_LOOP_OFF), "Line 123");
	printf("\n\n");

	for(auto& a : *m_control)
		printError(a->channel->setPaused(false), "Line 129");
	printError(cg->setPaused(false), "Line 130");

	cleanup();
}

void EmGineAudioPlayer::pause(unsigned int index)
{
	printError(m_control[0][index]->channel->setPaused(true));
}

void EmGineAudioPlayer::pause(std::string tag)
{
	int index = 0;
	for(; index < (int)m_control->size(); index++)
		if(m_control[0][index]->tag == tag)
			break;
	pause(index);
}

void EmGineAudioPlayer::pauseAll()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg));
	printError(cg->setPaused(true));
	for(auto& a : *m_control)
		printError(a->channel->setPaused(true));
}

void EmGineAudioPlayer::stop(unsigned int index)
{
	//stopIndex = index;
	printError(m_control[0][index]->channel->stop());
	cleanup();
}

void EmGineAudioPlayer::stop(std::string tag)
{
	int index = 0;
	for(; index < (int)m_control->size(); index++)
		if(m_control[0][index]->tag == tag)
			break;
	stop(index);
}

void EmGineAudioPlayer::stopAll()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg));
	bool paused;
	printError(cg->getPaused(&paused));
	printError(cg->setPaused(true));


	stopIndex = 0;
	for(unsigned int a = 0; a < m_control->size(); a++)
		printError(m_control[0][a]->channel->stop());


	cleanup();

	cg->setPaused(paused);
}

unsigned int EmGineAudioPlayer::getPosition(unsigned int index, FMOD_TIMEUNIT type)
{
	unsigned int pos;
	printError(m_control[0][index]->channel->getPosition(&pos, type));
	return pos;
}

unsigned int EmGineAudioPlayer::getPosition( std::string tag, FMOD_TIMEUNIT type)
{
	int index = 0;
	for(; index < (int)m_control->size(); index++)
		if(m_control[0][index]->tag == tag)
			break;
	return getPosition(type, index);
}

bool EmGineAudioPlayer::isStoped(unsigned int index)
{
	bool play;
	printError(m_control[0][index]->channel->isPlaying(&play));
	return !play;
}

bool EmGineAudioPlayer::isStoped(std::string tag)
{
	int index = 0;
	for(; index < (int)m_control->size(); index++)
		if(m_control[0][index]->tag == tag)
			break;
	return isStoped(index);
}

bool EmGineAudioPlayer::isPaused(unsigned int index)
{
	bool pause;
	printError(m_control[0][index]->channel->getPaused(&pause));
	return pause;
}

bool EmGineAudioPlayer::isPaused(std::string tag)
{
	int index = 0;
	for(; index < (int)m_control->size(); index++)
		if(m_control[0][index]->tag == tag)
			break;

	return isPaused(index);
}

unsigned int EmGineAudioPlayer::size()
{
	return (unsigned int)m_control->size();
}

void EmGineAudioPlayer::setVolume(float vol, unsigned int index)
{
	printError(m_control[0][index]->channel->setVolume(vol));
}

void EmGineAudioPlayer::setVolume(std::string tag, float vol)
{
	int index = 0;
	for(; index < (int)m_control->size(); index++)
		if(m_control[0][index]->tag == tag)
			break;
	setVolume(vol, index);
}

void EmGineAudioPlayer::setMasterVolume(float vol)
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg));
	printError(cg->setVolume(vol));
}

AudioSystem* EmGineAudioPlayer::getAudioSystem()
{
	return m_system;
}

void EmGineAudioPlayer::mute(unsigned int index)
{
	bool mute;
	printError(m_control[0][index]->channel->getMute(&mute));
	printError(m_control[0][index]->channel->setMute(!mute));
}

void EmGineAudioPlayer::mute(std::string tag)
{
	int index = 0;
	for(; index < (int)m_control->size(); index++)
		if(m_control[0][index]->tag == tag)
			break;

	mute(index);
}

void EmGineAudioPlayer::muteAll()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg));
	bool mute;
	cg->getMute(&mute);
	cg->setMute(!mute);
}

FMOD::ChannelGroup* EmGineAudioPlayer::getMasterChannelGroup()
{
	AudioChannelGroup* cg;
	printError(m_system->getMasterChannelGroup(&cg));
	return cg;
}

std::vector<AudioControl*>* EmGineAudioPlayer::getAudioControl()
{
	return m_control;
}

void EmGineAudioPlayer::update()
{
	for(auto& a : *m_control)
		a->channel->set3DAttributes(&a->listener->pos, &a->listener->vel);
	

	printError(m_system->update());
}

void EmGineAudioPlayer::cleanup()
{
	bool play;

	for(unsigned a = 0; a < m_control->size(); a++)
	{
		AudioChannel* channel = m_control[0][a]->channel;
		channel->isPlaying(&play);
		if(!play)
		{
			if(find(m_control->begin() + a + 1, m_control->end(), m_control[0][a]) == m_control->end() &&
				find(m_control->begin(), m_control->begin() + a, m_control[0][a]) == m_control->begin() + a)
				printError(m_control[0][a]->sound->release());

			m_control->erase(m_control->begin() + a);
			a--;
		}
	}
}

void EmGineAudioPlayer::printError(FMOD_RESULT error, const char* where)
{
	error, where;
#if _DEBUG
	if(error)
	{
		std::string str(FMOD_ErrorString(error));
		printf(("Error:\n" + str + '\n' + where + "\n\n").c_str());
	}
#endif
}

FMOD_RESULT F_CALLBACK EmGineAudioPlayer::cleanUpCallback(FMOD_CHANNELCONTROL * chanCtrl, FMOD_CHANNELCONTROL_TYPE ctrlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void* commandData1, void* commandData2)
{
	callbackType, commandData1, commandData2;//referenced but not quite needed


	if(ctrlType == FMOD_CHANNELCONTROL_CHANNEL)
	{
		// Channel specific functions here...
		AudioChannel* channel;
		channel = (AudioChannel*)chanCtrl;
		bool play;
		channel->isPlaying(&play);

		if(!play)
		{
			for(int a = stopIndex; a < (int)m_control->size(); a++)
				if(m_control[0][a]->channel == channel)
				{
					if(find(m_control->begin() + a + 1, m_control->end(), m_control[0][a]) == m_control->end())
						m_control[0][a]->sound->release();
					break;
				}
			stopIndex = 0;
		}
	}
	else
	{// ChannelGroup specific functions here...
		//	ChannelGroup *group = (ChannelGroup *) chanCtrl;
	}

	// ChannelControl generic functions here...
	return FMOD_OK;
}

// for later reference
FMOD_RESULT F_CALLBACK mycallback(FMOD_CHANNELCONTROL * chanCtrl, FMOD_CHANNELCONTROL_TYPE ctrlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void* commandData1, void* commandData2)
{
	chanCtrl, callbackType, commandData1, commandData2;//unreferenced

	if(ctrlType == FMOD_CHANNELCONTROL_CHANNEL)
	{
		//AudioChannel *channel = (AudioChannel *)chanCtrl;
		/// Channel specific functions here...
	}
	else
	{
		//	ChannelGroup *group = (ChannelGroup *) chanCtrl;
			/// ChannelGroup specific functions here...
	}

	// ChannelControl generic functions here...
	return FMOD_OK;
}