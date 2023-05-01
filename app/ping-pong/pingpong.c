/*******************************************************************************
 * @file    pingpong.c
 * @brief   Mutliplayer Ping-pong game which allows two players to connect and
 *          play using Raspberry Pi.
 *
 * @details
 *
 * @cite    vicente.bolea@gmail.com
 *          https://github.com/vicentebolea/Pong-curses
 *
 *          The game logic (ball movement) of this code is taken from above git
 *          repo, which has the implementation of game logic in 70 lines of code
 *          and the logic is written for vertical orientation and allows to play
 *          game using 'a' and 'q' for player 1 and 'left' and 'right' arrow for
 *          player 2.
 *
 * @change  Apr 12th 2023, Ajay Kandagal, ajka9053@colorado.edu
 *
 *          Reformatted the code to make modular. Modified the code to play game
 *          in horizontal orientation which makes the game more playable.
 *
 * @change  Apr 16th 2023, Ajay Kandagal, ajka9053@colorado.edu
 *
 *          Multiplayer functionality added. Integrated API calls to libtcpipc
 *          and linked with the library. Two players can connect over network
 *          and play with each other. Players will use <- and -> for controls.
 *
 * @change  16th 2023, Ajay Kandagal, ajka9053@colorado.edu
 *
 *          If two players launch the game at different window resolutions, then
 *          the lowest resolution is chosen for play. This resolves the
 *          opponensts' pad offset issue.
 *******************************************************************************/
#include <ncurses.h>
#include <unistd.h>

#include "tcpipc.h"
#include "joystick.h"

#define PINGPONG_EN_LOGS 0
#define PINGPONG_EN_JOYSTICK 0

#if PINGPONG_EN_JOYSTICK
#define PINGPONG_BALL_SPEED   6
#else
#define PINGPONG_BALL_SPEED   12
#endif

#define PINGPONG_REFRESH_DELAY 8000

#define PAD_WIDTH 5
#define PAD_WIDTH_HALF 2

#define MY_PADDLE_COLOR 1
#define OPP_PADDLE_COLOR  2
#define BALL_COLOR 3
#define SCORE_COLOR 4
#define MAIN_WINDOW_COLOR 5

/** Typedefs **/
typedef enum
{
  MOV_NONE = 0,
  MOV_LEFT = 1,
  MOV_RIGHT = 2
} mov_dir_t;

struct window_info_t
{
  int width;
  int height;
};

struct ball_obj_t
{
  short int x, y;
  bool movhor;
  bool movver;
};

struct pad_obj_t
{
  short int x, y;
  uint8_t wins;
};

/** Function Prototypes **/
void pingpong_init();
void pingpong_close();
void pingpong_new_game();
void pingpong_new_round();
void pingpong_ball_mov();
void pingpong_read_keypad();
void pingpong_pad_mov(struct pad_obj_t *pad, mov_dir_t dir);
void pingpong_update_scrn();

int pingpong_send_msg(enum msg_id_e msg_id);
int pingpong_recv_msg();

/** Global Variables **/
bool is_server = true;
int win_width = 0;
int win_height = 0;
bool end = false;

struct ball_obj_t ball_obj;
struct pad_obj_t p1_pad, p2_pad;
WINDOW *main_window;

struct window_info_t term_win_info;
struct window_info_t opp_term_win_info;

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int main(int argc, char **argv)
{
  int cont = 0;

  if (argc == 2)
  {
    if (!strcmp(argv[1], "0"))
      is_server = true;
    else if (!strcmp(argv[1], "1"))
      is_server = false;
    else
      exit(EXIT_FAILURE);
  }
  else
  {
    exit(EXIT_FAILURE);
  }

  pingpong_init();

  for (nodelay(stdscr, 1); !end; usleep(PINGPONG_REFRESH_DELAY))
  {
    while (pingpong_recv_msg() > 0)
      ;

    if (++cont % PINGPONG_BALL_SPEED == 0)
    {
      pingpong_ball_mov();
    }

    pingpong_read_keypad();
    pingpong_update_scrn();
  }

  pingpong_close();
  return 0;
}

