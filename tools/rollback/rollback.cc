/*
* rollback.cc
* git-rollback
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <git2.h>
#include "rollback.hpp"

RollbackDriver::RollbackDriver() { git_libgit2_init(); }

RollbackDriver::~RollbackDriver() { git_libgit2_shutdown(); }
