// 3D World - OpenAL Interface Code
// by Frank Gennari
// 8/28/11
// Sounds from http://www.findsounds.com
#include "openal_wrap.h"
#include <iostream>
#include <assert.h>
#include <al.h>
#include <alc.h>
#include <AL/alut.h>

using namespace std;


unsigned const NUM_CHANNELS = 8;
string const sounds_path("sounds/");

buffer_manager_t sounds;
source_manager_t sources;


void setup_sounds() {

	sources.create_channels(NUM_CHANNELS);
	sounds.add_file_buffer("explosion1.au"); // SOUND_EXPLODE
}


void alut_sleep(float seconds) {
	alutSleep(seconds);
}


bool had_al_error  () {return (alGetError  () != AL_NO_ERROR);}
bool had_alut_error() {return (alutGetError() != AL_NO_ERROR);}

bool check_and_print_alut_error() { // returns 1 on error

	ALenum const error_id(alutGetError());

	if (error_id != AL_NO_ERROR) {
		cerr << "alut error: " << alutGetErrorString(error_id) << endl;
		return 1;
	}
	return 0;
}


// openal_buffer
void openal_buffer::alloc() {
	assert(!is_valid());
	alGenBuffers(1, &buffer);
    
	if (had_al_error()) {
		cerr << "Error creating OpenAL buffers" << endl;
		exit(1);
	}
	assert(is_valid());
}

void openal_buffer::free() {
	if (is_valid()) alDeleteBuffers(1, &buffer);
	buffer = 0;
	time   = 0.0;
}

bool openal_buffer::load_check() {
	if (check_and_print_alut_error()) {
		free();
		return 0;
	}
	return 1;
}

bool openal_buffer::load_from_file(string const &fn) {
	assert(!fn.empty());
	buffer = alutCreateBufferFromFile(fn.c_str());
	return load_check();
}

bool openal_buffer::load_from_file_std_path(std::string const &fn) {
	assert(!fn.empty());
	return load_from_file(sounds_path + fn);
}

bool openal_buffer::load_from_memory(void const *data, size_t length) {
	assert(data);
	assert(length > 0);
	buffer = alutCreateBufferFromFileImage(data, length);
	return load_check();
}

bool openal_buffer::load_from_waveform(int waveshape, float frequency, float phase, float duration) {
	assert(frequency > 0.0 && duration > 0.0);
	buffer = alutCreateBufferWaveform(waveshape, frequency, phase, duration);
	time   = duration;
	return load_check();
}


unsigned buffer_manager_t::add_file_buffer(std::string const &fn) {

	unsigned const ix(buffers.size());
	buffers.push_back(openal_buffer());
	
	if (!buffers.back().load_from_file_std_path(fn)) {
		cerr << "Failed to load sound file: " << fn << endl;
		exit(1);
	}
	return ix;
}


// openal_source
void openal_source::alloc() {
	assert(!is_valid());
	alGenSources(1, &source);

	if (had_al_error()) {
		cerr << "Error creating OpenAL sources" << endl;
		exit(1);
	}
	assert(is_valid());
}

void openal_source::free() {
	if (is_valid()) {
		alDeleteSources(1, &source);
		source = 0;
	}
}

void openal_source::setup(openal_buffer const &buffer, point const &pos, vector3d const &vel, float pitch, float gain, bool looping) {
	assert(is_valid() && buffer.is_valid());
	alSourcef (source, AL_PITCH,    pitch);
	alSourcef (source, AL_GAIN,     gain);
	alSourcefv(source, AL_POSITION, &pos.x);
	alSourcefv(source, AL_VELOCITY, &vel.x);
	alSourcei (source, AL_LOOPING,  looping);
	set_buffer_ix(buffer.get_buffer_ix());
}

void openal_source::set_buffer_ix(unsigned buffer_ix) {alSourcei(source, AL_BUFFER, buffer_ix);}

void openal_source::play()   const {alSourcePlay  (source);}
void openal_source::stop()   const {alSourceStop  (source);}
void openal_source::pause()  const {alSourcePause (source);}
void openal_source::rewind() const {alSourceRewind(source);}


// listner code
void setup_openal_listener(point const &pos, vector3d const &vel, openal_orient const &orient) {

	alListenerfv(AL_POSITION,    &pos.x);
    alListenerfv(AL_VELOCITY,    &vel.x);
    alListenerfv(AL_ORIENTATION, &orient.at.x);
}


void set_openal_listener_as_player() {

	// Note: velocity is zero for now because the actual player velocity is not constant,
	// and passing it into the listener setup as a constant is incorrect
	vector3d const cvel(zero_vector);
	setup_openal_listener(get_camera_all(), cvel, openal_orient(get_vdir_all(), get_upv_all()));
}


void gen_sound(unsigned id, point const &pos, vector3d const &vel, float pitch, float gain, bool looping) { // non-blocking

	//RESET_TIME;
	openal_buffer &buffer(sounds.get_buffer(id));
	openal_source &source(sources.get_source());
	set_openal_listener_as_player();
	source.setup(buffer, pos, vel, pitch, gain, looping);
	source.play();
	//PRINT_TIME("Play Sound");
}


void openal_hello_world() {

	openal_buffer buffer(alutCreateBufferHelloWorld());
	openal_source source;
	source.alloc();
	source.set_buffer(buffer);
	source.play();
	alut_sleep(1.0);
	source.free();
	buffer.free();
}


void init_openal(int &argc, char** argv) {

	//return; // not yet ready

	if (!alutInit(&argc, argv)) {
		check_and_print_alut_error();
		cerr << "alutInit failed" << endl;
		exit(1);
	}
	alGetError(); // ignore any previous errors
	setup_sounds();
	//cout << "Supported OpenAL types: " << alutGetMIMETypes(ALUT_LOADER_BUFFER) << endl;
	//openal_hello_world();
}


void exit_openal() {

	if (!alutExit()) check_and_print_alut_error();
}


