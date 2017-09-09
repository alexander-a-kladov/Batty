#include "mywidget.h"
#include <iostream>
#include <math.h>
#include <QCursor>
#include <QImage>
#include <stdlib.h>
#include <unistd.h>

#include "abstractaudiooutput.h"
#include "abstractmediastream.h"
#include "abstractvideooutput.h"
#include "addoninterface.h"
#include "audiooutput.h"
#include "audiooutputinterface.h"
#include "backendcapabilities.h"
#include "backendinterface.h"
#include "effect.h"
#include "effectinterface.h"
#include "effectparameter.h"
#include "effectwidget.h"
#include "mediacontroller.h"
#include "medianode.h"
#include "mediaobject.h"
#include "mediaobjectinterface.h"
#include "mediasource.h"
#include "objectdescription.h"
#include "objectdescriptionmodel.h"
#include "path.h"
#include "phonondefs.h"
#include "phononnamespace.h"
#include "platformplugin.h"
#include "seekslider.h"
#include "streaminterface.h"
#include "videoplayer.h"
#include "videowidget.h"
#include "videowidgetinterface.h"
#include "volumefadereffect.h"
#include "volumefaderinterface.h"
#include "volumeslider.h"


using namespace std;


Phonon::MediaObject *music;
Phonon::MediaObject *sound_trash;
Phonon::MediaObject *sound_chimes;
Phonon::MediaObject *sound_critical;
Phonon::MediaObject *sound_fire;
Phonon::MediaObject *sound_newgame;


MyWidget::MyWidget(QWidget *parent, Qt::WindowFlags f) : 
QWidget(parent, f)
{
    k = 2;
    setFixedSize(380*k,400*k);
    bw = 10*k;
    tmr = new QTimer();
    kbd_tmr = new QTimer();
    kbd_tmr->start(10);
    load_tiles();
    connect(kbd_tmr, SIGNAL(timeout()), this, SLOT(kbd_calc()));
    level = 1;
    setCursor(QCursor(Qt::BlankCursor));
    music = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource("sounds/clicked.wav"));
    sound_trash = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource("sounds/tooltip.wav"));
    sound_chimes = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource("sounds/chimes.wav"));
    sound_critical = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource("sounds/critical.wav"));
    sound_fire = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource("sounds/notify.wav"));
    sound_newgame = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource("sounds/newgame.wav"));
    new_game(level);

    //setMouseTracking(true);
    connect(tmr, SIGNAL(timeout()), this, SLOT(recalc()));
}

void MyWidget::kbd_calc()
{
    switch (move_cmd) {
	case ELEFT:
	    nx-=8*k;
	    break;
	case ERIGHT:
	    nx+=8*k;
	    break;
    }
    return;
}

void MyWidget::new_game(int level)
{
    tmr->stop();
    load_level(level);
    move_cmd = ENOP;
    w = 80*k;
    nx = 0;
    x = width()/2-w;
    bn = 0;
    timer_v = 50;
    catcher = false;
    gunner = false;
    new_ball(NEWBALL);
    //fill_bricks();
    prizes_n = 0;
    bullets_n = 0;
    sound_newgame->seek(0);
    sound_newgame->play();
    tmr->start(timer_v);
    return;
}

void MyWidget::load_level(int num)
{
    QImage img;
    QString s;
    s.sprintf("levels/%02d.xpm", num);
    img.load(s);
    int index, type;
    bricks_num = 0;
    if (!img.isNull())
    {
	if ((img.width()!=18)||(img.height()!=10)) {
	    cout << "Wrong size of the level must be 18x10" << endl;
	    exit(1);
	}
	cout << img.width() << " " << img.height() << endl;
	for (int i=0;i<img.width();i++)
	    for (int j=0;j<img.height();j++) {
		index = i+j*18;
		type = img.pixelIndex(i,j);
		bricks[index].type = type%5;
		bricks[index].x = 10*k+i*20*k;
		bricks[index].y = 50*k+j*10*k;
		bricks[index].prz = (type>0)?1:0;
		bricks[index].w = 20*k;
		bricks[index].h = 10*k;
		if (type>0) bricks_num++;
	    }
    } else if (level>1) {cout << "Congratulation! You complete your game :)" << endl;exit(0);}
    return;
}

