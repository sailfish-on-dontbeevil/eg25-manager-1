/*
 * Copyright (C) 2020 Oliver Smith <ollieparanoid@postmarketos.org>
 * Copyright (C) 2021 Bhushan Shah <bshah@kde.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ofono-iface.h"

#include <string.h>

#include <libgdbofono/gdbo-manager.h>
#include <libgdbofono/gdbo-modem.h>

#define OFONO_SERVICE "org.ofono"

static void modem_added_cb(GDBOManager *manager_proxy,
                           const gchar *path,
                           GVariant *properties,
                           struct EG25Manager *manager)
{
    GVariant *modem_path;
    g_debug("Adding ofono modem '%s'", path);

    if (manager->modem_state == EG25_STATE_RESUMING) {
        if (manager->modem_recovery_timer) {
            g_source_remove(manager->modem_recovery_timer);
            manager->modem_recovery_timer = 0;
        }
        modem_resume_post(manager);
        manager->modem_state = EG25_STATE_CONFIGURED;
    }

    if (manager->modem_state < EG25_STATE_ACQUIRED)
        manager->modem_state = EG25_STATE_ACQUIRED;

    if (manager->modem_state < EG25_STATE_CONFIGURED)
        modem_configure(manager);

    modem_path = g_variant_lookup_value(properties, "SystemPath", G_VARIANT_TYPE_STRING);
    if (manager->modem_usb_id)
        g_free(manager->modem_usb_id);
    manager->modem_usb_id = g_strdup(strrchr(g_variant_dup_string(modem_path, NULL), '/') + 1);
}

static void modem_removed_cb(GDBOManager *manager_proxy,
                             const gchar *path,
                             struct EG25Manager *manager)
{
}

static void get_modems_cb(GDBOManager *manager_proxy,
                          GAsyncResult *res,
                          struct EG25Manager *manager)
{
    gboolean ok;
    GVariant *modems;
    GVariantIter *modems_iter = NULL;
    g_autoptr(GError) error = NULL;

    const gchar *path;
    GVariant *properties;

    ok = gdbo_manager_call_get_modems_finish(manager_proxy, &modems,
                                             res, &error);
    if (!ok) {
        g_warning("Error getting modems from ofono manager: %s", error->message);
        return;
    }

    g_variant_get(modems, "a(oa{sv})", &modems_iter);
    while(g_variant_iter_loop(modems_iter, "(&o@a{sv})", &path, &properties)) {
        g_debug("Got modem object path '%s'", path);
        modem_added_cb(manager_proxy, path, properties, manager);
    }
    g_variant_iter_free(modems_iter);
    g_variant_unref(modems);
}

static void ofono_appeared_cb(GDBusConnection    *connection,
                              const gchar        *name,
                              const gchar        *name_owner,
                              struct EG25Manager *manager)
{
    GError *error = NULL;

    g_message("oFono appeared on D-Bus");

    if (manager->modem_iface != MODEM_IFACE_NONE) {
        g_critical("Modem interface already found! Make sure to only run either of ModemManager or oFono.");
        return;
    }
    /* now connect to oFono! */
    manager->ofono_connection = connection;
    manager->ofono_manager = gdbo_manager_proxy_new_sync(connection,
                                                         G_DBUS_PROXY_FLAGS_NONE,
                                                         OFONO_SERVICE,
                                                         "/",
                                                         NULL,
                                                         &error);
    if (!manager->ofono_manager) {
        g_critical("Error creating ofono object manager proxy: %s", error->message);
        return;
    }

    manager->modem_iface = MODEM_IFACE_OFONO;

    g_signal_connect(manager->ofono_manager, "modem-added",
                     G_CALLBACK(modem_added_cb), manager);
    g_signal_connect(manager->ofono_manager, "modem-removed",
                     G_CALLBACK(modem_removed_cb), manager);

    gdbo_manager_call_get_modems(manager->ofono_manager,
                                 NULL,
                                 (GAsyncReadyCallback) get_modems_cb,
                                 manager);
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
