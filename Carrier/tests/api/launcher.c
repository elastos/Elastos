#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "config.h"

/******************************************************************************/
/* Output windows section                                                     */
/******************************************************************************/
WINDOW *test_out_border, *test_out_win;
WINDOW *test_log_border, *test_log_win;
WINDOW *robot_log_border, *robot_log_win;

pthread_mutex_t screen_lock = PTHREAD_MUTEX_INITIALIZER;

#define TEST_OUT        1
#define TEST_LOG        2
#define ROBOT_LOG       3

FILE *run_log_file;
FILE *robot_log_file;
FILE *test_log_file;

static void get_layout(int win, int *w, int *h, int *x, int *y)
{
    if (win == TEST_OUT) {
        *x = 0;
        *y = 0;

        *w = (COLS - 1) / 2;
        *h = LINES;
    } else if (win == TEST_LOG) {
        *x = COLS - (COLS / 2);
        *y = 0;

        *w = (COLS - 1) / 2;
        *h = (LINES - 1) / 2;
    } else if (win == ROBOT_LOG) {
        *x = COLS - (COLS / 2);
        *y = LINES - (LINES -1) / 2;

        *w = (COLS - 1) / 2;
        *h = (LINES - 1) / 2;
    }
}

static void handle_winch(int sig)
{
    int w, h, x, y;

    endwin();

    if (LINES < 24 || COLS < 80) {
        printf("Terminal size too small!\n");
        exit(-1);
    }

    refresh();
    clear();

    wresize(stdscr, LINES, COLS);

    get_layout(TEST_OUT, &w, &h, &x, &y);

    wresize(test_out_border, h, w);
    mvwin(test_out_border, y, x);
    box(test_out_border, 0, 0);
    mvwprintw(test_out_border, 0, 4, "Test Output");

    wresize(test_out_win, h-2, w-2);
    mvwin(test_out_win, y+1, x+1);

    get_layout(TEST_LOG, &w, &h, &x, &y);

    wresize(test_log_border, h, w);
    mvwin(test_log_border, y, x);
    box(test_log_border, 0, 0);
    mvwprintw(test_log_border, 0, 4, "Test Log");

    wresize(test_log_win, h-2, w-2);
    mvwin(test_log_win, y+1,  x+1);

    get_layout(ROBOT_LOG, &w, &h, &x, &y);

    wresize(robot_log_border, h, w);
    mvwin(robot_log_border, y, x);
    box(robot_log_border, 0, 0);
    mvwprintw(robot_log_border, 0, 4, "Robot Log");

    wresize(robot_log_win, h-2, w-2);
    mvwin(robot_log_win,  y+1,  x+1);

    clear();
    refresh();

    wrefresh(test_out_border);
    wrefresh(test_out_win);

    wrefresh(test_log_border);
    wrefresh(test_log_win);

    wrefresh(robot_log_border);
    wrefresh(robot_log_win);
}

static void init_screen(void)
{
    int w, h, x, y;

    initscr();

    if (LINES < 24 || COLS < 80) {
        printf("Terminal size too small!\n");
        endwin();
        exit(-1);
    }

    noecho();
    nodelay(stdscr, TRUE);
    refresh();

    get_layout(TEST_OUT, &w, &h, &x, &y);

    test_out_border = newwin(h, w, y, x);
    box(test_out_border, 0, 0);
    mvwprintw(test_out_border, 0, 4, "Test Result");
    wrefresh(test_out_border);

    test_out_win = newwin(h-2, w-2, y+1, x+1);
    scrollok(test_out_win, TRUE);
    wrefresh(test_out_win);

    get_layout(TEST_LOG, &w, &h, &x, &y);

    test_log_border = newwin(h, w, y, x);
    box(test_log_border, 0, 0);
    mvwprintw(test_log_border, 0, 4, "Test Log");
    wrefresh(test_log_border);

    test_log_win = newwin(h-2, w-2, y+1,  x+1);
    scrollok(test_log_win, TRUE);
    wrefresh(test_log_win);

    get_layout(ROBOT_LOG, &w, &h, &x, &y);

    robot_log_border = newwin(h, w, y, x);
    box(robot_log_border, 0, 0);
    mvwprintw(robot_log_border, 0, 4, "Robot Log");
    wrefresh(robot_log_border);

    robot_log_win = newwin(h-2, w-2, y+1,  x+1);
    scrollok(robot_log_win, true);
    wrefresh(robot_log_win);

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);
}

