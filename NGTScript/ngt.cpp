﻿#define DLLCALL   __declspec( dllimport )
//#define MA_NO_OPUS      /* Disable the (not yet implemented) built-in Opus decoder to ensure the libopus decoder is picked. */
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 
#include <random>
#include <type_traits>
#include <Windows.h>
#include <thread>
#include "Tolk.h"
#include <chrono>
#include <string>
#include"sdl/SDL.h"
#include <commctrl.h>
#include<map>
#include <stdio.h>
#include "NGT.H"
#include <locale>
#include <codecvt>
#include<fstream>
#include <cstdlib>
#include <cstring>
SDL_Window* win = NULL;
HMODULE bass;
std::wstring wstr(const std::string& utf8String)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(utf8String);
}
std::wstring reader;
std::string sound_path;
std::unordered_map<SDL_Keycode,bool> keys;
bool keyhook = false;
std::string inputtext;
ma_engine sound_engine;
ALCdevice* device = NULL;
ALCcontext* context = NULL;
void init_engine() {
    device = alcOpenDevice(nullptr);
    context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);

    ma_engine_init(NULL, &sound_engine);
    ma_engine_listener_set_velocity(&sound_engine, 0, 20, 20, 20);
    SDL_Init(SDL_INIT_EVERYTHING);
    SDLNet_Init();
    Tolk_TrySAPI(true);

    Tolk_Load();
    reader=Tolk_DetectScreenReader();
    if (!Tolk_HasSpeech()) {
        Tolk_PreferSAPI(true);
}
}
std::random_device rd;
std::mt19937 gen(rd());
long random(long min, long max) {
    static_assert(std::is_integral<long>::value, "Type must be integral");

    std::uniform_int_distribution<long> dis(min, max);

    return dis(gen); // Ensure 'gen' is a valid random number generator
}
double randomDouble(double min, double max) {
    static_assert(std::is_floating_point<double>::value, "Type must be floating point");

    std::uniform_real_distribution<double> dis(min, max);

    return dis(gen); // Ensure 'gen' is a valid random number generator
}


int get_last_error() {
    return alGetError();
}
void speak(const std::string & text, bool stop) {
    std::wstring textstr = wstr(text);
    Tolk_Speak(textstr.c_str(), stop);
}
void speak_wait(const std::string & text, bool stop) {
    std::wstring textstr = wstr(text);
    Tolk_Speak(textstr.c_str(), stop);
    while (Tolk_IsSpeaking()) {
        update_game_window();
    }
}

void stop_speech() {
        Tolk_Silence();
    }
bool window_closable;
SDL_Event e;
bool show_game_window(const std::string & title,int width, int height, bool closable)
{
if(reader==L"JAWS")
    win=SDL_CreateWindow(title.c_str(),SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,width,height,SDL_WINDOW_SHOWN | SDL_WINDOW_KEYBOARD_GRABBED);
else
win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
SDL_RegisterApp("NGTWindow", 0, win);

SDL_StartTextInput();
window_closable = closable;
if (win!=NULL)
{
    update_game_window();
    return true;
}
return false;
}
    void hide_game_window() {
    SDL_StopTextInput();

    SDL_DestroyWindow(win);
}
void set_game_window_title(const std::string & new_title) {
    SDL_SetWindowTitle(win, new_title.c_str());
}
void set_game_window_closable(bool set_closable) {
    window_closable = set_closable;
}

void update_game_window()
{
    SDL_PollEvent(&e);
    if (e.type == SDL_QUIT and window_closable == true)
        {
            exit_engine();
        }
        else if (e.type == SDL_TEXTINPUT)
            inputtext += e.text.text;

        else if (e.type == SDL_KEYDOWN)
        {
            keys[e.key.keysym.sym] = true;
        }
        else if (e.type == SDL_KEYUP)
        {
            keys[e.key.keysym.sym] = false;
        }
    }
