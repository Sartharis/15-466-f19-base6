#pragma once

#include <SDL.h>
#include <glm/glm.hpp>

#include <memory>

struct Mode : std::enable_shared_from_this< Mode > {
	virtual ~Mode() { }
	bool paused = false;

	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) { return false; }

	//update is called at the start of a new frame, after events are handled:
	// 'elapsed' is time in seconds since the last call to 'update'
	virtual void update(float elapsed) { }

	virtual void on_resize( glm::uvec2 const& new_drawable_size ) {}
	virtual void on_paused(){}
	virtual void on_unpaused(){}

	//draw is called after update:
	virtual void draw(glm::uvec2 const &drawable_size) = 0;

	//Mode::current is the Mode to which events are dispatched.
	// use 'set_current' to change the current Mode (e.g., to switch to a menu)
	static std::shared_ptr< Mode > current;
	static void set_current(std::shared_ptr< Mode > const &);
};

