#include "headers/astrolib.h"
#include <sweodef.h>
#include <swephexp.h>

#include <QProcess>
#include <QTimeZone>
#include <QSettings>

/**
 * this is currently dependant on QSettings having the following values set in the calling program:
 * "locale/lat" // in the format astrolog expects
 * "locale/long" // in the format astrolog expects
 * "locale/tz" // in the format astrolog expects
 * "paths/astrolog" // path to astrolog binary
 * "paths/ephem" // path to sweph ephemeris files
 */
namespace AL {
  QHash<QString,planets_t> alPlanetsHash;
  QHash<QString,zodiac_t> alZodiacHash;
  
  void init() {
    QSettings settings;
    
    // sigh...........
    char* cstr;
    cstr = new char [settings.value("paths/ephem").toString().toStdString().size()+1];
    strcpy( cstr, settings.value("paths/ephem").toString().toStdString().c_str() );
    swe_set_ephe_path(cstr);
    
   /* alTz = tz;
    alLat = lat;
    alLong = longitude;*/
    // build hashes
    alPlanetsHash["Satu"] = Saturn;
    alPlanetsHash["Jupi"] = Jupiter;
    alPlanetsHash["Mars"] = Mars;
    alPlanetsHash["Sun"] = Sun;
    alPlanetsHash["Venu"] = Venus;
    alPlanetsHash["Merc"] = Mercury;
    alPlanetsHash["Moon"] = Moon;
    alPlanetsHash["Uran"] = Uranus;
    alPlanetsHash["Nept"] = Neptune;
    alPlanetsHash["Plut"] = Pluto;
   
    alZodiacHash["Ari"] = Aries;
    alZodiacHash["Tau"] = Taurus;
    alZodiacHash["Gem"] = Gemini;
    alZodiacHash["Can"] = Cancer;
    alZodiacHash["Leo"] = Leo;
    alZodiacHash["Vir"] = Virgo;
    alZodiacHash["Lib"] = Libra;
    alZodiacHash["Sco"] = Scorpio;
    alZodiacHash["Sag"] = Sagittarius;
    alZodiacHash["Cap"] = Capricorn;
    alZodiacHash["Aqu"] = Aquarius;
    alZodiacHash["Pis"] = Pisces;  
    
  }
  
