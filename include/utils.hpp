#pragma once

#include "common.hpp"

void home();
void pause();

void show_success();
void show_fail();
void show_error(const std::exception& e);

void clear_screen();

std::vector<std::string>
split(const std::string& text);