void exit_engine(int return_number)
{
    alcDestroyContext(context);
    alcCloseDevice(device);

    ma_engine_uninit(&sound_engine);

    SDL_StopTextInput();
    SDL_DestroyWindow(win);
win=NULL;
SDL_UnregisterApp();
Tolk_Unload();
SDL_Quit();
        SDLNet_Quit();
exit(return_number);
}
std::string read_environment_variable(const std::string& path) {
#ifdef _WIN32
    // Use _dupenv_s on Windows
    char* value;
    size_t size;
    if (_dupenv_s(&value, &size, path.c_str()) != 0 || value == nullptr) {
        // Environment variable not found or error occurred
        return "";
    }
    const std::string & result(value);
    free(value);  // Free the allocated memory
    return result;
#else
    // Use getenv for non-Windows platforms
    char* value = std::getenv(path.c_str());
    if (value == nullptr) {
        // Environment variable not found
        return "";
    }
    return const std::string &(value);
#endif
}
bool clipboard_copy_text(const std::string & text) {
    SDL_SetClipboardText(text.c_str());
    return true;
}
std::string clipboard_read_text() {
    return SDL_GetClipboardText();
}
std::string get_input() {
    std::string temp=inputtext;
    inputtext="";
    return temp;
}
bool key_pressed(SDL_Keycode key_code)
{
    if (e.type == SDL_KEYDOWN)
    {
        if (e.key.keysym.sym == key_code && e.key.repeat == 0)
        {
            return true;
        }
    }
return false;
}
bool key_released(SDL_Keycode key_code)
{
    if (e.type == SDL_KEYUP)
    {
        if (e.key.keysym.sym == key_code)
        {
            return true;
        }
    }
    return false;
}
bool key_down(SDL_Keycode key_code)
{
    if (keys.find(key_code) != keys.end()) {
        return keys[key_code];
    }
return false;
}
bool key_repeat(SDL_Keycode key_code)
{
    if (e.type == SDL_KEYDOWN)
    {
        if (e.key.keysym.sym == key_code)
        {
            return true;
        }
    }
    return false;
}
bool alert(const std::string & title, const std::string & text, const std::string &button_name)
{
    SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, button_name.c_str()},
    };

    SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION,
        win,
        title.c_str(),
        text.c_str(),
        SDL_arraysize(buttons),
        buttons,
        NULL
    };

    int buttonid;
    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
        SDL_Log("Error displaying message box");
        return false;
    }



return true;
}
int question(const std::string& title, const std::string& text) {
    SDL_MessageBoxButtonData buttons[] = {
    { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" },
    { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "No" },
    };

    SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION,
        win,
        title.c_str(),
        text.c_str(),
        SDL_arraysize(buttons),
        buttons,
        NULL
    };

    int buttonid;
    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
        SDL_Log("Error displaying message box");
        return -1;
    }

    return buttonid;
}


void set_listener_position(float l_x, float l_y, float l_z) {
    alListener3f(AL_POSITION, l_x, l_y, l_z);
    ma_engine_listener_set_position(&sound_engine, 0, l_x, l_y, l_z);
}
void wait(int time) {
    timer waittimer;
    int el = 0;
    while (el < time) {
        update_game_window();
        el = waittimer.elapsed_millis();
    }
}
void delay(int ms)
{
SDL_Delay(ms);
}
void set_sound_storage(const std::string & path) {
    sound_path = path;
}
 std::string get_sound_storage() {
    return sound_path;
}
void set_master_volume(float volume) {
    ma_engine_set_gain_db(&sound_engine, volume);
    alListenerf(AL_GAIN, static_cast<float>(volume));
}
float get_master_volume() {
    return ma_engine_get_gain_db(&sound_engine);
}
void sound::construct() {
}