  QList<QPair<QDateTime,QDateTime>> voids(const QDate &day)
  {
    QStringList daylines = astrolog_getVoids(day);
    QList<QPair<QDateTime,QDateTime>> theVoids;
    
    for (int i=0; i<daylines.size(); ++i) {
      QStringList cols = daylines.at(i).split(QRegExp("\\s+"));
      if (cols.at(3) == "Moon" && cols.at(5) == "-->") { 
        // void end found!
        // now find where it started
        QStringList scols;
        int j=i-1;
        bool found = false;
        while (j>=0) {
          scols = daylines.at(j).split(QRegExp("\\s+"));
          if (scols.at(3) == "Moon") {
            // found it!
            found = true;
            break;
          }
          --j;
        }
        int cnt = 0; // how far back in days to search.
        while (!found) {
          // search through previous days until we find the start.
          cnt++;
          QStringList prevlines = astrolog_getVoids(day.addDays(0-cnt));
          j=prevlines.size()-1;
          while (j>=0) {
            scols = prevlines.at(j).split(QRegExp("\\s+"));
            if (scols.at(3) == "Moon") {
              // found it!
              found = true;
              break;
            }
            --j;
          }
          if (cnt>5) {
            // bail out after 5 days, it'll never be that long.
            err("Something went wrong with astrolog parsing...");
            assert(found); // it absolutely should be found by now.
            break;
          }
        }
        
        // add both to map.
        QDateTime startdt, enddt;
        QString date = scols.at(1);
        date.remove(QRegExp("\\s+"));
        startdt.setDate(QDate::fromString(date, "M/d/yyyy"));
        startdt.setTime(QTime::fromString(scols.at(2), "h:mmap"));
        date = cols.at(1);
        date.remove(QRegExp("\\s+"));
        enddt.setDate(QDate::fromString(date, "M/d/yyyy"));
        enddt.setTime(QTime::fromString(cols.at(2), "h:mmap"));
        theVoids << QPair<QDateTime,QDateTime>(startdt, enddt);
      }
    }
    
    // now find next moon transit in future (starting next day) and if it's a void end, find its start.
    bool found = false;
    int cnt = 0;
    while (!found) {
      cnt++;
      QStringList nextlines = astrolog_getVoids(day.addDays(cnt));
      for (int k=0; k<nextlines.size(); ++k) {
        QStringList cols = nextlines.at(k).split(QRegExp("\\s+"));
        if (cols.at(3) == "Moon") {
          if (cols.at(5) != "-->") { 
            // Okay we're good, no void started yesterday, break loops and finish.
            found = true;
            break;
          }
          else {
            // void end found!
            // now find where it started
            found = true; // break outer while loop after this.
            bool startFound = false;
            cnt = 0;
            QStringList scols;
            while (!startFound) {
              // search through previous days until we find the start.
              QStringList prevlines = ( cnt==0 ) ? daylines : astrolog_getVoids(day.addDays(0-cnt));
              int j=prevlines.size()-1;
              while (j>=0) {
                scols = prevlines.at(j).split(QRegExp("\\s+"));
                if (scols.at(3) == "Moon") {
                  // found it!
                  startFound = true;
                  break;
                }
                --j;
              }
              if (!startFound && cnt>5) {
                // bail out after 5 days, it'll never be that long.
                err("Something went wrong with astrolog parsing...");
                assert(startFound); // it absolutely should be found by now.
                break;
              }
              cnt++;
            }
            
            // add both to map.
            QDateTime startdt, enddt;
            QString date = scols.at(1);
            date.remove(QRegExp("\\s+"));
            startdt.setDate(QDate::fromString(date, "M/d/yyyy"));
            startdt.setTime(QTime::fromString(scols.at(2), "h:mmap"));
            date = cols.at(1);
            date.remove(QRegExp("\\s+"));
            enddt.setDate(QDate::fromString(date, "M/d/yyyy"));
            enddt.setTime(QTime::fromString(cols.at(2), "h:mmap"));
            theVoids << QPair<QDateTime,QDateTime>(startdt, enddt);
            
            break; // break inner for loop now that we've found it
          }
        }
      }
    }
    debug(QString("%1 VOIDS FOUND").arg(theVoids.count()));
    return theVoids;
  }
  
