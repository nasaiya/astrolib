#ifndef astrolib_H
#define astrolib_H

#include <QTime>
#include <QDateTime>
#include <QDate>
#include "al_global.h"

namespace AL {
  enum voids_t {
    VoidBegin,
    VoidEnd
  };
  
  enum elements_t {
    Element_Null = 0,
    Earth = 1,
    Air = 2,
    Water = 3,
    Fire = 4,
    Spirit = 5
  };
  
  enum planets_t {
    Planet_Null = 0,
    Saturn = 1,
    Jupiter = 2,
    Mars = 3,
    Sun = 4,
    Venus = 5,
    Mercury = 6,
    Moon = 7,
    Uranus = 8,
    Neptune = 9,
    Pluto = 10
  };
  
  enum zodiac_t {
    Zodiac_Null = 0,
    Aries = 20,
    Taurus = 21,
    Gemini = 22,
    Cancer = 23,
    Leo = 24,
    Virgo = 25,
    Libra = 26,
    Scorpio = 27,
    Sagittarius = 28,
    Capricorn = 29,
    Aquarius = 30,
    Pisces = 31
  };
  
  typedef struct {
    elements_t element;
    QTime begin;
    QTime end;
    QDate date;
  } ElementalTide;
  
  typedef struct {
    planets_t planet;
    QTime begin;
    QTime end;
    QDate date;
  } PlanetaryHour;
  
  typedef struct {
    QDateTime dateTime;
    planets_t planet;
    zodiac_t sign;
    int degrees;
    int minutes;
    bool retrograde;
  } PlanetPosition; 
  
  ASTROLIB_EXPORT void init(); //const QString &tz, const QString &lat, const QString &longitude
  
  // return void of course start and end times for day.
  ASTROLIB_EXPORT QList<QPair<QDateTime,QDateTime>> voids(const QDate &day);

  // calculate elemental tides
  ASTROLIB_EXPORT QList<AL::ElementalTide> elementalTides(const QDateTime &from, const QDateTime &to);
  ASTROLIB_EXPORT AL::ElementalTide elementalTide(const QDateTime &dateTime);

  // calculate planetary hours
  ASTROLIB_EXPORT QList<AL::PlanetaryHour> planetaryHours(const QDateTime &from, const QDateTime &to);
  ASTROLIB_EXPORT AL::PlanetaryHour planetaryHour(const QDateTime &dateTime);

  // return position of every availiable planet/etc in zodiac.
  ASTROLIB_EXPORT QMap<AL::planets_t,AL::PlanetPosition> zodiacPositions(const QDateTime &dateTime);

  // return sunrise/set for given date range
  ASTROLIB_EXPORT QPair<QDateTime, QDateTime> riseSet(const QDate& date);
  ASTROLIB_EXPORT QList<QPair<QDateTime, QDateTime>> riseSet(const QDate& from, const QDate& to);

// internal functions/vars (NOT exported)
  // return output as list of lines, no processing done.
  QStringList astrolog_getVoids(const QDate &day);
  QStringList astrolog_getChart(const QDateTime &dt);
  
}

#endif