static void cleanup_screen(void)
{
    endwin();

    delwin(test_out_border);
    delwin(test_out_win);

    delwin(test_log_border);
    delwin(test_log_win);

    delwin(robot_log_border);
    delwin(robot_log_win);
}

static void open_log_files(void)
{
    run_log_file = fopen("run.log", "w+");
    test_log_file = fopen("tests.log", "w+");
    robot_log_file = fopen("robot.log", "w+");
}

static void close_log_files(void)
{
    if (run_log_file) {
        fclose(run_log_file);
        run_log_file = NULL;        
    }

    if (test_log_file) {
        fclose(test_log_file);
        test_log_file = NULL;
    }

    if (robot_log_file) {
        fclose(robot_log_file);
        robot_log_file = NULL;
    }
}

static void win_printf(int win, const char *format, ...)
{
    WINDOW *w = NULL;
    FILE *log = NULL;
    va_list ap;

    if (win == TEST_OUT) {
        w = test_out_win;
        log = run_log_file;
    } else if (win == TEST_LOG) {
        w = test_log_win;
        log = test_log_file;
    } else if (win == ROBOT_LOG) {
        w = robot_log_win;
        log = robot_log_file;
    } else
        return;

    va_start(ap, format);
    vwprintw(w, format, ap);
    wrefresh(w);
    va_end(ap);

    if (log) {
        va_start(ap, format);
        vfprintf(log, format, ap);
        fflush(log);
        va_end(ap);
    }
}

/******************************************************************************/
/* STDIO redirect section                                                     */
/******************************************************************************/

int test_out[2] = { -1, -1 };
int test_log[2] = { -1, -1 };
int robot_log[2] = { -1, -1 };
int robot_control[2] = { -1, -1 };
int robot_acknowledge[2] = { -1, -1 };

FILE *robot_control_in = NULL;
FILE *robot_acknowledge_in = NULL;

static void close_pipes(void)
{
    if (test_out[0] >= 0)
        close(test_out[0]);
    if (test_out[1] >= 0)
        close(test_out[1]);

    if (test_log[0] >= 0)
        close(test_log[0]);
    if (test_log[1] >= 0)
        close(test_log[1]);

    if (robot_log[0] >= 0)
        close(robot_log[0]);
    if (robot_log[1] >= 0)
        close(robot_log[1]);

    if (robot_control[0] >= 0)
        close(robot_control[0]);
    if (robot_control[1] >= 0)
        close(robot_control[1]);

    if (robot_acknowledge[0] >= 0)
        close(robot_acknowledge[0]);
    if (robot_acknowledge[1] >= 0)
        close(robot_acknowledge[1]);

    if (robot_control_in)
        fclose(robot_control_in);

    if (robot_acknowledge_in)
        fclose(robot_acknowledge_in);
}

static void create_pipes(void)
{
    int rc;

    rc = pipe(test_out);
    if (rc < 0)
        goto error_exit;

    rc = pipe(test_log);
    if (rc < 0)
        goto error_exit;

    rc = pipe(robot_log);
    if (rc < 0)
        goto error_exit;

    rc = pipe(robot_control);
    if (rc < 0)
        goto error_exit;

    rc = pipe(robot_acknowledge);
    if (rc < 0)
        goto error_exit;

    return;

error_exit:
    perror("Launch tests failed");
    close_pipes();
    exit(-1);
}

#define PIPE_IN         0
#define PIPE_OUT        1

#define TEST_LAUNCHER   0
#define TEST_RUNNER     1
#define TEST_ROBOT      2

