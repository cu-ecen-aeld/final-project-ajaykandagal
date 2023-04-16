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
 *          The base of this code is from below mentioned git repo, which has 
 *          the implementation of game logic in 70 lines of code and the logic 
 *          is written for vertical orientation and allows to play game using 
 *          'a' and 'q' for player and 'left' and 'right' arrow for player 2.
 * 
 * @change  Apr 12th 2023, Ajay Kandagal, ajka9053@colorado.edu
 * 
 *          Reformatted the code to make code more modular. Working on modifying 
 *          the code so that game can be played in horizontal orientation which 
 *          makes the game more playable.
 * 
 * @change  Apr 16th 2023, Ajay Kandagal, ajka9053@colorado.edu
 * 
 *          Multiplayer functionality added. Integrated API calls to libtcpipc
 *          and linked with the library. Two players can connect over network
 *          and play with each other. Both players will use <- and -> for
 *          controls.
*******************************************************************************/
#include <ncurses.h>
#include <unistd.h>

#define IS_SERVER       0

#include "tcpipc.h"

#define PAD_WIDTH       5
#define PAD_WIDTH_HALF  2
#define ENABLE_DEBUG    0

/** Typedefs **/
typedef enum {
  MOV_NONE = 0,
  MOV_LEFT = 1,
  MOV_RIGHT = 2
} mov_dir_t;

typedef struct
{
  short int x, y;
  bool movhor, movver;
} ball_obj_t;

typedef struct
{
  short int x, y;
  bool lost;
  short int wins;
} pad_obj_t;

/** Function Prototypes **/
void pong_new_game();
void pong_new_round();
void pong_ball_pos_update();
void pong_pad_mov(pad_obj_t *pad, mov_dir_t dir);
void pong_update_scrn();

/** Global Variables **/
int win_width = 0;
int win_height = 0;

int origin_x = 0;
int origin_y = 0;

ball_obj_t ball_obj;
pad_obj_t p1_pad, p2_pad;

struct msg_packet_t msg_packet;

int main()
{
  int cont = 0;
  bool end = false;

  initscr();
  start_color();
  init_pair(1, COLOR_BLUE, COLOR_BLACK);
  keypad(stdscr, true);
  noecho();
  curs_set(0);
  getmaxyx(stdscr, win_height, win_width);
  pong_new_game();

  #if IS_SERVER
  tcpipc_init(TCP_ROLE_SERVER, 9000);
  origin_x = 0;
  origin_y = 0;
  #else
  tcpipc_init(TCP_ROLE_CLIENT, 9000);
  origin_x = win_width;
  origin_y = win_height;
  #endif

  for (nodelay(stdscr, 1); !end; usleep(12000))
  {
    if (!tcpipc_recv(&msg_packet)) {
      switch(msg_packet.msg_id)
      {
        case MSG_ID_PAD_POS:
          p2_pad.x = win_width - msg_packet.msg_data[0];
        break;
        case MSG_ID_BALL_POS:
            ball_obj.x = win_width - msg_packet.msg_data[0];
            ball_obj.y = win_height - msg_packet.msg_data[1];
            ball_obj.movhor = !(bool)msg_packet.msg_data[2];
            ball_obj.movver = !(bool)msg_packet.msg_data[4];
        break;
        default:
        break;
      }
      free(msg_packet.msg_data);
    }

    if (++cont % 16 == 0)
    {
      // ball update
      #if IS_SERVER
      pong_ball_pos_update();
      #endif
    }
    switch (getch())
    {
    case KEY_RIGHT:
      pong_pad_mov(&p1_pad, MOV_RIGHT);
      break;
    case KEY_LEFT:
      pong_pad_mov(&p1_pad, MOV_LEFT);
      break;
    case 'p':
      getchar();
      break;
    case 0x1B:
      endwin();
      end = true;
      break;
    }
    pong_update_scrn();
  }

  tcpipc_close();
  return 0;
}

void pong_new_game()
{
  ball_obj.x = win_width / 2;
  ball_obj.y = win_height / 2;
  ball_obj.movhor = false;
  ball_obj.movhor = true;

  p1_pad.x = win_width / 2;
  p1_pad.y = win_height - 1;
  p1_pad.lost = 0;
  p1_pad.wins = 0;

  p2_pad.x = win_width / 2;
  p2_pad.y = 1;
  p2_pad.lost = 0;
  p2_pad.wins = 0;
}

