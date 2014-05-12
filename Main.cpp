#include <QApplication>
#include "Window.h"
#include <QDir>
#include <QPixmap>
#include <QSplashScreen>
#include <QPlastiqueStyle>
#include <QCleanlooksStyle>
#include <string>
#include <iostream>

#include "QTUtils.h"

using namespace std;

int main (int argc, char **argv)
{
  QApplication TheBeatGoesOn (argc, argv);
  setBoubekQTStyle (TheBeatGoesOn);
  QApplication::setStyle (new QPlastiqueStyle);
  Window * window = new Window ();
  window->setWindowTitle ("The Beat Goes On : Un raytracer completement hallucinant");
  window->show();
  TheBeatGoesOn.connect (&TheBeatGoesOn, SIGNAL (lastWindowClosed()), &TheBeatGoesOn, SLOT (quit()));
  Scene * scene = Scene::getInstance ();

  scene->computeAO(20,2.);
  return TheBeatGoesOn.exec ();
}

