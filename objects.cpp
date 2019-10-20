/****************************************************************************
** Author: J.-O. Lachaud, University Savoie Mont Blanc
** (adapted from Qt colliding mices example)
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
****************************************************************************/

#include <cmath>
#include <cassert>
#include <QGraphicsScene>
#include <QRandomGenerator>
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QStyleOption>
#include "objects.hpp"

static const double Pi = 3.14159265358979323846264338327950288419717;
//static double TwoPi = 2.0 * Pi;

// Global variables for simplicity.
static QRandomGenerator RG;
LogicalScene* logical_scene = 0;


///////////////////////////////////////////////////////////////////////////////
// class Disk
///////////////////////////////////////////////////////////////////////////////

Disk::Disk( qreal r, const MasterShape* master_shape )
  : _r( r ), _master_shape( master_shape ) {}

QPointF
Disk::randomPoint() const
{
  QPointF p;
  do {
    p = QPointF( ( RG.generateDouble() * 2.0 - 1.0 ),
                 ( RG.generateDouble() * 2.0 - 1.0 ) );
  } while ( ( p.x() * p.x() + p.y() * p.y() ) > 1.0 );
  return p * _r;
}

bool
Disk::isInside( const QPointF& p ) const
{
  return QPointF::dotProduct( p, p ) <= _r * _r;
}

QRectF
Disk::boundingRect() const
{
  return QRectF( -_r, -_r, 2.0 *_r, 2.0 * _r );
}

void
Disk::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  painter->setBrush( _master_shape->currentColor() );
  painter->drawEllipse( QPointF( 0.0, 0.0 ), _r, _r );
}


///////////////////////////////////////////////////////////////////////////////
// class Rectangle
///////////////////////////////////////////////////////////////////////////////

Rectangle::Rectangle( const QPointF & x, const QPointF & y, const MasterShape* master_shape )
  : _rect( QRectF( x, y ) ), _master_shape( master_shape ) {}

QPointF
Rectangle::randomPoint() const
{
    return QPointF(rand()/RAND_MAX*_rect.x(), rand()/RAND_MAX*_rect.y());
}

bool
Rectangle::isInside( const QPointF& p ) const
{
    return 0 <= _rect.x() && p.x() <= _rect.x()
            && 0 <= p.y() && p.y() <= _rect.y();
}

QRectF
Rectangle::boundingRect() const
{
    return _rect;
}

void
Rectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  painter->setBrush( _master_shape->currentColor() );
  painter->drawRect( _rect );
}


///////////////////////////////////////////////////////////////////////////////
// class MasterShape
///////////////////////////////////////////////////////////////////////////////

MasterShape::MasterShape( QColor cok, QColor cko )
  : _f( 0 ), _state( Ok ), _cok( cok ), _cko( cko )
{
}

void
MasterShape::setGraphicalShape( GraphicalShape* f )
{
  _f = f;
  if ( _f != 0 )  _f->setParentItem( this );
}
  
QColor
MasterShape::currentColor() const
{
  if ( _state == Ok ) return _cok;
  else                return _cko;
}

MasterShape::State
MasterShape::currentState() const
{
  return _state;
}

void
MasterShape::paint( QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
  // nothing to do, Qt automatically calls paint of every QGraphicsItem
}

void
MasterShape::advance(int step) 
{
  if ( !step ) return;

  // (I) Garde les objets dans la scene.  
  auto p = scenePos(); // pareil que pos si MasterShape est bien à la racine.
  // pos() est dans les coordonnées parent et setPos aussi.
  if ( p.x() < -SZ_BD ) {
    auto point = parentItem() != 0
      ? parentItem()->mapFromScene( QPointF( IMAGE_SIZE + SZ_BD - 1, p.y() ) )
      : QPointF( IMAGE_SIZE + SZ_BD - 1, p.y() );
    setPos(point);
  } else if ( p.x() > IMAGE_SIZE + SZ_BD ) {
    auto point = parentItem() != 0
      ? parentItem()->mapFromScene( QPointF( -SZ_BD + 1, p.y() ) )
      : QPointF( -SZ_BD + 1, p.y() );
    setPos(point);
  }
  if ( p.y() < -SZ_BD ) {
    auto point = parentItem() != 0 ?
      parentItem()->mapFromScene( QPointF( p.x(), IMAGE_SIZE + SZ_BD - 1 ) )
      : QPointF( p.x(), IMAGE_SIZE + SZ_BD - 1 );
    setPos(point);
  } else if ( p.y() > IMAGE_SIZE + SZ_BD ) {
    auto point = parentItem() != 0
      ? parentItem()->mapFromScene( QPointF( p.x(), -SZ_BD + 1 ) )
      : QPointF( p.x(), -SZ_BD + 1 );
    setPos(point);
  }

  // (II) regarde les intersections avec les autres objets.
  if ( logical_scene->intersect( this ) )
    _state = Collision;
  else
    _state = Ok;
}