void MyWidget::load_tiles()
{
    tiles.load("tiles.xpm");
    if (tiles.isNull()) {
	cout << "tiles.xpm not found" << endl;
	exit(1);
    }
    balls_img.load("balls.xpm");
    if (balls_img.isNull()) {
	cout << "balls.xpm not found" << endl;
	exit(1);
    }
    batty_img.load("batty.xpm");
    if (batty_img.isNull()) {
	cout << "batty.xpm not found" << endl;
	exit(1);
    }
    bricks_img.load("bricks.xpm");
    if (bricks_img.isNull()) {
	cout << "bricks.xpm not found" << endl;
	exit(1);
    }
    return;
}

void MyWidget::new_ball(int type)
{
    int angle;
    if (bn<MAX_BALLS) {
	balls[bn].bx = x+w/2;
	balls[bn].by = height()-k*10-bw/2;
    angle = rand()%45;
	balls[bn].vx = 10*k*sin(angle*(M_PI/180.));
	balls[bn].vy = -10*k*cos(angle*(M_PI/180.));
	balls[bn].type = type;
	if (catcher) balls[bn].stick = 1;
	else balls[bn].stick = 0;
	bn++;
    }
    //printf("bn = %d\n", bn);
    return;
}

void MyWidget::minus_ball(int i)
{
    if (bn>1) {
	balls[i] = balls[bn-1];
    }
    if (bn) bn--;
    return;
}

void MyWidget::new_bullet()
{
    if (bullets_n<10) {
	bullets[bullets_n].x = x+w/2;
	bullets[bullets_n].y = height()-10;
	sound_fire->seek(0);
	sound_fire->play();
	bullets_n++;
    }
    return;
}

int MyWidget::inside_brick(int x, int y)
{
    int i;
    for (i=0;i<MAX_BRICKS;i++)
    {
	if ((bricks[i].prz)&&(x>=bricks[i].x)&&(x<=(bricks[i].x+bricks[i].w))&&(y>=bricks[i].y)&&(y<=(bricks[i].y+bricks[i].h)))
	    break;
    }
    if (i!=MAX_BRICKS) return i;
    return -1;
}

void MyWidget::check_bullets_hits()
{
    int bi;
    for (int i=0;i<bullets_n;i++) {
	bi = inside_brick(bullets[i].x, bullets[i].y);
	
	if (bi>=0) {
	    ///cout << "bi = " << bi << endl;
	    delete_brick(bi);
	    new_prize(bricks[bi].x, bricks[bi].y);
	    minus_bullet(i);
	}
	if (bullets[i].y<=0) 
	    minus_bullet(i);
    }
    return;
}

void MyWidget::minus_bullet(int i)
{
    bullets[i]=bullets[bullets_n-1];
    if (bullets_n) bullets_n--;
    return;
}

void MyWidget::new_prize(int x, int y)
{
    if (rand()%10>8)
    if (prizes_n<MAX_PRIZES)
    {
	int i=prizes_n;
	prizes[i].x = x;
	prizes[i].y = y;
    prizes[i].type = rand()%9;
	prizes_n++;
    }
    return;
}


void MyWidget::eat_prize(int i)
{
    //cout << "eat prize " << i << endl;
    switch(prizes[i].type) 
    {
	case NEWBALL:
	case FIREBALL:
	case ICEBALL:
	    new_ball(prizes[i].type);
	    break;
	case LARGER:
	    if (w<=80*k) w+=40*k;
	    break;
	case SMALLER:
	    if (w>40*k) w-=40*k;
	    break;
	case FASTER:
	    if (timer_v>=50) {
	    tmr->stop();
	    timer_v /= 2;
	    tmr->start(timer_v);
	    }
	    break;
	case SLOWER:
	    if (timer_v<=100) {
	    tmr->stop();
	    timer_v *= 2;
	    tmr->start(timer_v);
	    }
	    break;
	case CATCHER:
	    catcher = true;
	    break;
	case GUNNER:
	    gunner = true;
	    break;
    }
    minus_prize(i);
    return;
}

