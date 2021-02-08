/*
 * Copyright (C) 2020 Oliver Smith <ollieparanoid@postmarketos.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ofono-iface.h"

#include <string.h>

#define OFONO_SERVICE "org.ofono"

static void ofono_appeared_cb(GDBusConnection    *connection,
                              const gchar        *name,
                              const gchar        *name_owner,
                              struct EG25Manager *manager)
{
    g_message("oFono appeared on D-Bus");

    if (manager->modem_iface != MODEM_IFACE_NONE) {
        g_critical("Modem interface already found! Make sure to only run either of ModemManager or oFono.");
        return;
    }
    manager->modem_iface = MODEM_IFACE_OFONO;

    /* now connect to oFono! */
}

static void ofono_vanished_cb(GDBusConnection    *connection,
                              const gchar        *name,
                              struct EG25Manager *manager)
{
    g_message("oFono vanished from D-Bus");

    if (manager->modem_iface == MODEM_IFACE_OFONO) {
        manager->modem_iface = MODEM_IFACE_NONE;
        ofono_iface_destroy(manager);
    }
}

void ofono_iface_init(struct EG25Manager *manager)
{
    manager->ofono_watch = g_bus_watch_name(G_BUS_TYPE_SYSTEM, OFONO_SERVICE,
                                            G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                                            (GBusNameAppearedCallback)ofono_appeared_cb,
                                            (GBusNameVanishedCallback)ofono_vanished_cb,
                                            manager, NULL);
}

void ofono_iface_destroy(struct EG25Manager *manager)
{
    if (manager->modem_usb_id) {
        g_free(manager->modem_usb_id);
        manager->modem_usb_id = NULL;
    }
    if (manager->ofono_watch != 0) {
        g_bus_unwatch_name(manager->ofono_watch);
        manager->ofono_watch = 0;
    }
}
