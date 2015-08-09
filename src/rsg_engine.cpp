/* Copyright (c) 2015. The SimGrid Team. All rights reserved.              */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero Licence (see in file LICENCE).        */

#include "rsg/engine.hpp"
#include "command.h"
#include "xbt/ex.h"

namespace rsg = simgrid::rsg;

rsg::Engine *rsg::Engine::p_instance = NULL;
rsg::Engine *rsg::Engine::getInstance() {
	if (p_instance == NULL) {
		p_instance = new Engine();
		check_commands();
	}
	return p_instance;
}