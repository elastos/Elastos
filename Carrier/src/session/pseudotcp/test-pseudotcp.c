/*
 * This file is part of the Nice GLib ICE library.
 *
 * (C) 2010 Collabora Ltd.
 *  Contact: Youness Alaoui
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Nice GLib ICE library.
 *
 * The Initial Developers of the Original Code are Collabora Ltd and Nokia
 * Corporation. All Rights Reserved.
 *
 * Contributors:
 *   Youness Alaoui, Collabora Ltd.
 *
 * Alternatively, the contents of this file may be used under the terms of the
 * the GNU Lesser General Public License Version 2.1 (the "LGPL"), in which
 * case the provisions of LGPL are applicable instead of those above. If you
 * wish to allow use of your version of this file only under the terms of the
 * LGPL and not to allow others to use your version of this file under the
 * MPL, indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL. If you do
 * not delete the provisions above, a recipient may use your version of this
 * file under either the MPL or the LGPL.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <locale.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "pseudotcp.h"

////////////////////////////////////////////////////////////////////////////////
// Porting section

#if !defined(WIN32) && !defined(WIN64)
#include <unistd.h>
#else
#include <crystal.h>
#endif
#include <pthread.h>

#include "gfake.h"

typedef struct GMainLoop {
    pthread_t thread;
    int started;
    int quit;
} GMainLoop;

#define MAX_TIMERS          16

struct TimerEntry {
    guint id;
    GSourceFunc function;
    guint interval;
    gint64 next_trigger_time;
    gpointer data;
};

static struct TimerEntry timers[MAX_TIMERS];
static guint timers_count = 0;
static guint last_timer_id = 1;

static int timer_compare(const void *a, const void *b)
{
    const struct TimerEntry *t1 = (const struct TimerEntry *)a;
    const struct TimerEntry *t2 = (const struct TimerEntry *)b;

    if (t1->next_trigger_time > t2->next_trigger_time)
        return 1;

    if (t1->next_trigger_time < t2->next_trigger_time)
        return -1;

    if (t1->id > t2->id)
        return 1;

    if (t1->id < t2->id)
        return -1;

    return 0;
}

static void g_main_loop_destory(void *obj)
{
    GMainLoop *loop = (GMainLoop *)obj;
    if (loop->started) {
        loop->quit = 1;
        //pthread_join(loop->thread, NULL);
    }
}

gboolean g_source_remove (guint tag);

static void g_source_update (guint tag)
{
    int i;
    guint interval;

    for (i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].id != tag)
            continue;

        interval = timers[i].interval;
        timers[i].next_trigger_time = interval == UINT_MAX ? INT64_MAX :
        g_get_monotonic_time() + interval * 1000;
        return;
    }
}

static void *g_main_loop(void *arg)
{
    int i;
    GMainLoop *loop = (GMainLoop *)arg;
    gint64 now;
    struct TimerEntry local_timers[MAX_TIMERS];

    ref(loop);
    loop->started = 1;

    while (!loop->quit) {
        usleep(100);

        if (timers_count == 0)
            continue;

        memcpy(local_timers, timers, sizeof(timers));
        qsort(local_timers, MAX_TIMERS, sizeof(struct TimerEntry), timer_compare);

        now = g_get_monotonic_time();

        for (i = 0; i < MAX_TIMERS; i++) {
            if (local_timers[i].function == NULL)
                continue;

            if (local_timers[i].next_trigger_time <= now ||
                local_timers[i].next_trigger_time == INT64_MAX) {
                gboolean rc = local_timers[i].function(local_timers[i].data);
                if (rc) {
                    if (local_timers[i].next_trigger_time != INT64_MAX)
                        g_source_update(local_timers[i].id);
                } else {
                    g_source_remove(local_timers[i].id);
                }
            }
        }
    }

    loop->started = 0;
    deref(loop);

    return NULL;
}

void g_main_loop_run (GMainLoop *loop)
{
    if (loop && !loop->started) {
        //pthread_create(&loop->thread, NULL, g_main_loop, loop);
        g_main_loop(loop);
    }
}

GMainLoop *g_main_loop_new (void *context, gboolean is_running)
{
    GMainLoop *loop = rc_zalloc(sizeof(GMainLoop), g_main_loop_destory);
    if (!loop)
        return NULL;

    if (is_running) {
        g_main_loop_run(loop);
    }

    return loop;
}

void g_main_loop_quit (GMainLoop *loop)
{
    if (loop && loop->started) {
        loop->quit = 1;
        //pthread_join(loop->thread, NULL);
    }
}

guint g_timeout_add (guint interval, GSourceFunc function, gpointer data)
{
    int i;

    for (i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].id != 0)
            continue;

        timers[i].id = last_timer_id++;
        timers[i].interval = interval;
        timers[i].next_trigger_time = interval == UINT_MAX ? INT64_MAX :
        g_get_monotonic_time() + interval * 1000;
        timers[i].data = data;
        timers[i].function = function;

        timers_count++;

        return timers[i].id;
    }

    return 0;
}

guint g_idle_add (GSourceFunc function, gpointer data)
{
    return g_timeout_add(UINT_MAX, function, data);
}

gboolean g_source_remove (guint tag)
{
    int i;

    for (i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].id != tag)
            continue;

        memset(&timers[i], 0, sizeof(struct TimerEntry));
        timers_count--;

        return true;
    }

    return false;
}

const char *g_strerror (int error)
{
    static char buf[64];

    sprintf(buf, "%d", error);
    return (const char *)buf;
}

// Porting section
////////////////////////////////////////////////////////////////////////////////

PseudoTcpSocket *left;
PseudoTcpSocket *right;
GMainLoop *mainloop = NULL;
FILE *in = NULL;
FILE *out = NULL;
int total_read = 0;
int total_wrote = 0;
guint left_clock = 0;
guint right_clock = 0;
gboolean left_closed;
gboolean right_closed;

gboolean reading_done = FALSE;

static void adjust_clock (PseudoTcpSocket *sock);

static void write_to_sock (PseudoTcpSocket *sock)
{
  gchar buf[1024];
  gsize len;
  gint wlen;
  guint total = 0;

  while (TRUE) {
    len = fread (buf, 1, sizeof(buf), in);
    if (len == 0) {
      g_debug ("Done reading data from file");
      g_assert (feof (in));
      reading_done = TRUE;
      pseudo_tcp_socket_close (sock, FALSE);
      break;
    } else {
      wlen = pseudo_tcp_socket_send (sock, buf, len);
      g_debug ("Sending %" G_GSIZE_FORMAT " bytes : %d", len, wlen);
      total += wlen;
      total_read += wlen;
      if (wlen < (gint) len) {
        g_debug ("seeking  %d from %lu", wlen - (gint)len, ftell (in));
        fseek (in, wlen - len, SEEK_CUR);
        g_assert (!feof (in));
        g_debug ("Socket queue full after %d bytes written", total);
        break;
      }
    }
  }
  adjust_clock (sock);
}

static void opened (PseudoTcpSocket *sock, gpointer data)
{
  g_debug ("Socket %p Opened", sock);
  if (sock == left) {
    if (in)
      write_to_sock (sock);
    else {
      pseudo_tcp_socket_send (sock, "abcdefghijklmnopqrstuvwxyz", 26);
      reading_done = TRUE;
      pseudo_tcp_socket_close (sock, FALSE);
    }
  }
}

static void readable (PseudoTcpSocket *sock, gpointer data)
{
  gchar buf[1024];
  gint len;
  g_debug ("Socket %p Readable", sock);

  do {
    len = pseudo_tcp_socket_recv (sock, buf, sizeof(buf));

    if (len > 0) {
      g_debug ("Read %d bytes", len);
      if (out) {
        if (fwrite (buf, len, 1, out) == 0)
          g_debug ("Error writing to output file");
        else {
          total_wrote += len;

          g_assert (total_wrote <= total_read);
          g_debug ("Written %d bytes, need %d bytes", total_wrote, total_read);
          if (total_wrote == total_read && feof (in)) {
            g_assert (reading_done);
            pseudo_tcp_socket_close (sock, FALSE);
          }
        }
      } else {
        if (len == 26 && strncmp (buf, "abcdefghijklmnopqrstuvwxyz", len) == 0) {
          pseudo_tcp_socket_close (sock, FALSE);
        } else {
          g_debug ("Error reading data.. read %d bytes : %s", len, buf);
          exit (-1);
        }
      }
    } else if (len == 0) {
      pseudo_tcp_socket_close (sock, FALSE);
    }
  } while (len > 0);

  if (len == -1 &&
      pseudo_tcp_socket_get_error (sock) != EWOULDBLOCK) {
    g_debug ("Error reading from socket %p: %s", sock,
        g_strerror (pseudo_tcp_socket_get_error (sock)));
    exit (-1);
  }
}

static void writable (PseudoTcpSocket *sock, gpointer data)
{
  g_debug ("Socket %p Writable", sock);
  if (in && sock == left)
    write_to_sock (sock);
}

static void closed (PseudoTcpSocket *sock, guint32 err, gpointer data)
{
  g_error ("Socket %p Closed : %d", sock, err);
}

struct notify_data {
  PseudoTcpSocket *sock;
  guint32 len;
  gchar buffer[];
};

static gboolean notify_packet (gpointer user_data)
{
  struct notify_data *data = (struct notify_data*) user_data;

  pseudo_tcp_socket_notify_packet (data->sock, data->buffer, data->len);
  adjust_clock (data->sock);

  g_free (data);
  return FALSE;
}

static PseudoTcpWriteResult write_packet (PseudoTcpSocket *sock,
    const gchar *buffer, guint32 len, gpointer user_data)
{
  struct notify_data *data;
  PseudoTcpState state;
  int drop_rate = rand () % 100;
  pseudo_tcp_socket_get_property (sock, PROP_STATE, &state);

  if (drop_rate < 5) {
    g_debug ("*********************Dropping packet (%d) from %p", drop_rate,
        sock);
    return WR_SUCCESS;
  }

  data = g_malloc (sizeof(struct notify_data) + len);

  g_debug ("Socket %p(%d) Writing : %d bytes", sock, state, len);

  memcpy (data->buffer, buffer, len);
  data->len = len;

  if (sock == left)
    data->sock = right;
  else
    data->sock = left;

  g_idle_add (notify_packet, data);

  return WR_SUCCESS;
}


static gboolean notify_clock (gpointer data)
{
  PseudoTcpSocket *sock = (PseudoTcpSocket *)data;
  //g_debug ("Socket %p: Notifying clock", sock);
  pseudo_tcp_socket_notify_clock (sock);
  adjust_clock (sock);
  return FALSE;
}

static void adjust_clock (PseudoTcpSocket *sock)
{
  guint64 timeout = 0;

  if (pseudo_tcp_socket_get_next_clock (sock, &timeout)) {
    timeout -= g_get_monotonic_time () / 1000;
    g_debug ("Socket %p: Adjusting clock to %" G_GUINT64_FORMAT " ms", sock, timeout);
    if (sock == left) {
      if (left_clock != 0)
         g_source_remove (left_clock);
      left_clock = g_timeout_add (timeout, notify_clock, sock);
    } else {
      if (right_clock != 0)
         g_source_remove (right_clock);
      right_clock = g_timeout_add (timeout, notify_clock, sock);
    }
  } else {
    g_debug ("Socket %p should be destroyed, it's done", sock);
    if (sock == left)
      left_closed = TRUE;
    else
      right_closed = TRUE;
    if (left_closed && right_closed)
      g_main_loop_quit (mainloop);
  }
}


int main (int argc, char *argv[])
{
  PseudoTcpCallbacks cbs = {
    NULL, opened, readable, writable, closed, write_packet
  };

  setlocale (LC_ALL, "");

  mainloop = g_main_loop_new (NULL, FALSE);

  pseudo_tcp_set_debug_level (PSEUDO_TCP_DEBUG_VERBOSE);

  left_closed = right_closed = FALSE;

  left = pseudo_tcp_socket_new (0, &cbs);
  right = pseudo_tcp_socket_new (0, &cbs);
  g_debug ("Left: %p. Right: %p", left, right);

  pseudo_tcp_socket_notify_mtu (left, 1496);
  pseudo_tcp_socket_notify_mtu (right, 1496);

  pseudo_tcp_socket_connect (left);
  adjust_clock (left);
  adjust_clock (right);

  if (argc == 3) {
    in = fopen (argv[1], "r");
    out = fopen (argv[2], "w");
  }

  g_main_loop_run (mainloop);

  g_object_unref (left);
  g_object_unref (right);

  if (in)
    fclose (in);
  if (out)
    fclose (out);

  return 0;
}

