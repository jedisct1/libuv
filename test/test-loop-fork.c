/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "uv.h"
#include "task.h"

#ifndef _WIN32
# include <sys/wait.h>
# include <unistd.h>
#endif

static int timer_cb_called = 0;
static int timer_close_cb_called = 0;

static void timer_close_cb(uv_handle_t* handle) {
  printf("TIMER_CLOSE_CB\n");

  ASSERT(handle != NULL);

  timer_close_cb_called++;
}

static void timer_cb(uv_timer_t* handle, int status) {
  printf("TIMER_CB\n");

  ASSERT(handle != NULL);
  ASSERT(status == 0);

  timer_cb_called++;

  uv_close((uv_handle_t*)handle, timer_close_cb);
}

TEST_IMPL(loop_fork) {
  uv_timer_t timer;
  pid_t child;
  int r;

  r = uv_timer_init(uv_default_loop(), &timer);
  ASSERT(r == 0);
  r = uv_timer_start(&timer, timer_cb, 100, 0);
  ASSERT(r == 0);

#ifndef _WIN32
  child = fork();
  ASSERT(child != (pid_t) -1);
#endif

  uv_loop_fork(uv_default_loop());

  uv_run(uv_default_loop());

  ASSERT(timer_cb_called == 1);
  ASSERT(timer_close_cb_called == 1);

#ifndef _WIN32
  if (child != (pid_t) 0) {
      int status;

      ASSERT(waitpid(child, &status, 0) == child);
      ASSERT(WIFEXITED(status));
      ASSERT(WEXITSTATUS(status) == 0);
  }
#endif

  return 0;
}
