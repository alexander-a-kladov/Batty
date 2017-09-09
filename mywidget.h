#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QImage>

typedef struct {
    int x,y;
    int prz;
    int type;
    int w,h;
} TBrick;

typedef struct {
    int x1,y1;
    int x2,y2;
} TShort;

enum ESIDES {
    NOP=0,
    TOP=1,
    RIGHT=2,
    BOTTOM=3,
    LEFT=4
};

enum EPRIZES {
    NEWBALL,
    FIREBALL,
    ICEBALL,
    LARGER,
    SMALLER,
    FASTER,
    SLOWER,
    CATCHER,
    GUNNER
};

enum EMOVE {
    ENOP,
    ELEFT,
    ERIGHT
};

typedef struct {
    int bx,by;
    int vx,vy;
    int type;
    unsigned char stick;
} TBall;

typedef struct {
    int type;
    int x,y;
} TPrize;

typedef struct {
    int x,y;
} TBullet;

#define MAX_BRICKS 180
#define MAX_BALLS 3
#define MAX_PRIZES 10
#define MAX_BULLETS 10

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    MyWidget(QWidget *parent=0, Qt::WindowFlags f=0);
protected:
    QTimer *tmr,*kbd_tmr;
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void draw_prizes(QPainter &p);
    void draw_balls(QPainter &p);
    void draw_batty(QPainter &p);
    void draw_bullets(QPainter &p);
    void draw_bricks(QPainter &p);
    void move_prizes();
    void move_balls();
    void move_batty();
    void load_level(int num);
    void load_tiles();
    void release_balls();
    void move_bullets();
    void fill_bricks();
    void new_game(int level);
    void new_ball(int type);
    void new_prize(int x, int y);
    void eat_prize(int i);
    void delete_brick(int i);
    int inside_brick(int x, int y);
    void minus_prize(int i);
    void minus_ball(int index);
    void minus_bullet(int i);
    void new_bullet();
    void check_bullets_hits();
    int bricks_intersect(int index, TShort s);
    int short_intersect(TShort s1, TShort s2);
    int x,nx;
    int w,bw;
    TBall balls[MAX_BALLS];
    int bn;
    int prizes_n;
    int bullets_n;
    int timer_v;
    bool catcher, gunner;
    TPrize prizes[MAX_PRIZES];
    TBullet bullets[MAX_BULLETS];
    TBrick bricks[MAX_BRICKS];
    int level;
    int bricks_num;
    int move_cmd;
    QImage tiles;
    QImage balls_img;
    QImage batty_img;
    QImage bricks_img;
    int k;
protected slots:
    void recalc();
    void kbd_calc();

};