  QMap< planets_t, PlanetPosition > zodiacPositions ( const QDateTime &dateTime )
  {
    QStringList lines = astrolog_getChart(dateTime);
    // chart begins at line three
    lines.removeFirst();
    lines.removeFirst();
    lines.removeFirst();
    QMap< planets_t, PlanetPosition > map;
    for (QString line : lines) {
      QStringList cols = line.split(QRegExp("\\s+"));
      /*
       *    Sun :  8Vir17   - 0:00' (-) [10th house] [-] +0.967  -  House cusp  1: 13Sco25
       *    Moon:  8Tau55   - 5:11' (e) [ 6th house] [-] ______  -  House cusp  2: 12Sag46
       *    Merc: 21Leo13   + 0:40' (d) [ 9th house] [F] +1.455  -  House cusp  3: 16Cap38
       *    Venu: 23Lib18   - 3:11' (R) [12th house] [e] +0.832  -  House cusp  4: 22Aqu26
       *    Mars: 28Cap44   - 5:41' (e) [ 3rd house] [-] +0.057  -  House cusp  5: 24Pis56
       *    Jupi: 17Sco12   + 0:51' (-) [ 1st house] [-] +0.136  -  House cusp  6: 21Ari39
       *    Satu:  2Cap31 R + 0:43' (R) [ 2nd house] [-] -0.009  -  House cusp  7: 13Tau25
       *    Uran:  2Tau22 R - 0:33' (d) [ 6th house] [-] -0.019  -  House cusp  8: 12Gem46
       *    Nept: 15Pis16 R - 1:00' (R) [ 4th house] [-] -0.027  -  House cusp  9: 16Can38
       *    Plut: 18Cap55 R + 0:05' (-) [ 3rd house] [-] -0.014  -  House cusp 10: 22Leo26
       *    Chir:  1Ari54 R + 3:33' (-) [ 5th house] [-] -0.041  -  House cusp 11: 24Vir56
       *    Cere: 28Vir22   + 7:34' (-) [11th house] [-] +0.449  -  House cusp 12: 21Lib39
       *    Pall:  0Vir11   -12:21' (R) [10th house] [e] +0.527
       *    Juno: 29Tau59   -10:14' (-) [ 7th house] [R] +0.342     Car Fix Mut TOT   +: 7
       *    Vest:  2Cap28   - 2:08' (-) [ 2nd house] [F] +0.156  Fir  1   3   0   4   -:15
       *    Node:  4Leo03 R + 0:00' (F) [ 9th house] [-] ______  Ear  4   3   3  10   M: 9
       *    S.No:  4Aqu03 R + 0:00' (F) [ 3rd house] [-] ______  Air  1   1   1   3   N:11
       *    Fort: 14Can03   _______ (-) [ 8th house] [-] ______  Wat  1   3   1   5   A:11
       *    Vert: 26Gem06   _______ (-) [ 8th house] [-] ______  TOT  7  10   5  22   D: 9
       *    East: 27Sco04   _______ (-) [ 1st house] [R] ______                       <:12*/
      
      PlanetPosition pp;
      pp.dateTime = dateTime;
      
      // planet/object
      int c=0; // column
      QString planet = cols.at(c);
      planet.remove(":");
      if (!alPlanetsHash.contains(planet))
        continue; // not all reported planets/objects are yet handled.
        
        pp.planet = alPlanetsHash.value(planet);
      
      // sign + degrees,minutes
      c++;
      if (cols.at(c) == ":") c++; // sun does this...
      QRegExp rx;
      rx.setPattern("(\\d+)(\\D{3})(\\d+)");
      assert(cols.at(c).contains(rx));
      pp.degrees = rx.cap(1).toInt();
      pp.sign = alZodiacHash.value(rx.cap(2));
      pp.minutes = rx.cap(3).toInt();
      
      // retrograde
      c++;
      rx.setPattern("(R|\\+|\\-|_)");
      assert(cols.at(c).contains(rx));
      if (rx.cap(1) == "R")
        pp.retrograde = true;
      else
        pp.retrograde = false;
      
      map.insert(pp.planet, pp);
      debug(QString("pl %1 dg %2 sign %3 min %4 ret %5")
      .arg(static_cast<int>(pp.planet))
      .arg(pp.degrees)
      .arg(static_cast<int>(pp.sign))
      .arg(pp.minutes)
      .arg(pp.retrograde));
    } 
    
    return map;
  }
  