void pingpong_init()
{
#if PINGPONG_EN_JOYSTICK
  joystick_init();
#endif

  // Initialize window
  main_window = initscr();

  if (main_window == NULL)
  {
    perror("Could not initialize screen");
    exit(EXIT_FAILURE);
  }

  // Check if terminal supports colors
  if (start_color() == ERR || !has_colors() || !can_change_color())
  {
    delwin(main_window);
    endwin();
    refresh();
    perror("Could not use colors");
  }

  // Get terminal width and height
  getmaxyx(stdscr, term_win_info.height, term_win_info.width);

  // Set color pair for terminal
  init_pair(MAIN_WINDOW_COLOR, COLOR_WHITE, COLOR_BLACK);
  wbkgd(main_window, COLOR_PAIR(MAIN_WINDOW_COLOR));

  keypad(stdscr, true);
  noecho();
  curs_set(0);

  if (is_server)
  {
    tcpipc_init(TCP_ROLE_SERVER, "", 9000);

    // set color pair for player paddle
    init_pair(MY_PADDLE_COLOR, COLOR_CYAN, COLOR_BLACK);

    // set color pair for opponent paddle
    init_pair(OPP_PADDLE_COLOR, COLOR_YELLOW, COLOR_BLACK);
  }
  else
  {
    tcpipc_init(TCP_ROLE_CLIENT, "10.0.0.242", 9000);

    // set color pair for player paddle
    init_pair(MY_PADDLE_COLOR, COLOR_YELLOW, COLOR_BLACK);

    // set color pair for opponent paddle
    init_pair(OPP_PADDLE_COLOR, COLOR_CYAN, COLOR_BLACK);
  }

  /* set color pair for ball */
  init_pair(BALL_COLOR, COLOR_RED, COLOR_BLACK);

  /* set color pair for title */
  init_pair(SCORE_COLOR, COLOR_GREEN, COLOR_BLACK);

  pingpong_send_msg(MSG_ID_WIN_SIZE);

  // Get opponents window size
  while (pingpong_recv_msg() != MSG_ID_WIN_SIZE);

  // Set game window size to minimum specs
  if (term_win_info.width <= opp_term_win_info.width)
    win_width = term_win_info.width;
  else
    win_width = opp_term_win_info.width;

  if (term_win_info.height <= opp_term_win_info.height)
    win_height = term_win_info.height;
  else
    win_height = opp_term_win_info.height;

  pingpong_new_game();
}

