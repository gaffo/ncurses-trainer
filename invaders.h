#pragma once

#include <vector>


enum Colork
{
  COLORK_Red_Black,
  COLORK_Blue_Black,
  COLORK_White_Black,
  COLORK_Yellow_Black,
  COLORK_Red_White,
};



struct SDim
{
  SDim(int x, int y)
  : m_x(x),
    m_y(y)
  {}

  int m_x;
  int m_y;
};




struct SRect
{
  SRect(int x, int y, int w, int h)
  : m_x(x),
    m_y(y),
    m_w(w),
    m_h(h)
  {}

  int m_x, m_y, m_w, m_h;
};



enum Drawctxk
{
  DRAWCTXK_Root,
  DRAWCTXK_Board,

  DRAWCTXK_Max,
};



class IDrawCtx
{
 public:
  IDrawCtx(Drawctxk drawctxk, IDrawCtx * pDcParent);

  // Draw Methods (Move to a drawing context with one draw-er?)

  void DrawChar(const SDim & pos, char c, Colork colork);
  void FillRect(const SRect & rect, char c, Colork colork);
  void DrawString(const SDim & pos, const char * pChzStr, Colork colork);
  virtual void DebugString(const char * pChzDebug);

  // Parenting & Screen Space Methods 
 
  IDrawCtx * PdcParent() const { return m_pDcParent; }
  
  virtual SDim Dim() const = 0;
  virtual SDim PosParent() const = 0;
  virtual void Recalculate() = 0;

 private:

  SRect RectScreen(const SRect & rect) const;
  SDim PosScreen(const SDim & pos) const;

  IDrawCtx * m_pDcParent;
};



class CDrawCtxProvider
{
 public:
  CDrawCtxProvider() {}
  
  void SetProvider(Drawctxk drawctxk, IDrawCtx * pDc) { m_apDc[drawctxk] = pDc; }
  void RemoveProvider(Drawctxk drawctxk) { m_apDc[drawctxk] = NULL; }
  IDrawCtx * Pctx(Drawctxk drawctxk) { return m_apDc[drawctxk]; }
  void Recalculate();

  private:
  IDrawCtx * m_apDc[DRAWCTXK_Max];
};

extern CDrawCtxProvider s_dcp;


class CCursesDrawContext : public IDrawCtx
{
  typedef IDrawCtx super;
 public:
  CCursesDrawContext();

  // IDrawCtx

  void Flip();
  void Clear();
  virtual void Recalculate();
  virtual SDim PosParent() const;
  virtual SDim Dim() const { return m_dim; }

#ifdef DEBUG
  virtual void DebugString(const char * pChzDebug);
#endif // DEBUG

 private:
  SDim m_dim;

#ifdef DEBUG
  int m_cDebug;
#endif // DEBUG
};



class IStep
{
 public:
  virtual void Step() = 0;
};



class CRenderable
{
 public:
  CRenderable(const SDim &);
  
  virtual void Draw(CDrawCtxProvider * pDcp) = 0;

  void SetPos(SDim pos) { m_pos = pos; }
  SDim Pos() const { return m_pos; }

 protected:
  SDim m_pos;

};



class CShip : public CRenderable
{
  typedef CRenderable super;
 public:
  CShip(int x, int y);

  virtual void Draw(CDrawCtxProvider * pDcp);

};



class CBullet : public CRenderable, public IStep
{
  typedef CRenderable super;

 public: 

  CBullet(SDim pos, SDim vec);

  virtual void Draw(CDrawCtxProvider * pDcp);

  virtual void Step();

 private:
  SDim m_vec;
};


class CBoard;

class CPlayer : public CRenderable, public IStep
{
  typedef CRenderable super;
 public:
  CPlayer(const SDim &, CBoard * pBoard);

  virtual void Draw(CDrawCtxProvider * pDcp);

  void Move(int dir);
  void Fire();
  virtual void Step();

 private:
  CBoard * m_pBoard;
  
  std::vector<CBullet> m_vBullet;
};



class CBoard : public CRenderable, public IDrawCtx
{
  typedef CRenderable super;
 public:
  CBoard(IDrawCtx * pDcParent);

  // IRenderable Methods
  virtual void Draw(CDrawCtxProvider * pDcp);

  // Class Methods
  void SetScore(int nScore);
  int SetLives(int nLives);
  void SetStatus(const char * pChzStatus);

  virtual SDim Dim() const { return SDim(40, 40); }
  virtual SDim PosParent() const;
  virtual void Recalculate();

 private:
  int m_score;
  int m_lives;
  char m_aChzStatus[256];
};