  QList<ElementalTide> elementalTides(const QDateTime& from, const QDateTime& to)
  {
    QList<ElementalTide> list;
    // get rise set time including 1 previous next couple days because we need sunrise from next day for night tides.
    QList<QPair<QDateTime, QDateTime>> pairs = riseSet(from.date().addDays(-1), to.date().addDays(2));
    
    /// get first tide, then count forward to end tide spanning days if necessary, adding each to list.  
    QDateTime prevRise, prevSet, rise, set, nextRise, nextSet, start, end, current;
    int daycnt = 0;
    int tideNum = 0;
    double tideLength;
    bool daytime = false;
    elements_t element = Element_Null; // current element
    ElementalTide tide;
    
    bool finished = false;
    bool haveFirst = false;
    current = from;
    while (!finished) {
      if (!haveFirst) {
        // hunt down the first one...
        prevRise = pairs.at(daycnt).first;
        prevSet = pairs.at(daycnt).second;
        rise = pairs.at(daycnt+1).first;
        set = pairs.at(daycnt+1).second;
        nextRise = pairs.at(daycnt+2).first;
        
        // adjust for DST
        if (prevRise.isDaylightTime()) prevRise = prevRise.addSecs(60*60*1);
        if (prevSet.isDaylightTime()) prevSet = prevSet.addSecs(60*60*1);
        if (rise.isDaylightTime()) rise = rise.addSecs(60*60*1);
        if (set.isDaylightTime()) set = set.addSecs(60*60*1);
        if (nextRise.isDaylightTime()) nextRise = nextRise.addSecs(60*60*1);
        
        // now find it
        if (current > prevRise && current < prevSet) {
          daytime = true;
          start = prevRise;
          end = prevSet;
        }
        else if (current > prevSet && current < rise) {
          daytime = false;
          start = prevSet;
          end = rise;
        }
        else if (current > rise && current < set) {
          daytime = true;
          start = rise;
          end = set;
        }
        else if (current > set && current < nextRise) {
          daytime = false;
          start = set;
          end = nextRise;
        }
        else {
          err("This probably isn't possible...");
          assert(false);
        }
        
        haveFirst = true;
      }
      
      tideLength = ((double)start.secsTo(end) / 60) / 30;
      //  tideNum = (start.secsTo(current) / 60) / tideLength;
      debug(QString("tidenum: %1").arg(tideNum));
      switch (tideNum % 5) {
        case 0:
          element = Spirit;
          debug("spirit");
          break;
        case 1:
          element = Air;
          debug("air");
          break;
        case 2:
          element = Fire;
          debug("fire");
          break;
        case 3:
          element = Water;
          debug("water");
          break;
        case 4:
          element = Earth;
          debug("earth");
          break;
        default:
          element = Element_Null;
      };
      
      tide.begin = start.time();
      tide.begin = tide.begin.addSecs(tideNum*tideLength*60);
      tide.end = tide.begin.addSecs(tideLength*60);  
      tide.element = element;
      
      if (tide.end > from.time()) {
        debug(tide.begin.toString());
        debug(tide.end.toString());
        list << tide; 
      }
      
      if (tide.end >= to.time()) {
        finished=true;
        break; // just break out of loop here and don't bother with pointless calculations.
      }
      
      // still haven't got to "to" yet, so go on to the next tide
      ++tideNum;
      if (tideNum >= 30 ) {
        tideNum = 0;
        daytime = !daytime;
        if (daytime) { 
          // rolled over into the next day!
          daycnt++;
          
          start = pairs.at(daycnt).first;
          end = pairs.at(daycnt).second;
        }
        else {
          start = pairs.at(daycnt).second;
          end = pairs.at(daycnt+1).first;
        }
        // adjust for DST
        if (start.isDaylightTime()) start = start.addSecs(60*60*1);
        if (end.isDaylightTime()) end = end.addSecs(60*60*1);
        // recalculate tide length
        tideLength = ((double)start.secsTo(end) / 60) / 30;
      }
    }
    
    return list;
  }
  
  ElementalTide elementalTide(const QDateTime& dateTime)
  {
    return elementalTides(dateTime, dateTime).first();
  }
  
  PlanetaryHour planetaryHour(const QDateTime& dateTime)
  {
    return planetaryHours(dateTime, dateTime).first();
  }
  
