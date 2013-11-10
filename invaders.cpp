#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "invaders.h"



void PrintDim(const SDim & dim, char * pChzOut, int sz)
{
  memset(pChzOut, 0, sz);
  sprintf(pChzOut, "%dx%d", dim.m_x, dim.m_y);
}

void PrintRect(const SRect & rect, char * pChzOut, int sz)
{
  memset(pChzOut, 0, sz);
  sprintf(pChzOut, "[%d,%d, %d,%d]", rect.m_x, rect.m_y, rect.m_w, rect.m_h);
}



void CDrawCtxProvider::Recalculate()
{
  for (int i = 0; i < DRAWCTXK_Max; ++i)
  {
    // BB (gaffo): This really needs to be a list of child widgets
    // BB (gaffo): Relying on the enum declaring the widget parentage order

    m_apDc[i]->Recalculate();    
  }

}



IDrawCtx::IDrawCtx(Drawctxk drawctxk, IDrawCtx * pDcParent)
  : m_pDcParent(pDcParent)
{
  s_dcp.SetProvider(drawctxk, this);
}

void IDrawCtx::DrawChar(const SDim & pos, char c, Colork colork)
{
  const SDim posScreen = PosScreen(pos);
  attrset(COLOR_PAIR(colork + 1));
  mvaddch(posScreen.m_x, posScreen.m_y, c);
}

void IDrawCtx::FillRect(const SRect & rect, char c, Colork colork)
{
  SRect rectScreen = RectScreen(rect);
  attrset(COLOR_PAIR(colork + 1));

  for (int x = rectScreen.m_x; x < rectScreen.m_x + rectScreen.m_w; ++x)
  {
    for (int y = rectScreen.m_y; y < rectScreen.m_y + rectScreen.m_h; ++y)
    {
      mvaddch(y, x, c);
    }
  }
}

void IDrawCtx::DrawString(const SDim & pos, const char * pChzStr, Colork colork)
{
  SDim posScreen = PosScreen(pos);

  attrset(COLOR_PAIR(colork + 1));
  int cCh = strlen(pChzStr);
  for (int iCh = 0; iCh < cCh; ++iCh)
  {
    mvaddch(posScreen.m_y, posScreen.m_x + iCh, pChzStr[iCh]);
  }
}

void IDrawCtx::DebugString(const char * pChzDebug)
{
  s_dcp.Pctx(DRAWCTXK_Root)->DebugString(pChzDebug);
}

SRect IDrawCtx::RectScreen(const SRect & rect) const
{
  SDim pos = PosParent();
  SRect rectScreen(pos.m_x + rect.m_x, pos.m_y + rect.m_y, rect.m_w, rect.m_h);

  if (PdcParent() == NULL)
    return rectScreen;

  return PdcParent()->RectScreen(rectScreen);
}

SDim IDrawCtx::PosScreen(const SDim & pos) const
{
  SDim posParent = PosParent();
  SDim posResult(posParent.m_x + pos.m_x, posParent.m_y + pos.m_y);
  
  if (PdcParent() == NULL)
    return posResult;

  return PdcParent()->PosScreen(posResult);
}

CDrawCtxProvider s_dcp;



CCursesDrawContext::CCursesDrawContext()
: super(DRAWCTXK_Root, NULL),
    m_dim(0,0),
    m_cDebug(0)
{
  s_dcp.SetProvider(DRAWCTXK_Root, this);
  (void) initscr();
  keypad(stdscr, TRUE);
  (void) nonl();
  (void) cbreak();
  (void) noecho();
  Recalculate();

  if (has_colors())
  {
    start_color();

    int nPair = 1;
    init_pair(nPair++, COLOR_RED, COLOR_BLACK);
    init_pair(nPair++, COLOR_BLUE, COLOR_BLACK);
    init_pair(nPair++, COLOR_WHITE, COLOR_BLACK);
    init_pair(nPair++, COLOR_YELLOW, COLOR_BLACK);
    init_pair(nPair++, COLOR_RED, COLOR_WHITE);
  }
}

void CCursesDrawContext::Recalculate()
{
  getmaxyx(stdscr, m_dim.m_y, m_dim.m_x);
}

void CCursesDrawContext::Flip()
{
  m_cDebug = 0;
  erase();
}

#ifdef DEBUG
void CCursesDrawContext::DebugString(const char * pChzDebug)
{
  DrawString(SDim(0, m_cDebug++), pChzDebug, COLORK_Yellow_Black);
}
#endif // DEBUG

SDim CCursesDrawContext::PosParent() const
{
  return SDim(0, 0);
}



CRenderable::CRenderable(const SDim & pos)
  : m_pos(pos)
{
}



CBullet::CBullet(SDim pos, SDim vec)
  : super(pos),
    m_vec(vec)
{
}

void CBullet::Draw(CDrawCtxProvider * pDcp)
{
  IDrawCtx * pDc = pDcp->Pctx(DRAWCTXK_Board);

  SDim dim = pDc->Dim();
  SDim pos(m_pos.m_x, m_pos.m_y + dim.m_y);

  pDc->DrawString(pos, "^", COLORK_Yellow_Black);

  char aChz[256];
  PrintDim(pos, aChz, 256);
  pDc->DebugString(aChz);
}

void CBullet::Step() 
{
  SDim posCur = Pos();
  SDim posNew(posCur.m_x + m_vec.m_x, posCur.m_y + m_vec.m_y);
  SetPos(posNew);
}



CPlayer::CPlayer(const SDim & pos, CBoard * pBoard)
  : super(pos),
    m_pBoard(pBoard),
    m_vBullet()
{
}