static int setup_redirections(int role)
{
    int rc;

    if (role == TEST_LAUNCHER) {
        close(test_out[PIPE_OUT]);
        test_out[PIPE_OUT] = -1;

        close(test_log[PIPE_OUT]);
        test_log[PIPE_OUT] = -1;

        close(robot_log[PIPE_OUT]);
        robot_log[PIPE_OUT] = -1;

        close(robot_control[PIPE_IN]);
        robot_control[PIPE_IN] = -1;

        close(robot_acknowledge[PIPE_IN]);
        close(robot_acknowledge[PIPE_OUT]);
        robot_acknowledge[PIPE_IN] = -1;
        robot_acknowledge[PIPE_OUT] = -1;
    } else if (role == TEST_RUNNER) {
        close(test_out[PIPE_IN]);
        test_out[PIPE_IN] = -1;

        close(test_log[PIPE_IN]);
        test_log[PIPE_IN] = -1;

        close(robot_control[PIPE_IN]);
        robot_control[PIPE_IN] = -1;

        close(robot_acknowledge[PIPE_OUT]);
        robot_acknowledge[PIPE_OUT] = -1;

        close(robot_log[PIPE_IN]);
        close(robot_log[PIPE_OUT]);
        robot_log[PIPE_IN] = -1;
        robot_log[PIPE_OUT] = -1;

        rc = dup2(test_out[PIPE_OUT], STDOUT_FILENO);
        if (rc < 0)
            return -1;

        rc = dup2(test_out[PIPE_OUT], STDERR_FILENO);
        if (rc < 0)
            return -1;

        setbuf(stdout, NULL);
        setbuf(stderr, NULL);
        //fclose(stdin);

        robot_acknowledge_in = fdopen(robot_acknowledge[PIPE_IN], "r");
        if (robot_acknowledge_in == NULL)
            return -1;
    } else if (role == TEST_ROBOT) {
        close(robot_control[PIPE_OUT]);
        robot_control[PIPE_OUT] = -1;

        close(robot_acknowledge[PIPE_IN]);
        robot_acknowledge[PIPE_IN] = -1;

        close(robot_log[PIPE_IN]);
        robot_log[PIPE_IN] = -1;

        close(test_log[PIPE_IN]);
        close(test_log[PIPE_OUT]);
        test_log[PIPE_IN] = -1;
        test_log[PIPE_OUT] = -1;

        close(test_out[PIPE_IN]);
        close(test_out[PIPE_OUT]);
        test_out[PIPE_IN] = -1;
        test_out[PIPE_OUT] = -1;

        rc = dup2(robot_log[PIPE_OUT], STDOUT_FILENO);
        if (rc < 0) {
            return -1;
        }

        rc = dup2(robot_log[PIPE_OUT], STDERR_FILENO);
        if (rc < 0)
            return -1;

        setbuf(stdout, NULL);
        setbuf(stderr, NULL);
        //fclose(stdin);

        robot_control_in = fdopen(robot_control[PIPE_IN], "r");
        if (robot_control_in == NULL)
            return -1;
    }

    return 0;
}

static int log_vprintf(int role, const char * format, va_list ap)
{
    int fd = -1;

    if (role == TEST_RUNNER)
        fd = test_log[PIPE_OUT];
    else if (role == TEST_ROBOT)
        fd = robot_log[PIPE_OUT];
    else {
        assert(role != TEST_LAUNCHER);
        return -1;
    }

    assert(fd != -1);

    return vdprintf(fd, format, ap);
}

static int robot_log_vprintf(int level, const char *format, va_list ap)
{
    if (global_config.tests.loglevel >= level)
        return vprintf(format, ap);
    else
        return 0;
}

int robot_log_debug(const char *format, ...)
{
    int rc;
    va_list ap;

    va_start(ap, format);
    rc = robot_log_vprintf(ElaLogLevel_Debug, format, ap);
    va_end(ap);

    return rc;
}

int robot_log_info(const char *format, ...)
{
    int rc;
    va_list ap;

    va_start(ap, format);
    rc = robot_log_vprintf(ElaLogLevel_Info, format, ap);
    va_end(ap);

    return rc;
}