void pong_new_round()
{
//   p1_pad.x = win_width / 2;
//   p1_pad.y = win_height - 1;

//   p2_pad.x = win_width / 2;
//   p2_pad.y = 1;

#if IS_SERVER
  ball_obj.x = win_width / 2;
  ball_obj.y = win_height / 2;
  msg_packet.msg_id = MSG_ID_BALL_POS;
  msg_packet.msg_len = 4;
  msg_packet.msg_data = (uint8_t*) malloc(sizeof(msg_packet.msg_len));
  msg_packet.msg_data[0] = ball_obj.x;
  msg_packet.msg_data[1] = ball_obj.y;
  msg_packet.msg_data[2] = (uint8_t)ball_obj.movhor;
  msg_packet.msg_data[4] = (uint8_t)ball_obj.movver;

  tcpipc_send(&msg_packet);
  free(msg_packet.msg_data);
#endif
}

void pong_pad_mov(pad_obj_t *pad, mov_dir_t dir)
{
  if ((pad->x > PAD_WIDTH_HALF) && (dir == MOV_LEFT))
    pad->x--;
  if ((pad->x < (win_width - PAD_WIDTH_HALF - 1)) && (dir == MOV_RIGHT))
    pad->x++;

  msg_packet.msg_id = MSG_ID_PAD_POS;
  msg_packet.msg_len = 2;
  msg_packet.msg_data = (uint8_t*) malloc(sizeof(msg_packet.msg_len));
  msg_packet.msg_data[0] = p1_pad.x;
  msg_packet.msg_data[1] = p1_pad.y;

  tcpipc_send(&msg_packet);
  free(msg_packet.msg_data);
}

void pong_ball_pos_update()
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
    else if (ball_obj.x != p2_pad.x) {
      p1_pad.wins++;
      pong_new_round();
    }
  }
  else if (ball_obj.y >= win_height - 2)
  {
    ball_obj.movver = false;

    if (ball_obj.x == (p1_pad.x - PAD_WIDTH_HALF) || ball_obj.x == (p1_pad.x - PAD_WIDTH_HALF + 1))
      ball_obj.movhor = false;
    else if (ball_obj.x == (p1_pad.x + PAD_WIDTH_HALF) || ball_obj.x == (p1_pad.x + PAD_WIDTH_HALF - 1))
      ball_obj.movhor = true;
    else if (ball_obj.x != p1_pad.x) {
      p2_pad.wins++;
      pong_new_round();
    }
  }
  
  ball_obj.x = ball_obj.movhor ? ball_obj.x + 1 : ball_obj.x - 1;
  ball_obj.y = ball_obj.movver ? ball_obj.y + 1 : ball_obj.y - 1;

#if IS_SERVER
  msg_packet.msg_id = MSG_ID_BALL_POS;
  msg_packet.msg_len = 4;
  msg_packet.msg_data = (uint8_t*) malloc(sizeof(msg_packet.msg_len));
  msg_packet.msg_data[0] = ball_obj.x;
  msg_packet.msg_data[1] = ball_obj.y;
  msg_packet.msg_data[2] = (uint8_t)ball_obj.movhor;
  msg_packet.msg_data[4] = (uint8_t)ball_obj.movver;

  tcpipc_send(&msg_packet);
  free(msg_packet.msg_data);
#endif
}


void pong_update_scrn()
{
    erase();

    mvprintw((win_height / 2), (win_width / 2) - 2, "%i | %i", p1_pad.wins, p2_pad.wins);

    // mvvline(0, win_width / 2, ACS_VLINE, win_height);
    attron(COLOR_PAIR(1));

    mvprintw(ball_obj.y, ball_obj.x, "o");

    for (int i = -PAD_WIDTH_HALF; i <= PAD_WIDTH_HALF; i++)
    {
      mvprintw(p1_pad.y, p1_pad.x + i, "=");
      mvprintw(p2_pad.y, p2_pad.x + i, "=");
    }

#if ENABLE_DEBUG
    mvprintw(0, 0, "%d,%d", ball_obj.x, ball_obj.y);
    mvprintw(1, 0, "%d,%d", p1_pad.x, p1_pad.y);
    mvprintw(2, 0, "%d,%d", p2_pad.x, p2_pad.y);
#endif
    attroff(COLOR_PAIR(1));
}