void CPlayer::Draw(CDrawCtxProvider * pDcp)
{
  IDrawCtx * pDc = pDcp->Pctx(DRAWCTXK_Board);
  SDim dim = pDc->Dim();
  SDim pos(m_pos.m_x, dim.m_y - 1);
  //  pDcp->Pctx(DRAWCTXK_Board)->DrawString(pos, "/^\\", COLORK_Red_White);
  pDc->DrawString(pos, "/^\\", COLORK_Red_White);

  char aChz[256];
  sprintf(aChz, "Bullets: %d", int(m_vBullet.size()));
  pDc->DebugString(aChz);

  PrintDim(pos, aChz, 256);
  pDc->DebugString(aChz);
  PrintDim(Pos(), aChz, 256);
  pDc->DebugString(aChz);

  std::vector<CBullet>::iterator iter = m_vBullet.begin();
  while (iter != m_vBullet.end())
  {
    iter->Draw(pDcp);
    iter++;
  }
}

void CPlayer::Move(int dir)
{
  m_pos.m_x += dir;
  if (m_pos.m_x < 0)
    m_pos.m_x = 0;
  if (m_pos.m_x > m_pBoard->Dim().m_x - 3)
    m_pos.m_x = m_pBoard->Dim().m_x - 3;
}

void CPlayer::Fire()
{  
  SDim pos = Pos();
  CBullet bullet(SDim(pos.m_x + 1, pos.m_y - 1), SDim(0, -1));
  m_vBullet.push_back(bullet);
}

void CPlayer::Step()
{
  std::vector<CBullet>::iterator iter = m_vBullet.begin();
  while (iter != m_vBullet.end())
  {
    iter->Step();
    iter++;
  }
}



SDim s_dimBoard(40,40);

CBoard::CBoard(IDrawCtx * pDcParent)
  : super(SDim(0,0)),
    IDrawCtx(DRAWCTXK_Board, pDcParent),
    m_score(0),
    m_lives(0),
    m_aChzStatus()
{
  sprintf(m_aChzStatus, "STATUS LINE======");
}

void CBoard::Draw(CDrawCtxProvider * pDcp)
{
  IDrawCtx * pDcRoot = pDcp->Pctx(DRAWCTXK_Root);

  // Draw in the root context borders around the actual game board

  SDim dimScreen = pDcRoot->Dim();
  SDim posBoard = PosParent();
  SDim dimBoard = Dim();
    
  SRect rectLeft(0, 0, posBoard.m_x, dimScreen.m_y);
  SRect rectRight(posBoard.m_x + dimBoard.m_x, 0, posBoard.m_x + dimBoard.m_x, dimScreen.m_y);
  SRect rectTop(posBoard.m_x, 0, posBoard.m_x + dimBoard.m_x, posBoard.m_y);
  SRect rectBottom(posBoard.m_x, posBoard.m_y + dimBoard.m_y, posBoard.m_x + dimBoard.m_x, dimScreen.m_y);

  pDcRoot->FillRect(rectLeft, '%', COLORK_Red_Black);
  pDcRoot->FillRect(rectRight, '%', COLORK_Blue_Black);
  pDcRoot->FillRect(rectTop, '%', COLORK_White_Black);
  pDcRoot->FillRect(rectBottom, '%', COLORK_Yellow_Black);

  char aChz[256];

  SDim dim = Dim();
  sprintf(aChz, "Lives: %d", m_lives);
  DrawString(SDim(1, 0), aChz, COLORK_White_Black);
  sprintf(aChz, "Score: %d", m_score);
  DrawString(SDim(dim.m_x / 2, 0), aChz, COLORK_White_Black);
  
  DrawString(SDim(1, dim.m_y), m_aChzStatus, COLORK_White_Black);

  PrintDim(dimScreen, aChz, 256);
  DebugString(aChz);
  
  PrintDim(posBoard, aChz, 256);
  DebugString(aChz);

  PrintDim(dimBoard, aChz, 256);
  DebugString(aChz);

  PrintRect(rectLeft, aChz, 256);
  DebugString(aChz);

  PrintRect(rectRight, aChz, 256);
  DebugString(aChz);

  PrintRect(rectTop, aChz, 256);
  DebugString(aChz);

  PrintRect(rectBottom, aChz, 256);
  DebugString(aChz);
}

void CBoard::Recalculate()
{
  SDim dimParent = PdcParent()->Dim();
  SDim pos((dimParent.m_x - Dim().m_x) / 2, (dimParent.m_y - Dim().m_y) / 2);
  SetPos(pos);
}

SDim CBoard::PosParent() const
{
  return Pos();
}



CCursesDrawContext s_dc;

static void finish(int sig)
{
  endwin();
  exit(0);
}

static void handle_winch(int sig)
{
  s_dcp.Recalculate();
}

enum KEY
{
  KEY_Space = 32,
  KEY_Left = 260,
  KEY_Right = 261,
};

int main(int argc, char *argv[])
{
  int num = 0;

  (void) signal(SIGINT, finish);
  (void) signal(SIGWINCH, handle_winch);

  CBoard board(&s_dc);
  CPlayer player(SDim(0, 0), &board);

  s_dcp.Recalculate();
  board.Draw(&s_dcp);
  player.Draw(&s_dcp);

  for (;;)
  {
    
    int key = getch();
    switch (key)
    {
    case KEY_Space: // Fire
      player.Fire();
      break;

    case KEY_Left: // Left
      player.Move(-1);
      break;

    case KEY_Right:
      player.Move(1);
      break;

    default:
      break;
    }

    player.Step();

    s_dc.Flip();
    board.Draw(&s_dcp);

    char aCh[256];
    memset(aCh, 0, 256);
    sprintf(aCh, "Char: %d", key);

    player.Draw(&s_dcp);

    s_dcp.Pctx(DRAWCTXK_Root)->DebugString(aCh);
  }

  finish(0);
}