  QList<PlanetaryHour> planetaryHours(const QDateTime& from, const QDateTime& to)
  {
    QList<PlanetaryHour> list;
    /******** FIXME lost db table... need to redo this
    // get rise set time including 1 previous next couple days because we need sunrise from next day for night hours.
    QList<QPair<QDateTime, QDateTime>> pairs = riseSet(from.date().addDays(-1), to.date().addDays(2));
    
    /// get first hour, then count forward to end hour spanning days if necessary, adding each to list.  
    QDateTime prevRise, prevSet, rise, set, nextRise, nextSet, start, end, current;
    int daycnt = 0;
    int dow;
    double hourLength;
    bool daytime = false;
    int hour; // planetary hour 1-12 (day or night)
    PlanetaryHour phour;
    
    bool finished = false;
    bool haveFirst = false;
    current = from;
    while (!finished) {
      if (!haveFirst) {
        // hunt down the first one...
        prevRise = pairs.at(daycnt).first;
        prevSet = pairs.at(daycnt).second;
        rise = pairs.at(daycnt+1).first;
        set = pairs.at(daycnt+1).second;
        nextRise = pairs.at(daycnt+2).first;
        
        // adjust for DST
        if (prevRise.isDaylightTime()) prevRise = prevRise.addSecs(60*60*1);
        if (prevSet.isDaylightTime()) prevSet = prevSet.addSecs(60*60*1);
        
        if (nextRise.isDaylightTime()) nextRise = nextRise.addSecs(60*60*1);
        
        // now find it
        if (current > prevRise && current < prevSet) {
          daytime = true;
          start = prevRise;
          end = prevSet;
          dow = prevRise.date().dayOfWeek();
        }
        else if (current > prevSet && current < rise) {
          daytime = false;
          start = prevSet;
          end = rise;
          dow = prevRise.date().dayOfWeek();
        }
        else if (current > rise && current < set) {
          daytime = true;
          start = rise;
          end = set;
          dow = rise.date().dayOfWeek();
        }
        else if (current > set && current < nextRise) {
          daytime = false;
          start = set;
          end = nextRise;
          dow = rise.date().dayOfWeek();
        }
        else {
          err("This probably isn't possible...");
          assert(false);
        }
        
        hourLength = ((double)start.secsTo(end) / 60) / 12;
        hour = (start.secsTo(current) / 60) / hourLength;
        haveFirst = true;
      }
      
      phour.begin = start.time();
      phour.begin = phour.begin.addSecs(hour*hourLength*60);
      phour.end = phour.begin.addSecs(hourLength*60);        
      phour.planet = DB.planetForHour(dow, daytime, hour+1);
      list << phour; 
      
      if (phour.end >= to.time()) {
        finished=true;
        break; // just break out of loop here and don't bother with pointless calculations.
      }
      
      // still haven't got to "to" yet, so go on to the next hour
      ++hour;
      if (hour > 11 ) {
        hour = 0;
        daytime = !daytime;
        if (daytime) { 
          // rolled over into the next day!
          daycnt++;
          
          start = pairs.at(daycnt).first;
          end = pairs.at(daycnt).second;
        }
        else {
          start = pairs.at(daycnt).second;
          end = pairs.at(daycnt+1).first;
        }
        // adjust for DST
        if (start.isDaylightTime()) start = start.addSecs(60*60*1);
        if (end.isDaylightTime()) end = end.addSecs(60*60*1);
        // always use sunrise of current day for dayOfWeek
        dow = pairs.at(daycnt).first.date().dayOfWeek(); 
        // recalculate hour length
        hourLength = ((double)start.secsTo(end) / 60) / 12;
      }
    }
    */
    return list;
  }
  
  QPair<QDateTime, QDateTime> riseSet(const QDate& date)
  {
    return riseSet(date, date).first();
  }
  
