/****************************************************************************
** Authors: J.-O. Lachaud, University Savoie Mont Blanc
**          A. Ascenci, just a student
** (vaguely adapted from Qt colliding mices example)
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
****************************************************************************/

#include <cmath>
#include <QtWidgets>
#include "objects.hpp"

/****************************************************************************
** Configuration
****************************************************************************/

// Game
static const char* GameTitle = "Space - the final frontier";
static const int GameRefresh = 30; // ms

// Background
static const char* BackgroundSrc = ":/images/stars.jpg";

// Asteroid
static const int AsteroidCount = 10;
static const QColor AsteroidOkColor = QColor( 150, 130, 110 );
static const QColor AsteroidKoColor = QColor( 255, 240, 0 );

// SpaceTruck
static const int SpaceTruckCount = 5;
static const QColor SpaceTruckOkColor = QColor( 102, 204, 0 );
static const QColor SpaceTruckKoColor = QColor( 255, 255, 102 );

// Enterprise
static const int EnterpriseCount = 1;
static const QColor EnterpriseOkColor = QColor( 122, 122, 122 );
static const QColor EnterpriseKoColor = QColor( 200, 0, 0 );

/***************************************************************************/

int main(int argc, char **argv)
{
  // Initializes Qt.
  QApplication app(argc, argv);
  // Initializes the random generator.
  qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

  // Creates a graphics scene where we will put graphical objects.
  QGraphicsScene graphical_scene;
  graphical_scene.setSceneRect(0, 0, IMAGE_SIZE, IMAGE_SIZE);
  graphical_scene.setItemIndexMethod(QGraphicsScene::NoIndex);

  // We choose to check intersection with 100 random points.
  logical_scene = new LogicalScene( 100 );

  // Creates a few asteroids...
  for (int i = 0; i < AsteroidCount; ++i) {

    // A master shape gathers all the elements of the shape.
    MasterShape* asteroid = new NiceAsteroid( AsteroidOkColor,
                                          AsteroidKoColor,
                                          ( qrand() % 20 + 20 ) / 10.0, /* speed */
                                          (10.0 + qrand() % 40) );
    // Set direction and position
    asteroid->setRotation(qrand() % 360);
    asteroid->setPos( IMAGE_SIZE/2 + ::sin((i * 6.28) / AsteroidCount) * 200,
                      IMAGE_SIZE/2 + ::cos((i * 6.28) / AsteroidCount) * 200 );
    // Add it to the graphical scene
    graphical_scene.addItem( asteroid );
    // and to the logical scene
    logical_scene->formes.push_back( asteroid );
  }

  // Creates a few space trucks...
  for (int i = 0; i < SpaceTruckCount; ++i) {

    // A master shape gathers all the elements of the shape.
    MasterShape* spaceTruck = new SpaceTruck( SpaceTruckOkColor,
                                              SpaceTruckKoColor,
                                              ( qrand() % 20 + 20 ) / 10.0 );
    // Set direction and position
    spaceTruck->setRotation( qrand() % 360 );
    spaceTruck->setPos( IMAGE_SIZE/2 + ::sin((i * 6.28) / SpaceTruckCount) * 200,
                      IMAGE_SIZE/2 + ::cos((i * 6.28) / SpaceTruckCount) * 200 );
    // Add it to the graphical scene
    graphical_scene.addItem( spaceTruck );
    // and to the logical scene
    logical_scene->formes.push_back( spaceTruck );
  }

  // Creates a few space enterprises...
  for (int i = 0; i < EnterpriseCount; ++i) {

    // A master shape gathers all the elements of the shape.
    MasterShape* enterprise = new Enterprise( EnterpriseOkColor,
                                              EnterpriseKoColor,
                                              ( qrand() % 20 + 20 ) / 10.0 );
    // Set direction and position
    enterprise->setRotation( qrand() % 360 );
    enterprise->setPos( IMAGE_SIZE/2 + ::sin((i * 6.28) / SpaceTruckCount) * 200,
                      IMAGE_SIZE/2 + ::cos((i * 6.28) / SpaceTruckCount) * 200 );
    // Add it to the graphical scene
    graphical_scene.addItem( enterprise );
    // and to the logical scene
    logical_scene->formes.push_back( enterprise );
  }

  // Standard stuff to initialize a graphics view with some background.
  QGraphicsView view(&graphical_scene);
  view.setRenderHint(QPainter::Antialiasing);
  view.setBackgroundBrush( QPixmap( BackgroundSrc ) );
  view.setCacheMode(QGraphicsView::CacheBackground);
  view.setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
  view.setDragMode(QGraphicsView::NoDrag); // QGraphicsView::ScrollHandDrag
  view.setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, GameTitle));
  view.setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
  view.setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
  view.resize( IMAGE_SIZE, IMAGE_SIZE );
  view.show();

  // Creates a timer that will call `advance()` method regularly.
  QTimer timer;
  QObject::connect(&timer, SIGNAL(timeout()), &graphical_scene, SLOT(advance()));
  timer.start( GameRefresh ); // every 30ms
  
  return app.exec();
}


