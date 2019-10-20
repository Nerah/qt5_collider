/****************************************************************************
** Author: J.-O. Lachaud, University Savoie Mont Blanc
** (adapted from Qt colliding mices example)
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
****************************************************************************/

#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <iostream>
#include <vector>
#include <QGraphicsItem>

static const int IMAGE_SIZE = 600;
static const int SZ_BD      = 100;


/// @brief Abstract class that describes a graphical object with additional
/// methods for testing collisions.
struct GraphicalShape : public QGraphicsItem
{
  virtual QPointF randomPoint() const = 0;
  virtual bool    isInside( const QPointF& p ) const = 0;
  // Already in QGraphicsItem
  // virtual QRectF  boundingRect() const override;
};


/// @brief Polymorphic class that represents the top class of any complex
/// shape.
///
/// It takes care of memorizing collisions and storing the
/// current main coordinates of a shape.
struct MasterShape : public GraphicalShape
{
  enum State { Ok, Collision };
  MasterShape( QColor cok, QColor cko );
  void setGraphicalShape( GraphicalShape* f );
  virtual void    paint( QPainter *painter, const QStyleOptionGraphicsItem *option,
                         QWidget *widget) override;
  virtual QPointF randomPoint() const override;
  virtual bool    isInside( const QPointF& p ) const override;
  virtual QRectF  boundingRect() const override;

  // Checks if this shape collides with another shape and forces also
  // the shapes to stay in the graphical view.
  virtual void    advance(int step) override;
  State           currentState() const;
  QColor          currentColor() const;

protected:
  GraphicalShape* _f;
  State           _state;
  QColor          _cok, _cko;
};

/// @brief Merge two shapes to create a more complex shape.
struct Union: public GraphicalShape
{
    // The two shapes that are merged.
    GraphicalShape & _f1;
    GraphicalShape & _f2;

    Union( GraphicalShape & f1, GraphicalShape & f2 );

    virtual void paint( QPainter *, const QStyleOptionGraphicsItem *,
                           QWidget *) override;
    virtual QPointF randomPoint() const override;
    virtual bool isInside( const QPointF& p ) const override;
    virtual QRectF  boundingRect() const override;
};

struct Transformation: public GraphicalShape
{
    // The shape that is transformed.
    GraphicalShape & _f;
    // A vector used for the translation of the shape.
    QPointF _dx;
    // A real used for the rotation of the shape.
    qreal _angle;

    Transformation( GraphicalShape & f, QPointF dx );
    Transformation( GraphicalShape & f, QPointF dx, qreal angle );

    virtual void paint( QPainter *, const QStyleOptionGraphicsItem *,
                           QWidget *) override;
    virtual QPointF randomPoint() const override;
    virtual bool isInside( const QPointF& p ) const override;
    virtual QRectF  boundingRect() const override;
    void setAngle( qreal angle );
};

struct ImageShape: public GraphicalShape
{
    const QPixmap _pixmap;
    const MasterShape* _master_shape;
    QBitmap* _mask;
    const QImage* _mask_img;
    const QGraphicsPixmapItem _qpgi; // used for boundingRect method

    ImageShape( const QPixmap & pixmap, const MasterShape* master_shape );

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *,
                           QWidget *) override;
    virtual QPointF randomPoint() const override;
    virtual bool isInside( const QPointF& p ) const override;
    virtual QRectF  boundingRect() const override;
};

/// @brief An asteroid is a simple shape that moves linearly in some direction.
struct Asteroid : public MasterShape
{
  Asteroid( QColor cok, QColor cko, double speed, double r );
  // moves the asteroid forward according to its speed.
  virtual void    advance(int step) override;
protected:
  double          _speed;
};

/// @brief A NiceAsteroid is a simple shape that moves linearly in some direction.
struct NiceAsteroid : public MasterShape
{
  NiceAsteroid( QColor cok, QColor cko, double speed, double r );
  // moves the asteroid forward according to its speed.
  virtual void    advance(int step) override;
protected:
  double          _speed;
  Transformation* _t;
};

/// @brief An asteroid is a simple shape that moves linearly in some direction.
struct SpaceTruck : public MasterShape
{
  SpaceTruck( QColor cok, QColor cko, double speed );
  // moves the asteroid forward according to its speed.
  virtual void    advance(int step) override;
protected:
  double          _speed;
};

/// @brief An enterprise is a simple shape that moves linearly in some direction.
struct Enterprise : public MasterShape
{
  Enterprise( QColor cok, QColor cko, double speed );
  // moves the asteroid forward according to its speed.
  virtual void    advance(int step) override;
protected:
  double          _speed;
};

/// @brief A disk is a simple graphical shape.
///
/// It points to its master shape
/// in order to know in which color it must be painted.
struct Disk : public GraphicalShape
{
  Disk( qreal r, const MasterShape* master_shape );
  virtual void    paint( QPainter *painter, const QStyleOptionGraphicsItem *option,
                         QWidget *widget) override;
  virtual QPointF randomPoint() const override;
  virtual bool    isInside( const QPointF& p ) const override;
  virtual QRectF  boundingRect() const override;
  const qreal     _r;
  const MasterShape* _master_shape;
};

/// @brief A rectangle is a simple graphical shape.
///
/// It points to its master shape
/// in order to know in which color it must be painted.
struct Rectangle : public GraphicalShape
{
  Rectangle( const QPointF & x, const QPointF & y, const MasterShape* master_shape );
  virtual void    paint( QPainter *painter, const QStyleOptionGraphicsItem *option,
                         QWidget *widget) override;
  virtual QPointF randomPoint() const override;
  virtual bool    isInside( const QPointF& p ) const override;
  virtual QRectF  boundingRect() const override;
  const QRectF     _rect;
  const MasterShape* _master_shape;
};

/// @brief A class to store master shapes and to test their possible
/// collisions with a randomized algorithm.
struct LogicalScene {
  std::vector< MasterShape*> formes;
  int nb_tested;

  /// Builds a logical scene where collisions are detected by checking
  /// \a n random points within shapes.
  ///
  /// @param n any positive integer.
  LogicalScene( int n );
  /// Given two shapes \a f1 and \a f2, returns if they collide.
  /// @param f1 any master shape.
  /// @param f2 any different master shape.
  /// @return 'true' iff they collide, i.e. have a common intersection.
  bool intersect( MasterShape* f1, MasterShape* f2 );
  /// @param f1 any master shape.
  /// @return 'true' iff it collides with a different master shape stored in this logical scene.
  bool intersect( MasterShape* f1 );
};

extern LogicalScene* logical_scene;

#endif