void MyWidget::minus_prize(int i)
{
    //cout << "minus prize " << i << endl;
    if (prizes_n>0) {
	prizes_n--;
	prizes[i]=prizes[prizes_n];
    }
    return;
}

void MyWidget::mouseMoveEvent(QMouseEvent *ev)
{
    //cout << "mouseMove" << endl;
//    if ((ev->x()>=10)&&(ev->x()+w<width())) x=ev->x();
    return;
}

void MyWidget::mousePressEvent(QMouseEvent *ev)
{

}

void MyWidget::fill_bricks()
{
    for (int i=0;i<MAX_BRICKS;i++) {
	bricks[i].x = 10+(i%18)*20;
	bricks[i].y = 30+(i/18)*10;
	bricks[i].prz = 1;
    bricks[i].type = rand()%5+1;
	bricks[i].w = 20;
	bricks[i].h = 10;
    }
    return;
}

int MyWidget::bricks_intersect(int index, TShort s)
{
    TShort bs;
    bs.x1 = bricks[index].x;
    bs.y1 = bricks[index].y;
    bs.x2 = bs.x1 + bricks[index].w;
    bs.y2 = bs.y1;
    if (short_intersect(s, bs)>0) return TOP;
    bs.x1 = bs.x2;
    bs.y1 = bs.y2;
    bs.x2 = bs.x1;
    bs.y2 = bs.y1 + bricks[index].h;
    if (short_intersect(s, bs)>0) return RIGHT;
    bs.x1 = bs.x2;
    bs.y1 = bs.y2;
    bs.x2 = bs.x1 - bricks[index].w;
    bs.y2 = bs.y1;
    if (short_intersect(s, bs)>0) return BOTTOM;
    bs.x1 = bs.x2;
    bs.y1 = bs.y2;
    bs.x2 = bs.x1;
    bs.y2 = bs.y1 - bricks[index].h;
    if (short_intersect(s, bs)>0) return LEFT;
    return NOP;
}

int MyWidget::short_intersect(TShort s1, TShort s2)
{
    float x,y,x1,y1;
    int sign1,sign2,sign3,sign4;
    x = s1.x2 - s1.x1;
    y = s1.y2 - s1.y1;
    x1 = s2.x1 - s1.x1;
    y1 = s2.y1 - s1.y1;
    sign1 = y1*x-y*x1;
    x1 = s2.x2 - s1.x1;
    y1 = s2.y2 - s1.y1;
    sign2 = y1*x-y*x1;
    
    if (((sign1>=0)&&(sign2>=0))||((sign1<0)&&(sign2<0)))
	return 0;
    
    x = s2.x2 - s2.x1;
    y = s2.y2 - s2.y1;
    x1 = s1.x1 - s2.x1;
    y1 = s1.y1 - s2.y1;
    sign3 = y1*x-y*x1;
    x1 = s1.x2 - s2.x1;
    y1 = s1.y2 - s2.y1;
    sign4 = y1*x-y*x1;
    
    if (((sign3>=0)&&(sign4>=0))||((sign3<0)&&(sign4<0)))
	return 0;
    
    if ((sign1>=0)&&(sign2<=0)) return 1;
    return -1;
}

void MyWidget::move_prizes()
{
    // Move prizes
    for (int j=0;j<prizes_n;j++) {
	prizes[j].y+=10*k;
	if (prizes[j].y>=height()-10*k) {
	    if ((prizes[j].x>x)&&(prizes[j].x<x+w)) {
		eat_prize(j);
	    } else minus_prize(j);
	}
    }
    return;
}

void MyWidget::delete_brick(int i)
{
    bricks[i].prz = 0;
    bricks_num--;
//    cout << bricks_num << endl;
    if (bricks_num<=0) new_game(level++);
    return;
}

void MyWidget::move_balls()
{

    return;
}