void pingpong_close()
{
#if PINGPONG_EN_JOYSTICK
  joystick_close();
#endif
  tcpipc_close();
  delwin(main_window);
  endwin();
  refresh();
}

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void pingpong_new_game()
{
  ball_obj.x = win_width / 2;
  ball_obj.y = win_height / 2;
  ball_obj.movhor = false;
  ball_obj.movhor = true;

  p1_pad.x = win_width / 2;
  p1_pad.y = win_height - 1;
  p1_pad.wins = 0;

  p2_pad.x = win_width / 2;
  p2_pad.y = 1;
  p2_pad.wins = 0;

  pingpong_new_round();
}

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void pingpong_new_round()
{
  ball_obj.x = win_width / 2;
  ball_obj.y = win_height / 2;

  if (is_server)
  {
    pingpong_send_msg(MSG_ID_BALL_POS);
    usleep(1000);
    pingpong_send_msg(MSG_ID_GAME_STATUS);
    usleep(1000);
    pingpong_send_msg(MSG_ID_SYNC);
  }
  else
  {
    while (pingpong_recv_msg() != MSG_ID_SYNC);
  }
}

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void pingpong_ball_mov()
{
  if ((ball_obj.x == win_width - 1) || (ball_obj.x == 1))
    ball_obj.movhor = !ball_obj.movhor;

  if (ball_obj.y <= 2)
  {
    ball_obj.movver = true;

    if (ball_obj.x == (p2_pad.x - PAD_WIDTH_HALF) || ball_obj.x == (p2_pad.x - PAD_WIDTH_HALF + 1))
      ball_obj.movhor = false;
    else if (ball_obj.x == (p2_pad.x + PAD_WIDTH_HALF) || ball_obj.x == (p2_pad.x + PAD_WIDTH_HALF - 1))
      ball_obj.movhor = true;
    else if (ball_obj.x != p2_pad.x)
    {
      p1_pad.wins++;
      pingpong_new_round();
    }
  }
  else if (ball_obj.y >= win_height - 2)
  {
    ball_obj.movver = false;

    if (ball_obj.x == (p1_pad.x - PAD_WIDTH_HALF) || ball_obj.x == (p1_pad.x - PAD_WIDTH_HALF + 1))
      ball_obj.movhor = false;
    else if (ball_obj.x == (p1_pad.x + PAD_WIDTH_HALF) || ball_obj.x == (p1_pad.x + PAD_WIDTH_HALF - 1))
      ball_obj.movhor = true;
    else if (ball_obj.x != p1_pad.x)
    {
      p2_pad.wins++;
      pingpong_new_round();
    }
  }

  ball_obj.x = ball_obj.movhor ? ball_obj.x + 1 : ball_obj.x - 1;
  ball_obj.y = ball_obj.movver ? ball_obj.y + 1 : ball_obj.y - 1;
}

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void pingpong_pad_mov(struct pad_obj_t *pad, mov_dir_t dir)
{
  if ((pad->x > PAD_WIDTH_HALF) && (dir == MOV_LEFT))
    pad->x--;
  if ((pad->x < (win_width - PAD_WIDTH_HALF - 1)) && (dir == MOV_RIGHT))
    pad->x++;

  pingpong_send_msg(MSG_ID_PAD_POS);
}

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void pingpong_update_scrn()
{
  erase();

  attron(COLOR_PAIR(SCORE_COLOR));
  mvprintw((win_height / 2), (win_width / 2) - 2, "%i | %i", p1_pad.wins, p2_pad.wins);
  attroff(COLOR_PAIR(SCORE_COLOR));

  attron(COLOR_PAIR(BALL_COLOR));
  mvprintw(ball_obj.y, ball_obj.x, "o");
  attroff(COLOR_PAIR(BALL_COLOR));

  attron(COLOR_PAIR(MY_PADDLE_COLOR));
  for (int i = -PAD_WIDTH_HALF; i <= PAD_WIDTH_HALF; i++)
    mvprintw(p1_pad.y, p1_pad.x + i, "=");
  attroff(COLOR_PAIR(MY_PADDLE_COLOR));

  attron(COLOR_PAIR(OPP_PADDLE_COLOR));
  for (int i = -PAD_WIDTH_HALF; i <= PAD_WIDTH_HALF; i++)
    mvprintw(p2_pad.y, p2_pad.x + i, "=");
  attroff(COLOR_PAIR(OPP_PADDLE_COLOR));

#if PINGPONG_EN_LOGS
  mvprintw(0, 0, "%d,%d", ball_obj.x, ball_obj.y);
  mvprintw(1, 0, "%d,%d", p1_pad.x, p1_pad.y);
  mvprintw(2, 0, "%d,%d", p2_pad.x, p2_pad.y);
#endif
}

