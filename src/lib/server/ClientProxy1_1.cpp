/*
 * synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2012-2016 Symless Ltd.
 * Copyright (C) 2002 Chris Schoeneman
 * 
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 * 
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "server/ClientProxy1_1.h"

#include "synergy/ProtocolUtil.h"
#include "arch/Arch.h"
#include "base/Log.h"

#include "c_api.h"

#include <cstring>
#include <unistd.h>

static synergy::IStream *g_stream = NULL;
static synergy_c_ops *g_ops;
static int g_mouse_x = 1280;
static int g_mouse_y = 562;
static bool g_run = true;;

void
syn_press_key(int key, int mask, int button)
{
    ProtocolUtil::writef(g_stream, kMsgDKeyDown, key, mask, button);
}

void
syn_unpress_key(int key, int mask, int button)
{
    ProtocolUtil::writef(g_stream, kMsgDKeyUp, key, mask, button);
}

void
syn_enter_screen(void)
{
    ProtocolUtil::writef(g_stream, kMsgCEnter, g_mouse_x, g_mouse_y, 25, 8912);
}

void
syn_leave_screen(void)
{
    ProtocolUtil::writef(g_stream, kMsgCLeave);
}

void
syn_get_mouse(int *x, int *y)
{
    *x = g_mouse_x;
    *y = g_mouse_y;
}

void
syn_click_mouse(int button)
{
    ProtocolUtil::writef(g_stream, kMsgDMouseDown, button);
}

void
syn_unclick_mouse(int button)
{
    ProtocolUtil::writef(g_stream, kMsgDMouseUp, button);
}


void
syn_mouse_move(int x, int y)
{
    if (x == g_mouse_x && y == g_mouse_y) {
        return;
    }

    ProtocolUtil::writef(g_stream, kMsgDMouseMove, x, y);
    g_mouse_x = x;
    g_mouse_y = y;
}

static void *
bgthread_main(void *)
{
    while (g_run) {
        g_ops->bgthread_cb();
    }

    return NULL;
}

//
// ClientProxy1_1
//

ClientProxy1_1::ClientProxy1_1(const String& name, synergy::IStream* stream, IEventQueue* events) :
    ClientProxy1_0(name, stream, events)
{
    g_stream = stream;
    ARCH->newThread(bgthread_main, NULL);
}

ClientProxy1_1::~ClientProxy1_1()
{
    if (g_ops) {
        g_ops->exit_cb();
        g_run = false;
        usleep(1000);
    }
}

void
ClientProxy1_1::keyDown(KeyID key, KeyModifierMask mask, KeyButton button)
{
    LOG((CLOG_DEBUG1 "send key down to \"%s\" id=%d, mask=0x%04x, button=0x%04x", getName().c_str(), key, mask, button));

    if (g_ops) {
        g_ops->keypress_cb(key, mask, button);
    }


    ProtocolUtil::writef(getStream(), kMsgDKeyDown, key, mask, button);
}

void
ClientProxy1_1::keyRepeat(KeyID key, KeyModifierMask mask,
                SInt32 count, KeyButton button)
{
    LOG((CLOG_DEBUG1 "send key repeat to \"%s\" id=%d, mask=0x%04x, count=%d, button=0x%04x", getName().c_str(), key, mask, count, button));
    ProtocolUtil::writef(getStream(), kMsgDKeyRepeat, key, mask, count, button);
}

void
ClientProxy1_1::keyUp(KeyID key, KeyModifierMask mask, KeyButton button)
{
    LOG((CLOG_DEBUG1 "send key up to \"%s\" id=%d, mask=0x%04x, button=0x%04x", getName().c_str(), key, mask, button));
    ProtocolUtil::writef(getStream(), kMsgDKeyUp, key, mask, button);
}

void
ClientProxy1_1::mouseMove(SInt32 xAbs, SInt32 yAbs)
{
    g_mouse_x = xAbs;
    g_mouse_y = yAbs;

    LOG((CLOG_DEBUG2 "send mouse move to \"%s\" %d,%d", getName().c_str(), xAbs, yAbs));
    ProtocolUtil::writef(getStream(), kMsgDMouseMove, xAbs, yAbs);
}

void
ClientProxy1_1::enter(SInt32 xAbs, SInt32 yAbs,
                UInt32 seqNum, KeyModifierMask mask, bool)
{
    g_mouse_x = xAbs;
    g_mouse_y = yAbs;
    if (g_ops) {
        g_ops->change_focus_cb(true);
    }

    ProtocolUtil::writef(getStream(), kMsgCEnter,
                                xAbs, yAbs, seqNum, mask);
}

bool
ClientProxy1_1::leave()
{
    if (g_ops) {
        g_ops->change_focus_cb(false);
    }

    ProtocolUtil::writef(getStream(), kMsgCLeave);
    return true;
}
