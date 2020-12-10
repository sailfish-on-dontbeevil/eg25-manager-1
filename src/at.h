/*
 * Copyright (C) 2020 Arnaud Ferraris <arnaud.ferraris@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "manager.h"

int at_init(struct EG25Manager *data);
void at_destroy(struct EG25Manager *data);

void at_sequence_configure(struct EG25Manager *data);
void at_sequence_suspend(struct EG25Manager *data);
void at_sequence_resume(struct EG25Manager *data);
void at_sequence_reset(struct EG25Manager *data);