void sound::destruct() {
}
BOOL sound_check(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
bool sound::load(const std::string & filename, bool set3d) {
    std::string result;
    if (sound_path != "") {
        result = sound_path + "/" + filename.c_str();
    }
    else {
        result = filename;
    }
    std::wstring filenameW = wstr(result);
    if (!sound_check(filenameW.c_str())) {
        active = false;
        return false;
    }
    if (active){
        this->close();
    active = false;
}

    if (set3d)
            ma_sound_init_from_file(&sound_engine, result.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &handle_);
        else
            ma_sound_init_from_file(&sound_engine, result.c_str(), MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_DECODE, NULL, NULL, &handle_);
        ma_sound_set_doppler_factor(&handle_, 0.1);
        active = true;
        SNDFILE* sndfile;
        SF_INFO sfinfo;
        sndfile = sf_open(result.c_str(), SFM_READ, &sfinfo);
        if (!sndfile) {
            active = false;
            return false;
        }

        std::vector<short> samples(sfinfo.channels * sfinfo.frames);
        sf_readf_short(sndfile, &samples[0], sfinfo.frames);
        sf_close(sndfile);
        if (sfinfo.channels > 1) {
            std::vector<short> monoSamples(sfinfo.frames);
            for (int i = 0; i < sfinfo.frames; i++) {
                int index = i * sfinfo.channels;
                monoSamples[i] = (samples[index] + samples[index + 1]) / 2;
            }
            samples = monoSamples;
            sfinfo.channels = 1;
        }


        alGenBuffers(1, &buffer_);
        alBufferData(buffer_,
            (sfinfo.channels == 1) ? AL_FORMAT_MONO16: AL_FORMAT_STEREO16,
            &samples[0],
            static_cast<ALsizei>(sfinfo.frames * sfinfo.channels * sizeof(short)),
            sfinfo.samplerate);

        alGenSources(1, &source_);

        // Set source properties
        alSourcei(source_, AL_BUFFER, buffer_);
        alSourcef(source_, AL_PITCH, 1.0f);
        alSourcef(source_, AL_GAIN, 1.0f);
        alSourcei(source_, AL_LOOPING, AL_FALSE);

        // Set 3D properties if requested
        if (set3d) {
            is_3d_ = true;
        }
            alSource3f(source_, AL_POSITION, 0.0f, 0.0f, 0.0f);
            alSource3f(source_, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

            alSourcei(source_, AL_SOURCE_RELATIVE, AL_FALSE);
            alSourcef(source_, AL_ROLLOFF_FACTOR, 0.1f);
            alSourcei(source_, AL_REFERENCE_DISTANCE, 1);
            alSourcei(source_, AL_MAX_DISTANCE, 500);

        active = true;
        return true;
}


    bool sound::load_from_memory(const std::string & data, bool set3d) {
        return false;
    }

    bool sound::play() {
        if (!active)return false;
        if (audio_system == 0) {
            stop();
            ma_sound_set_looping(&handle_, false);
            ma_sound_start(&handle_);
            return true;
        }
        else {
            alSourcei(source_, AL_LOOPING, AL_FALSE);
            alSourcePlay(source_);
            return true;
        }
    }

    bool sound::play_looped() {
        if (!active)return false;
        if (audio_system == 0) {
            stop();

            ma_sound_set_looping(&handle_, true);
            ma_sound_start(&handle_);
            return true;
        }
        else {
            alSourcei(source_, AL_LOOPING, AL_TRUE);
            alSourcePlay(source_);
            return true;
        }
        }

    bool sound::pause() {
        if (!active)return false;
        if (audio_system == 0) {
            ma_sound_stop(&handle_);
            return true;
        }
        else {
            alSourcePause(source_);
        }
    }

    bool sound::play_wait() {
        if (audio_system == 0) {
            stop();
            ma_sound_set_looping(&handle_, false);
            ma_sound_start(&handle_);
            if (!active)return false;
        }
        else {
            alSourcei(source_, AL_LOOPING, AL_FALSE);
            alSourcePlay(source_);
        }
            while (true) {
            update_game_window();
            delay(5);
            bool ac = sound::is_playing();
            if (ac == false) {
                break;
            }
        }
        return true;
    }

    bool sound::stop() {
        if (!active)return false;
        if (audio_system == 0) {
            ma_sound_stop(&handle_);
            ma_sound_seek_to_pcm_frame(&handle_, 0);
        }
        else {
            alSourceStop(source_);
            alSourcei(source_, AL_BUFFER, 0);
        }
        return true;
    }
    bool sound::close() {
        if (!active)return false;
            ma_sound_uninit(&handle_);
            if (source_) {
                alDeleteSources(1, &source_);
                source_ = 0;
            }

            if (buffer_) {
                alDeleteBuffers(1, &buffer_);
                buffer_ = 0;
            }
            active = false;
        return true;
    }

    void sound::set_sound_position(float s_x, float s_y, float s_z) {
        if (!active)return;
        ma_sound_set_position(&handle_, s_x, s_y, s_z);
        alSource3f(source_, AL_POSITION, s_x, s_y, s_z);
    }
    void sound::set_sound_reverb(float input_gain, float reverb_mix, float reverb_time) {
        return;
    }
    void sound::set_sound_hrtf(bool hrtf) {
        if (hrtf)
            this->audio_system = 1;
        else
            this->audio_system = 0;
    }
    bool sound::seek(double new_position) {
        if (!active)return false;
//        ma_sound_seek_to_pcm_frame(&handle_, );
    }

    void sound::cancel_reverb() {
        return;
    }

    double sound::get_pan() const {
        if (!active)return -17435;

        double pan;
        pan=ma_sound_get_pan(&handle_);
        return pan*100;
    }

    void sound::set_pan(double pan) {
        if (!active)return;
        ma_sound_set_pan(&handle_, pan/100);
    }

    double sound::get_volume() const {
        if (!active)return -17435;

        float volume;
        if (audio_system == 0) {

            volume = ma_sound_get_volume(&handle_);
            return ma_volume_linear_to_db(volume);

        }
        else {
            alGetSourcef(source_, AL_GAIN, &volume);
            return volume;
        }
    }
    void sound::set_volume(double volume) {
        if (!active)return;
        if (volume > 0 or volume < -100)return;
        if(audio_system == 0) {
            ma_sound_set_volume(&handle_, ma_volume_db_to_linear(volume));
        }
        else {
            alSourcef(source_, AL_GAIN, static_cast<float>(volume/100));

        }
    }
    double sound::get_pitch() const {
        if (!active)return -17435;

        double pitch;
        pitch = ma_sound_get_pitch(&handle_);
        return pitch*100;
    }

    void sound::set_pitch(double pitch) {
        if (!active)return;
        ma_sound_set_pitch(&handle_, pitch/100);
    }

    bool sound::is_active() const {
        return active;
    }

    bool sound::is_playing() const {
        ALenum state;
        if(audio_system==0)
        return ma_sound_is_playing(&handle_);
        else {
            alGetSourcei(source_, AL_SOURCE_STATE, &state);
            return state == AL_PLAYING;
        }
    }
    bool sound::is_paused() const {
        ALenum state;

        return paused;
    }

    double sound::get_position() const {
        if (!active)return -17435;

        double position;
        position = ma_sound_get_positioning(&handle_);
        return position;
    }

    double sound::get_length() const {
        if (!active)return-17435;

        double length;
//        length=ma_sound_get_length_in_seconds(&handle_, 0);
    }
        double sound::get_sample_rate() const {
    float rate=0;
    return rate;
}
void timer::construct() {
}

void timer::destruct() {
}
    uint64_t timer::elapsed_seconds() {
        return pausedNanos != 0 ? std::chrono::duration_cast<std::chrono::seconds>(std::chrono::nanoseconds(pausedNanos)).count() 
                                : std::chrono::duration_cast<std::chrono::seconds>(
                                  std::chrono::steady_clock::now() - inittime).count();
    }
    uint64_t timer::elapsed_minutes() {
        return pausedNanos != 0 ? std::chrono::duration_cast<std::chrono::minutes>(std::chrono::nanoseconds(pausedNanos)).count() 
                                : std::chrono::duration_cast<std::chrono::minutes>(
                                  std::chrono::steady_clock::now() - inittime).count();
    }
    uint64_t timer::elapsed_hours() {
        return pausedNanos != 0 ? std::chrono::duration_cast<std::chrono::hours>(std::chrono::nanoseconds(pausedNanos)).count() 
                                : std::chrono::duration_cast<std::chrono::hours>(
                                  std::chrono::steady_clock::now() - inittime).count();
    }
    uint64_t timer::elapsed_millis() {
        return pausedNanos != 0 ? std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds(pausedNanos)).count() 
                                : std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::steady_clock::now() - inittime).count();
    }

    uint64_t timer::elapsed_micros() {
        return pausedNanos != 0 ? std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::nanoseconds(pausedNanos)).count() 
                                : std::chrono::duration_cast<std::chrono::microseconds>(
                                  std::chrono::steady_clock::now() - inittime).count();
    }
    uint64_t timer::elapsed_nanos() {
        return pausedNanos != 0 ? pausedNanos 
                                : std::chrono::duration_cast<std::chrono::nanoseconds>(
                                  std::chrono::steady_clock::now() - inittime).count();
    }
    void timer::force_seconds(uint64_t seconds) {
        inittime = std::chrono::steady_clock::now() - std::chrono::seconds(seconds);
        pausedNanos = 0;
    }
    void timer::force_minutes(uint64_t minutes) {
        inittime = std::chrono::steady_clock::now() - std::chrono::minutes(minutes);
        pausedNanos = 0;
    }
    void timer::force_hours(uint64_t hours) {
        inittime = std::chrono::steady_clock::now() - std::chrono::hours(hours);
        pausedNanos = 0;
    }
    void timer::force_millis(uint64_t millis) {
        inittime = std::chrono::steady_clock::now() - std::chrono::milliseconds(millis);
        pausedNanos = 0;
    }

    // Force the timer to a specific time in microseconds
    void timer::force_micros(uint64_t micros) {
        inittime = std::chrono::steady_clock::now() - std::chrono::microseconds(micros);
        pausedNanos = 0;
    }

    // Force the timer to a specific time in nanoseconds
    void timer::force_nanos(uint64_t nanos) {
        inittime = std::chrono::steady_clock::now() - std::chrono::nanoseconds(nanos);
        pausedNanos = 0;
    }

    void timer::restart() {
        inittime = std::chrono::steady_clock::now();
        pausedNanos = 0;
    }
    void timer::pause() {
        if (pausedNanos == 0) {
            pausedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          std::chrono::steady_clock::now() - inittime).count();
        }
}
bool timer::is_running() {
    return pausedNanos == 0;
}

    void timer::resume() {
        if (pausedNanos != 0) {
            inittime = std::chrono::steady_clock::now() - std::chrono::nanoseconds(pausedNanos);
            pausedNanos = 0;
        }
    }

        void reverb::construct() {
        }

        void reverb::destruct() {
        }



        void reverb::set_input_gain(float input_gain) {
        }
        void reverb::set_reverb_mix(float reverb_mix) {
        }       
        void reverb::set_reverb_time(float reverb_time) {
        }
        float reverb::get_input_gain() {
            return 2;
        }
        float reverb::get_reverb_mix() {
            return 2;
        }
        float reverb::get_reverb_time() {
            return 2;
        }
        void network_event::construct() {}
        void network_event::destruct() {}

        void network::construct() {}
        void network::destruct() {}
        unsigned int network::connect(const std::string & host, int port) {
                    IPaddress ipAddress;

                    if (SDLNet_ResolveHost(&ipAddress, host.c_str(), port) == -1) {
                        return 0;
                    }

                    TCPsocket socket = SDLNet_TCP_Open(&ipAddress);

                    if (!socket) {
                        return 0;
                    }

                    unsigned int peerId = SDLNet_TCP_GetPeerAddress(socket)->host;

                    m_connectedPeers++;
                    m_active = true;

                    m_socketSet = SDLNet_AllocSocketSet(1);

                    if (!m_socketSet) {
                        return 0;
                    }

                    if (SDLNet_TCP_AddSocket(m_socketSet, socket) == -1) {
                        return 0;
                    }

                    m_clientSocket = socket;

                    return peerId;
                }

                bool network::destroy() {
                    if (m_serverSocket) {
                        SDLNet_TCP_Close(m_serverSocket);
                        m_serverSocket = nullptr;
                    }

                    if (m_clientSocket) {
                        SDLNet_TCP_Close(m_clientSocket);
                        m_clientSocket = nullptr;
                    }

                    if (m_socketSet) {
                        SDLNet_FreeSocketSet(m_socketSet);
                        m_socketSet = nullptr;
                    }

                    m_connectedPeers = 0;
                    m_bytesSent = 0.0;
                    m_bytesReceived = 0.0;
                    m_active = false;

                    return true;
                }

                bool network::disconnect_peer(unsigned int peerId) {
                    if (!m_active) {
                        std::cerr << "Error: Network is not active." << std::endl;
                        return false;
                    }

                    if (m_serverSockets.find(peerId) != m_serverSockets.end()) {
                        SDLNet_TCP_Close(m_serverSockets[peerId]);
                        m_serverSockets.erase(peerId);
                    }

                    if (m_clientSocket && SDLNet_TCP_GetPeerAddress(m_clientSocket)->host == peerId) {
                        SDLNet_TCP_Close(m_clientSocket);
                        m_clientSocket = nullptr;
                    }

                    m_connectedPeers=0;

                    return true;
                }

                bool network::disconnect_peer_forcefully(unsigned int peerId) {
                    if (!m_active) {
                        return false;
                    }

                    if (m_serverSockets.find(peerId) != m_serverSockets.end()) {
                        SDLNet_TCP_Close(m_serverSockets[peerId]);
                        m_serverSockets.erase(peerId);
                    }

                    if (m_clientSocket && SDLNet_TCP_GetPeerAddress(m_clientSocket)->host == peerId) {
                        SDLNet_TCP_Close(m_clientSocket);
                        m_clientSocket = nullptr;
                    }

                    m_connectedPeers=0;

                    return true;
                }

                bool network::disconnect_peer_softly(unsigned int peerId) {
                    if (!m_active) {
                        return false;
                    }

                    if (m_serverSockets.find(peerId) != m_serverSockets.end()) {
                        SDLNet_TCP_Send(m_serverSockets[peerId], nullptr, 0);
                    }

                    if (m_clientSocket && SDLNet_TCP_GetPeerAddress(m_clientSocket)->host == peerId) {
                        SDLNet_TCP_Send(m_clientSocket, nullptr, 0);
                    }

                    return true;
                }

                 std::string network::get_peer_address(unsigned int peerId) {
                    IPaddress ipAddress;
                    ipAddress.host = peerId;

                    std::string address = SDLNet_ResolveIP(&ipAddress);

                    if (address=="") {
                        return "";
                    }



                    return address;
                }

                double network::get_peer_average_round_trip_time(unsigned int peerId) {
                    // Not implemented
                    return 0.0;
                }

                std::vector<unsigned int> network::get_peer_list() {
                    std::vector<unsigned int> result;

                    if (m_clientSocket) {
                        result.push_back(SDLNet_TCP_GetPeerAddress(m_clientSocket)->host);
                    }

                    for (auto const& pair : m_serverSockets) {
                        result.push_back(pair.first);
                    }

                    return result;
                }

                network_event* network::request
                () {
                    network_event event;

                    if (!m_active) {
                        event.m_type = 0;
                        return &event;
                    }

                    if (!m_socketSet) {
                        event.m_type = 0;
                        return &event;
                    }

                    int numReadySockets = SDLNet_CheckSockets(m_socketSet, 0);

                    if (numReadySockets == -1) {
                        event.m_type = 0;
                        return &event;
                    }

                    if (numReadySockets == 0) {
                        event.m_type = 0;
                        return &event;
                    }

                    if (m_clientSocket && SDLNet_SocketReady(m_clientSocket)) {
                        char buffer[1024];
                        int bytesReceived = SDLNet_TCP_Recv(m_clientSocket, buffer, sizeof(buffer));

                        if (bytesReceived <= 0) {
                            disconnect_peer(SDLNet_TCP_GetPeerAddress(m_clientSocket)->host);
                            event.m_type = 3;
                            event.m_peerId = SDLNet_TCP_GetPeerAddress(m_clientSocket)->host;
                            return &event;
                        }

                        m_bytesReceived += bytesReceived;

                        event.m_type = 2;
                        event.m_peerId = SDLNet_TCP_GetPeerAddress(m_clientSocket)->host;
                        event.m_channel = 0;
                        event.m_message = std::string (buffer, bytesReceived);

                        return &event;
                    }

                    for (auto const& pair : m_serverSockets) {
                        if (SDLNet_SocketReady(pair.second)) {
                            char buffer[1024];
                            int bytesReceived = SDLNet_TCP_Recv(pair.second, buffer, sizeof(buffer));

                            if (bytesReceived <= 0) {
                                disconnect_peer(pair.first);
                                event.m_type = 3;
                                event.m_peerId = pair.first;
                                return &event;
                            }

                            m_bytesReceived += bytesReceived;

                            event.m_type = 2;
                            event.m_peerId = pair.first;
                            event.m_channel = 0;
                            event.m_message = std::string (buffer, bytesReceived);

                            return &event;
                        }
                    }

                    return &event;
                }

                bool network::send_reliable(unsigned int peerId, const std::string & packet, int channel) {
                    if (!m_active) {
                        return false;
                    }

                    if (m_clientSocket && SDLNet_TCP_GetPeerAddress(m_clientSocket)->host == peerId) {
                        int bytesSent = SDLNet_TCP_Send(m_clientSocket, packet.c_str(), packet.length());

                        if (bytesSent < packet.length()) {
                            return false;
                        }

                        m_bytesSent += bytesSent;

                        return true;
                    }

                    if (m_serverSockets.find(peerId) != m_serverSockets.end()) {
                        int bytesSent = SDLNet_TCP_Send(m_serverSockets[peerId], packet.c_str(), packet.length());

                        if (bytesSent < packet.length()) {
                            return false;
                        }

                        m_bytesSent += bytesSent;

                        return true;
                    }

                    return false;
                }

                bool network::send_unreliable(unsigned int peerId, const std::string & packet, int channel) {
                    // Not implemented
                    return false;
                }

                bool network::set_bandwidth_limits(double incomingBandwidth, double outgoingBandwidth) {
                    // Not implemented
                    return false;
                }

                bool network::setup_client(int channels, int maxPeers) {
                    if (m_active) {
                        return false;
                    }


                    m_socketSet = SDLNet_AllocSocketSet(1);

                    if (!m_socketSet) {
                        return false;
                    }

                    m_active = true;

                    return true;
                }

                bool network::setup_server(int listeningPort, int channels, int maxPeers) {
                    if (m_active) {
                        return false;
                    }

                    IPaddress ipAddress;

                    if (SDLNet_ResolveHost(&ipAddress, nullptr, listeningPort) == -1) {
                        return false;
                    }

                    TCPsocket socket = SDLNet_TCP_Open(&ipAddress);

                    if (!socket) {
                        return false;
                    }

                    m_socketSet = SDLNet_AllocSocketSet(1);

                    if (!m_socketSet) {
                        return false;
                    }

                    if (SDLNet_TCP_AddSocket(m_socketSet, socket) == -1) {
                        return false;
                    }

                    m_serverSocket = socket;

                    m_active = true;

                    return true;
                }

                int network::get_connected_peers() const {
                    return m_connectedPeers;
                }

                double network::get_bytes_sent() const {
                    return m_bytesSent;
                }

                double network::get_bytes_received() const {
                    return m_bytesReceived;
                }

                bool network::is_active() const {
                    return m_active;
                }
                void library::construct() {}
                void library::destruct() {}
                bool library::load(const std::string & libname) {
                    return lib = LoadLibraryA(libname.c_str());
                }
                template<typename... Args>
                CScriptDictionary* library::call(const std::string function_name, Args... args) {
                    CScriptDictionary CallResult;
                    if (lib == nullptr) {
                        return nullptr; // library not loaded
                    }
//                    auto function = reinterpret_cast<ReturnType(*)(Args...)>(GetProcAddress(lib, function_name));
//                    if (function == nullptr) {
//                        return nullptr; // function not found
//                    }
//                    function(args...);


                }
                    void library::unload() {
                    FreeLibrary(lib);
                }
                void instance::construct() {}
                void instance::destruct() {}

                bool instance::is_running() {
                            DWORD result = WaitForSingleObject(mutex, 0);
                            if (result == WAIT_OBJECT_0) {
                                ReleaseMutex(mutex);
                                return false;
                            }
                            return true;
                        }

user_idle::user_idle() {}
uint64_t user_idle::elapsed_millis() {
    return get_idle_time();
}
uint64_t user_idle::elapsed_seconds() {
    return get_idle_time() / 1000;
}
uint64_t user_idle::elapsed_minutes() {
    return elapsed_seconds() / 60;
}
uint64_t user_idle::elapsed_hours() {
    return elapsed_minutes() / 60;
}
uint64_t user_idle::elapsed_days() {
    return elapsed_hours() / 24;
}
uint64_t user_idle::elapsed_weeks() {
    return elapsed_days() / 7;
}
uint64_t user_idle::get_idle_time() {
    LASTINPUTINFO lii = { sizeof(LASTINPUTINFO) };
    GetLastInputInfo(&lii);
    DWORD currentTick = GetTickCount();
    return (currentTick - lii.dwTime);
}

uint64_t get_time_stamp_millis() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

uint64_t get_time_stamp_seconds() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return seconds;
}