int robot_log_vinfo(const char *format, va_list ap)
{
    return robot_log_vprintf(ElaLogLevel_Info, format, ap);
}

int robot_log_error(const char *format, ...)
{
    int rc;
    va_list ap;

    va_start(ap, format);
    rc = robot_log_vprintf(ElaLogLevel_Error, format, ap);
    va_end(ap);

    return rc;
}

int test_log_vprintf(int level, const char *format, va_list ap)
{
    if (global_config.tests.loglevel >= level)
        return log_vprintf(TEST_RUNNER, format, ap);
    else
        return 0;
}

int test_log_debug(const char *format, ...)
{
    int rc;
    va_list ap;

    va_start(ap, format);
    rc = test_log_vprintf(ElaLogLevel_Debug, format, ap);
    va_end(ap);

    return rc;
}

int test_log_info(const char *format, ...)
{
    int rc;
    va_list ap;

    va_start(ap, format);
    rc = test_log_vprintf(ElaLogLevel_Info, format, ap);
    va_end(ap);

    return rc;
}

static int test_log_vinfo(const char *format, va_list ap)
{
    return test_log_vprintf(ElaLogLevel_Info, format, ap);
}

int test_log_error(const char *format, ...)
{
    int rc;
    va_list ap;

    va_start(ap, format);
    rc = test_log_vprintf(ElaLogLevel_Error, format, ap);
    va_end(ap);

    return rc;
}

int robot_ctrl(const char *cmd, ...)
{
    int rc;
    va_list ap;

    assert(robot_control[PIPE_OUT] != -1);
    assert(cmd);

    va_start(ap, cmd);
    rc = vdprintf(robot_control[PIPE_OUT], cmd, ap);
    va_end(ap);
    if (test_log[PIPE_OUT] != -1) {
        test_log_info("\n>>>>>>>>Control Robot: ");
        va_start(ap, cmd);
        test_log_vinfo(cmd, ap);
        va_end(ap);
    }

    return rc;
}

int robot_ack(const char *what, ...)
{
    int rc;
    va_list ap;

    assert(robot_acknowledge[PIPE_OUT] != -1);
    assert(what);

    va_start(ap, what);
    rc = vdprintf(robot_acknowledge[PIPE_OUT], what, ap);
    fsync(robot_acknowledge[PIPE_OUT]);
    va_end(ap);

    robot_log_info("\n>>>>>>>>Acknowledge: ");
    va_start(ap, what);
    robot_log_vinfo(what, ap);
    va_end(ap);

    return rc;
}

int wait_robot_ctrl(const char *format, ...)
{
    int rc;
    va_list ap;

    assert(robot_control_in);
    assert(format);

    va_start(ap, format);
    rc = vfscanf(robot_control_in, format, ap);
    va_end(ap);

    return rc;
}

int robot_ctrl_getchar(void)
{
    return fgetc(robot_control_in);
}

void robot_ctrl_nonblock(void)
{
    int val = 0;

    assert(robot_control[PIPE_IN] != -1);

    val = fcntl(robot_control[PIPE_IN], F_GETFL, 0);
    fcntl(robot_control[PIPE_IN], F_SETFL, val | O_NONBLOCK);
}

int wait_robot_ack(const char *format, ...)
{
    int rc;
    va_list ap;

    assert(robot_acknowledge_in);
    assert(format);

    va_start(ap, format);
    rc = vfscanf(robot_acknowledge_in, format, ap);
    va_end(ap);

    return rc;
}

/******************************************************************************/
/* Main section                                                               */
/******************************************************************************/

int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}

void wait_for_debugger_attach(void)
{
    printf("\nWait for debugger attaching, process id is: %d.\n", getpid());
    printf("After debugger attached, press any key to continue......");
    getchar();
}

static int test_pid = -1;
static int robot_pid = -1;

static void signal_handler(int signum)
{
    if (test_pid > 0)
        kill(test_pid, SIGKILL);
    
    if (robot_pid > 0)
        kill(robot_pid, SIGKILL);

    close_pipes();
    cleanup_screen();
    close_log_files();
    exit(-1);
}

