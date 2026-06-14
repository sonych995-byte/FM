#pragma once

#include "common.hpp"

void cmd_cp(const std::vector<std::string>& args);
void cmd_rn(const std::vector<std::string>& args);
void cmd_rm(const std::vector<std::string>& args);
void cmd_cd(const std::vector<std::string>& args);
void cmd_mk(const std::vector<std::string>& args);
void cmd_info(const std::vector<std::string>& args);
void cmd_ls(const std::vector<std::string>& args);
void cmd_oscmd(const std::vector<std::string>& args);

void cmd_pwd();
void cmd_help();