QPointF
MasterShape::randomPoint() const
{  
  assert( _f != 0 );
  return mapToParent( _f->randomPoint() );
}

bool
MasterShape::isInside( const QPointF& p ) const
{
  assert( _f != 0 );
  return _f->isInside( mapFromParent( p ) );
}

QRectF
MasterShape::boundingRect() const
{
  assert( _f != 0 );
  return mapRectToParent( _f->boundingRect() );
}

///////////////////////////////////////////////////////////////////////////////
// class Union
///////////////////////////////////////////////////////////////////////////////

Union::Union( GraphicalShape & f1, GraphicalShape & f2 )
    : _f1(f1), _f2(f2)
{
    f1.setParentItem( this );
    f2.setParentItem( this );
}

void Union::paint( QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
  //
}

QPointF Union::randomPoint() const
{
    static bool flipACoin = (rand() / RAND_MAX) > 0.5;

    if(flipACoin) {
        return _f1.randomPoint();
    }
    return _f2.randomPoint();
}

bool Union::isInside(const QPointF &p) const
{
    return _f1.isInside(p) || _f2.isInside(p);
}

QRectF
Union::boundingRect() const
{
  QRectF f1 = mapRectToParent( _f1.boundingRect() );
  QRectF f2 = mapRectToParent( _f2.boundingRect() );
  return f1 | f2;
}

///////////////////////////////////////////////////////////////////////////////
// class Transformation
///////////////////////////////////////////////////////////////////////////////

Transformation::Transformation( GraphicalShape & f, QPointF dx )
    : _f( f ), _dx( dx ), _angle( 0.0 )
{
    f.setParentItem( this );
    this->setPos( _dx );
};

Transformation::Transformation( GraphicalShape & f, QPointF dx, qreal angle )
    : _f( f ), _dx( dx ), _angle( angle )
{
    f.setParentItem( this );
    this->setPos( _dx );
    this->setRotation( _angle );
}

void Transformation::paint( QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
    //
}

QPointF Transformation::randomPoint() const
{
    QPointF orgRandPoint = _f.randomPoint();
    QTransform transf = QTransform().rotate( _angle ).translate( _dx.x(), _dx.y() );

    return transf.map( orgRandPoint );
}

bool Transformation::isInside(const QPointF &p) const
{
    QTransform transf = QTransform().translate( -1.0 * _dx.x(), -1.0 * _dx.y() ).rotate( -1* _angle );

    return _f.isInside( transf.map( p ) );
}

QRectF
Transformation::boundingRect() const
{
    return mapRectToParent( _f.boundingRect() );
}

void Transformation::setAngle( qreal angle )
{
    _angle = angle;
}

///////////////////////////////////////////////////////////////////////////////
// class ImageShape
///////////////////////////////////////////////////////////////////////////////

ImageShape::ImageShape( const QPixmap & pixmap, const MasterShape* master_shape )
    : _pixmap( pixmap ), _master_shape( master_shape )
{
    QBitmap tmpMask = _pixmap.mask();
    _mask = &tmpMask;

    QImage tmpImgMask = QImage( _mask->toImage().convertToFormat( QImage::Format_Mono ) );
    _mask_img = &tmpImgMask;

    QGraphicsPixmapItem tmpQpgi( _pixmap );
};

void ImageShape::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawPixmap( QPointF( 0.0, 0.0 ), _pixmap );
    if ( _master_shape->currentState() == MasterShape::Collision )
    {
      painter->setOpacity( 0.5 );
      painter->setBackgroundMode( Qt::TransparentMode );
      painter->setPen  ( _master_shape->currentColor() );
      painter->drawPixmap( QPointF( 0.0, 0.0 ), *_mask );
    }
}

QPointF ImageShape::randomPoint() const
{
    QPointF p;
    do {
        p = QPointF( ( RG.generateDouble() * _mask_img->width() ),
                   ( RG.generateDouble() * _mask_img->height() ) );
    } while ( !_mask_img->pixelIndex( p.toPoint() ) );
    return p;
}

bool ImageShape::isInside(const QPointF &p) const
{
    return _mask_img->pixelIndex( p.toPoint() );
}

QRectF
ImageShape::boundingRect() const
{
    return _qpgi.boundingRect();
}

///////////////////////////////////////////////////////////////////////////////
// class Asteroid
///////////////////////////////////////////////////////////////////////////////

Asteroid::Asteroid( QColor cok, QColor cko, double speed, double r )
  : MasterShape( cok, cko ), _speed( speed )
{
  // This shape is very simple : just a disk.
  Disk* d = new Disk( r, this );
  // Tells the asteroid that it is composed of just a disk.
  this->setGraphicalShape( d );
}

void
Asteroid::advance(int step)
{
  if (!step) return;
  setPos( mapToParent( _speed, 0.0 ) );
  MasterShape::advance( step );
}