int robot_main(int argc, char *argv[]);

int test_main(int argc, char *argv[]);

static const char *default_config = "tests.conf";

int main(int argc, char *argv[])
{
    int rc;

    fd_set fds, rfds;
    int total;
    int nfds;
    struct timeval timeout;

    char buffer[2048];

    const char *config_file;

    sys_coredump_set(true);

    if (argc == 1)
        config_file = default_config;
    else if (argc == 2)
        config_file = argv[1];
    else {
        printf("\nUsage: %s [config_file]\n", argv[0]);
        return -1;
    }

    load_config(config_file);

    create_pipes();

    robot_pid = fork();
    if (robot_pid < 0) {
        perror("Launch test robot failed");
        close_pipes();
        return -1;
    } else if (robot_pid == 0) {
        // Test Robot
        rc = setup_redirections(TEST_ROBOT);
        if (rc < 0) {
            perror("Can not setup test robot I/O redirections");
            close_pipes();
            return -1;
        }

        rc = robot_main(argc, argv);

        printf("Test robot terminated.\n");
        close_pipes();
        return rc;
    }

    test_pid = fork();
    if (test_pid < 0) {
        perror("Launch test runner failed");
        kill(robot_pid, SIGKILL);
        close_pipes();
        return -1;
    } else if (test_pid == 0) {
        // Test Runner
        rc = setup_redirections(TEST_RUNNER);
        if (rc < 0) {
            perror("Can not setup test runner I/O redirections");
            kill(robot_pid, SIGKILL);
            close_pipes();
            return -1;
        }

        rc = test_main(argc, argv);
        
        robot_ctrl("kill\n");
        test_log_info("Test runner terminated.\n");
        close_pipes();
        return rc;
    }

    printf("Test runner pid: %d, Test robot pid: %d\n", test_pid, robot_pid);

    setup_redirections(TEST_LAUNCHER);

    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGSEGV, signal_handler);

    if (global_config.log2file)
        open_log_files();

    init_screen();

    FD_ZERO(&fds);
    FD_SET(test_out[PIPE_IN], &fds);
    FD_SET(test_log[PIPE_IN], &fds);
    FD_SET(robot_log[PIPE_IN], &fds);

    total = 3;

    timerclear(&timeout);
    while (total) {
        if (!timerisset(&timeout))
            timeout.tv_sec = 5;

        memcpy(&rfds, &fds, sizeof(fd_set));

        nfds = select(FD_SETSIZE, &rfds, NULL, NULL, &timeout);
        if (nfds < 0)
            break;

        if (nfds == 0)
            continue;

        if (FD_ISSET(test_out[PIPE_IN], &rfds)) {
            memset(buffer, 0, sizeof(buffer));
            if (read(test_out[PIPE_IN], buffer, sizeof(buffer)) > 0)
                win_printf(TEST_OUT, buffer);
            else {
                FD_CLR(test_out[PIPE_IN], &fds);
                total--;
            }
        }

        if (FD_ISSET(test_log[PIPE_IN], &rfds)) {
            memset(buffer, 0, sizeof(buffer));
            if (read(test_log[PIPE_IN], buffer, sizeof(buffer)) > 0)
                win_printf(TEST_LOG, buffer);
            else {
                FD_CLR(test_log[PIPE_IN], &fds);
                total--;
            }
        }

        if (FD_ISSET(robot_log[PIPE_IN], &rfds)) {
            memset(buffer, 0, sizeof(buffer));
            if (read(robot_log[PIPE_IN], buffer, sizeof(buffer)) > 0)
                win_printf(ROBOT_LOG, buffer);
            else {
                FD_CLR(robot_log[PIPE_IN], &fds);
                total--;
            }
        }
    }

    win_printf(TEST_OUT, "\n\nPress q to quit...");
    while (getchar() != 'q');

    close_pipes();
    cleanup_screen();
    close_log_files();

    return 0;
}
