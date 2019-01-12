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
    QDateTime begin;
    QDateTime end;
  } ElementalTide;
  
  typedef struct {
    planets_t planet;
    QDateTime begin;
    QDateTime end;
  } PlanetaryHour;
  
  typedef struct {
    QDateTime dateTime;
    planets_t planet;
    zodiac_t sign;
    int degrees;
    int minutes;
    bool retrograde;
  } PlanetPosition; 
}

// return void of course start and end times for day.
//QMap<QDateTime, voids_t> voids(const QDate &day);
ASTROLIB_EXPORT QList<QPair<QDateTime,QDateTime>> al_voids(const QDate &day);

// calculate elemental tides
QList<AL::ElementalTide> al_elementalTides(const QDateTime &from, const QDateTime &to);
AL::ElementalTide al_elementalTide(const QDateTime &dateTime);

// calculate planetary hours
QList<AL::PlanetaryHour> al_planetaryHours(const QDateTime &from, const QDateTime &to);
AL::PlanetaryHour al_planetaryHour(const QDateTime &dateTime);

// return position of every availiable planet/etc in zodiac.
QMap<AL::planets_t,AL::PlanetPosition> al_zodiacPositions(const QDateTime &dateTime);


#endif