int pingpong_send_msg(enum msg_id_e msg_id)
{
  uint8_t msg_buffer[16];
  struct msg_packet_t msg_packet;

  switch (msg_id)
  {
  case MSG_ID_WIN_SIZE:
    msg_buffer[0] = (term_win_info.width >> 0) & 0xFF;
    msg_buffer[1] = (term_win_info.width >> 8) & 0xFF;
    msg_buffer[2] = (term_win_info.height >> 0) & 0xFF;
    msg_buffer[3] = (term_win_info.height >> 8) & 0xFF;

    msg_packet.msg_len = 4;
    break;

  case MSG_ID_PAD_POS:
    msg_buffer[0] = (p1_pad.x >> 0) & 0xFF;
    msg_buffer[1] = (p1_pad.x >> 8) & 0xFF;

    msg_packet.msg_len = 2;
    break;

  case MSG_ID_BALL_POS:
    msg_buffer[0] = (ball_obj.x >> 0) & 0xFF;
    msg_buffer[1] = (ball_obj.x >> 8) & 0xFF;
    msg_buffer[2] = (ball_obj.y >> 0) & 0xFF;
    msg_buffer[3] = (ball_obj.y >> 8) & 0xFF;
    msg_buffer[4] = (ball_obj.movhor & 0x0F) |
                    ((ball_obj.movver << 4) & 0xF0);

    msg_packet.msg_len = 5;
    break;

  case MSG_ID_GAME_STATUS:
    msg_buffer[0] = p1_pad.wins;
    msg_buffer[1] = p2_pad.wins;

    msg_packet.msg_len = 2;
    break;

  case MSG_ID_SYNC:
    msg_packet.msg_len = 1;
    break;

  default:
    printf("Invalid msg id received\n");
    return -1;
  }

  msg_packet.msg_id = msg_id;
  msg_packet.msg_data = msg_buffer;
  tcpipc_send(&msg_packet);
  return 0;
}

int pingpong_recv_msg()
{
  struct msg_packet_t msg_packet;

  if (tcpipc_recv(&msg_packet))
    return -1;

  switch (msg_packet.msg_id)
  {
  case MSG_ID_WIN_SIZE:
    opp_term_win_info.width = (msg_packet.msg_data[0] << 0) |
                              (msg_packet.msg_data[1] << 8);
    opp_term_win_info.height = (msg_packet.msg_data[2] << 0) |
                               (msg_packet.msg_data[3] << 8);
    break;

  case MSG_ID_PAD_POS:
    p2_pad.x = (msg_packet.msg_data[0] | (msg_packet.msg_data[1] << 8));
    p2_pad.x = win_width - p2_pad.x;
    break;

  case MSG_ID_BALL_POS:
    ball_obj.x = (msg_packet.msg_data[0] << 0) |
                 (msg_packet.msg_data[1] << 8);
    ball_obj.x = win_width - ball_obj.x;
    ball_obj.y = (msg_packet.msg_data[2] << 0) |
                 (msg_packet.msg_data[3] << 8);
    ball_obj.y = win_height - ball_obj.y;

    ball_obj.movhor = !(bool)(msg_packet.msg_data[4] & 0x0F);
    ball_obj.movver = !(bool)(msg_packet.msg_data[4] & 0xF0);
    break;

  case MSG_ID_GAME_STATUS:
    p2_pad.wins = msg_packet.msg_data[0];
    p1_pad.wins = msg_packet.msg_data[1];
    break;

  case MSG_ID_SYNC:
    break;

  default:
    printf("Invalid msg id received\n");
    return -1;
  }

  return msg_packet.msg_id;
}

void pingpong_read_keypad()
{
#if PINGPONG_EN_JOYSTICK
  struct joystick_data_t jd;

  joystick_read(&jd);

  if (jd.x_pos > 100)
  {
    pingpong_pad_mov(&p1_pad, MOV_RIGHT);
  }
  else if (jd.x_pos < -100)
  {
    pingpong_pad_mov(&p1_pad, MOV_LEFT);
  }
  else if (jd.button)
  {
    endwin();
    end = true;
  }
  
  getch();
#else
  switch (getch())
  {
  case KEY_RIGHT:
    pingpong_pad_mov(&p1_pad, MOV_RIGHT);
    break;
  case KEY_LEFT:
    pingpong_pad_mov(&p1_pad, MOV_LEFT);
    break;
  case 'p':
    getchar();
    break;
  case 0x1B:
    endwin();
    end = true;
    break;
  }
#endif
}
