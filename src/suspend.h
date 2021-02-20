/*
 * Copyright (C) 2020 Arnaud Ferraris <arnaud.ferraris@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "manager.h"

void suspend_init (struct EG25Manager *data, toml_table_t *config);
void suspend_destroy (struct EG25Manager *data);

void suspend_inhibit (struct EG25Manager *data, gboolean inhibit, gboolean block);