  QList<QPair<QDateTime, QDateTime>> riseSet(const QDate& from, const QDate& to) 
  {
    QList<QPair<QDateTime, QDateTime>> dlist;
    QDate cdate = from;
    while (cdate <= to) {
      char serr[AS_MAXCH];
      double epheflag = SEFLG_SWIEPH;
      int gregflag = SE_GREG_CAL;
      
      int year = cdate.year();
      int month = cdate.month();
      int day = cdate.day();
      
      double geo_longitude = -105.097938; // positive for east, negative for west of Greenwich
      //double geo_longitude = alLong.toDouble(); // positive for east, negative for west of Greenwich
      double geo_latitude = 40.044488;
      //double geo_latitude = alLat.toDouble();
      double geo_altitude = 1564.0; ///FIXME is this meters??
      
      double rhour;
      double shour;
      
      // array for atmospheric conditions
      double datm[2];
      datm[0] = 1013.25; // atmospheric pressure;
      // irrelevant with Hindu method, can be set to 0
      datm[1] = 15;      // atmospheric temperature;
      // irrelevant with Hindu method, can be set to 0
      
      double geopos[3];
      geopos[0] = geo_longitude;
      geopos[1] = geo_latitude;
      geopos[2] = geo_altitude;
      
      swe_set_topo(geopos[0], geopos[1], geopos[2]);
      
      int ipl = SE_SUN; // object whose rising is wanted
      char starname[255]; // name of star, if a star's rising is wanted
      // is "" or NULL, if Sun, Moon, or planet is calculated
      double trise; // for rising time
      double tset;  // for setting time
      
      // calculate the julian day number of the date at 0:00 UT:
      double tjd = swe_julday(year,month,day,0,gregflag);
      
      // convert geographic longitude to time (day fraction) and subtract it from tjd
      // this method should be good for all geographic latitudes except near in
      // polar regions
      double dt = geo_longitude / 360.0;
      tjd = tjd - dt;
      
      int rsmi = SE_CALC_RISE;
      int return_code = swe_rise_trans(tjd, ipl, starname, epheflag, rsmi, geopos, datm[0], datm[1], &trise, serr);
      
      if (return_code == ERR) {
        err(serr);
        return QList<QPair<QDateTime, QDateTime>>();
      }
      
      swe_revjul(trise, gregflag, &year, &month, &day, &rhour);
    
      rsmi = SE_CALC_SET;
      return_code = swe_rise_trans(trise, ipl, starname, epheflag, rsmi, geopos, datm[0], datm[1], &tset, serr);
      
      if (return_code == ERR) {
        err(serr);
        return QList<QPair<QDateTime, QDateTime>>();
      }
      
      swe_revjul(tset, gregflag, &year, &month, &day, &shour);
      // printf("Day length: %f\tJulian days\nDay length: %f\tHours\n\n", (tset - trise), (tset - trise) * 24 );
      
      // Turn Julian day into first UTC then localtime
      int iyear_utc;
      int imonth_utc;
      int iday_utc;
      int ihour_utc;
      int imin_utc;
      double dsec_utc;
      QPair<QDateTime,QDateTime> pair;
      
      // calculate sunrise
      swe_jdet_to_utc(trise, gregflag, &iyear_utc, &imonth_utc, &iday_utc, &ihour_utc, &imin_utc, &dsec_utc);
      //printf("UTC   sunrise : date=%i/%i/%i, time=%02i:%02i:%05.2f\n", iyear_utc, imonth_utc, iday_utc, ihour_utc, imin_utc, dsec_utc);
      QDateTime dtrise(QDate(iyear_utc,imonth_utc,iday_utc),QTime(ihour_utc,imin_utc,dsec_utc), Qt::UTC);
      
      // calculate sunset
      swe_jdet_to_utc(tset, gregflag, &iyear_utc, &imonth_utc, &iday_utc, &ihour_utc, &imin_utc, &dsec_utc);
      // printf("UTC   sunset  : date=%i/%i/%i, time=%02i:%02i:%05.2f\n", iyear_utc, imonth_utc, iday_utc, ihour_utc, imin_utc, dsec_utc);
      QDateTime dtset(QDate(iyear_utc,imonth_utc,iday_utc),QTime(ihour_utc,imin_utc,dsec_utc), Qt::UTC);
      
      // convert to localtime
      pair.first = dtrise.toLocalTime();
      pair.second = dtset.toLocalTime();
      // adjust for DST if applicable
      if (pair.first.isDaylightTime()) pair.first = pair.first.addSecs(60*60*1);
      if (pair.second.isDaylightTime()) pair.second = pair.second.addSecs(60*60*1);
      dlist << pair;
      
      cdate = cdate.addDays(1);
    }
    
    swe_close();
    return dlist;
  }
  