///////////////////////////////////////////////////////////////////////////////
// class NiceAsteroid
///////////////////////////////////////////////////////////////////////////////

NiceAsteroid::NiceAsteroid( QColor cok, QColor cko, double speed, double r )
  : MasterShape( cok, cko ), _speed( speed )
{
    QPixmap asteroid_pixmap(":/images/asteroid.gif");

    Asteroid asteroid( cok, cko, speed, r );
    Asteroid* tmp_asteroid = &asteroid;

    // This shape is very simple : just a disk.
    ImageShape* i = new ImageShape( asteroid_pixmap, tmp_asteroid );

    _t = new Transformation( *i, QPointF(0.0,0.0), 0.0 );

    // Tells the asteroid that it is composed of just a disk.
    this->setGraphicalShape( _t );
}

void
NiceAsteroid::advance(int step)
{
    if (!step) return;
    setPos( mapToParent( _speed, 0.0 ) );
    _t->setAngle( _t->_angle + 2.0 );
    MasterShape::advance( step );
}



///////////////////////////////////////////////////////////////////////////////
// class SpaceTruck
///////////////////////////////////////////////////////////////////////////////

SpaceTruck::SpaceTruck( QColor cok, QColor cko, double speed )
  : MasterShape( cok, cko ), _speed( speed )
{
  // This shape is very simple : just a disk.
  Rectangle* d1 = new Rectangle( QPointF( -80, -10 ), QPointF( 0, 10 ), this );
  Rectangle* d2 = new Rectangle( QPointF( 10, -10 ), QPointF( 30, 10 ), this );
  Rectangle* d3 = new Rectangle( QPointF( 0, -3 ), QPointF( 10, 3 ), this );

  Union* u23 = new Union( *d2, *d3 );
  Union* u = new Union( *d1, *u23 );
  // Tells the SpaceTruck that it is composed of just a disk.
  this->setGraphicalShape( u );
}

void
SpaceTruck::advance(int step)
{
  if (!step) return;
  setPos( mapToParent( _speed, 0.0 ) );
  MasterShape::advance( step );
  MasterShape::setRotation( MasterShape::rotation() + 1 % 360 );
}



///////////////////////////////////////////////////////////////////////////////
// class Enterprise
///////////////////////////////////////////////////////////////////////////////

Enterprise::Enterprise( QColor cok, QColor cko, double speed )
  : MasterShape( cok, cko ), _speed( speed )
{
    Rectangle*      r1 = new Rectangle( QPointF( -100, -8 ), QPointF( 0, 8 ), this );
    Rectangle*      r2 = new Rectangle( QPointF( -100, -8 ), QPointF( 0, 8 ), this );
    Rectangle*      rb = new Rectangle( QPointF( -40, -9 ), QPointF( 40, 9 ), this );
    Rectangle*      s1 = new Rectangle( QPointF( -25, -5 ), QPointF( 25, 5 ), this );
    Rectangle*      s2 = new Rectangle( QPointF( -25, -5 ), QPointF( 25, 5 ), this );
    Disk*            d = new Disk( 40.0, this );
    Transformation* t1 = new Transformation( *r1, QPointF( 0., 40.0 ) );
    Transformation* t2 = new Transformation( *r2, QPointF( 0., -40.0 ) );
    Transformation* td = new Transformation( *d, QPointF( 70., 0.0 ) );
    Transformation*ts1 = new Transformation( *s1, QPointF(-30.0,0.0), 0.0 );
    Transformation*us1 = new Transformation( *ts1, QPointF(0.0,0.0), 45.0 );
    Transformation*ts2 = new Transformation( *s2, QPointF(-30.0,0.0), 0.0 );
    Transformation*us2 = new Transformation( *ts2, QPointF(0.0,0.0), -45.0 );
    Union*        back = new Union( *t1, *t2 );
    Union*        head = new Union( *rb, *td );
    Union*        legs = new Union( *us1, *us2 );
    Union*        body = new Union( *legs, *back );
    Union*         all = new Union( *head, *body );
    this->setGraphicalShape( all );
}

void
Enterprise::advance(int step)
{
  if (!step) return;
  setPos( mapToParent( _speed, 0.0 ) );
  MasterShape::advance( step );
}



///////////////////////////////////////////////////////////////////////////////
// class LogicalScene
///////////////////////////////////////////////////////////////////////////////

LogicalScene::LogicalScene( int n )
  : nb_tested( n ) {}
    
bool
LogicalScene::intersect( MasterShape* f1, MasterShape* f2 )
{
  for ( int i = 0; i < nb_tested; ++i )
    {
      if ( f2->isInside( f1->randomPoint() )
	   || f1->isInside( f2->randomPoint() ) )
	return true;
    }
  return false;
}

bool
LogicalScene::intersect( MasterShape* f1 )
{
  for ( auto f : formes )
    if ( ( f != f1 ) && intersect( f, f1 ) )
      return true;
  return false;
}