void MyWidget::move_bullets()
{
    // Move bullets
    for (int j=0;j<bullets_n;j++) {
	bullets[j].y-=10*k;
    }
    return;
}

void MyWidget::recalc()
{
    int nbx, nby;
    TShort bf;
    int side;
    
    move_batty();
    
    move_bullets();
    check_bullets_hits();
    move_prizes();

    for (int i=0;i<bn;i++) {
    // Check for ball hit bat
    if ((balls[i].vy>0)&&(balls[i].by>=height()-10*k-bw/2)) {
	if ((balls[i].bx>x)&&(balls[i].bx<x+w)) {
	    float ang=-((float)(w/2-(balls[i].bx-x)))/w;
	    balls[i].vx=10*k*sin(ang);
	    balls[i].vy=-10*k*cos(ang);
	    music->seek(0);
	    music->play();
	    if (catcher) balls[i].stick=true;
	}
    }
    
    // Move ball
    if (!balls[i].stick) {
    nbx = balls[i].bx+balls[i].vx;
    nby = balls[i].by+balls[i].vy;
    
    bf.x1 = balls[i].bx;
    bf.y1 = balls[i].by;
    bf.x2 = nbx;
    bf.y2 = nby;
    
    // Check for bricks collision
    int j;
    for (j=0;j<MAX_BRICKS;j++)
    {
	if (bricks[j].prz==0) continue;
	if ((side = bricks_intersect(j, bf))>0)
	    {
	    if (balls[i].type==FIREBALL) bricks[j].type=0;
	    else {
	    nbx = balls[i].bx;
	    nby = balls[i].by;
		switch (side)
		{
		case TOP: 
		case BOTTOM:
		//cout << "TB" << endl;
		balls[i].vy=-1*balls[i].vy;
		    break;
		case LEFT:
		case RIGHT:
		//cout << "LR" << endl;
		balls[i].vx=-1*balls[i].vx;
		    break;
		default:
		    break;
		}
	    }
	    }
	if (side>0) 
	{	
	if (balls[i].type==NEWBALL) bricks[j].type--;
	if (balls[i].type==ICEBALL) bricks[j].type/=2;
	if (bricks[j].type<=0) {sound_trash->seek(0);sound_trash->play();delete_brick(j);new_prize(nbx,nby);}
	else {music->seek(0);music->play();}
	break;
	}
	
    }
    
    //Check for failure
    if (nby>height()) {minus_ball(i);if (!bn) {sound_critical->seek(0);sound_critical->play();new_game(level);}return;}
    
    // Check for walls collision
    if ((nbx>width())||(nbx<0)) {balls[i].vx=-1*balls[i].vx;nbx=balls[i].bx;}
    if ((nby>height())||(nby<0)) {balls[i].vy=-1*balls[i].vy;nby=balls[i].by;}
    
    // Fix new position
    balls[i].bx = nbx;
    balls[i].by = nby;
    }
     else {
        balls[i].bx += nx;
        balls[i].by = height()-10*k-bw/2;
     }
    }
    nx = 0;
    repaint();
    return;
}

void MyWidget::release_balls()
{
    //cout << "rb bn = " << bn << endl;
    for (int i=0;i<bn;i++)
	balls[i].stick = 0;
    return;
}

void MyWidget::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
    case Qt::Key_Left:
	move_cmd = ELEFT;
	//nx-=10;
	break;
    case Qt::Key_Right:
	move_cmd = ERIGHT;
	//nx+=10;
	break;
    case Qt::Key_Space:
    if (gunner) 
	new_bullet();
    if (catcher)
	release_balls();
	break;
    }
    return;
}

void MyWidget::keyReleaseEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
    case Qt::Key_Left:
	move_cmd = ENOP;
	//nx-=10;
	break;
    case Qt::Key_Right:
	move_cmd = ENOP;
	//nx+=10;
	break;
    }
    return;
}

void MyWidget::move_batty()
{
    x += nx;
    if (x<0) {nx-=x;x = 0;}
    if (x>width()-w) {nx-=x+w-width();x = width()-w;}
    return;
}

void MyWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.setBrush(Qt::white);
    draw_batty(p);
    draw_balls(p);
    draw_bricks(p);
    draw_bullets(p);
    draw_prizes(p);
    return;
}

void MyWidget::draw_bricks(QPainter &p)
{
    QColor abc[6]={Qt::white, Qt::blue, Qt::magenta, Qt::yellow, Qt::black, Qt::red};
    p.save();
    p.setBrush(QBrush(Qt::blue, Qt::Dense7Pattern));
    p.drawRect(0,0,width(),height());
    for (int i=0;i<MAX_BRICKS;i++)
	if (bricks[i].prz)  {
	    p.setBrush(abc[bricks[i].type%5]);
	    p.drawImage(QRectF(bricks[i].x, bricks[i].y,bricks[i].w,bricks[i].h), bricks_img, QRectF(0,bricks[i].type*10, 20, 10));
	    //p.drawRect(bricks[i].x,bricks[i].y,bricks[i].w,bricks[i].h);
	}
    p.restore();
    return;
}

void MyWidget::draw_batty(QPainter &p)
{
    p.save();
//    p.drawRect(x,height()-10,w,10);
    for (int i=0;i<(w/(40*k));i++)
	p.drawImage(QRectF(x+i*40*k,height()-10*k,40*k,10*k), batty_img);
    if (gunner) {
	p.drawRect(x+5,height()-15,5,10);
	p.drawRect(x+w-10,height()-15,5,10);
    }
    p.restore();
    return;
}

void MyWidget::draw_balls(QPainter &p)
{
    QColor abc[3]={Qt::white, Qt::red, Qt::blue};
    p.save();
    for (int i=0;i<bn;i++) {
	p.setBrush(abc[balls[i].type]);
        //p.drawEllipse(balls[i].bx-bw/2,balls[i].by-bw/2,bw,bw);
        p.drawImage(QRectF(balls[i].bx-bw/2, balls[i].by-bw/2, bw, bw), balls_img, QRectF(0,balls[i].type*10,10,10));
    }
    p.restore();
    return;
}

void MyWidget::draw_bullets(QPainter &p)
{
    p.save();
    for (int i=0;i<bullets_n;i++)
	p.drawRect(bullets[i].x-2, bullets[i].y-5,5,5);
    p.restore();
}

void MyWidget::draw_prizes(QPainter &p)
{
    TPrize *pp;
    
    p.save();
    p.setBrush(Qt::white);
    for (int i=0;i<prizes_n;i++)
    {
    pp = &prizes[i];
    p.drawImage(QRectF(pp->x-20*k, pp->y-10*k, 40*k, 20*k), tiles, QRectF(0, prizes[i].type*20, 40, 20));
//    p.drawRect(pp->x-20,pp->y-10,40,20);
/*	switch(prizes[i].type) {
	case NEWBALL:
	    p.drawEllipse(pp->x-bw/2,pp->y-bw/2,bw,bw);
	    break;
	case FIREBALL:
	    p.save();
	    p.setBrush(Qt::red);
	    p.drawEllipse(pp->x-bw/2,pp->y-bw/2,bw,bw);
	    p.restore();
	    break;
	case ICEBALL:
	    p.save();
	    p.setBrush(Qt::blue);
	    p.drawEllipse(pp->x-bw/2,pp->y-bw/2,bw,bw);
	    p.restore();
	    break;
	case LARGER:
	    p.drawText(pp->x, pp->y, "L");
	    break;
	case SMALLER:
	    p.drawText(pp->x, pp->y, "S");
	    break;
	case FASTER:
	    p.drawText(pp->x, pp->y, "FF");
	    break;
	case SLOWER:
	    p.drawText(pp->x, pp->y, "SL");
	    break;
	case CATCHER:
	    p.drawText(pp->x, pp->y, "CA");
	    break;
	case GUNNER:
	    p.drawText(pp->x, pp->y, "GU");
	    break;
	}
*/
    }
    p.restore();
    return;
}