  QStringList astrolog_getVoids(const QDate& day)
  {
    QSettings settings;
    QProcess *al = new QProcess();
    QString cmd = settings.value("paths/astrolog").toString();
    QStringList args;
    // astrolog -qb 8 16 2018 12:00 1 MT 105W05:51 40N02:39 -dm
    args << "-qb" << QString::number(day.month()) << QString::number(day.day()) << QString::number(day.year()) << "12:00";
    QDateTime dt;
    dt.setDate(day);
    dt.setTime(QTime(12,0,0));
    args << QString( (dt.isDaylightTime()) ? "1" : "0");
    args << settings.value("locale/tz").toString();
    args << settings.value("locale/long").toString();
    args << settings.value("locale/lat").toString();
    args << "-d"; // get transits for the day including void of course.
    al->start(cmd, args);
    if (!al->waitForFinished()) {
      err("Something went wrong calling astrolog!!");
      assert(false);
    }
    
    QStringList lines;
    while (al->canReadLine()) {
      QString line = al->readLine();
      // fix whitespace padded dates! eg: "11/ 3/1999" 
      QRegExp rx;
      rx.setPattern("\\s+(\\d+)/\\s*(\\d+)/(\\d+)\\s+");
      if (line.contains(rx)) // line.contains causes rx captures to happen, while line.replace alone does not!
        line.replace(rx, QString(" %1/%2/%3 ").arg(rx.cap(1), rx.cap(2), rx.cap(3)));
      lines << line;
    }
    
    return lines;
  }
  
  QStringList astrolog_getChart(const QDateTime& dt)
  {
    QSettings settings;
    QProcess *al = new QProcess();
    QString cmd = settings.value("paths/astrolog").toString();
    QStringList args;
    // astrolog -qb 8 16 2018 12:00 1 MT 105W05:51 40N02:39 -dm
    args << "-qb" 
    << QString::number(dt.date().month()) 
    << QString::number(dt.date().day()) 
    << QString::number(dt.date().year()) 
    << dt.time().toString("HH:mm:ss")
    << QString( (dt.isDaylightTime()) ? "1" : "0")
    << settings.value("locale/tz").toString()
    << settings.value("locale/long").toString()
    << settings.value("locale/lat").toString()
    << "-v"; // get normal chart output
    al->start(cmd, args);
    if (!al->waitForFinished()) {
      err("Something went wrong calling astrolog!!");
      assert(false);
    }
    
    QStringList lines;
    while (al->canReadLine()) {
      lines << al->readLine();
    }
    
    return lines;
  }
  
} // namespace AL





/*
 / /*  Calculate Nocturnal Tides
 int i;
 int j;
 long double nocday = ( tset - trise ) / 30 ;
 printf("nocday: %Lf\n\n", nocday);
 for ( i=0; i<30; i++) {
   
   j = i % 5;
   
   switch(j){
     case 0 :
       printf("Spirit:\t");
       break;
     case 1 :
       printf("Air:\t");
       break;
     case 2 :
       printf("Fire:\t");
       break;
     case 3 :
       printf("Water:\t");
       break;
     case 4 :
       printf("Earth:\t");
       break;
       }
       
       printf("%Lf\t", trise + i * nocday);
       
       swe_jdet_to_utc(trise + i * nocday, gregflag, &iyear_utc, &imonth_utc, &iday_utc, &ihour_utc, &imin_utc, &dsec_utc);
       swe_utc_time_zone(iyear_utc, imonth_utc, iday_utc, ihour_utc, imin_utc, dsec_utc, -d_timezone, &iyear, &imonth, &iday, &ihour, &imin, &dsec);
       printf(": date=%i/%i/%i, time=%02i:%02i:%05.2f\n", iyear, imonth, iday, ihour, imin, dsec);
       
       }
       printf("\nCheck: %Lf\n\n", trise + 29 * nocday );
       
       // Calculate moon stuff. Needs more work
       double attr[20];
       return_code = swe_pheno_ut( tjd, SE_MOON, 0, attr, serr);
       
       if (return_code == ERR) {
         printf("%s\n", serr);
         }
         
         printf("Moon phase: %.2f%%  ", attr[1] * 100);
         
         double today = attr[1];
         return_code = swe_pheno_ut( tjd - 1, SE_MOON, 0, attr, serr);
         
         if (return_code == ERR) {
           // error action
           printf("%s\n", serr);
           }
           
           if ( today > attr[1] ) {
             printf("Waxing\n");
             }
             else {
               printf("Waning\n");
               }
               
               printf("Moon phase angle: %f\n", attr[0]);
               
               printf("\n");
               */
